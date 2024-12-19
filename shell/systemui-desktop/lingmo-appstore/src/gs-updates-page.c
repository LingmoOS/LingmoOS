/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "gs-shell.h"
#include "gs-updates-page.h"
#include "gs-common.h"
#include "gs-app-row.h"
#include "gs-plugin-private.h"
#include "gs-removal-dialog.h"
#include "gs-update-monitor.h"
#include "gs-updates-section.h"
#include "gs-upgrade-banner.h"
#include "gs-application.h"

typedef enum {
	GS_UPDATES_PAGE_FLAG_NONE		= 0,
	GS_UPDATES_PAGE_FLAG_HAS_UPDATES	= 1 << 0,
	GS_UPDATES_PAGE_FLAG_HAS_UPGRADES	= 1 << 1,
	GS_UPDATES_PAGE_FLAG_LAST
} GsUpdatesPageFlags;

typedef enum {
	GS_UPDATES_PAGE_STATE_STARTUP,
	GS_UPDATES_PAGE_STATE_ACTION_REFRESH,
	GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES,
	GS_UPDATES_PAGE_STATE_MANAGED,
	GS_UPDATES_PAGE_STATE_IDLE,
	GS_UPDATES_PAGE_STATE_FAILED,
	GS_UPDATES_PAGE_STATE_LAST,
} GsUpdatesPageState;

struct _GsUpdatesPage
{
	GsPage			 parent_instance;

	GsPluginLoader		*plugin_loader;
	GCancellable		*cancellable;
	GCancellable		*cancellable_refresh;
	GCancellable		*cancellable_upgrade;
	GSettings		*settings;
	GSettings		*desktop_settings;
	gboolean		 cache_valid;
	guint			 action_cnt;
	GsShell			*shell;
	GsUpdatesPageState	 state;
	GsUpdatesPageFlags	 result_flags;
	GtkWidget		*button_refresh;
	GtkWidget		*header_spinner_start;
	GtkWidget		*header_start_box;
	gboolean		 has_agreed_to_mobile_data;
	gboolean		 ampm_available;
	guint			 updates_counter;
	gboolean		 is_narrow;

	GtkWidget		*updates_box;
	GtkWidget		*button_updates_mobile;
	GtkWidget		*button_updates_offline;
	GtkWidget		*updates_failed_page;
	GtkLabel		*uptodate_description;
	GtkWidget		*scrolledwindow_updates;
	GtkWidget		*spinner_updates;
	GtkWidget		*stack_updates;
	GtkWidget		*upgrade_banner;
	GtkWidget		*banner_end_of_life;
	GtkWidget		*label_end_of_life;
	GtkWidget		*up_to_date_image;

	GtkSizeGroup		*sizegroup_name;
	GtkSizeGroup		*sizegroup_button_label;
	GtkSizeGroup		*sizegroup_button_image;
	GtkSizeGroup		*sizegroup_header;
	GsUpdatesSection	*sections[GS_UPDATES_SECTION_KIND_LAST];

	guint			 refresh_last_checked_id;
};

enum {
	COLUMN_UPDATE_APP,
	COLUMN_UPDATE_NAME,
	COLUMN_UPDATE_VERSION,
	COLUMN_UPDATE_LAST
};

G_DEFINE_TYPE (GsUpdatesPage, gs_updates_page, GS_TYPE_PAGE)

typedef enum {
	PROP_IS_NARROW = 1,
	/* Overrides: */
	PROP_VADJUSTMENT,
	PROP_TITLE,
	PROP_COUNTER,
} GsUpdatesPageProperty;

static GParamSpec *obj_props[PROP_IS_NARROW + 1] = { NULL, };

static void
gs_updates_page_set_flag (GsUpdatesPage *self, GsUpdatesPageFlags flag)
{
	self->result_flags |= flag;
}

static void
gs_updates_page_clear_flag (GsUpdatesPage *self, GsUpdatesPageFlags flag)
{
	self->result_flags &= ~flag;
}

static const gchar *
gs_updates_page_state_to_string (GsUpdatesPageState state)
{
	if (state == GS_UPDATES_PAGE_STATE_STARTUP)
		return "startup";
	if (state == GS_UPDATES_PAGE_STATE_ACTION_REFRESH)
		return "action-refresh";
	if (state == GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES)
		return "action-get-updates";
	if (state == GS_UPDATES_PAGE_STATE_MANAGED)
		return "managed";
	if (state == GS_UPDATES_PAGE_STATE_IDLE)
		return "idle";
	if (state == GS_UPDATES_PAGE_STATE_FAILED)
		return "failed";
	return NULL;
}

static void
gs_updates_page_invalidate (GsUpdatesPage *self)
{
	self->cache_valid = FALSE;
}

static GsUpdatesSectionKind
_get_app_section (GsApp *app)
{
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_OPERATING_SYSTEM &&
	    gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT))
		return GS_UPDATES_SECTION_KIND_OFFLINE;

	if (!gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT) &&
	    (gs_app_get_state (app) == GS_APP_STATE_UPDATABLE_LIVE ||
	     gs_app_get_state (app) == GS_APP_STATE_INSTALLING ||
	     gs_app_get_state (app) == GS_APP_STATE_DOWNLOADING)) {
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_FIRMWARE)
			return GS_UPDATES_SECTION_KIND_ONLINE_FIRMWARE;
		return GS_UPDATES_SECTION_KIND_ONLINE;
	}
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_FIRMWARE)
		return GS_UPDATES_SECTION_KIND_OFFLINE_FIRMWARE;
	return GS_UPDATES_SECTION_KIND_OFFLINE;
}

static GsAppList *
_get_all_apps (GsUpdatesPage *self)
{
	GsAppList *apps = gs_app_list_new ();
	for (guint i = 0; i < GS_UPDATES_SECTION_KIND_LAST; i++) {
		GsAppList *list = gs_updates_section_get_list (self->sections[i]);
		gs_app_list_add_list (apps, list);
	}
	return apps;
}

static guint
_get_num_updates (GsUpdatesPage *self)
{
	guint count = 0;
	g_autoptr(GsAppList) apps = _get_all_apps (self);

	for (guint i = 0; i < gs_app_list_length (apps); ++i) {
		GsApp *app = gs_app_list_index (apps, i);
		if (gs_app_is_updatable (app) ||
		    gs_app_get_state (app) == GS_APP_STATE_INSTALLING ||
		    gs_app_get_state (app) == GS_APP_STATE_DOWNLOADING)
			++count;
	}
	return count;
}

static gchar *
gs_updates_page_last_checked_time_string (GsUpdatesPage *self,
					  gint *out_hours_ago,
					  gint *out_days_ago)
{
	gint64 last_checked;
	gchar *res;

	g_settings_get (self->settings, "check-timestamp", "x", &last_checked);
	res = gs_utils_time_to_string (last_checked);
	if (res) {
		g_assert (gs_utils_split_time_difference (last_checked, NULL, out_hours_ago, out_days_ago, NULL, NULL, NULL));
	}

	return res;
}

static void
refresh_headerbar_updates_counter (GsUpdatesPage *self)
{
	guint new_updates_counter;

	new_updates_counter = _get_num_updates (self);
	if (!gs_plugin_loader_get_allow_updates (self->plugin_loader) ||
	    self->state == GS_UPDATES_PAGE_STATE_FAILED)
		new_updates_counter = 0;

	if (new_updates_counter == self->updates_counter)
		return;

	self->updates_counter = new_updates_counter;
	g_object_notify (G_OBJECT (self), "counter");
}

static void
gs_updates_page_remove_last_checked_timeout (GsUpdatesPage *self)
{
	if (self->refresh_last_checked_id) {
		g_source_remove (self->refresh_last_checked_id);
		self->refresh_last_checked_id = 0;
	}
}

static void
gs_updates_page_refresh_last_checked (GsUpdatesPage *self);

static gboolean
gs_updates_page_refresh_last_checked_cb (gpointer user_data)
{
	GsUpdatesPage *self = user_data;
	gs_updates_page_refresh_last_checked (self);
	return G_SOURCE_REMOVE;
}

static void
gs_updates_page_refresh_last_checked (GsUpdatesPage *self)
{
	g_autofree gchar *checked_str = NULL;
	gint hours_ago, days_ago;
	checked_str = gs_updates_page_last_checked_time_string (self, &hours_ago, &days_ago);
	if (checked_str != NULL) {
		g_autofree gchar *last_checked = NULL;
		guint interval;

		/* TRANSLATORS: This is the time when we last checked for updates */
		last_checked = g_strdup_printf (_("Last checked: %s"), checked_str);
		gtk_label_set_label (self->uptodate_description, last_checked);
		gtk_widget_set_visible (GTK_WIDGET (self->uptodate_description), TRUE);

		if (hours_ago < 1)
			interval = 60;
		else if (days_ago < 7)
			interval = 60 * 60;
		else
			interval = 60 * 60 * 24;

		gs_updates_page_remove_last_checked_timeout (self);

		self->refresh_last_checked_id = g_timeout_add_seconds (interval,
			gs_updates_page_refresh_last_checked_cb, self);
	} else {
		gtk_widget_set_visible (GTK_WIDGET (self->uptodate_description), FALSE);
	}
}

static void
gs_updates_page_update_ui_state (GsUpdatesPage *self)
{
	gboolean allow_mobile_refresh = TRUE;

	gs_updates_page_remove_last_checked_timeout (self);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_UPDATES)
		return;

	/* spinners */
	switch (self->state) {
	case GS_UPDATES_PAGE_STATE_STARTUP:
	case GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES:
	case GS_UPDATES_PAGE_STATE_ACTION_REFRESH:
		gtk_spinner_start (GTK_SPINNER (self->spinner_updates));
		break;
	default:
		gtk_spinner_stop (GTK_SPINNER (self->spinner_updates));
		gtk_spinner_stop (GTK_SPINNER (self->header_spinner_start));
		gtk_widget_set_visible (self->header_spinner_start, FALSE);
		break;
	}

	/* headerbar refresh icon */
	switch (self->state) {
	case GS_UPDATES_PAGE_STATE_ACTION_REFRESH:
	case GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES:
		gtk_button_set_icon_name (GTK_BUTTON (self->button_refresh), "media-playback-stop-symbolic");
		gtk_widget_set_tooltip_text(self->button_refresh, _("Stop"));
		gtk_widget_set_visible (self->button_refresh, TRUE);
		break;
	case GS_UPDATES_PAGE_STATE_STARTUP:
	case GS_UPDATES_PAGE_STATE_MANAGED:
		gtk_widget_set_visible (self->button_refresh, FALSE);
		break;
	case GS_UPDATES_PAGE_STATE_IDLE:
		gtk_button_set_icon_name (GTK_BUTTON (self->button_refresh), "view-refresh-symbolic");
		gtk_widget_set_tooltip_text(self->button_refresh, _("Check for Updates"));
		if (self->result_flags != GS_UPDATES_PAGE_FLAG_NONE) {
			gtk_widget_set_visible (self->button_refresh, TRUE);
		} else {
			if (gs_plugin_loader_get_network_metered (self->plugin_loader) &&
			    !self->has_agreed_to_mobile_data)
				allow_mobile_refresh = FALSE;
			gtk_widget_set_visible (self->button_refresh, allow_mobile_refresh);
		}
		break;
	case GS_UPDATES_PAGE_STATE_FAILED:
		gtk_button_set_icon_name (GTK_BUTTON (self->button_refresh), "view-refresh-symbolic");
		gtk_widget_set_tooltip_text(self->button_refresh, _("Check for Updates"));
		gtk_widget_set_visible (self->button_refresh, TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}
	gtk_widget_set_sensitive (self->button_refresh,
				  gs_plugin_loader_get_network_available (self->plugin_loader));

	/* stack */
	switch (self->state) {
	case GS_UPDATES_PAGE_STATE_MANAGED:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "managed");
		break;
	case GS_UPDATES_PAGE_STATE_FAILED:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "failed");
		break;
	case GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates),
						  "spinner");
		break;
	case GS_UPDATES_PAGE_STATE_ACTION_REFRESH:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "spinner");
		break;
	case GS_UPDATES_PAGE_STATE_STARTUP:
	case GS_UPDATES_PAGE_STATE_IDLE:

		/* if have updates, just show the view, otherwise show network */
		if (self->result_flags != GS_UPDATES_PAGE_FLAG_NONE) {
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "view");
			break;
		}

		/* check we have a "free" network connection */
		if (gs_plugin_loader_get_network_available (self->plugin_loader) &&
		    !gs_plugin_loader_get_network_metered (self->plugin_loader)) {
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "uptodate");
			break;
		}

		/* expensive network connection */
		if (gs_plugin_loader_get_network_metered (self->plugin_loader)) {
			if (self->has_agreed_to_mobile_data) {
				gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "uptodate");
			} else {
				gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "mobile");
			}
			break;
		}

		/* no network connection */
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_updates), "offline");
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	/* any updates? */
	gtk_widget_set_visible (self->updates_box,
				self->result_flags & GS_UPDATES_PAGE_FLAG_HAS_UPDATES);

	/* last checked label */
	if (g_strcmp0 (gtk_stack_get_visible_child_name (GTK_STACK (self->stack_updates)), "uptodate") == 0)
		gs_updates_page_refresh_last_checked (self);

	/* update the counter in headerbar */
	refresh_headerbar_updates_counter (self);
}

static void
gs_updates_page_set_state (GsUpdatesPage *self, GsUpdatesPageState state)
{
	g_debug ("setting state from %s to %s (has-update:%i, has-upgrade:%i)",
		 gs_updates_page_state_to_string (self->state),
		 gs_updates_page_state_to_string (state),
		 (self->result_flags & GS_UPDATES_PAGE_FLAG_HAS_UPDATES) > 0,
		 (self->result_flags & GS_UPDATES_PAGE_FLAG_HAS_UPGRADES) > 0);
	self->state = state;
	gs_updates_page_update_ui_state (self);
}

static void
gs_updates_page_decrement_refresh_count (GsUpdatesPage *self)
{
	/* every job increments this */
	if (self->action_cnt == 0) {
		g_warning ("action_cnt already zero!");
		return;
	}
	if (--self->action_cnt > 0)
		return;

	/* all done */
	gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_IDLE);
}

static void
gs_updates_page_network_available_notify_cb (GsPluginLoader *plugin_loader,
                                             GParamSpec *pspec,
                                             GsUpdatesPage *self)
{
	gs_updates_page_update_ui_state (self);
}

static void
gs_updates_page_get_updates_cb (GsPluginLoader *plugin_loader,
                                GAsyncResult *res,
                                GsUpdatesPage *self)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;

	self->cache_valid = TRUE;

	/* get the results */
	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		g_autofree gchar *escaped_text = NULL;
		gs_updates_page_clear_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPDATES);
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("updates-shell: failed to get updates: %s", error->message);
		escaped_text = g_markup_escape_text (error->message, -1);
		adw_status_page_set_description (ADW_STATUS_PAGE (self->updates_failed_page), escaped_text);
		gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_FAILED);
		refresh_headerbar_updates_counter (self);
		return;
	}

	/* add the results */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		GsUpdatesSectionKind section = _get_app_section (app);
		gs_updates_section_add_app (self->sections[section], app);
	}

	/* update the counter in headerbar */
	refresh_headerbar_updates_counter (self);

	/* no results */
	if (gs_app_list_length (list) == 0) {
		g_debug ("updates-shell: no updates to show");
		gs_updates_page_clear_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPDATES);
	} else {
		gs_updates_page_set_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPDATES);
	}

	/* only when both set */
	gs_updates_page_decrement_refresh_count (self);
}

static void
gs_updates_page_get_upgrades_cb (GObject *source_object,
                                 GAsyncResult *res,
                                 gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	GsUpdatesPage *self = GS_UPDATES_PAGE (user_data);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;

	/* get the results */
	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		gs_updates_page_clear_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPGRADES);
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_warning ("updates-shell: failed to get upgrades: %s",
				   error->message);
		}
	} else if (gs_app_list_length (list) == 0) {
		g_debug ("updates-shell: no upgrades to show");
		gs_updates_page_clear_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPGRADES);
		gtk_widget_set_visible (self->upgrade_banner, FALSE);
	} else {
		/* rely on the app list already being sorted with the
		 * chronologically newest release last */
		GsApp *app = gs_app_list_index (list, gs_app_list_length (list) - 1);
		g_debug ("got upgrade %s", gs_app_get_id (app));
		gs_upgrade_banner_set_app (GS_UPGRADE_BANNER (self->upgrade_banner), app);
		gs_updates_page_set_flag (self, GS_UPDATES_PAGE_FLAG_HAS_UPGRADES);
		gtk_widget_set_visible (self->upgrade_banner, TRUE);
	}

	/* only when both set */
	gs_updates_page_decrement_refresh_count (self);
}

typedef struct {
	GsApp		*app; /* (owned) */
	GsUpdatesPage	*self; /* (owned) */
	GsPluginJob	*job; /* (owned) */
} GsPageHelper;

static GsPageHelper *
gs_page_helper_new (GsUpdatesPage *self,
		    GsApp	  *app,
		    GsPluginJob   *job)
{
	GsPageHelper *helper;
	helper = g_slice_new0 (GsPageHelper);
	helper->self = g_object_ref (self);
	helper->app = g_object_ref (app);
	helper->job = g_object_ref (job);
	return helper;
}

static void
gs_page_helper_free (GsPageHelper *helper)
{
	g_clear_object (&helper->app);
	g_clear_object (&helper->job);
	g_clear_object (&helper->self);
	g_slice_free (GsPageHelper, helper);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GsPageHelper, gs_page_helper_free);

static void
gs_updates_page_refine_system_finished_cb (GObject *source_object,
					   GAsyncResult *res,
					   gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GsPageHelper) helper = user_data;
	GsUpdatesPage *self = helper->self;
	GsApp *app = helper->app;
	g_autofree char *str = NULL;
	g_autoptr(GError) error = NULL;

	/* get result */
	if (!gs_plugin_loader_job_action_finish (plugin_loader, res, &error)) {
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("Failed to refine system: %s", error->message);
		return;
	}

	/* show or hide the end of life notification */
	if (gs_app_get_state (app) != GS_APP_STATE_UNAVAILABLE) {
		adw_banner_set_revealed (ADW_BANNER (self->banner_end_of_life), FALSE);
		return;
	}

	/* construct a sufficiently scary message */
	if (gs_app_get_name (app) != NULL) {
		/* TRANSLATORS:  the first %s is the distro name, e.g. 'Fedora'
		 * and the second %s is the distro version, e.g. '25' */
		str = g_strdup_printf (_("%s %s has stopped receiving critical software updates"),
				       gs_app_get_name (app),
				       gs_app_get_version (app));
	} else {
		/* TRANSLATORS: This message is meant to tell users that they need to upgrade
		* or else their distro will not get important updates. */
		str = _("Your operating system has stopped receiving critical software updates");
	}

	adw_banner_set_title (ADW_BANNER (self->banner_end_of_life), str);
	adw_banner_set_revealed (ADW_BANNER (self->banner_end_of_life), TRUE);
}

static void
gs_updates_page_get_system_finished_cb (GObject *source_object,
					GAsyncResult *res,
					gpointer user_data)
{
	guint64 refine_flags;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	GsUpdatesPage *self = user_data;
	GsPageHelper *helper;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;

	app = gs_plugin_loader_get_system_app_finish (plugin_loader, res, &error);
	if (app == NULL) {
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("Failed to get system: %s", error->message);
		return;
	}

	g_return_if_fail (GS_IS_UPDATES_PAGE (self));

	refine_flags = GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION;

	plugin_job = gs_plugin_job_refine_new_for_app (app, refine_flags);
	gs_plugin_job_set_interactive (plugin_job, TRUE);
	helper = gs_page_helper_new (self, app, plugin_job);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    gs_updates_page_refine_system_finished_cb,
					    helper);
}

static void
gs_updates_page_load (GsUpdatesPage *self)
{
	guint64 refine_flags;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	if (self->action_cnt > 0)
		return;

	/* remove all existing apps */
	for (guint i = 0; i < GS_UPDATES_SECTION_KIND_LAST; i++)
		gs_updates_section_remove_all (self->sections[i]);

	refine_flags = GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY |
		       GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION;
	gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES);
	self->action_cnt++;
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_GET_UPDATES,
					 "interactive", TRUE,
					 "refine-flags", refine_flags,
					 "dedupe-flags", GS_APP_LIST_FILTER_FLAG_NONE,
					 NULL);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    (GAsyncReadyCallback) gs_updates_page_get_updates_cb,
					    self);

	/* get the system state */
	gs_plugin_loader_get_system_app_async (self->plugin_loader, self->cancellable,
		gs_updates_page_get_system_finished_cb, self);

	/* don't refresh every each time */
	if ((self->result_flags & GS_UPDATES_PAGE_FLAG_HAS_UPGRADES) == 0) {
		refine_flags |= GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPGRADE_REMOVED;
		g_object_unref (plugin_job);
		plugin_job = gs_plugin_job_list_distro_upgrades_new (GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_INTERACTIVE,
								     refine_flags);
		gs_plugin_loader_job_process_async (self->plugin_loader,
						    plugin_job,
						    self->cancellable,
						    gs_updates_page_get_upgrades_cb,
						    self);
		self->action_cnt++;
	}
}

static void
gs_updates_page_reload (GsPage *page)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (page);

	if (self->state == GS_UPDATES_PAGE_STATE_ACTION_REFRESH) {
		g_debug ("ignoring reload as refresh is already in progress");
		return;
	}

	gs_updates_page_invalidate (self);
	gs_updates_page_load (self);
}

static void
gs_updates_page_switch_to (GsPage *page)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (page);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_UPDATES) {
		g_warning ("Called switch_to(updates) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}

	gtk_widget_set_visible (self->button_refresh, TRUE);

	/* no need to refresh */
	if (self->cache_valid) {
		gs_updates_page_update_ui_state (self);
		return;
	}

	if (self->state == GS_UPDATES_PAGE_STATE_ACTION_GET_UPDATES) {
		gs_updates_page_update_ui_state (self);
		return;
	}
	gs_updates_page_load (self);
}

static void
gs_updates_page_switch_from (GsPage *page)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (page);
	gs_updates_page_remove_last_checked_timeout (self);
}

static void
gs_updates_page_refresh_cb (GsPluginLoader *plugin_loader,
                            GAsyncResult *res,
                            GsUpdatesPage *self)
{
	gboolean ret;
	g_autoptr(GDateTime) now = NULL;
	g_autoptr(GError) error = NULL;

	/* get the results */
	ret = gs_plugin_loader_job_action_finish (plugin_loader, res, &error);
	if (!ret) {
		g_autofree gchar *escaped_text = NULL;
		/* user cancel */
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_IDLE);
			return;
		}
		g_warning ("failed to refresh: %s", error->message);
		escaped_text = g_markup_escape_text (error->message, -1);
		adw_status_page_set_description (ADW_STATUS_PAGE (self->updates_failed_page), escaped_text);
		gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_FAILED);
		return;
	}

	/* update the last checked timestamp */
	now = g_date_time_new_now_local ();
	g_settings_set (self->settings, "check-timestamp", "x",
			g_date_time_to_unix (now));

	/* get the new list */
	gs_updates_page_invalidate (self);
	gs_page_switch_to (GS_PAGE (self));
	gs_page_scroll_up (GS_PAGE (self));
}

static void
gs_updates_page_get_new_updates (GsUpdatesPage *self)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* force a check for updates and download */
	gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_ACTION_REFRESH);

	g_cancellable_cancel (self->cancellable_refresh);
	g_clear_object (&self->cancellable_refresh);
	self->cancellable_refresh = g_cancellable_new ();

	plugin_job = gs_plugin_job_refresh_metadata_new (1,
							 GS_PLUGIN_REFRESH_METADATA_FLAGS_INTERACTIVE);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable_refresh,
					    (GAsyncReadyCallback) gs_updates_page_refresh_cb,
					    self);
}

static void
gs_updates_page_show_network_settings (GsUpdatesPage *self)
{
	g_autoptr(GError) error = NULL;
	if (!g_spawn_command_line_async ("lingmo-control-center wifi", &error))
		g_warning ("Failed to open the control center: %s", error->message);
}

static void
gs_updates_page_refresh_settings_cb (AdwMessageDialog *dialog,
                                     const gchar *response,
                                     GsUpdatesPage *self)
{
	if (g_strcmp0 (response, "settings") == 0) {
		/* open the control center */
		gs_updates_page_show_network_settings (self);
	}
}

static void
gs_updates_page_refresh_check_cb (AdwMessageDialog *dialog,
                                  const gchar *response,
                                  GsUpdatesPage *self)
{
	if (g_strcmp0 (response, "check") == 0) {
		self->has_agreed_to_mobile_data = TRUE;
		gs_updates_page_get_new_updates (self);
	}
}

static void
gs_updates_page_button_network_settings_cb (GtkWidget *widget,
                                            GsUpdatesPage *self)
{
	gs_updates_page_show_network_settings (self);
}

static void
gs_updates_page_button_mobile_refresh_cb (GtkWidget *widget,
                                          GsUpdatesPage *self)
{
	self->has_agreed_to_mobile_data = TRUE;
	gs_updates_page_get_new_updates (self);
}

static void
gs_updates_page_button_refresh_cb (GtkWidget *widget,
                                   GsUpdatesPage *self)
{
	GtkWidget *dialog;
	GtkWindow *parent_window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_WINDOW));

	/* cancel existing action? */
	if (self->state == GS_UPDATES_PAGE_STATE_ACTION_REFRESH) {
		g_cancellable_cancel (self->cancellable_refresh);
		g_clear_object (&self->cancellable_refresh);
		return;
	}

	/* check we have a "free" network connection */
	if (gs_plugin_loader_get_network_available (self->plugin_loader) &&
	    !gs_plugin_loader_get_network_metered (self->plugin_loader)) {
		gs_updates_page_get_new_updates (self);

	/* expensive network connection */
	} else if (gs_plugin_loader_get_network_available (self->plugin_loader) &&
	           gs_plugin_loader_get_network_metered (self->plugin_loader)) {
		if (self->has_agreed_to_mobile_data) {
			gs_updates_page_get_new_updates (self);
			return;
		}
		dialog = adw_message_dialog_new (parent_window,
						 /* TRANSLATORS: this is to explain that downloading updates may cost money */
						 _("Charges May Apply"),
						 /* TRANSLATORS: we need network to do the updates check */
						 _("Checking for updates while using mobile broadband could cause you to incur charges."));
		adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
						  "cancel",  _("_Cancel"),
						  /* TRANSLATORS: this is a link to the control-center network panel */
						  "check",  _("Check _Anyway"),
						  NULL);
		g_signal_connect (dialog, "response",
				  G_CALLBACK (gs_updates_page_refresh_check_cb),
				  self);
		gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (dialog));

	/* no network connection */
	} else {
		dialog = adw_message_dialog_new (parent_window,
						 /* TRANSLATORS: can't do updates check */
						 _("No Network"),
						 /* TRANSLATORS: we need network to do the updates check */
						 _("Internet access is required to check for updates."));
		adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
						  "cancel",  _("_Cancel"),
						  /* TRANSLATORS: this is a link to the control-center network panel */
						  "settings",  _("Network _Settings"),
						  NULL);
		g_signal_connect (dialog, "response",
				  G_CALLBACK (gs_updates_page_refresh_settings_cb),
				  self);
		gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (dialog));
	}
}

static void
gs_updates_page_pending_apps_changed_cb (GsPluginLoader *plugin_loader,
                                         GsUpdatesPage *self)
{
	gs_updates_page_invalidate (self);
}

static void
upgrade_download_finished_cb (GObject *source,
                              GAsyncResult *res,
                              gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	g_autoptr(GsPageHelper) helper = user_data;
	g_autoptr(GError) error = NULL;

	g_clear_object (&helper->self->cancellable_upgrade);

	if (!gs_plugin_loader_job_action_finish (plugin_loader, res, &error)) {
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			return;
		gs_plugin_loader_claim_job_error (plugin_loader,
						  NULL,
						  helper->job,
						  error);
	} else if (!gs_page_is_active_and_focused (GS_PAGE (helper->self))) {
		g_autoptr(GNotification) notif = NULL;

		notif = g_notification_new (_("Appstore Upgrades Downloaded"));
		g_notification_set_body (notif, _("Appstore upgrades have been downloaded and are ready to be installed."));
		g_notification_set_default_action_and_target (notif, "app.set-mode", "s", "updates");
		/* last the notification for an hour */
		gs_application_send_notification (GS_APPLICATION (g_application_get_default ()), "upgrades-downloaded", notif, 60);
	}
}

static void
gs_updates_page_upgrade_download_cb (GsUpgradeBanner *upgrade_banner,
                                     GsUpdatesPage *self)
{
	GsApp *app;
	GsPageHelper *helper;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	g_application_withdraw_notification (g_application_get_default (), "upgrades-downloaded");

	app = gs_upgrade_banner_get_app (upgrade_banner);
	if (app == NULL) {
		g_warning ("no upgrade available to download");
		return;
	}

	g_clear_object (&self->cancellable_upgrade);
	self->cancellable_upgrade = g_cancellable_new ();
	g_debug ("Starting upgrade download with cancellable %p", self->cancellable_upgrade);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
					 "interactive", TRUE,
					 "app", app,
					 "propagate-error", TRUE,
					 NULL);
	helper = gs_page_helper_new (self, app, plugin_job);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable_upgrade,
					    upgrade_download_finished_cb,
					    helper);
}

static void
_cancel_trigger_failed_cb (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (user_data);
	g_autoptr(GError) error = NULL;
	if (!gs_plugin_loader_job_action_finish (self->plugin_loader, res, &error)) {
		g_warning ("failed to cancel trigger: %s", error->message);
		return;
	}
}

static void
upgrade_reboot_failed_cb (GObject *source,
                          GAsyncResult *res,
                          gpointer user_data)
{
	GsUpdatesPage *self = (GsUpdatesPage *) user_data;
	GsApp *app;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get result */
	if (gs_utils_invoke_reboot_finish (source, res, &error))
		return;

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		g_debug ("Calling reboot had been cancelled");
	else if (error != NULL)
		g_warning ("Calling reboot failed: %s", error->message);

	app = gs_upgrade_banner_get_app (GS_UPGRADE_BANNER (self->upgrade_banner));
	if (app == NULL) {
		g_warning ("no upgrade to cancel");
		return;
	}

	/* cancel trigger */
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPDATE_CANCEL,
					 "app", app,
					 NULL);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    _cancel_trigger_failed_cb,
					    self);
}

static void
upgrade_trigger_finished_cb (GObject *source,
                             GAsyncResult *res,
                             gpointer user_data)
{
	GsUpdatesPage *self = (GsUpdatesPage *) user_data;
	g_autoptr(GError) error = NULL;

	g_clear_object (&self->cancellable_upgrade);

	/* get the results */
	if (!gs_plugin_loader_job_action_finish (self->plugin_loader, res, &error)) {
		g_warning ("Failed to trigger offline update: %s", error->message);
		return;
	}

	/* trigger reboot */
	gs_utils_invoke_reboot_async (NULL, upgrade_reboot_failed_cb, self);
}

static void
trigger_upgrade (GsUpdatesPage *self)
{
	GsApp *upgrade;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	upgrade = gs_upgrade_banner_get_app (GS_UPGRADE_BANNER (self->upgrade_banner));
	if (upgrade == NULL) {
		g_warning ("no upgrade available to install");
		return;
	}

	g_clear_object (&self->cancellable_upgrade);
	self->cancellable_upgrade = g_cancellable_new ();

	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPGRADE_TRIGGER,
					 "interactive", TRUE,
					 "app", upgrade,
					 NULL);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable_upgrade,
					    upgrade_trigger_finished_cb,
					    self);
}

static void
gs_updates_page_upgrade_confirm_cb (GtkDialog *dialog,
                                    GtkResponseType response_type,
                                    GsUpdatesPage *self)
{
	/* unmap the dialog */
	gtk_window_destroy (GTK_WINDOW (dialog));

	switch (response_type) {
	case GTK_RESPONSE_ACCEPT:
		g_debug ("agreed to upgrade removing apps");
		trigger_upgrade (self);
		break;
	case GTK_RESPONSE_CANCEL:
		g_debug ("cancelled removal dialog");
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	default:
		g_assert_not_reached ();
	}
}

static void
gs_updates_page_upgrade_install_cb (GsUpgradeBanner *upgrade_banner,
                                    GsUpdatesPage *self)
{
	GsAppList *removals;
	GsApp *upgrade;
	GtkWidget *dialog;
	guint cnt = 0;
	guint i;

	upgrade = gs_upgrade_banner_get_app (GS_UPGRADE_BANNER (self->upgrade_banner));
	if (upgrade == NULL) {
		g_warning ("no upgrade available to install");
		return;
	}

	/* count the removals */
	removals = gs_app_get_related (upgrade);
	for (i = 0; i < gs_app_list_length (removals); i++) {
		GsApp *app = gs_app_list_index (removals, i);
		if (gs_app_get_state (app) != GS_APP_STATE_UNAVAILABLE)
			continue;
		cnt++;
	}

	if (cnt == 0) {
		/* no need for a removal confirmation dialog */
		trigger_upgrade (self);
		return;
	}

	dialog = gs_removal_dialog_new ();
	g_signal_connect (dialog, "response",
	                  G_CALLBACK (gs_updates_page_upgrade_confirm_cb),
	                  self);
	gs_removal_dialog_show_upgrade_removals (GS_REMOVAL_DIALOG (dialog),
	                                         upgrade);
	gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (dialog));
}

static void
gs_updates_page_invalidate_downloaded_upgrade (GsUpdatesPage *self)
{
	GsApp *app;
	app = gs_upgrade_banner_get_app (GS_UPGRADE_BANNER (self->upgrade_banner));
	if (app == NULL)
		return;
	if (gs_app_get_state (app) != GS_APP_STATE_UPDATABLE)
		return;
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
	g_debug ("resetting %s to AVAILABLE as the updates have changed",
		 gs_app_get_id (app));
}

static gboolean
gs_shell_update_are_updates_in_progress (GsUpdatesPage *self)
{
	g_autoptr(GsAppList) list = _get_all_apps (self);
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		switch (gs_app_get_state (app)) {
		case GS_APP_STATE_INSTALLING:
		case GS_APP_STATE_REMOVING:
		case GS_APP_STATE_DOWNLOADING:
			return TRUE;
			break;
		default:
			break;
		}
	}
	return FALSE;
}

static void
gs_updates_page_changed_cb (GsPluginLoader *plugin_loader,
                            GsUpdatesPage *self)
{
	/* if we do a live update and the upgrade is waiting to be deployed
	 * then make sure all new packages are downloaded */
	gs_updates_page_invalidate_downloaded_upgrade (self);

	/* check to see if any apps in the app list are in a processing state */
	if (gs_shell_update_are_updates_in_progress (self)) {
		g_debug ("ignoring updates-changed as updates in progress");
		return;
	}

	/* refresh updates list */
	gs_updates_page_reload (GS_PAGE (self));
}

static void
gs_updates_page_status_changed_cb (GsPluginLoader *plugin_loader,
                                   GsApp *app,
                                   GsPluginStatus status,
                                   GsUpdatesPage *self)
{
	gs_updates_page_update_ui_state (self);
}

static void
gs_updates_page_allow_updates_notify_cb (GsPluginLoader *plugin_loader,
                                         GParamSpec *pspec,
                                         GsUpdatesPage *self)
{
	if (!gs_plugin_loader_get_allow_updates (plugin_loader)) {
		gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_MANAGED);
		return;
	}
	gs_updates_page_set_state (self, GS_UPDATES_PAGE_STATE_IDLE);
}

static void
gs_updates_page_upgrade_cancel_cb (GsUpgradeBanner *upgrade_banner,
                                   GsUpdatesPage *self)
{
	g_debug ("Cancelling upgrade with %p", self->cancellable_upgrade);
	g_cancellable_cancel (self->cancellable_upgrade);
}

static gboolean
gs_updates_page_setup (GsPage *page,
                       GsShell *shell,
                       GsPluginLoader *plugin_loader,
                       GCancellable *cancellable,
                       GError **error)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (page);

	g_return_val_if_fail (GS_IS_UPDATES_PAGE (self), TRUE);

	for (guint i = 0; i < GS_UPDATES_SECTION_KIND_LAST; i++) {
		self->sections[i] = gs_updates_section_new (i, plugin_loader, page);
		gs_updates_section_set_size_groups (self->sections[i],
						    self->sizegroup_name,
						    self->sizegroup_button_label,
						    self->sizegroup_button_image,
						    self->sizegroup_header);
		gtk_widget_set_vexpand (GTK_WIDGET (self->sections[i]), FALSE);
		g_object_bind_property (G_OBJECT (self), "is-narrow",
					self->sections[i], "is-narrow",
					G_BINDING_SYNC_CREATE);
		gtk_box_append (GTK_BOX (self->updates_box), GTK_WIDGET (self->sections[i]));
	}

	self->shell = shell;

	self->plugin_loader = g_object_ref (plugin_loader);
	g_signal_connect (self->plugin_loader, "pending-apps-changed",
			  G_CALLBACK (gs_updates_page_pending_apps_changed_cb),
			  self);
	g_signal_connect (self->plugin_loader, "status-changed",
			  G_CALLBACK (gs_updates_page_status_changed_cb),
			  self);
	g_signal_connect (self->plugin_loader, "updates-changed",
			  G_CALLBACK (gs_updates_page_changed_cb),
			  self);
	g_signal_connect_object (self->plugin_loader, "notify::allow-updates",
				 G_CALLBACK (gs_updates_page_allow_updates_notify_cb),
				 self, 0);
	g_signal_connect_object (self->plugin_loader, "notify::network-available",
				 G_CALLBACK (gs_updates_page_network_available_notify_cb),
				 self, 0);
	self->cancellable = g_object_ref (cancellable);

	/* setup system upgrades */
	g_signal_connect (self->upgrade_banner, "download-clicked",
			  G_CALLBACK (gs_updates_page_upgrade_download_cb), self);
	g_signal_connect (self->upgrade_banner, "install-clicked",
			  G_CALLBACK (gs_updates_page_upgrade_install_cb), self);
	g_signal_connect (self->upgrade_banner, "cancel-clicked",
			  G_CALLBACK (gs_updates_page_upgrade_cancel_cb), self);

	self->header_start_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_set_visible (self->header_start_box, TRUE);
	gs_page_set_header_start_widget (GS_PAGE (self), self->header_start_box);

	self->header_spinner_start = gtk_spinner_new ();
	gtk_box_prepend (GTK_BOX (self->header_start_box), self->header_spinner_start);

	/* setup update details window */
	self->button_refresh = gtk_button_new_from_icon_name ("view-refresh-symbolic");
	gtk_accessible_update_property (GTK_ACCESSIBLE (self->button_refresh),
					GTK_ACCESSIBLE_PROPERTY_LABEL, _("Check for updates"),
					-1);
	gtk_box_prepend (GTK_BOX (self->header_start_box), self->button_refresh);
	g_signal_connect (self->button_refresh, "clicked",
			  G_CALLBACK (gs_updates_page_button_refresh_cb),
			  self);

	g_signal_connect (self->button_updates_mobile, "clicked",
			  G_CALLBACK (gs_updates_page_button_mobile_refresh_cb),
			  self);
	g_signal_connect (self->button_updates_offline, "clicked",
			  G_CALLBACK (gs_updates_page_button_network_settings_cb),
			  self);

	/* set initial state */
	if (!gs_plugin_loader_get_allow_updates (self->plugin_loader))
		self->state = GS_UPDATES_PAGE_STATE_MANAGED;
	return TRUE;
}

static void
gs_updates_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (object);

	switch ((GsUpdatesPageProperty) prop_id) {
	case PROP_IS_NARROW:
		g_value_set_boolean (value, gs_updates_page_get_is_narrow (self));
		break;
	case PROP_VADJUSTMENT:
		g_value_set_object (value, gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_updates)));
		break;
	case PROP_TITLE:
		g_value_set_string (value, C_("Apps to be updated", "Updates"));
		break;
	case PROP_COUNTER:
		g_value_set_uint (value, self->updates_counter);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_updates_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (object);

	switch ((GsUpdatesPageProperty) prop_id) {
	case PROP_IS_NARROW:
		gs_updates_page_set_is_narrow (self, g_value_get_boolean (value));
		break;
	case PROP_VADJUSTMENT:
	case PROP_TITLE:
	case PROP_COUNTER:
		/* Read only */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_updates_page_dispose (GObject *object)
{
	GsUpdatesPage *self = GS_UPDATES_PAGE (object);

	gs_updates_page_remove_last_checked_timeout (self);

	g_cancellable_cancel (self->cancellable_refresh);
	g_clear_object (&self->cancellable_refresh);
	g_cancellable_cancel (self->cancellable_upgrade);
	g_clear_object (&self->cancellable_upgrade);

	for (guint i = 0; i < GS_UPDATES_SECTION_KIND_LAST; i++) {
		if (self->sections[i] != NULL) {
			gtk_widget_unparent (GTK_WIDGET (self->sections[i]));
			self->sections[i] = NULL;
		}
	}

	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->settings);
	g_clear_object (&self->desktop_settings);

	g_clear_object (&self->sizegroup_name);
	g_clear_object (&self->sizegroup_button_label);
	g_clear_object (&self->sizegroup_button_image);
	g_clear_object (&self->sizegroup_header);

	G_OBJECT_CLASS (gs_updates_page_parent_class)->dispose (object);
}

static void
gs_updates_page_class_init (GsUpdatesPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_updates_page_get_property;
	object_class->set_property = gs_updates_page_set_property;
	object_class->dispose = gs_updates_page_dispose;

	page_class->switch_to = gs_updates_page_switch_to;
	page_class->switch_from = gs_updates_page_switch_from;
	page_class->reload = gs_updates_page_reload;
	page_class->setup = gs_updates_page_setup;

	/**
	 * GsUpdatesPage:is-narrow:
	 *
	 * Whether the page is in narrow mode.
	 *
	 * In narrow mode, the page will take up less horizontal space, doing so
	 * by e.g. using icons rather than labels in buttons. This is needed to
	 * keep the UI useable on small form-factors like smartphones.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IS_NARROW] =
		g_param_spec_boolean ("is-narrow", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
	g_object_class_override_property (object_class, PROP_TITLE, "title");
	g_object_class_override_property (object_class, PROP_COUNTER, "counter");

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-updates-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, updates_box);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, button_updates_mobile);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, button_updates_offline);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, updates_failed_page);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, uptodate_description);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, scrolledwindow_updates);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, spinner_updates);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, stack_updates);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, upgrade_banner);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, banner_end_of_life);
	gtk_widget_class_bind_template_child (widget_class, GsUpdatesPage, up_to_date_image);
}

static void
gs_updates_page_init (GsUpdatesPage *self)
{
	g_type_ensure (GS_TYPE_UPGRADE_BANNER);

	gtk_widget_init_template (GTK_WIDGET (self));

	self->state = GS_UPDATES_PAGE_STATE_STARTUP;
	self->settings = g_settings_new ("org.lingmo.appstore");
	self->desktop_settings = g_settings_new ("org.gnome.desktop.interface");

	self->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_label = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_image = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_header = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

}

/**
 * gs_updates_page_get_is_narrow:
 * @self: a #GsUpdatesPage
 *
 * Get the value of #GsUpdatesPage:is-narrow.
 *
 * Returns: %TRUE if the page is in narrow mode, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_updates_page_get_is_narrow (GsUpdatesPage *self)
{
	g_return_val_if_fail (GS_IS_UPDATES_PAGE (self), FALSE);

	return self->is_narrow;
}

/**
 * gs_updates_page_set_is_narrow:
 * @self: a #GsUpdatesPage
 * @is_narrow: %TRUE to set the page in narrow mode, %FALSE otherwise
 *
 * Set the value of #GsUpdatesPage:is-narrow.
 *
 * Since: 41
 */
void
gs_updates_page_set_is_narrow (GsUpdatesPage *self, gboolean is_narrow)
{
	g_return_if_fail (GS_IS_UPDATES_PAGE (self));

	is_narrow = !!is_narrow;

	if (self->is_narrow == is_narrow)
		return;

	self->is_narrow = is_narrow;
	if (self->is_narrow)
		gtk_image_set_pixel_size (GTK_IMAGE (self->up_to_date_image), 280);
	else
		gtk_image_set_pixel_size (GTK_IMAGE (self->up_to_date_image), 300);
	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_IS_NARROW]);
}

GsUpdatesPage *
gs_updates_page_new (void)
{
	return GS_UPDATES_PAGE (g_object_new (GS_TYPE_UPDATES_PAGE, NULL));
}
