/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2013-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-application.h"

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#ifdef HAVE_PACKAGEKIT
#include "gs-dbus-helper.h"
#endif

#include "gs-build-ident.h"
#include "gs-common.h"
#include "gs-debug.h"
#include "gs-shell.h"
#include "gs-update-monitor.h"
#include "gs-shell-search-provider.h"

#define ENABLE_REPOS_DIALOG_CONF_KEY "enable-repos-dialog"

struct _GsApplication {
	AdwApplication	 parent;
	GCancellable	*cancellable;
	GsPluginLoader	*plugin_loader;
	gint		 pending_apps;
	GtkWindow	*main_window;
	GsShell		*shell;
	GsUpdateMonitor *update_monitor;
#ifdef HAVE_PACKAGEKIT
	GsDbusHelper	*dbus_helper;
#endif
	GsShellSearchProvider *search_provider;  /* (nullable) (owned) */
	GSettings       *settings;
	GSimpleActionGroup	*action_map;
	guint		 shell_loaded_handler_id;
	GsDebug		*debug;  /* (owned) (not nullable) */

	/* Created/freed on demand */
	GHashTable *withdraw_notifications; /* gchar *notification_id ~> GUINT_TO_POINTER (timeout_id) */
};

G_DEFINE_TYPE (GsApplication, gs_application, ADW_TYPE_APPLICATION);

typedef enum {
	PROP_DEBUG = 1,
} GsApplicationProperty;

static GParamSpec *obj_props[PROP_DEBUG + 1] = { NULL, };

enum {
	INSTALL_RESOURCES_DONE,
	REPOSITORY_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static const char *
get_version (void)
{
	if (g_strcmp0 (BUILD_TYPE, "release") == 0)
		return VERSION;
	else
		return GS_BUILD_IDENTIFIER;
}

typedef struct {
	GsApplication *app;
	GSimpleAction *action;
	GVariant *action_param;  /* (nullable) */
} GsActivationHelper;

static GsActivationHelper *
gs_activation_helper_new (GsApplication *app,
			  GSimpleAction *action,
			  GVariant *parameter)
{
	GsActivationHelper *helper = g_slice_new0 (GsActivationHelper);
	helper->app = app;
	helper->action = G_SIMPLE_ACTION (action);
	helper->action_param = (parameter != NULL) ? g_variant_ref_sink (parameter) : NULL;

	return helper;
}

static void
gs_activation_helper_free (GsActivationHelper *helper)
{
	g_clear_pointer (&helper->action_param, g_variant_unref);
	g_slice_free (GsActivationHelper, helper);
}

gboolean
gs_application_has_active_window (GsApplication *application)
{
	GList *windows;

	windows = gtk_application_get_windows (GTK_APPLICATION (application));
	for (GList *l = windows; l != NULL; l = l->next) {
		if (gtk_window_is_active (GTK_WINDOW (l->data)))
			return TRUE;
	}
	return FALSE;
}

static void
gs_application_init (GsApplication *application)
{
	const GOptionEntry options[] = {
		{ "mode", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  /* TRANSLATORS: this is a command line option */
		  _("Start up mode: either ‘updates’, ‘updated’, ‘installed’ or ‘overview’"), _("MODE") },
		{ "search", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("Search for applications"), _("SEARCH") },
		{ "details", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("Show application details (using application ID)"), _("ID") },
		{ "details-pkg", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("Show application details (using package name)"), _("PKGNAME") },
		{ "install", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("Install the application (using application ID)"), _("ID") },
		{ "uninstall", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("Uninstall the application (using application ID)"), _("ID") },
		{ "local-filename", '\0', 0, G_OPTION_ARG_FILENAME, NULL,
		  _("Open a local package file"), _("FILENAME") },
		{ "interaction", '\0', 0, G_OPTION_ARG_STRING, NULL,
		  _("The kind of interaction expected for this action: either "
		    "‘none’, ‘notify’, or ‘full’"), _("INTERACTION") },
		{ "show-metainfo", '\0', 0, G_OPTION_ARG_FILENAME, NULL,
		  _("Show a local metainfo or appdata file"), _("FILENAME") },
		{ "verbose", '\0', 0, G_OPTION_ARG_NONE, NULL,
		  _("Enable verbose debugging output (from the running instance, if already running)"), NULL },
		{ "autoupdate", 0, 0, G_OPTION_ARG_NONE, NULL,
		  _("Installs any pending updates in the background"), NULL },
		{ "prefs", 0, 0, G_OPTION_ARG_NONE, NULL,
		  _("Show preferences"), NULL },
		{ "quit", 0, 0, G_OPTION_ARG_NONE, NULL,
		  _("Quit the running instance"), NULL },
		{ "prefer-local", '\0', 0, G_OPTION_ARG_NONE, NULL,
		  _("Prefer local file sources to AppStream"), NULL },
		{ "version", 0, 0, G_OPTION_ARG_NONE, NULL,
		  _("Show version number"), NULL },
		{ NULL }
	};

	g_application_add_main_option_entries (G_APPLICATION (application), options);
}

static gboolean
gs_application_dbus_register (GApplication    *application,
                              GDBusConnection *connection,
                              const gchar     *object_path,
                              GError         **error)
{
	GsApplication *app = GS_APPLICATION (application);
	app->search_provider = gs_shell_search_provider_new ();
	return gs_shell_search_provider_register (app->search_provider, connection, error);
}

static void
gs_application_dbus_unregister (GApplication    *application,
                                GDBusConnection *connection,
                                const gchar     *object_path)
{
	GsApplication *app = GS_APPLICATION (application);

	if (app->search_provider != NULL)
		gs_shell_search_provider_unregister (app->search_provider);
}

static void
gs_application_shutdown (GApplication *application)
{
	GsApplication *app = GS_APPLICATION (application);

	g_cancellable_cancel (app->cancellable);
	g_clear_object (&app->cancellable);

	g_clear_object (&app->shell);

	G_APPLICATION_CLASS (gs_application_parent_class)->shutdown (application);
}

static void
gs_application_shell_loaded_cb (GsShell *shell, GsApplication *app)
{
	g_signal_handler_disconnect (app->shell, app->shell_loaded_handler_id);
	app->shell_loaded_handler_id = 0;
}

static void
gs_application_present_window (GsApplication *app, const gchar *startup_id)
{
	GList *windows;
	GtkWindow *window;

	windows = gtk_application_get_windows (GTK_APPLICATION (app));
	if (windows) {
		window = windows->data;

		if (startup_id != NULL)
			gtk_window_set_startup_id (window, startup_id);
		gtk_window_present (window);
	}
}

static void
sources_activated (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       app)
{
	gs_shell_show_sources (GS_APPLICATION (app)->shell);
}

static void
prefs_activated (GSimpleAction *action, GVariant *parameter, gpointer app)
{
	gs_shell_show_prefs (GS_APPLICATION (app)->shell);
}

static void
about_activated (GSimpleAction *action,
		 GVariant      *parameter,
		 gpointer       user_data)
{
	GsApplication *app = GS_APPLICATION (user_data);
	const gchar *developers[] = {
		"Richard Hughes",
		"Matthias Clasen",
		"Kalev Lember",
		"Allan Day",
		"Ryan Lerch",
		"William Jon McCann",
		"Milan Crha",
		"Joaquim Rocha",
		"Robert Ancell",
		"Philip Withnall",
		NULL
	};

	const gchar *designers[] = {
		"Allan Day",
		"Jakub Steiner",
		"William Jon McCann",
		"Tobias Bernard",
		NULL
	};

#if ADW_CHECK_VERSION(1,2,0)
	adw_show_about_window (app->main_window,
			       "application-name", g_get_application_name (),
			       "application-icon", APPLICATION_ID,
			       "developer-name", _("The Lingmo OS Project"),
			       "version", get_version(),
			       "website", "https://apps.lingmo.org/Appstore",
			       "support-url", "https://discourse.lingmo.org/tag/lingmo-appstore",
			       "issue-url", "https://github.com/LingmoOS/lingmo-appstore/-/issues",
			       "developers", developers,
			       "designers", designers,
			       "copyright", _("Copyright \xc2\xa9 2016–2023 LINGMO Appstore contributors"),
			       "license-type", GTK_LICENSE_GPL_2_0,
			       "translator-credits", _("translator-credits"),
			       NULL);
#else
	GtkAboutDialog *dialog;
	dialog = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());
	gtk_about_dialog_set_authors (dialog, developers);
	gtk_about_dialog_set_copyright (dialog, _("Copyright \xc2\xa9 2016–2023 LINGMO Appstore contributors"));
	gtk_about_dialog_set_license_type (dialog, GTK_LICENSE_GPL_2_0);
	gtk_about_dialog_set_logo_icon_name (dialog, APPLICATION_ID);
	gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
	gtk_about_dialog_set_version (dialog, get_version ());
	gtk_about_dialog_set_program_name (dialog, g_get_application_name ());

	/* TRANSLATORS: this is the title of the about window */
	gtk_window_set_title (GTK_WINDOW (dialog), _("About Appstore"));

	/* TRANSLATORS: well, we seem to think so, anyway */
	gtk_about_dialog_set_comments (dialog, _("A nice way to manage the "
						 "software on your system."));

	gs_shell_modal_dialog_present (app->shell, GTK_WINDOW (dialog));
#endif
}

static void
cancel_trigger_failed_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GsApplication *app = GS_APPLICATION (user_data);
	g_autoptr(GError) error = NULL;
	if (!gs_plugin_loader_job_action_finish (app->plugin_loader, res, &error)) {
		g_warning ("failed to cancel trigger: %s", error->message);
		return;
	}
}

static void
reboot_failed_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GsApplication *app = GS_APPLICATION (user_data);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get result */
	if (gs_utils_invoke_reboot_finish (source, res, &error))
		return;

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		g_debug ("Calling reboot had been cancelled");
	else if (error != NULL)
		g_warning ("Calling reboot failed: %s", error->message);

	/* cancel trigger */
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPDATE_CANCEL, NULL);
	gs_plugin_loader_job_process_async (app->plugin_loader, plugin_job,
					    app->cancellable,
					    cancel_trigger_failed_cb,
					    app);
}

static void
reboot_activated (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       data)
{
	gs_utils_invoke_reboot_async (NULL, NULL, NULL);
}

static void
job_manager_shutdown_ready_cb (GObject *source_object,
			       GAsyncResult *result,
			       gpointer user_data)
{
	g_autoptr(GsApplication) self = user_data;
	g_autoptr(GError) error = NULL;

	if (!gs_job_manager_shutdown_finish (GS_JOB_MANAGER (source_object), result, &error))
		g_warning ("Failed to shutdown jobs: %s", error->message);
	else
		g_debug ("Job manager shutdown finished, going to quit the application.");

	g_application_quit (G_APPLICATION (self));
}

static void
shutdown_activated (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       data)
{
	GsApplication *self = GS_APPLICATION (data);
	GsJobManager *job_manager = gs_plugin_loader_get_job_manager (self->plugin_loader);

	g_debug ("Initiating shutdown of the job manager from %s()", G_STRFUNC);
	gs_job_manager_shutdown_async (job_manager, NULL, job_manager_shutdown_ready_cb, g_object_ref (self));
}

static void offline_update_get_app_cb (GObject      *source_object,
                                       GAsyncResult *result,
                                       gpointer      user_data);
static void offline_update_cb (GsPluginLoader *plugin_loader,
                               GAsyncResult   *res,
                               GsApplication  *app);

static void
reboot_and_install (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);

	gs_plugin_loader_get_system_app_async (app->plugin_loader, app->cancellable,
					       offline_update_get_app_cb, app);
}

static void
offline_update_get_app_cb (GObject      *source_object,
                           GAsyncResult *result,
                           gpointer      user_data)
{
	GsApplication *app = GS_APPLICATION (user_data);
	g_autoptr(GsApp) system_app = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) local_error = NULL;

	system_app = gs_plugin_loader_get_system_app_finish (app->plugin_loader,
							     result,
							     &local_error);

	if (system_app == NULL) {
		g_warning ("Failed to trigger offline update: %s", local_error->message);
		return;
	}

	list = gs_app_list_new ();
	gs_app_list_add (list, system_app);

	plugin_job = gs_plugin_job_update_apps_new (list, GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD);
	gs_plugin_loader_job_process_async (app->plugin_loader, plugin_job,
					    app->cancellable,
					    (GAsyncReadyCallback) offline_update_cb,
					    app);
}

static void
offline_update_cb (GsPluginLoader *plugin_loader,
		   GAsyncResult *res,
		   GsApplication *app)
{
	g_autoptr(GError) error = NULL;
	if (!gs_plugin_loader_job_action_finish (plugin_loader, res, &error)) {
		g_warning ("Failed to trigger offline update: %s", error->message);
		return;
	}

	gs_utils_invoke_reboot_async (NULL, reboot_failed_cb, app);
}

static void
quit_activated (GSimpleAction *action,
		GVariant      *parameter,
		gpointer       user_data)
{
	GsApplication *self = GS_APPLICATION (user_data);
	GsJobManager *job_manager;
	GApplicationFlags flags;
	GList *windows;
	GtkWidget *window;

	flags = g_application_get_flags (G_APPLICATION (self));

	if (flags & G_APPLICATION_IS_SERVICE) {
		windows = gtk_application_get_windows (GTK_APPLICATION (self));
		if (windows) {
			window = windows->data;
			gtk_widget_set_visible (window, FALSE);
		}

		return;
	}

	job_manager = gs_plugin_loader_get_job_manager (self->plugin_loader);

	g_debug ("Initiating shutdown of the job manager from %s()", G_STRFUNC);
	gs_job_manager_shutdown_async (job_manager, NULL, job_manager_shutdown_ready_cb, g_object_ref (self));
}

static void
activate_on_shell_loaded_cb (GsActivationHelper *helper)
{
	GsApplication *app = helper->app;

	g_action_activate (G_ACTION (helper->action), helper->action_param);

	g_signal_handlers_disconnect_by_data (app->shell, helper);
	gs_activation_helper_free (helper);
}

static void
set_mode_activated (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *mode;

	gs_application_present_window (app, NULL);

	gs_shell_reset_state (app->shell);

	mode = g_variant_get_string (parameter, NULL);
	if (g_strcmp0 (mode, "updates") == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_UPDATES);
	} else if (g_strcmp0 (mode, "installed") == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_INSTALLED);
	} else if (g_strcmp0 (mode, "moderate") == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_MODERATE);
	} else if (g_strcmp0 (mode, "overview") == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_OVERVIEW);
	} else if (g_strcmp0 (mode, "updated") == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_UPDATES);
		gs_shell_show_installed_updates (app->shell);
	} else {
		g_warning ("Mode '%s' not recognised", mode);
	}
}

static void
search_activated (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *search;

	gs_application_present_window (app, NULL);

	search = g_variant_get_string (parameter, NULL);
	gs_shell_reset_state (app->shell);
	gs_shell_show_search (app->shell, search);
}

static void
_search_launchable_details_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GsApp *a;
	GsApplication *app = GS_APPLICATION (user_data);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;

	list = gs_plugin_loader_job_process_finish (app->plugin_loader, res, &error);
	if (list == NULL) {
		g_warning ("failed to find application: %s", error->message);
		return;
	}
	if (gs_app_list_length (list) == 0) {
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_OVERVIEW);
		gs_shell_show_notification (app->shell,
					    /* TRANSLATORS: we tried to show an app that did not exist */
					    _("Sorry! There are no details for that application."));
		return;
	}
	a = gs_app_list_index (list, 0);
	gs_shell_reset_state (app->shell);
	gs_shell_show_app (app->shell, a);
}

static void
gs_application_app_to_show_created_cb (GObject *source_object,
				       GAsyncResult *result,
				       gpointer user_data)
{
	GsApplication *gs_app = user_data;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;

	app = gs_plugin_loader_app_create_finish (GS_PLUGIN_LOADER (source_object), result, &error);
	if (app == NULL) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED))
			g_warning ("Failed to create application: %s", error->message);
	} else {
		g_return_if_fail (GS_IS_APPLICATION (gs_app));

		gs_shell_reset_state (gs_app->shell);
		gs_shell_show_app (gs_app->shell, app);
	}
}

static void
details_activated (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *id;
	const gchar *search;

	gs_application_present_window (app, NULL);

	g_variant_get (parameter, "(&s&s)", &id, &search);
	g_debug ("trying to activate %s:%s for details", id, search);
	if (search != NULL && search[0] != '\0') {
		gs_shell_reset_state (app->shell);
		gs_shell_show_search_result (app->shell, id, search);
	} else {
		g_autofree gchar *data_id = NULL;
		g_autoptr(GsPluginJob) plugin_job = NULL;
		g_autoptr(GsAppQuery) query = NULL;
		const gchar *keywords[] = { id, NULL };

		data_id = gs_utils_unique_id_compat_convert (id);
		if (data_id != NULL) {
			gs_plugin_loader_app_create_async (app->plugin_loader, data_id, app->cancellable,
				gs_application_app_to_show_created_cb, app);
			return;
		}

		/* find by launchable */
		query = gs_app_query_new ("keywords", keywords,
					  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
					  "dedupe-flags", GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED |
							  GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES,
					  "sort-func", gs_utils_app_sort_match_value,
					  "license-type", gs_shell_get_query_license_type (app->shell),
					  "developer-verified-type", gs_shell_get_query_developer_verified_type (app->shell),
					  NULL);
		plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
		gs_plugin_loader_job_process_async (app->plugin_loader, plugin_job,
						    app->cancellable,
						    _search_launchable_details_cb,
						    app);
	}
}

static void
details_pkg_activated (GSimpleAction *action,
		       GVariant      *parameter,
		       gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *name;
	const gchar *plugin_name;
	g_autoptr (GsApp) a = NULL;

	gs_application_present_window (app, NULL);

	g_variant_get (parameter, "(&s&s)", &name, &plugin_name);
	a = gs_app_new (NULL);
	gs_app_add_source (a, name);
	if (strcmp (plugin_name, "") != 0) {
		GsPlugin *plugin = gs_plugin_loader_find_plugin (app->plugin_loader, plugin_name);
		gs_app_set_management_plugin (a, plugin);
	}

	gs_shell_reset_state (app->shell);
	gs_shell_show_app (app->shell, a);
}

static void
details_url_activated (GSimpleAction *action,
		       GVariant      *parameter,
		       gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *url;
	g_autoptr (GsApp) a = NULL;

	gs_application_present_window (app, NULL);

	g_variant_get (parameter, "(&s)", &url);

	/* this is only used as a wrapper to transport the URL to
	 * the gs_shell_change_mode() function -- not in the GsAppList */
	a = gs_app_new (NULL);
	gs_app_set_metadata (a, "LingmoAppstore::from-url", url);
	gs_shell_reset_state (app->shell);
	gs_shell_show_app (app->shell, a);
}

typedef struct {
	GWeakRef gs_app_weakref;
	gchar *data_id;
	GsShellInteraction interaction;
} InstallActivatedHelper;

static void
gs_application_app_to_install_created_cb (GObject *source_object,
					  GAsyncResult *result,
					  gpointer user_data)
{
	InstallActivatedHelper *helper = user_data;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;

	app = gs_plugin_loader_app_create_finish (GS_PLUGIN_LOADER (source_object), result, &error);
	if (app == NULL) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED))
			g_warning ("Failed to create application '%s': %s", helper->data_id, error->message);
	} else {
		g_autoptr(GsApplication) gs_app = NULL;

		gs_app = g_weak_ref_get (&helper->gs_app_weakref);
		if (gs_app != NULL) {
			gs_shell_reset_state (gs_app->shell);
			gs_shell_install (gs_app->shell, app, helper->interaction);
		}
	}

	g_weak_ref_clear (&helper->gs_app_weakref);
	g_free (helper->data_id);
	g_slice_free (InstallActivatedHelper, helper);
}

static void
install_activated (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *id;
	GsShellInteraction interaction;
	InstallActivatedHelper *helper;
	g_autofree gchar *data_id = NULL;

	g_variant_get (parameter, "(&su)", &id, &interaction);
	data_id = gs_utils_unique_id_compat_convert (id);
	if (data_id == NULL) {
		g_warning ("Need to use a valid unique-id: %s", id);
		return;
	}

	if (interaction == GS_SHELL_INTERACTION_FULL)
		gs_application_present_window (app, NULL);

	helper = g_slice_new0 (InstallActivatedHelper);
	g_weak_ref_init (&helper->gs_app_weakref, app);
	helper->data_id = g_strdup (data_id);
	helper->interaction = interaction;

	gs_plugin_loader_app_create_async (app->plugin_loader, data_id, app->cancellable,
		gs_application_app_to_install_created_cb, helper);
}

typedef struct {
	GWeakRef gs_application_weakref;
	gchar *data_id;
} UninstallActivatedHelper;

static void
gs_application_app_to_uninstall_created_cb (GObject *source_object,
					    GAsyncResult *result,
					    gpointer user_data)
{
	UninstallActivatedHelper *helper = user_data;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr (GsApplication) self = g_weak_ref_get (&helper->gs_application_weakref);

	app = gs_plugin_loader_app_create_finish (GS_PLUGIN_LOADER (source_object), result, &error);

	if (app == NULL) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
			g_warning ("Failed to create application '%s': %s", helper->data_id, error->message);
		}
	} else {
		if (self != NULL) {
			gs_shell_reset_state (self->shell);
			gs_shell_uninstall (self->shell, app);
		}
	}

	g_weak_ref_clear (&helper->gs_application_weakref);
	g_free (helper->data_id);
}


static void
uninstall_activated (GSimpleAction *action,
		     GVariant	   *parameter,
		     gpointer	   data)
{
	GsApplication *self = GS_APPLICATION (data);
	const gchar *id;
	g_autofree gchar *data_id = NULL;
	UninstallActivatedHelper *helper;

	g_variant_get (parameter, "&s", &id);

	data_id = gs_utils_unique_id_compat_convert (id);
	if (data_id == NULL) {
		g_warning ("Need to use a valid unique-id: %s", id);
		return;
	}

	gs_application_present_window (self, NULL);

	helper = g_slice_new0 (UninstallActivatedHelper);
	g_weak_ref_init (&helper->gs_application_weakref, self);
	helper->data_id = g_strdup (data_id);

	gs_plugin_loader_app_create_async (self->plugin_loader, data_id, self->cancellable,
		gs_application_app_to_uninstall_created_cb, helper);
}

static GFile *
_copy_file_to_cache (GFile *file_src, GError **error)
{
	g_autoptr(GFile) file_dest = NULL;
	g_autofree gchar *cache_dir = NULL;
	g_autofree gchar *cache_fn = NULL;
	g_autofree gchar *basename = NULL;

	/* get destination location */
	cache_dir = g_dir_make_tmp ("lingmo-appstore-XXXXXX", error);
	if (cache_dir == NULL)
		return NULL;
	basename = g_file_get_basename (file_src);
	cache_fn = g_build_filename (cache_dir, basename, NULL);

	/* copy file to cache */
	file_dest = g_file_new_for_path (cache_fn);
	if (!g_file_copy (file_src, file_dest,
			  G_FILE_COPY_OVERWRITE,
			  NULL, /* cancellable */
			  NULL, NULL, /* progress */
			  error)) {
		return NULL;
	}
	return g_steal_pointer (&file_dest);
}

static void
filename_activated (GSimpleAction *action,
		    GVariant      *parameter,
		    gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *filename;
	g_autoptr(GFile) file = NULL;

	g_variant_get (parameter, "(&s)", &filename);

	/* this could go away at any moment, so make a local copy */
	if (g_str_has_prefix (filename, "/tmp") ||
	    g_str_has_prefix (filename, "/var/tmp")) {
		g_autoptr(GError) error = NULL;
		g_autoptr(GFile) file_src = g_file_new_for_path (filename);
		file = _copy_file_to_cache (file_src, &error);
		if (file == NULL) {
			g_warning ("failed to copy file, falling back to %s: %s",
				   filename, error->message);
			file = g_file_new_for_path (filename);
		}
	} else {
		file = g_file_new_for_path (filename);
	}
	gs_shell_reset_state (app->shell);
	gs_shell_show_local_file (app->shell, file);
}

static void
show_metainfo_activated (GSimpleAction *action,
			 GVariant      *parameter,
			 gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *filename;
	g_autoptr(GFile) file = NULL;

	g_variant_get (parameter, "(^&ay)", &filename);

	file = g_file_new_for_path (filename);

	gs_shell_reset_state (app->shell);
	gs_shell_show_metainfo (app->shell, file);
}

static void
launch_activated (GSimpleAction *action,
		  GVariant      *parameter,
		  gpointer       data)
{
	GsApplication *self = GS_APPLICATION (data);
	GsApp *app = NULL;
	const gchar *id, *management_plugin_name;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) search_job = NULL;
	g_autoptr(GsPluginJob) launch_job = NULL;
	g_autoptr(GError) error = NULL;
	guint ii, len;
	GsPlugin *management_plugin;
	g_autoptr(GsAppQuery) query = NULL;
	const gchar *keywords[2] = { NULL, };

	g_variant_get (parameter, "(&s&s)", &id, &management_plugin_name);

	keywords[0] = id;
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_match_value,
				  "license-type", gs_shell_get_query_license_type (self->shell),
				  "developer-verified-type", gs_shell_get_query_developer_verified_type (self->shell),
				  NULL);
	search_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (self->plugin_loader, search_job, self->cancellable, &error);
	if (!list) {
		g_warning ("Failed to search for application '%s' (from '%s'): %s", id, management_plugin_name, error ? error->message : "Unknown error");
		return;
	}

	management_plugin = gs_plugin_loader_find_plugin (self->plugin_loader, management_plugin_name);

	len = gs_app_list_length (list);
	for (ii = 0; ii < len && !app; ii++) {
		GsApp *list_app = gs_app_list_index (list, ii);

		if (gs_app_is_installed (list_app) &&
		    gs_app_has_management_plugin (list_app, management_plugin))
			app = list_app;
	}

	if (!app) {
		g_warning ("Did not find application '%s' from '%s'", id, management_plugin_name);
		return;
	}

	launch_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_LAUNCH,
					 "app", app,
					 NULL);
	if (!gs_plugin_loader_job_action (self->plugin_loader, launch_job, self->cancellable, &error)) {
		g_warning ("Failed to launch app: %s", error->message);
		return;
	}
}

static void
show_offline_updates_error (GSimpleAction *action,
			    GVariant      *parameter,
			    gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);

	gs_application_present_window (app, NULL);

	gs_shell_reset_state (app->shell);
	gs_shell_set_mode (app->shell, GS_SHELL_MODE_UPDATES);
	gs_update_monitor_show_error (app->update_monitor, app->main_window);
}

static void
autoupdate_activated (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	GsApplication *app = GS_APPLICATION (data);
	gs_shell_reset_state (app->shell);
	gs_shell_set_mode (app->shell, GS_SHELL_MODE_UPDATES);
	gs_update_monitor_autoupdate (app->update_monitor);
}

static void
install_resources_activated (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *mode;
	const gchar *startup_id;
	const gchar *desktop_id;
	const gchar *ident;
	g_autofree gchar **resources = NULL;

	g_variant_get (parameter, "(&s^a&s&s&s&s)", &mode, &resources, &startup_id, &desktop_id, &ident);

	gs_application_present_window (app, startup_id);

	gs_shell_reset_state (app->shell);
	gs_shell_show_extras_search (app->shell, mode, resources, desktop_id, ident);
}

static void
verbose_activated (GSimpleAction *action,
		   GVariant      *parameter,
		   gpointer       data)
{
	GsApplication *app = GS_APPLICATION (data);
	gs_debug_set_verbose (app->debug, TRUE);
}

static GActionEntry actions[] = {
	{ "about", about_activated, NULL, NULL, NULL },
	{ "quit", quit_activated, NULL, NULL, NULL },
	{ "verbose", verbose_activated, NULL, NULL, NULL },
	{ "nop", NULL, NULL, NULL }
};

static GActionEntry actions_after_loading[] = {
	{ "reboot-and-install", reboot_and_install, NULL, NULL, NULL },
	{ "reboot", reboot_activated, NULL, NULL, NULL },
	{ "shutdown", shutdown_activated, NULL, NULL, NULL },
	{ "launch", launch_activated, "(ss)", NULL, NULL },
	{ "show-offline-update-error", show_offline_updates_error, NULL, NULL, NULL },
	{ "autoupdate", autoupdate_activated, NULL, NULL, NULL },
	{ "sources", sources_activated, NULL, NULL, NULL },
	{ "prefs", prefs_activated, NULL, NULL, NULL },
	{ "set-mode", set_mode_activated, "s", NULL, NULL },
	{ "search", search_activated, "s", NULL, NULL },
	{ "details", details_activated, "(ss)", NULL, NULL },
	{ "details-pkg", details_pkg_activated, "(ss)", NULL, NULL },
	{ "details-url", details_url_activated, "(s)", NULL, NULL },
	{ "install", install_activated, "(su)", NULL, NULL },
	{ "uninstall", uninstall_activated, "s", NULL, NULL },
	{ "filename", filename_activated, "(s)", NULL, NULL },
	{ "install-resources", install_resources_activated, "(sassss)", NULL, NULL },
	{ "show-metainfo", show_metainfo_activated, "(ay)", NULL, NULL },
	{ "nop", NULL, NULL, NULL }
};

static void
gs_application_update_software_sources_presence (GApplication *self)
{
	GsApplication *app = GS_APPLICATION (self);
	GSimpleAction *action;
	gboolean enable_sources;

	action = G_SIMPLE_ACTION (g_action_map_lookup_action (G_ACTION_MAP (self),
							      "sources"));
	enable_sources = g_settings_get_boolean (app->settings,
						 ENABLE_REPOS_DIALOG_CONF_KEY);
	g_simple_action_set_enabled (action, enable_sources);
}

static void
gs_application_settings_changed_cb (GApplication *self,
				    const gchar *key,
				    gpointer data)
{
	if (g_strcmp0 (key, ENABLE_REPOS_DIALOG_CONF_KEY) == 0) {
		gs_application_update_software_sources_presence (self);
	}
}

static void
wrapper_action_activated_cb (GSimpleAction *action,
			     GVariant	   *parameter,
			     gpointer	    data)
{
	GsApplication *app = GS_APPLICATION (data);
	const gchar *action_name = g_action_get_name (G_ACTION (action));
	GAction *real_action = g_action_map_lookup_action (G_ACTION_MAP (app->action_map),
							   action_name);

	if (app->shell_loaded_handler_id != 0) {
		GsActivationHelper *helper = gs_activation_helper_new (app,
								       G_SIMPLE_ACTION (real_action),
								       parameter);

		g_signal_connect_swapped (app->shell, "loaded",
					  G_CALLBACK (activate_on_shell_loaded_cb), helper);
		return;
	}

	g_action_activate (real_action, parameter);
}

static void
gs_application_add_wrapper_actions (GApplication *application)
{
	GsApplication *app = GS_APPLICATION (application);
	GActionMap *map = NULL;

	app->action_map = g_simple_action_group_new ();
	map = G_ACTION_MAP (app->action_map);

	/* add the real actions to a different map and add wrapper actions to the
	 * application instead; the wrapper actions will call the real ones but
	 * after the "loading state" has finished */

	g_action_map_add_action_entries (G_ACTION_MAP (map), actions_after_loading,
					 G_N_ELEMENTS (actions_after_loading),
					 application);

	for (guint i = 0; i < G_N_ELEMENTS (actions_after_loading); ++i) {
		const GActionEntry *entry = &actions_after_loading[i];
		GAction *action = g_action_map_lookup_action (map, entry->name);
		g_autoptr (GSimpleAction) simple_action = NULL;

		simple_action = g_simple_action_new (g_action_get_name (action),
						     g_action_get_parameter_type (action));
		g_signal_connect (simple_action, "activate",
				  G_CALLBACK (wrapper_action_activated_cb),
				  application);
		g_object_bind_property (simple_action, "enabled", action,
					"enabled", G_BINDING_DEFAULT);
		g_action_map_add_action (G_ACTION_MAP (application),
					 G_ACTION (simple_action));
	}
}

static void startup_cb (GObject      *source_object,
                        GAsyncResult *result,
                        gpointer      user_data);

static void
gs_application_startup (GApplication *application)
{
	GSettings *settings;
	GsApplication *app = GS_APPLICATION (application);
	g_auto(GStrv) plugin_blocklist = NULL;
	g_auto(GStrv) plugin_allowlist = NULL;
	const gchar *tmp;

	G_APPLICATION_CLASS (gs_application_parent_class)->startup (application);

	gs_application_add_wrapper_actions (application);

	g_action_map_add_action_entries (G_ACTION_MAP (application),
					 actions, G_N_ELEMENTS (actions),
					 application);

	/* allow for debugging */
	tmp = g_getenv ("LINGMO_SOFTWARE_PLUGINS_BLOCKLIST");
	if (tmp != NULL)
		plugin_blocklist = g_strsplit (tmp, ",", -1);
	tmp = g_getenv ("LINGMO_SOFTWARE_PLUGINS_ALLOWLIST");
	if (tmp != NULL)
		plugin_allowlist = g_strsplit (tmp, ",", -1);

	app->plugin_loader = gs_plugin_loader_new (g_application_get_dbus_connection (application), NULL);
	if (g_file_test (LOCALPLUGINDIR, G_FILE_TEST_EXISTS))
		gs_plugin_loader_add_location (app->plugin_loader, LOCALPLUGINDIR);

	gs_shell_search_provider_setup (app->search_provider, app->plugin_loader);

#ifdef HAVE_PACKAGEKIT
	app->dbus_helper = gs_dbus_helper_new (g_application_get_dbus_connection (application));
#endif
	settings = g_settings_new ("org.lingmo.appstore");
	app->settings = settings;
	g_signal_connect_swapped (settings, "changed",
				  G_CALLBACK (gs_application_settings_changed_cb),
				  application);

	/* setup UI */
	app->shell = gs_shell_new ();
	app->cancellable = g_cancellable_new ();

	app->shell_loaded_handler_id = g_signal_connect (app->shell, "loaded",
							 G_CALLBACK (gs_application_shell_loaded_cb),
							 app);

	app->main_window = GTK_WINDOW (app->shell);
	gtk_application_add_window (GTK_APPLICATION (app), app->main_window);

	gs_application_update_software_sources_presence (application);

	/* Remove possibly obsolete notifications */
	g_application_withdraw_notification (application, "installed");
	g_application_withdraw_notification (application, "restart-required");
	g_application_withdraw_notification (application, "updates-available");
	g_application_withdraw_notification (application, "updates-downloaded");
	g_application_withdraw_notification (application, "updates-installed");
	g_application_withdraw_notification (application, "upgrades-available");
	g_application_withdraw_notification (application, "upgrades-downloaded");
	g_application_withdraw_notification (application, "offline-updates");
	g_application_withdraw_notification (application, "eol");

	/* Set up the plugins. */
	gs_plugin_loader_setup_async (app->plugin_loader,
				      (const gchar * const *) plugin_allowlist,
				      (const gchar * const *) plugin_blocklist,
				      NULL,
				      startup_cb,
				      app);
}

static void
startup_cb (GObject      *source_object,
            GAsyncResult *result,
            gpointer      user_data)
{
	GsApplication *app = GS_APPLICATION (user_data);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) local_error = NULL;

	if (!gs_plugin_loader_setup_finish (plugin_loader,
					    result,
					    &local_error)) {
		g_warning ("Failed to setup plugins: %s", local_error->message);
		exit (1);
	}

	/* show the priority of each plugin */
	gs_plugin_loader_dump_state (plugin_loader);

	app->update_monitor = gs_update_monitor_new (app, app->plugin_loader);

	/* Setup the shell only after the plugin loader finished its setup,
	   thus all plugins are loaded and ready for the jobs. */
	gs_shell_setup (app->shell, app->plugin_loader, app->cancellable);
}

static void
gs_application_activate (GApplication *application)
{
	GsApplication *app = GS_APPLICATION (application);

	if (app->shell_loaded_handler_id == 0)
		gs_shell_set_mode (app->shell, GS_SHELL_MODE_OVERVIEW);

	gs_shell_activate (GS_APPLICATION (application)->shell);
}

static void
gs_application_constructed (GObject *object)
{
	GsApplication *self = GS_APPLICATION (object);

	G_OBJECT_CLASS (gs_application_parent_class)->constructed (object);

	/* This is needed when the the application's ID isn't
	 * org.lingmo.Appstore, e.g. for the development profile (when
	 * `BUILD_PROFILE` is defined). Without this, icon resources can't
	 * be loaded appropriately. */
	g_application_set_resource_base_path (G_APPLICATION (self),
					      "/org/lingmo/Appstore");

	/* Check on our construct-only properties */
	g_assert (self->debug != NULL);
}

static void
gs_application_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	GsApplication *self = GS_APPLICATION (object);

	switch ((GsApplicationProperty) prop_id) {
	case PROP_DEBUG:
		g_value_set_object (value, self->debug);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_application_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	GsApplication *self = GS_APPLICATION (object);

	switch ((GsApplicationProperty) prop_id) {
	case PROP_DEBUG:
		/* Construct only */
		g_assert (self->debug == NULL);
		self->debug = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_application_dispose (GObject *object)
{
	GsApplication *app = GS_APPLICATION (object);

	g_clear_object (&app->search_provider);
	g_clear_object (&app->plugin_loader);
	g_clear_object (&app->update_monitor);
#ifdef HAVE_PACKAGEKIT
	g_clear_object (&app->dbus_helper);
#endif
	g_clear_object (&app->settings);
	g_clear_object (&app->action_map);
	g_clear_object (&app->debug);
	g_clear_pointer (&app->withdraw_notifications, g_hash_table_unref);

	G_OBJECT_CLASS (gs_application_parent_class)->dispose (object);
}

static GsShellInteraction
get_page_interaction_from_string (const gchar *interaction)
{
	if (g_strcmp0 (interaction, "notify") == 0)
		return GS_SHELL_INTERACTION_NOTIFY;
	else if (g_strcmp0 (interaction, "none") == 0)
		return GS_SHELL_INTERACTION_NONE;
	return GS_SHELL_INTERACTION_FULL;
}

static int
gs_application_handle_local_options (GApplication *app, GVariantDict *options)
{
	const gchar *id;
	const gchar *pkgname;
	const gchar *local_filename;
	const gchar *mode;
	const gchar *search;
	gint rc = -1;
	g_autoptr(GError) error = NULL;

	/* prefer local sources */
	if (g_variant_dict_contains (options, "prefer-local"))
		g_setenv ("LINGMO_SOFTWARE_PREFER_LOCAL", "true", TRUE);

	if (g_variant_dict_contains (options, "version")) {
		g_print ("lingmo-appstore %s\n", get_version());
		return 0;
	}

	if (!g_application_register (app, NULL, &error)) {
		g_printerr ("%s\n", error->message);
		return 1;
	}

	if (g_variant_dict_contains (options, "verbose")) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"verbose",
						NULL);
	}

	if (g_variant_dict_contains (options, "autoupdate")) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"autoupdate",
						NULL);
	}
	if (g_variant_dict_contains (options, "prefs")) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"prefs",
						NULL);
	}
	if (g_variant_dict_contains (options, "quit")) {
		/* The 'quit' command-line option shuts down everything,
		 * including the backend service */
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"shutdown",
						NULL);
		return 0;
	}

	if (g_variant_dict_lookup (options, "mode", "&s", &mode)) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"set-mode",
						g_variant_new_string (mode));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "search", "&s", &search)) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"search",
						g_variant_new_string (search));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "details", "&s", &id)) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"details",
						g_variant_new ("(ss)", id, ""));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "details-pkg", "&s", &pkgname)) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"details-pkg",
						g_variant_new ("(ss)", pkgname, ""));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "install", "&s", &id)) {
		GsShellInteraction interaction = GS_SHELL_INTERACTION_FULL;
		const gchar *str_interaction = NULL;

		if (g_variant_dict_lookup (options, "interaction", "&s",
					   &str_interaction))
			interaction = get_page_interaction_from_string (str_interaction);

		g_action_group_activate_action (G_ACTION_GROUP (app),
						"install",
						g_variant_new ("(su)", id,
							       interaction));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "uninstall", "&s", &id)) {
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"uninstall",
						g_variant_new_string (id));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "local-filename", "^&ay", &local_filename)) {
		g_autoptr(GFile) file = NULL;
		g_autofree gchar *absolute_filename = NULL;

		file = g_file_new_for_path (local_filename);
		absolute_filename = g_file_get_path (file);
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"filename",
						g_variant_new ("(s)", absolute_filename));
		rc = 0;
	} else if (g_variant_dict_lookup (options, "show-metainfo", "^&ay", &local_filename)) {
		g_autoptr(GFile) file = NULL;
		g_autofree gchar *absolute_filename = NULL;

		file = g_file_new_for_path (local_filename);
		absolute_filename = g_file_get_path (file);
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"show-metainfo",
						g_variant_new ("(^ay)", absolute_filename));
		rc = 0;
	}

	return rc;
}

static void
gs_application_open (GApplication  *application,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint)
{
	GsApplication *app = GS_APPLICATION (application);
	gint i;

	for (i = 0; i < n_files; i++) {
		g_autofree gchar *str = g_file_get_uri (files[i]);
		g_action_group_activate_action (G_ACTION_GROUP (app),
						"details-url",
						g_variant_new ("(s)", str));
	}
}

static void
gs_application_class_init (GsApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

	object_class->constructed = gs_application_constructed;
	object_class->get_property = gs_application_get_property;
	object_class->set_property = gs_application_set_property;
	object_class->dispose = gs_application_dispose;

	application_class->startup = gs_application_startup;
	application_class->activate = gs_application_activate;
	application_class->handle_local_options = gs_application_handle_local_options;
	application_class->open = gs_application_open;
	application_class->dbus_register = gs_application_dbus_register;
	application_class->dbus_unregister = gs_application_dbus_unregister;
	application_class->shutdown = gs_application_shutdown;

	/**
	 * GsApplication:debug: (nullable)
	 *
	 * A #GsDebug object to control debug and logging output from the
	 * application and everything within it.
	 *
	 * This may be %NULL if you don’t care about log output.
	 *
	 * Since: 40
	 */
	obj_props[PROP_DEBUG] =
		g_param_spec_object ("debug", NULL, NULL,
				     GS_TYPE_DEBUG,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	/**
	 * GsApplication::install-resources-done:
	 * @ident: Operation identificator, as string
	 * @op_error: (nullable): an install #GError, or %NULL on success
	 *
	 * Emitted after a resource installation operation identified by @ident
	 * had finished. The @op_error can hold eventual error message, when
	 * the installation failed.
	 */
	signals[INSTALL_RESOURCES_DONE] = g_signal_new (
		"install-resources-done",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 2,
		G_TYPE_STRING, G_TYPE_ERROR);

	/**
	 * GsApplication::repository-changed:
	 * @repository: a #GsApp of the repository
	 *
	 * Emitted when the repository changed, usually when it is enabled or disabled.
	 *
	 * Since: 40
	 */
	signals[REPOSITORY_CHANGED] = g_signal_new (
		"repository-changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_ACTION | G_SIGNAL_NO_RECURSE,
		0,
		NULL, NULL,
		NULL,
		G_TYPE_NONE, 1,
		GS_TYPE_APP);
}

/**
 * gs_application_new:
 * @debug: (transfer none) (not nullable): a #GsDebug for the application instance
 *
 * Create a new #GsApplication.
 *
 * Returns: (transfer full): a new #GsApplication
 * Since: 40
 */
GsApplication *
gs_application_new (GsDebug *debug)
{
	return g_object_new (GS_APPLICATION_TYPE,
			     "application-id", APPLICATION_ID,
			     "flags", G_APPLICATION_HANDLES_OPEN,
			     "inactivity-timeout", 12000,
			     "debug", debug,
			     NULL);
}

void
gs_application_emit_install_resources_done (GsApplication *application,
					    const gchar *ident,
					    const GError *op_error)
{
	g_signal_emit (application, signals[INSTALL_RESOURCES_DONE], 0, ident, op_error, NULL);
}

static gboolean
gs_application_withdraw_notification_cb (gpointer user_data)
{
	GApplication *application = g_application_get_default ();
	const gchar *notification_id = user_data;

	gs_application_withdraw_notification (GS_APPLICATION (application), notification_id);

	return G_SOURCE_REMOVE;
}

/**
 * gs_application_send_notification:
 * @self: a #GsApplication
 * @notification_id: the @notification ID
 * @notification: a #GNotification
 * @timeout_minutes: how many minutes to wait, before withdraw the notification; 0 for not withdraw
 *
 * Sends the @notification and schedules withdraw of it after
 * @timeout_minutes. This is used to auto-hide notifications
 * after certain period of time. The @timeout_minutes set to 0
 * means to not auto-withdraw it.
 *
 * Since: 43
 **/
void
gs_application_send_notification (GsApplication *self,
				  const gchar *notification_id,
				  GNotification *notification,
				  guint timeout_minutes)
{
	guint timeout_id;

	g_return_if_fail (GS_IS_APPLICATION (self));
	g_return_if_fail (notification_id != NULL);
	g_return_if_fail (G_IS_NOTIFICATION (notification));
	g_return_if_fail (timeout_minutes < G_MAXUINT / 60);

	g_application_send_notification (G_APPLICATION (self), notification_id, notification);

	if (timeout_minutes > 0) {
		if (self->withdraw_notifications == NULL)
			self->withdraw_notifications = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

		timeout_id = GPOINTER_TO_UINT (g_hash_table_lookup (self->withdraw_notifications, notification_id));
		if (timeout_id)
			g_source_remove (timeout_id);
		timeout_id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT, timeout_minutes * 60,
			gs_application_withdraw_notification_cb, g_strdup (notification_id), g_free);
		g_hash_table_insert (self->withdraw_notifications, g_strdup (notification_id), GUINT_TO_POINTER (timeout_id));
	} else if (self->withdraw_notifications != NULL) {
		timeout_id = GPOINTER_TO_UINT (g_hash_table_lookup (self->withdraw_notifications, notification_id));
		if (timeout_id) {
			g_source_remove (timeout_id);
			g_hash_table_remove (self->withdraw_notifications, notification_id);
		}
	}
}

/**
 * gs_application_withdraw_notification:
 * @self: a #GsApplication
 * @notification_id: a #GNotification ID
 *
 * Immediately withdraws the notification @notification_id and
 * removes any previously scheduled withdraw by gs_application_schedule_withdraw_notification().
 *
 * Since: 43
 **/
void
gs_application_withdraw_notification (GsApplication *self,
				      const gchar *notification_id)
{
	g_return_if_fail (GS_IS_APPLICATION (self));
	g_return_if_fail (notification_id != NULL);

	g_application_withdraw_notification (G_APPLICATION (self), notification_id);

	if (self->withdraw_notifications != NULL) {
		g_hash_table_remove (self->withdraw_notifications, notification_id);
		if (g_hash_table_size (self->withdraw_notifications) == 0)
			g_clear_pointer (&self->withdraw_notifications, g_hash_table_unref);
	}
}
