/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2007-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2020 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <locale.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <appstream.h>
#include <math.h>

#ifdef HAVE_SYSPROF
#include <sysprof-capture.h>
#endif

#include "gs-app-collation.h"
#include "gs-app-private.h"
#include "gs-app-list-private.h"
#include "gs-category-manager.h"
#include "gs-category-private.h"
#include "gs-external-appstream-utils.h"
#include "gs-ioprio.h"
#include "gs-os-release.h"
#include "gs-plugin-loader.h"
#include "gs-plugin.h"
#include "gs-plugin-event.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-private.h"
#include "gs-profiler.h"
#include "gs-utils.h"

#define GS_PLUGIN_LOADER_UPDATES_CHANGED_DELAY	3	/* s */
#define GS_PLUGIN_LOADER_RELOAD_DELAY		5	/* s */

struct _GsPluginLoader
{
	GObject			 parent;

	gboolean		 setup_complete;
	GCancellable		*setup_complete_cancellable;  /* (nullable) (owned) */

	GThreadPool		*old_api_thread_pool;  /* (owned) */

	GPtrArray		*plugins;
	GPtrArray		*locations;
	gchar			*language;
	gboolean		 plugin_dir_dirty;
	GPtrArray		*file_monitors;
	GsPluginStatus		 global_status_last;

	GMutex			 pending_apps_mutex;
	GsAppList		*pending_apps;		/* (nullable) (owned) */
	GCancellable		*pending_apps_cancellable;  /* (nullable) (owned) */

	GThreadPool		*queued_ops_pool;
	gint			 active_jobs;

	GSettings		*settings;

	GMutex			 events_by_id_mutex;
	GHashTable		*events_by_id;		/* unique-id : GsPluginEvent */

	gchar			**compatible_projects;
	guint			 scale;

	guint			 updates_changed_id;
	guint			 updates_changed_cnt;
	guint			 reload_id;
	GHashTable		*disallow_updates;	/* GsPlugin : const char *name */

	GNetworkMonitor		*network_monitor;
	gulong			 network_changed_handler;
	gulong			 network_available_notify_handler;
	gulong			 network_metered_notify_handler;

	GPowerProfileMonitor	*power_profile_monitor;  /* (owned) (nullable) */

	GsJobManager		*job_manager;  /* (owned) (not nullable) */
	GsCategoryManager	*category_manager;
	GsOdrsProvider		*odrs_provider;  /* (owned) (nullable) */

	GDBusConnection		*session_bus_connection;  /* (owned); (not nullable) after setup */
	GDBusConnection		*system_bus_connection;  /* (owned); (not nullable) after setup */
};

static void gs_plugin_loader_monitor_network (GsPluginLoader *plugin_loader);
static void add_app_to_install_queue (GsPluginLoader *plugin_loader, GsApp *app);
static gboolean remove_app_from_install_queue (GsPluginLoader *plugin_loader, GsApp *app);
static void gs_plugin_loader_process_in_thread_pool_cb (gpointer data, gpointer user_data);
static void gs_plugin_loader_status_changed_cb (GsPlugin       *plugin,
                                                GsApp          *app,
                                                GsPluginStatus  status,
                                                GsPluginLoader *plugin_loader);
static void async_result_cb (GObject      *source_object,
                             GAsyncResult *result,
                             gpointer      user_data);
static void gs_plugin_loader_process_old_api_job_cb (gpointer task_data,
                                                     gpointer user_data);

G_DEFINE_TYPE (GsPluginLoader, gs_plugin_loader, G_TYPE_OBJECT)

enum {
	SIGNAL_STATUS_CHANGED,
	SIGNAL_PENDING_APPS_CHANGED,
	SIGNAL_UPDATES_CHANGED,
	SIGNAL_RELOAD,
	SIGNAL_BASIC_AUTH_START,
	SIGNAL_ASK_UNTRUSTED,
	SIGNAL_LAST
};

static guint signals [SIGNAL_LAST] = { 0 };

typedef enum {
	PROP_EVENTS = 1,
	PROP_ALLOW_UPDATES,
	PROP_NETWORK_AVAILABLE,
	PROP_NETWORK_METERED,
	PROP_SESSION_BUS_CONNECTION,
	PROP_SYSTEM_BUS_CONNECTION,
} GsPluginLoaderProperty;

static GParamSpec *obj_props[PROP_SYSTEM_BUS_CONNECTION + 1] = { NULL, };

typedef void		 (*GsPluginFunc)		(GsPlugin	*plugin);
typedef gboolean	 (*GsPluginSetupFunc)		(GsPlugin	*plugin,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginSearchFunc)		(GsPlugin	*plugin,
							 gchar		**value,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginAlternatesFunc)	(GsPlugin	*plugin,
							 GsApp		*app,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginCategoryFunc)	(GsPlugin	*plugin,
							 GsCategory	*category,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginGetRecentFunc)	(GsPlugin	*plugin,
							 GsAppList	*list,
							 guint64	 age,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginResultsFunc)		(GsPlugin	*plugin,
							 GsAppList	*list,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginCategoriesFunc)	(GsPlugin	*plugin,
							 GPtrArray	*list,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginActionFunc)		(GsPlugin	*plugin,
							 GsApp		*app,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginRefreshFunc)		(GsPlugin	*plugin,
							 guint		 cache_age,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginFileToAppFunc)	(GsPlugin	*plugin,
							 GsAppList	*list,
							 GFile		*file,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginUrlToAppFunc)	(GsPlugin	*plugin,
							 GsAppList	*list,
							 const gchar	*url,
							 GCancellable	*cancellable,
							 GError		**error);
typedef gboolean	 (*GsPluginUpdateFunc)		(GsPlugin	*plugin,
							 GsAppList	*apps,
							 GCancellable	*cancellable,
							 GError		**error);
typedef void		 (*GsPluginAdoptAppFunc)	(GsPlugin	*plugin,
							 GsApp		*app);
typedef gboolean	 (*GsPluginGetLangPacksFunc)	(GsPlugin	*plugin,
							 GsAppList	*list,
							 const gchar    *locale,
							 GCancellable	*cancellable,
							 GError		**error);


/* async helper */
typedef struct {
	GsPluginLoader			*plugin_loader;
	const gchar			*function_name;
	const gchar			*function_name_parent;
	GPtrArray			*catlist;
	GsPluginJob			*plugin_job;
	gboolean			 anything_ran;
	gchar				**tokens;
} GsPluginLoaderHelper;

static GsPluginLoaderHelper *
gs_plugin_loader_helper_new (GsPluginLoader *plugin_loader, GsPluginJob *plugin_job)
{
	GsPluginLoaderHelper *helper = g_slice_new0 (GsPluginLoaderHelper);
	GsPluginAction action = gs_plugin_job_get_action (plugin_job);
	helper->plugin_loader = g_object_ref (plugin_loader);
	helper->plugin_job = g_object_ref (plugin_job);
	helper->function_name = gs_plugin_action_to_function_name (action);
	return helper;
}

static void
reset_app_progress (GsApp *app)
{
	g_autoptr(GsAppList) addons = gs_app_dup_addons (app);
	GsAppList *related = gs_app_get_related (app);

	gs_app_set_progress (app, GS_APP_PROGRESS_UNKNOWN);

	for (guint i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
		GsApp *app_addons = gs_app_list_index (addons, i);
		gs_app_set_progress (app_addons, GS_APP_PROGRESS_UNKNOWN);
	}
	for (guint i = 0; i < gs_app_list_length (related); i++) {
		GsApp *app_related = gs_app_list_index (related, i);
		gs_app_set_progress (app_related, GS_APP_PROGRESS_UNKNOWN);
	}
}

static void
gs_plugin_loader_helper_free (GsPluginLoaderHelper *helper)
{
	/* reset progress */
	switch (gs_plugin_job_get_action (helper->plugin_job)) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_REMOVE:
		{
			GsApp *app;
			GsAppList *list;

			app = gs_plugin_job_get_app (helper->plugin_job);
			if (app != NULL)
				reset_app_progress (app);

			list = gs_plugin_job_get_list (helper->plugin_job);
			for (guint i = 0; i < gs_app_list_length (list); i++) {
				GsApp *app_tmp = gs_app_list_index (list, i);
				reset_app_progress (app_tmp);
			}
		}
		break;
	default:
		break;
	}

	g_object_unref (helper->plugin_loader);
	if (helper->plugin_job != NULL)
		g_object_unref (helper->plugin_job);
	if (helper->catlist != NULL)
		g_ptr_array_unref (helper->catlist);
	g_strfreev (helper->tokens);
	g_slice_free (GsPluginLoaderHelper, helper);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GsPluginLoaderHelper, gs_plugin_loader_helper_free)

GsPlugin *
gs_plugin_loader_find_plugin (GsPluginLoader *plugin_loader,
			      const gchar *plugin_name)
{
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		if (g_strcmp0 (gs_plugin_get_name (plugin), plugin_name) == 0)
			return plugin;
	}
	return NULL;
}

static gboolean
gs_plugin_loader_notify_idle_cb (gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);
	g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_EVENTS]);
	return FALSE;
}

void
gs_plugin_loader_add_event (GsPluginLoader *plugin_loader, GsPluginEvent *event)
{
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&plugin_loader->events_by_id_mutex);

	/* events should always have a unique ID, either constructed from the
	 * app they are processing or preferably from the GError message */
	if (gs_plugin_event_get_unique_id (event) == NULL) {
		g_warning ("failed to add event from action %s",
			   gs_plugin_action_to_string (gs_plugin_event_get_action (event)));
		return;
	}

	g_hash_table_insert (plugin_loader->events_by_id,
			     g_strdup (gs_plugin_event_get_unique_id (event)),
			     g_object_ref (event));
	g_idle_add (gs_plugin_loader_notify_idle_cb, plugin_loader);
}

/**
 * gs_plugin_loader_claim_error:
 * @plugin_loader: a #GsPluginLoader
 * @plugin: (nullable): a #GsPlugin to get an application from, or %NULL
 * @action: a #GsPluginAction associated with the @error
 * @app: (nullable): a #GsApp for the event, or %NULL
 * @interactive: whether to set interactive flag
 * @error: a #GError to claim
 *
 * Convert the @error into a plugin event and add it to the queue.
 *
 * The @plugin is used only if the @error contains a reference
 * to a concrete application, in which case any cached application
 * overrides the passed in @app.
 *
 * The %GS_PLUGIN_ERROR_CANCELLED and %G_IO_ERROR_CANCELLED errors
 * are automatically ignored.
 *
 * Since: 41
 **/
void
gs_plugin_loader_claim_error (GsPluginLoader *plugin_loader,
			      GsPlugin *plugin,
			      GsPluginAction action,
			      GsApp *app,
			      gboolean interactive,
			      const GError *error)
{
	g_autoptr(GError) error_copy = NULL;
	g_autofree gchar *app_id = NULL;
	g_autofree gchar *origin_id = NULL;
	g_autoptr(GsPluginEvent) event = NULL;
	g_autoptr(GsApp) event_app = NULL;
	g_autoptr(GsApp) event_origin = NULL;

	g_return_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader));
	g_return_if_fail (error != NULL);

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
	    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		return;

	/* find and strip any unique IDs from the error message */
	error_copy = g_error_copy (error);

	for (guint i = 0; i < 2; i++) {
		if (app_id == NULL)
			app_id = gs_utils_error_strip_app_id (error_copy);
		if (origin_id == NULL)
			origin_id = gs_utils_error_strip_origin_id (error_copy);
	}

	/* invalid */
	if (error_copy->domain != GS_PLUGIN_ERROR) {
		if (g_strcmp0 (BUILD_TYPE, "debug") == 0) {
			g_warning ("not GsPlugin error %s:%i: %s",
				   g_quark_to_string (error_copy->domain),
				   error_copy->code,
				   error_copy->message);
		} else {
			g_debug ("not GsPlugin error %s:%i: %s",
				 g_quark_to_string (error_copy->domain),
				 error_copy->code,
				 error_copy->message);
		}
		error_copy->domain = GS_PLUGIN_ERROR;
		error_copy->code = GS_PLUGIN_ERROR_FAILED;
	}

	/* set the app and origin IDs if we managed to scrape them from the error above */
	if (app != NULL)
		event_app = g_object_ref (app);
	event_origin = NULL;

	if (plugin != NULL && as_utils_data_id_valid (app_id)) {
		g_autoptr(GsApp) cached_app = gs_plugin_cache_lookup (plugin, app_id);
		if (cached_app != NULL) {
			g_debug ("found app %s in error", app_id);
			g_set_object (&event_app, cached_app);
		} else {
			g_debug ("no unique ID found for app %s", app_id);
		}
	}
	if (plugin != NULL && as_utils_data_id_valid (origin_id)) {
		g_autoptr(GsApp) origin = gs_plugin_cache_lookup (plugin, origin_id);
		if (origin != NULL) {
			g_debug ("found origin %s in error", origin_id);
			g_set_object (&event_origin, origin);
		} else {
			g_debug ("no unique ID found for origin %s", origin_id);
		}
	}

	/* create event which is handled by the GsShell */
	event = gs_plugin_event_new ("error", error_copy,
				     "action", action,
				     "app", event_app,
				     "origin", event_origin,
				     NULL);
	if (interactive)
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
	gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);

	/* add event to the queue */
	gs_plugin_loader_add_event (plugin_loader, event);
}

/**
 * gs_plugin_loader_claim_job_error:
 * @plugin_loader: a #GsPluginLoader
 * @plugin: (nullable): a #GsPlugin to get an application from, or %NULL
 * @job: a #GsPluginJob for the @error
 * @error: a #GError to claim
 *
 * The same as gs_plugin_loader_claim_error(), only reads the information
 * from the @job.
 *
 * Since: 41
 **/
void
gs_plugin_loader_claim_job_error (GsPluginLoader *plugin_loader,
				  GsPlugin *plugin,
				  GsPluginJob *job,
				  const GError *error)
{
	g_return_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader));
	g_return_if_fail (GS_IS_PLUGIN_JOB (job));
	g_return_if_fail (error != NULL);

	gs_plugin_loader_claim_error (plugin_loader, plugin,
		gs_plugin_job_get_action (job),
		gs_plugin_job_get_app (job),
		gs_plugin_job_get_interactive (job),
		error);
}

static gboolean
gs_plugin_loader_is_error_fatal (const GError *err)
{
	if (g_error_matches (err, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_TIMED_OUT))
		return TRUE;
	if (g_error_matches (err, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED))
		return TRUE;
	if (g_error_matches (err, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID))
		return TRUE;
	return FALSE;
}

static gboolean
gs_plugin_error_handle_failure (GsPluginLoaderHelper *helper,
				GsPlugin *plugin,
				const GError *error_local,
				GError **error)
{
	g_autoptr(GError) error_local_copy = NULL;
	g_autofree gchar *app_id = NULL;
	g_autofree gchar *origin_id = NULL;

	/* badly behaved plugin */
	if (error_local == NULL) {
		g_critical ("%s did not set error for %s",
			    gs_plugin_get_name (plugin),
			    helper->function_name);
		return TRUE;
	}

	if (gs_plugin_job_get_propagate_error (helper->plugin_job)) {
		g_propagate_error (error, g_error_copy (error_local));
		return FALSE;
	}

	/* this is only ever informational */
	if (g_error_matches (error_local, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
	    g_error_matches (error_local, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		g_debug ("ignoring error cancelled: %s", error_local->message);
		return TRUE;
	}

	/* find and strip any unique IDs from the error message */
	error_local_copy = g_error_copy (error_local);

	for (guint i = 0; i < 2; i++) {
		if (app_id == NULL)
			app_id = gs_utils_error_strip_app_id (error_local_copy);
		if (origin_id == NULL)
			origin_id = gs_utils_error_strip_origin_id (error_local_copy);
	}

	/* fatal error */
	if (gs_plugin_loader_is_error_fatal (error_local_copy) ||
	    g_getenv ("GS_SELF_TEST_PLUGIN_ERROR_FAIL_HARD") != NULL) {
		if (error != NULL)
			*error = g_steal_pointer (&error_local_copy);
		return FALSE;
	}

	gs_plugin_loader_claim_job_error (helper->plugin_loader, plugin, helper->plugin_job, error_local);

	return TRUE;
}

/**
 * gs_plugin_loader_run_adopt:
 * @plugin_loader: a #GsPluginLoader
 * @list: list of apps to try and adopt
 *
 * Call the gs_plugin_adopt_app() function on each plugin on each app in @list
 * to try and find the plugin which should manage each app.
 *
 * This function is intended to be used by internal lingmo-appstore code.
 *
 * Since: 42
 */
void
gs_plugin_loader_run_adopt (GsPluginLoader *plugin_loader, GsAppList *list)
{
	guint i;
	guint j;

	/* go through each plugin in order */
	for (i = 0; i < plugin_loader->plugins->len; i++) {
		GsPluginAdoptAppFunc adopt_app_func = NULL;
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		adopt_app_func = gs_plugin_get_symbol (plugin, "gs_plugin_adopt_app");
		if (adopt_app_func == NULL)
			continue;
		for (j = 0; j < gs_app_list_length (list); j++) {
			GsApp *app = gs_app_list_index (list, j);

			if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD))
				continue;
			if (!gs_app_has_management_plugin (app, NULL))
				continue;

			adopt_app_func (plugin, app);

			if (!gs_app_has_management_plugin (app, NULL)) {
				g_debug ("%s adopted %s",
					 gs_plugin_get_name (plugin),
					 gs_app_get_unique_id (app));
			}
		}
	}
	for (j = 0; j < gs_app_list_length (list); j++) {
		GsApp *app = gs_app_list_index (list, j);

		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD))
			continue;
		if (!gs_app_has_management_plugin (app, NULL))
			continue;

		g_debug ("nothing adopted %s", gs_app_get_unique_id (app));
	}
}

static gboolean
gs_plugin_loader_call_vfunc (GsPluginLoaderHelper *helper,
			     GsPlugin *plugin,
			     GsApp *app,
			     GsAppList *list,
			     GsPluginRefineFlags refine_flags,
			     GCancellable *cancellable,
			     GError **error)
{
	GsPluginLoader *plugin_loader = helper->plugin_loader;
	GsPluginAction action = gs_plugin_job_get_action (helper->plugin_job);
	gboolean ret = TRUE;
	gpointer func = NULL;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GTimer) timer = g_timer_new ();
	g_autofree gchar *sysprof_name = NULL;
	g_autofree gchar *sysprof_message = NULL;

	sysprof_name = g_strconcat ("vfunc:", gs_plugin_action_to_string (action), NULL);
	sysprof_message = gs_plugin_job_to_string (helper->plugin_job);

	GS_PROFILER_BEGIN_SCOPED (PluginLoader, sysprof_name, sysprof_message);

	/* load the possible symbol */
	func = gs_plugin_get_symbol (plugin, helper->function_name);
	if (func == NULL)
		return TRUE;

	/* at least one plugin supports this vfunc */
	helper->anything_ran = TRUE;

	/* fallback if unset */
	if (app == NULL)
		app = gs_plugin_job_get_app (helper->plugin_job);
	if (list == NULL)
		list = gs_plugin_job_get_list (helper->plugin_job);
	if (refine_flags == GS_PLUGIN_REFINE_FLAGS_NONE)
		refine_flags = gs_plugin_job_get_refine_flags (helper->plugin_job);

	/* set what plugin is running on the job */
	gs_plugin_job_set_plugin (helper->plugin_job, plugin);

	/* run the correct vfunc */
	if (gs_plugin_job_get_interactive (helper->plugin_job))
		gs_plugin_interactive_inc (plugin);
	switch (action) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_REMOVE:
	case GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD:
	case GS_PLUGIN_ACTION_UPGRADE_TRIGGER:
	case GS_PLUGIN_ACTION_LAUNCH:
	case GS_PLUGIN_ACTION_UPDATE_CANCEL:
		{
			GsPluginActionFunc plugin_func = func;
			ret = plugin_func (plugin, app, cancellable, &error_local);
		}
		break;
	case GS_PLUGIN_ACTION_GET_UPDATES:
	case GS_PLUGIN_ACTION_GET_UPDATES_HISTORICAL:
	case GS_PLUGIN_ACTION_GET_SOURCES:
		{
			GsPluginResultsFunc plugin_func = func;
			ret = plugin_func (plugin, list, cancellable, &error_local);
		}
		break;
	case GS_PLUGIN_ACTION_FILE_TO_APP:
		{
			GsPluginFileToAppFunc plugin_func = func;
			ret = plugin_func (plugin, list,
					   gs_plugin_job_get_file (helper->plugin_job),
					   cancellable, &error_local);
		}
		break;
	case GS_PLUGIN_ACTION_URL_TO_APP:
		{
			GsPluginUrlToAppFunc plugin_func = func;
			ret = plugin_func (plugin, list,
					   gs_plugin_job_get_search (helper->plugin_job),
					   cancellable, &error_local);
		}
		break;
	case GS_PLUGIN_ACTION_GET_LANGPACKS:
		{
			GsPluginGetLangPacksFunc plugin_func = func;
			ret = plugin_func (plugin, list,
					   gs_plugin_job_get_search (helper->plugin_job),
					   cancellable, &error_local);
		}
		break;
	default:
		g_critical ("no handler for %s", helper->function_name);
		break;
	}
	if (gs_plugin_job_get_interactive (helper->plugin_job))
		gs_plugin_interactive_dec (plugin);

	/* plugin did not return error on cancellable abort */
	if (ret && g_cancellable_set_error_if_cancelled (cancellable, &error_local)) {
		g_debug ("plugin %s did not return error with cancellable set",
			 gs_plugin_get_name (plugin));
		gs_utils_error_convert_gio (&error_local);
		ret = FALSE;
	}

	/* failed */
	if (!ret) {
		return gs_plugin_error_handle_failure (helper,
							plugin,
							error_local,
							error);
	}

	/* add app to the pending installation queue if necessary */
	if (action == GS_PLUGIN_ACTION_INSTALL &&
	    app != NULL && gs_app_get_state (app) == GS_APP_STATE_QUEUED_FOR_INSTALL) {
	        add_app_to_install_queue (plugin_loader, app);
	}

	GS_PROFILER_END_SCOPED (PluginLoader);

	/* check the plugin didn't take too long */
	if (g_timer_elapsed (timer, NULL) > 1.0f) {
		g_log_structured_standard (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
					   __FILE__, G_STRINGIFY (__LINE__),
					   G_STRFUNC,
					   "plugin %s took %.1f seconds to do %s",
					   gs_plugin_get_name (plugin),
					   g_timer_elapsed (timer, NULL),
					   gs_plugin_action_to_string (action));
	}

	return TRUE;
}

static void
gs_plugin_loader_job_sorted_truncation (GsPluginJob *plugin_job,
					GsAppList *list)
{
	GsPluginAction action = gs_plugin_job_get_action (plugin_job);
	guint max_results;

	/* not valid */
	if (list == NULL)
		return;

	/* unset */
	max_results = gs_plugin_job_get_max_results (plugin_job);
	if (max_results == 0)
		return;

	/* already small enough */
	if (gs_app_list_length (list) <= max_results)
		return;

	/* nothing set */
	g_debug ("truncating results to %u from %u",
		 max_results, gs_app_list_length (list));

	g_debug ("randomising %s", gs_plugin_action_to_string (action));
	gs_app_list_randomize (list);
	gs_app_list_truncate (list, max_results);
}

static gboolean
gs_plugin_loader_run_results (GsPluginLoaderHelper *helper,
			      GCancellable *cancellable,
			      GError **error)
{
	GsPluginLoader *plugin_loader = helper->plugin_loader;
	g_autofree gchar *sysprof_name = NULL;
	g_autofree gchar *sysprof_message = NULL;

	sysprof_name = g_strconcat ("run-results:",
				    gs_plugin_action_to_string (gs_plugin_job_get_action (helper->plugin_job)),
				    NULL);
	sysprof_message = gs_plugin_job_to_string (helper->plugin_job);

	GS_PROFILER_BEGIN_SCOPED (PluginLoader, sysprof_name, sysprof_message);

	/* Refining is done separately as it’s a special action */
	g_assert (!GS_IS_PLUGIN_JOB_REFINE (helper->plugin_job));

	/* run each plugin */
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		g_autoptr(GError) local_error = NULL;
		if (g_cancellable_set_error_if_cancelled (cancellable, error)) {
			gs_utils_error_convert_gio (error);
			return FALSE;
		}
		if (!gs_plugin_loader_call_vfunc (helper, plugin, NULL, NULL,
						  GS_PLUGIN_REFINE_FLAGS_NONE,
						  cancellable, &local_error)) {
			gboolean mask_error;

			if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
			    g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
				g_propagate_error (error, g_steal_pointer (&local_error));
				gs_utils_error_convert_gio (error);
				return FALSE;
			} else if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
				g_clear_error (&local_error);
				continue;
			}

			/* Let some actions forgive plugin errors, in case other plugins can handle it,
			   when one plugin fails. */
			switch (gs_plugin_job_get_action (helper->plugin_job)) {
			case GS_PLUGIN_ACTION_GET_UPDATES:
			case GS_PLUGIN_ACTION_GET_SOURCES:
			case GS_PLUGIN_ACTION_GET_LANGPACKS:
				mask_error = TRUE;
				break;
			default:
				mask_error = GS_IS_PLUGIN_JOB_UPDATE_APPS (helper->plugin_job);
				break;
			}
			if (mask_error) {
				g_debug ("plugin '%s' failed to call '%s': %s",
					 gs_plugin_get_name (plugin),
					 helper->function_name,
					 local_error->message);
			} else {
				g_propagate_error (error, g_steal_pointer (&local_error));
				return FALSE;
			}
		}
		gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);
	}

	GS_PROFILER_END_SCOPED (PluginLoader);

	return TRUE;
}

static const gchar *
gs_plugin_loader_get_app_str (GsApp *app)
{
	const gchar *id;

	/* first try the actual id */
	id = gs_app_get_unique_id (app);
	if (id != NULL)
		return id;

	/* then try the source */
	id = gs_app_get_source_default (app);
	if (id != NULL)
		return id;

	/* lastly try the source id */
	id = gs_app_get_source_id_default (app);
	if (id != NULL)
		return id;

	/* urmmm */
	return "<invalid>";
}

gboolean
gs_plugin_loader_app_is_valid (GsApp               *app,
                               GsPluginRefineFlags  refine_flags)
{
	/* never show addons */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_ADDON) {
		g_debug ("app invalid as addon %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* never show CLI apps */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_CONSOLE_APP) {
		g_debug ("app invalid as console %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show unknown state */
	if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN) {
		g_debug ("app invalid as state unknown %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show unconverted unavailables */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_UNKNOWN &&
		gs_app_get_state (app) == GS_APP_STATE_UNAVAILABLE) {
		g_debug ("app invalid as unconverted unavailable %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show blocklisted apps */
	if (gs_app_has_quirk (app, GS_APP_QUIRK_HIDE_EVERYWHERE)) {
		g_debug ("app invalid as blocklisted %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* Don’t show parentally filtered apps unless they’re already
	 * installed. See the comments in gs-details-page.c for details. */
	if (!gs_app_is_installed (app) &&
	    gs_app_has_quirk (app, GS_APP_QUIRK_PARENTAL_FILTER)) {
		g_debug ("app invalid as parentally filtered %s",
		         gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show apps with hide-from-search quirk, unless they are already installed */
	if (!gs_app_is_installed (app) &&
	    gs_app_has_quirk (app, GS_APP_QUIRK_HIDE_FROM_SEARCH)) {
		g_debug ("app invalid as hide-from-search quirk set %s",
		         gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show sources */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY) {
		g_debug ("app invalid as source %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show unknown kind */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_UNKNOWN) {
		g_debug ("app invalid as kind unknown %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show unconverted packages in the application view */
	if (!(refine_flags & GS_PLUGIN_REFINE_FLAGS_ALLOW_PACKAGES) &&
	    gs_app_get_kind (app) == AS_COMPONENT_KIND_GENERIC &&
	    gs_app_get_special_kind (app) == GS_APP_SPECIAL_KIND_NONE) {
		g_debug ("app invalid as only a %s: %s",
			 as_component_kind_to_string (gs_app_get_kind (app)),
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* don't show apps that do not have the required details */
	if (gs_app_get_name (app) == NULL) {
		g_debug ("app invalid as no name %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}
	if (gs_app_get_summary (app) == NULL) {
		g_debug ("app invalid as no summary %s",
			 gs_plugin_loader_get_app_str (app));
		return FALSE;
	}

	/* ignore this crazy application */
	if (g_strcmp0 (gs_app_get_id (app), "lingmo-system-monitor-kde.desktop") == 0) {
		g_debug ("Ignoring KDE version of %s", gs_app_get_id (app));
		return FALSE;
	}
	return TRUE;
}

static gboolean
gs_plugin_loader_app_is_valid_filter (GsApp    *app,
                                      gpointer  user_data)
{
	GsPluginLoaderHelper *helper = (GsPluginLoaderHelper *) user_data;

	return gs_plugin_loader_app_is_valid (app, gs_plugin_job_get_refine_flags (helper->plugin_job));
}

static gboolean
gs_plugin_loader_app_is_valid_updatable (GsApp *app, gpointer user_data)
{
	return gs_plugin_loader_app_is_valid_filter (app, user_data) &&
		(gs_app_is_updatable (app) ||
		 gs_app_get_state (app) == GS_APP_STATE_DOWNLOADING ||
		 gs_app_get_state (app) == GS_APP_STATE_INSTALLING);
}

gboolean
gs_plugin_loader_app_is_compatible (GsPluginLoader *plugin_loader,
                                    GsApp          *app)
{
	const gchar *tmp;
	guint i;

	/* search for any compatible projects */
	tmp = gs_app_get_project_group (app);
	if (tmp == NULL)
		return TRUE;
	for (i = 0; plugin_loader->compatible_projects[i] != NULL; i++) {
		if (g_strcmp0 (tmp,  plugin_loader->compatible_projects[i]) == 0)
			return TRUE;
	}
	g_debug ("removing incompatible %s from project group %s",
		 gs_app_get_id (app), gs_app_get_project_group (app));
	return FALSE;
}

/******************************************************************************/

/**
 * gs_plugin_loader_job_process_finish:
 * @plugin_loader: A #GsPluginLoader
 * @res: a #GAsyncResult
 * @error: A #GError, or %NULL
 *
 * Return value: (element-type GsApp) (transfer full): A list of applications
 **/
GsAppList *
gs_plugin_loader_job_process_finish (GsPluginLoader *plugin_loader,
				     GAsyncResult *res,
				     GError **error)
{
	GTask *task;
	GsAppList *list = NULL;

	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);
	g_return_val_if_fail (G_IS_TASK (res), NULL);
	g_return_val_if_fail (g_task_is_valid (res, plugin_loader), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	task = G_TASK (res);

	/* Return cancelled if the task was cancelled and there is no other error set.
	 *
	 * This is needed because we set the task `check_cancellable` to FALSE,
	 * to be able to catch other errors such as timeout, but that means
	 * g_task_propagate_pointer() will ignore if the task was cancelled and only
	 * check if there was an error (i.e. g_task_return_*error*).
	 *
	 * We only do this if there is no error already set in the task (e.g.
	 * timeout) because in that case we want to return the existing error.
	 */
	if (!g_task_had_error (task)) {
		GCancellable *cancellable = g_task_get_cancellable (task);

		if (g_cancellable_set_error_if_cancelled (cancellable, error)) {
			gs_utils_error_convert_gio (error);
			return NULL;
		}
	}
	list = g_task_propagate_pointer (task, error);
	gs_utils_error_convert_gio (error);
	return list;
}

/**
 * gs_plugin_loader_job_action_finish:
 * @plugin_loader: A #GsPluginLoader
 * @res: a #GAsyncResult
 * @error: A #GError, or %NULL
 *
 * Return value: success
 **/
gboolean
gs_plugin_loader_job_action_finish (GsPluginLoader *plugin_loader,
				     GAsyncResult *res,
				     GError **error)
{
	g_autoptr(GsAppList) list = NULL;

	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), FALSE);
	g_return_val_if_fail (G_IS_TASK (res), FALSE);
	g_return_val_if_fail (g_task_is_valid (res, plugin_loader), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	list = g_task_propagate_pointer (G_TASK (res), error);
	return list != NULL;
}

/******************************************************************************/

static gboolean
emit_pending_apps_idle (gpointer loader)
{
	g_signal_emit (loader, signals[SIGNAL_PENDING_APPS_CHANGED], 0);
	g_object_unref (loader);

	return G_SOURCE_REMOVE;
}

static void
gs_plugin_loader_pending_apps_add (GsPluginLoader *plugin_loader,
				   GsPluginLoaderHelper *helper)
{
	GsAppList *list = gs_plugin_job_get_list (helper->plugin_job);

	g_assert (gs_app_list_length (list) > 0);
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		switch (gs_plugin_job_get_action (helper->plugin_job)) {
		case GS_PLUGIN_ACTION_INSTALL:
			if (gs_app_get_state (app) != GS_APP_STATE_AVAILABLE_LOCAL)
				add_app_to_install_queue (plugin_loader, app);
			/* make sure the progress is properly initialized */
			gs_app_set_progress (app, GS_APP_PROGRESS_UNKNOWN);
			break;
		case GS_PLUGIN_ACTION_REMOVE:
			remove_app_from_install_queue (plugin_loader, app);
			break;
		default:
			break;
		}
	}
	g_idle_add (emit_pending_apps_idle, g_object_ref (plugin_loader));
}

static void
gs_plugin_loader_pending_apps_remove (GsPluginLoader *plugin_loader,
				      GsPluginLoaderHelper *helper)
{
	GsAppList *list = gs_plugin_job_get_list (helper->plugin_job);

	g_assert (gs_app_list_length (list) > 0);
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		remove_app_from_install_queue (plugin_loader, app);

		/* check the app is not still in an action helper */
		switch (gs_app_get_state (app)) {
		case GS_APP_STATE_DOWNLOADING:
		case GS_APP_STATE_INSTALLING:
		case GS_APP_STATE_REMOVING:
			g_warning ("application %s left in %s helper",
				   gs_app_get_unique_id (app),
				   gs_app_state_to_string (gs_app_get_state (app)));
			gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
			break;
		default:
			break;
		}

	}
	g_idle_add (emit_pending_apps_idle, g_object_ref (plugin_loader));
}

static void
async_result_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GAsyncResult **result_out = user_data;

	*result_out = g_object_ref (result);
	g_main_context_wakeup (g_main_context_get_thread_default ());
}

/* This will load the install queue and add it to #GsPluginLoader.pending_apps,
 * but it won’t refine the loaded apps. */
static GsAppList *
load_install_queue (GsPluginLoader  *plugin_loader,
                    GError         **error)
{
	g_autofree gchar *contents = NULL;
	g_autofree gchar *file = NULL;
	g_auto(GStrv) names = NULL;
	g_autoptr(GsAppList) list = NULL;

	/* load from file */
	file = g_build_filename (g_get_user_data_dir (),
				 "lingmo-appstore",
				 "install-queue",
				 NULL);
	if (!g_file_test (file, G_FILE_TEST_EXISTS))
		return gs_app_list_new ();

	g_debug ("loading install queue from %s", file);
	if (!g_file_get_contents (file, &contents, NULL, error))
		return NULL;

	/* add to GsAppList, deduplicating if required */
	list = gs_app_list_new ();
	names = g_strsplit (contents, "\n", 0);
	for (guint i = 0; names[i] != NULL; i++) {
		g_autoptr(GsApp) app = NULL;
		g_auto(GStrv) split = g_strsplit (names[i], "\t", -1);
		if (split[0] == NULL || split[1] == NULL)
			continue;
		app = gs_app_new (NULL);
		gs_app_set_from_unique_id (app, split[0], as_component_kind_from_string (split[1]));
		gs_app_set_state (app, GS_APP_STATE_QUEUED_FOR_INSTALL);
		gs_app_add_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
		gs_app_list_add (list, app);
	}

	/* add to pending list */
	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		g_debug ("adding pending app %s", gs_app_get_unique_id (app));
		if (plugin_loader->pending_apps == NULL)
			plugin_loader->pending_apps = gs_app_list_new ();
		gs_app_list_add (plugin_loader->pending_apps, app);
	}
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);

	return g_steal_pointer (&list);
}

static void
save_install_queue (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GString) s = NULL;
	g_autofree gchar *file = NULL;

	s = g_string_new ("");
	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	for (guint i = 0; plugin_loader->pending_apps != NULL && i < gs_app_list_length (plugin_loader->pending_apps); i++) {
		GsApp *app = gs_app_list_index (plugin_loader->pending_apps, i);
		if (gs_app_get_state (app) == GS_APP_STATE_QUEUED_FOR_INSTALL &&
		    gs_app_get_unique_id (app) != NULL) {
			g_string_append (s, gs_app_get_unique_id (app));
			g_string_append_c (s, '\t');
			g_string_append (s, as_component_kind_to_string (gs_app_get_kind (app)));
			g_string_append_c (s, '\n');
		}
	}
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);

	/* save file */
	file = g_build_filename (g_get_user_data_dir (),
				 "lingmo-appstore",
				 "install-queue",
				 NULL);
	if (s->len == 0) {
		if (g_unlink (file) == -1 && errno != ENOENT) {
			gint errn = errno;
			g_warning ("Failed to unlink '%s': %s", file, g_strerror (errn));
		}
		return;
	}

	if (!gs_mkdir_parent (file, &error)) {
		g_warning ("failed to create dir for %s: %s",
			   file, error->message);
		return;
	}
	g_debug ("saving install queue to %s", file);
	ret = g_file_set_contents (file, s->str, (gssize) s->len, &error);
	if (!ret)
		g_warning ("failed to save install queue: %s", error->message);
}

static void
add_app_to_install_queue (GsPluginLoader *plugin_loader, GsApp *app)
{
	g_autoptr(GsAppList) addons = NULL;
	g_autoptr(GSource) source = NULL;
	guint i;

	/* queue the app itself */
	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	if (plugin_loader->pending_apps == NULL)
		plugin_loader->pending_apps = gs_app_list_new ();
	gs_app_list_add (plugin_loader->pending_apps, app);
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);

	gs_app_set_state (app, GS_APP_STATE_QUEUED_FOR_INSTALL);

	source = g_idle_source_new ();
	g_source_set_callback (source, emit_pending_apps_idle, g_object_ref (plugin_loader), NULL);
	g_source_set_name (source, "[lingmo-appstore] emit_pending_apps_idle");
	g_source_attach (source, NULL);

	save_install_queue (plugin_loader);

	/* recursively queue any addons */
	addons = gs_app_dup_addons (app);
	for (i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
		GsApp *addon = gs_app_list_index (addons, i);
		if (gs_app_get_to_be_installed (addon))
			add_app_to_install_queue (plugin_loader, addon);
	}
}

static gboolean
remove_app_from_install_queue (GsPluginLoader *plugin_loader, GsApp *app)
{
	g_autoptr(GsAppList) addons = NULL;
	gboolean ret;
	guint i;

	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	ret = plugin_loader->pending_apps != NULL && gs_app_list_remove (plugin_loader->pending_apps, app);
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);

	if (ret) {
		g_autoptr(GSource) source = NULL;

		if (gs_app_get_state (app) == GS_APP_STATE_QUEUED_FOR_INSTALL)
			gs_app_set_state (app, GS_APP_STATE_UNKNOWN);

		source = g_idle_source_new ();
		g_source_set_callback (source, emit_pending_apps_idle, g_object_ref (plugin_loader), NULL);
		g_source_set_name (source, "[lingmo-appstore] emit_pending_apps_idle");
		g_source_attach (source, NULL);

		save_install_queue (plugin_loader);

		/* recursively remove any queued addons */
		addons = gs_app_dup_addons (app);
		for (i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
			GsApp *addon = gs_app_list_index (addons, i);
			remove_app_from_install_queue (plugin_loader, addon);
		}
	}

	return ret;
}

/******************************************************************************/

gboolean
gs_plugin_loader_get_allow_updates (GsPluginLoader *plugin_loader)
{
	GHashTableIter iter;
	gpointer value;

	/* nothing */
	if (g_hash_table_size (plugin_loader->disallow_updates) == 0)
		return TRUE;

	/* list */
	g_hash_table_iter_init (&iter, plugin_loader->disallow_updates);
	while (g_hash_table_iter_next (&iter, NULL, &value)) {
		const gchar *reason = value;
		g_debug ("managed updates inhibited by %s", reason);
	}
	return FALSE;
}

GsAppList *
gs_plugin_loader_get_pending (GsPluginLoader *plugin_loader)
{
	GsAppList *array;

	array = gs_app_list_new ();
	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	if (plugin_loader->pending_apps != NULL)
		gs_app_list_add_list (array, plugin_loader->pending_apps);
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);

	return array;
}

gboolean
gs_plugin_loader_get_enabled (GsPluginLoader *plugin_loader,
			      const gchar *plugin_name)
{
	GsPlugin *plugin;
	plugin = gs_plugin_loader_find_plugin (plugin_loader, plugin_name);
	if (plugin == NULL)
		return FALSE;
	return gs_plugin_get_enabled (plugin);
}

/**
 * gs_plugin_loader_get_events:
 * @plugin_loader: A #GsPluginLoader
 *
 * Gets all plugin events, even ones that are not active or visible anymore.
 *
 * Returns: (transfer container) (element-type GsPluginEvent): events
 **/
GPtrArray *
gs_plugin_loader_get_events (GsPluginLoader *plugin_loader)
{
	GPtrArray *events = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&plugin_loader->events_by_id_mutex);
	GHashTableIter iter;
	gpointer key, value;

	/* just add everything */
	g_hash_table_iter_init (&iter, plugin_loader->events_by_id);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		const gchar *id = key;
		GsPluginEvent *event = value;
		if (event == NULL) {
			g_warning ("failed to get event for '%s'", id);
			continue;
		}
		g_ptr_array_add (events, g_object_ref (event));
	}
	return events;
}

/**
 * gs_plugin_loader_get_event_default:
 * @plugin_loader: A #GsPluginLoader
 *
 * Gets an active plugin event where active means that it was not been
 * already dismissed by the user.
 *
 * Returns: (transfer full): a #GsPluginEvent, or %NULL for none
 **/
GsPluginEvent *
gs_plugin_loader_get_event_default (GsPluginLoader *plugin_loader)
{
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&plugin_loader->events_by_id_mutex);
	GHashTableIter iter;
	gpointer key, value;

	/* just add everything */
	g_hash_table_iter_init (&iter, plugin_loader->events_by_id);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		const gchar *id = key;
		GsPluginEvent *event = value;
		if (event == NULL) {
			g_warning ("failed to get event for '%s'", id);
			continue;
		}
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INVALID))
			return g_object_ref (event);
	}
	return NULL;
}

/**
 * gs_plugin_loader_remove_events:
 * @plugin_loader: A #GsPluginLoader
 *
 * Removes all plugin events from the loader. This function should only be
 * called from the self tests.
 **/
void
gs_plugin_loader_remove_events (GsPluginLoader *plugin_loader)
{
	g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&plugin_loader->events_by_id_mutex);
	g_hash_table_remove_all (plugin_loader->events_by_id);
}

static void
gs_plugin_loader_report_event_cb (GsPlugin *plugin,
				  GsPluginEvent *event,
				  GsPluginLoader *plugin_loader)
{
	if (gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE))
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
	gs_plugin_loader_add_event (plugin_loader, event);
}

static void
gs_plugin_loader_allow_updates_cb (GsPlugin *plugin,
				   gboolean allow_updates,
				   GsPluginLoader *plugin_loader)
{
	gboolean changed = FALSE;

	/* plugin now allowing lingmo-appstore to show updates panel */
	if (allow_updates) {
		if (g_hash_table_remove (plugin_loader->disallow_updates, plugin)) {
			g_debug ("plugin %s no longer inhibited managed updates",
				 gs_plugin_get_name (plugin));
			changed = TRUE;
		}

	/* plugin preventing the updates panel from being shown */
	} else {
		if (g_hash_table_replace (plugin_loader->disallow_updates,
					  (gpointer) plugin,
					  (gpointer) gs_plugin_get_name (plugin))) {
			g_debug ("plugin %s inhibited managed updates",
				 gs_plugin_get_name (plugin));
			changed = TRUE;
		}
	}

	/* notify display layer */
	if (changed)
		g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_ALLOW_UPDATES]);
}

static void
gs_plugin_loader_status_changed_cb (GsPlugin *plugin,
				    GsApp *app,
				    GsPluginStatus status,
				    GsPluginLoader *plugin_loader)
{
	/* nothing specific */
	if (app == NULL || gs_app_get_id (app) == NULL) {
		if (plugin_loader->global_status_last != status) {
			g_debug ("emitting global %s",
				 gs_plugin_status_to_string (status));
			g_signal_emit (plugin_loader,
				       signals[SIGNAL_STATUS_CHANGED],
				       0, app, status);
			plugin_loader->global_status_last = status;
		}
		return;
	}

	/* a specific app */
	g_debug ("emitting %s(%s)",
		 gs_plugin_status_to_string (status),
		 gs_app_get_id (app));
	g_signal_emit (plugin_loader,
		       signals[SIGNAL_STATUS_CHANGED],
		       0, app, status);
}

static void
gs_plugin_loader_basic_auth_start_cb (GsPlugin *plugin,
                                      const gchar *remote,
                                      const gchar *realm,
                                      GCallback callback,
                                      gpointer user_data,
                                      GsPluginLoader *plugin_loader)
{
	g_debug ("emitting basic-auth-start %s", realm);
	g_signal_emit (plugin_loader,
		       signals[SIGNAL_BASIC_AUTH_START], 0,
		       remote,
		       realm,
		       callback,
		       user_data);
}

static gboolean
gs_plugin_loader_ask_untrusted_cb (GsPlugin *plugin,
				   const gchar *title,
				   const gchar *msg,
				   const gchar *details,
				   const gchar *accept_label,
				   GsPluginLoader *plugin_loader)
{
	gboolean accepts = FALSE;
	g_debug ("emitting ask-untrusted title:'%s', msg:'%s' details:'%s' accept-label:'%s'", title, msg, details, accept_label);
	g_signal_emit (plugin_loader,
		       signals[SIGNAL_ASK_UNTRUSTED], 0,
		       title, msg, details, accept_label, &accepts);
	return accepts;
}

static gboolean
gs_plugin_loader_job_updates_changed_delay_cb (gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);

	/* notify shells */
	g_debug ("updates-changed");
	g_signal_emit (plugin_loader, signals[SIGNAL_UPDATES_CHANGED], 0);
	plugin_loader->updates_changed_id = 0;
	plugin_loader->updates_changed_cnt = 0;

	return FALSE;
}

static void
gs_plugin_loader_updates_changed (GsPluginLoader *plugin_loader)
{
	if (plugin_loader->updates_changed_id != 0)
		return;
	plugin_loader->updates_changed_id =
		g_timeout_add_seconds_full (G_PRIORITY_DEFAULT,
					    GS_PLUGIN_LOADER_UPDATES_CHANGED_DELAY,
					    gs_plugin_loader_job_updates_changed_delay_cb,
					    g_object_ref (plugin_loader),
					    g_object_unref);
}

static void
gs_plugin_loader_job_updates_changed_cb (GsPlugin *plugin,
					 GsPluginLoader *plugin_loader)
{
	plugin_loader->updates_changed_cnt++;

	/* Schedule emit of updates changed when no job is active.
	   This helps to avoid a race condition when a plugin calls
	   updates-changed at the end of the job, but the job is
	   finished before the callback gets called in the main thread. */
	if (!g_atomic_int_get (&plugin_loader->active_jobs))
		gs_plugin_loader_updates_changed (plugin_loader);
}

static gboolean
gs_plugin_loader_reload_delay_cb (gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);

	/* notify shells */
	g_debug ("emitting ::reload");
	g_signal_emit (plugin_loader, signals[SIGNAL_RELOAD], 0);
	plugin_loader->reload_id = 0;

	g_object_unref (plugin_loader);
	return FALSE;
}

static void
gs_plugin_loader_reload_cb (GsPlugin *in_plugin,
			    GsPluginLoader *plugin_loader)
{
	if (plugin_loader->reload_id != 0)
		return;
	/* Let also the plugins know that the reload had been initiated;
	   The GsPluginClass::reload is a signal function, but its default
	   implementation can be used to notify the plugin. */
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
		if (plugin != in_plugin && plugin_class != NULL && plugin_class->reload != NULL) {
			g_signal_handlers_block_by_func (plugin, gs_plugin_loader_reload_cb, plugin_loader);
			plugin_class->reload (plugin);
			g_signal_handlers_unblock_by_func (plugin, gs_plugin_loader_reload_cb, plugin_loader);
		}
	}
	plugin_loader->reload_id =
		g_timeout_add_seconds (GS_PLUGIN_LOADER_RELOAD_DELAY,
				       gs_plugin_loader_reload_delay_cb,
				       g_object_ref (plugin_loader));
}

static void
gs_plugin_loader_repository_changed_cb (GsPlugin *plugin,
					GsApp *repository,
					GsPluginLoader *plugin_loader)
{
	GApplication *application = g_application_get_default ();

	/* Can be NULL when running the self tests */
	if (application) {
		g_signal_emit_by_name (application,
			"repository-changed",
			repository);
	}
}

static void
gs_plugin_loader_open_plugin (GsPluginLoader *plugin_loader,
			      const gchar *filename)
{
	GsPlugin *plugin;
	g_autoptr(GError) error = NULL;

	/* create plugin from file */
	plugin = gs_plugin_create (filename,
				   plugin_loader->session_bus_connection,
				   plugin_loader->system_bus_connection,
				   &error);
	if (plugin == NULL) {
		g_warning ("Failed to load %s: %s", filename, error->message);
		return;
	}
	g_signal_connect (plugin, "updates-changed",
			  G_CALLBACK (gs_plugin_loader_job_updates_changed_cb),
			  plugin_loader);
	g_signal_connect (plugin, "reload",
			  G_CALLBACK (gs_plugin_loader_reload_cb),
			  plugin_loader);
	g_signal_connect (plugin, "status-changed",
			  G_CALLBACK (gs_plugin_loader_status_changed_cb),
			  plugin_loader);
	g_signal_connect (plugin, "basic-auth-start",
			  G_CALLBACK (gs_plugin_loader_basic_auth_start_cb),
			  plugin_loader);
	g_signal_connect (plugin, "report-event",
			  G_CALLBACK (gs_plugin_loader_report_event_cb),
			  plugin_loader);
	g_signal_connect (plugin, "allow-updates",
			  G_CALLBACK (gs_plugin_loader_allow_updates_cb),
			  plugin_loader);
	g_signal_connect (plugin, "repository-changed",
			  G_CALLBACK (gs_plugin_loader_repository_changed_cb),
			  plugin_loader);
	g_signal_connect (plugin, "ask-untrusted",
			  G_CALLBACK (gs_plugin_loader_ask_untrusted_cb),
			  plugin_loader);
	gs_plugin_set_language (plugin, plugin_loader->language);
	gs_plugin_set_scale (plugin, gs_plugin_loader_get_scale (plugin_loader));
	gs_plugin_set_network_monitor (plugin, plugin_loader->network_monitor);
	g_debug ("opened plugin %s: %s", filename, gs_plugin_get_name (plugin));

	/* add to array */
	g_ptr_array_add (plugin_loader->plugins, plugin);
}

static void
gs_plugin_loader_remove_all_plugins (GsPluginLoader *plugin_loader)
{
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = GS_PLUGIN (plugin_loader->plugins->pdata[i]);
		g_signal_handlers_disconnect_by_data (plugin, plugin_loader);
	}

	g_ptr_array_set_size (plugin_loader->plugins, 0);
}

void
gs_plugin_loader_set_scale (GsPluginLoader *plugin_loader, guint scale)
{
	/* save globally, and update each plugin */
	plugin_loader->scale = scale;
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		gs_plugin_set_scale (plugin, scale);
	}
}

guint
gs_plugin_loader_get_scale (GsPluginLoader *plugin_loader)
{
	return plugin_loader->scale;
}

void
gs_plugin_loader_add_location (GsPluginLoader *plugin_loader, const gchar *location)
{
	for (guint i = 0; i < plugin_loader->locations->len; i++) {
		const gchar *location_tmp = g_ptr_array_index (plugin_loader->locations, i);
		if (g_strcmp0 (location_tmp, location) == 0)
			return;
	}
	g_info ("adding plugin location %s", location);
	g_ptr_array_add (plugin_loader->locations, g_strdup (location));
}

static gint
gs_plugin_loader_plugin_sort_fn (gconstpointer a, gconstpointer b)
{
	GsPlugin *pa = *((GsPlugin **) a);
	GsPlugin *pb = *((GsPlugin **) b);
	if (gs_plugin_get_order (pa) < gs_plugin_get_order (pb))
		return -1;
	if (gs_plugin_get_order (pa) > gs_plugin_get_order (pb))
		return 1;
	return g_strcmp0 (gs_plugin_get_name (pa), gs_plugin_get_name (pb));
}

static void
gs_plugin_loader_software_app_created_cb (GObject *source_object,
					  GAsyncResult *result,
					  gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginEvent) event = NULL;
	g_autoptr(GError) error = NULL;

	app = gs_plugin_loader_app_create_finish (plugin_loader, result, NULL);

	g_set_error_literal (&error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_RESTART_REQUIRED,
			     "A restart is required");
	event = gs_plugin_event_new ("action", GS_PLUGIN_ACTION_UNKNOWN,
				     "app", app,
				     "error", error,
				     NULL);

	gs_plugin_loader_add_event (plugin_loader, event);
}

static void
gs_plugin_loader_plugin_dir_changed_cb (GFileMonitor *monitor,
					GFile *file,
					GFile *other_file,
					GFileMonitorEvent event_type,
					GsPluginLoader *plugin_loader)
{
	/* already triggered */
	if (plugin_loader->plugin_dir_dirty)
		return;

	gs_plugin_loader_app_create_async (plugin_loader, "system/*/*/org.lingmo.Appstore.desktop/*",
		NULL, gs_plugin_loader_software_app_created_cb, NULL);

	plugin_loader->plugin_dir_dirty = TRUE;
}

void
gs_plugin_loader_clear_caches (GsPluginLoader *plugin_loader)
{
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		gs_plugin_cache_invalidate (plugin);
	}
}

static void
gs_plugin_loader_remove_all_file_monitors (GsPluginLoader *plugin_loader)
{
	for (guint i = 0; i < plugin_loader->file_monitors->len; i++) {
		GFileMonitor *file_monitor = G_FILE_MONITOR (plugin_loader->file_monitors->pdata[i]);

		g_signal_handlers_disconnect_by_data (file_monitor, plugin_loader);
		g_file_monitor_cancel (file_monitor);
	}

	g_ptr_array_set_size (plugin_loader->file_monitors, 0);
}

typedef struct {
	GsPluginLoader *plugin_loader;  /* (unowned) */
	GMainContext *context;  /* (owned) */
	guint n_pending;
} ShutdownData;

static void plugin_shutdown_cb (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data);

/**
 * gs_plugin_loader_shutdown:
 * @plugin_loader: a #GsPluginLoader
 * @cancellable: a #GCancellable, or %NULL
 *
 * Shut down the plugins.
 *
 * This blocks until the operation is complete. It may be refactored in future
 * to be asynchronous.
 *
 * Since: 42
 */
void
gs_plugin_loader_shutdown (GsPluginLoader *plugin_loader,
                           GCancellable   *cancellable)
{
	ShutdownData shutdown_data;

	shutdown_data.plugin_loader = plugin_loader;
	shutdown_data.n_pending = 1;  /* incremented until all operations have been started */
	shutdown_data.context = g_main_context_new ();

	g_main_context_push_thread_default (shutdown_data.context);

	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = GS_PLUGIN (plugin_loader->plugins->pdata[i]);

		if (!gs_plugin_get_enabled (plugin))
			continue;

		if (GS_PLUGIN_GET_CLASS (plugin)->shutdown_async != NULL) {
			GS_PLUGIN_GET_CLASS (plugin)->shutdown_async (plugin, cancellable,
								      plugin_shutdown_cb, &shutdown_data);
			shutdown_data.n_pending++;
		}
	}

	/* Wait for shutdown to complete in all plugins. */
	shutdown_data.n_pending--;

	while (shutdown_data.n_pending > 0)
		g_main_context_iteration (shutdown_data.context, TRUE);

	g_main_context_pop_thread_default (shutdown_data.context);
	g_clear_pointer (&shutdown_data.context, g_main_context_unref);

	/* Clear some internal data structures. */
	gs_plugin_loader_remove_all_plugins (plugin_loader);
	gs_plugin_loader_remove_all_file_monitors (plugin_loader);
	plugin_loader->setup_complete = FALSE;
	g_clear_object (&plugin_loader->setup_complete_cancellable);
	plugin_loader->setup_complete_cancellable = g_cancellable_new ();
}

static void
plugin_shutdown_cb (GObject      *source_object,
                    GAsyncResult *result,
                    gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	ShutdownData *data = user_data;
	g_autoptr(GError) local_error = NULL;

	g_assert (GS_PLUGIN_GET_CLASS (plugin)->shutdown_finish != NULL);

	if (!GS_PLUGIN_GET_CLASS (plugin)->shutdown_finish (plugin, result, &local_error)) {
		g_debug ("disabling %s as shutdown failed: %s",
			 gs_plugin_get_name (plugin),
			 local_error->message);
		gs_plugin_set_enabled (plugin, FALSE);
	}

	/* Indicate this plugin has finished shutting down. */
	data->n_pending--;
	g_main_context_wakeup (data->context);
}

static gint
gs_plugin_loader_path_sort_fn (gconstpointer a, gconstpointer b)
{
	const gchar *sa = *((const gchar **) a);
	const gchar *sb = *((const gchar **) b);
	return g_strcmp0 (sa, sb);
}

static GPtrArray *
gs_plugin_loader_find_plugins (const gchar *path, GError **error)
{
	const gchar *fn_tmp;
	g_autoptr(GPtrArray) fns = g_ptr_array_new_with_free_func (g_free);
	g_autoptr(GDir) dir = g_dir_open (path, 0, error);
	if (dir == NULL)
		return NULL;
	while ((fn_tmp = g_dir_read_name (dir)) != NULL) {
		if (!g_str_has_suffix (fn_tmp, ".so"))
			continue;
		g_ptr_array_add (fns, g_build_filename (path, fn_tmp, NULL));
	}
	g_ptr_array_sort (fns, gs_plugin_loader_path_sort_fn);
	return g_steal_pointer (&fns);
}

typedef struct {
	guint n_pending;
	gchar **allowlist;
	gchar **blocklist;
#ifdef HAVE_SYSPROF
	gint64 setup_begin_time_nsec;
	gint64 plugins_begin_time_nsec;
#endif
} SetupData;

static void
setup_data_free (SetupData *data)
{
	g_clear_pointer (&data->allowlist, g_strfreev);
	g_clear_pointer (&data->blocklist, g_strfreev);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (SetupData, setup_data_free)

static void get_session_bus_cb (GObject      *object,
                                GAsyncResult *result,
                                gpointer      user_data);
static void get_system_bus_cb (GObject      *object,
                               GAsyncResult *result,
                               gpointer      user_data);
static void finish_setup_get_bus (GTask *task);
static void plugin_setup_cb (GObject      *source_object,
                             GAsyncResult *result,
                             gpointer      user_data);
static void finish_setup_op (GTask *task);
static void finish_setup_install_queue_cb (GObject      *source_object,
                                           GAsyncResult *result,
                                           gpointer      user_data);

/* Mark the asynchronous setup operation as complete. This will notify any
 * waiting tasks by cancelling the #GCancellable. It’s safe to clear the
 * #GCancellable as each waiting task holds its own reference. */
static void
notify_setup_complete (GsPluginLoader *plugin_loader)
{
	plugin_loader->setup_complete = TRUE;
	g_cancellable_cancel (plugin_loader->setup_complete_cancellable);
	g_clear_object (&plugin_loader->setup_complete_cancellable);
}

/**
 * gs_plugin_loader_setup_async:
 * @plugin_loader: a #GsPluginLoader
 * @allowlist: list of plugin names, or %NULL
 * @blocklist: list of plugin names, or %NULL
 * @cancellable: A #GCancellable, or %NULL
 * @callback: callback to indicate completion of the asynchronous operation
 * @user_data: data to pass to @callback
 *
 * Sets up the plugin loader ready for use.
 *
 * Since: 42
 */
void
gs_plugin_loader_setup_async (GsPluginLoader      *plugin_loader,
                              const gchar * const *allowlist,
                              const gchar * const *blocklist,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
	SetupData *setup_data;
	g_autoptr(SetupData) setup_data_owned = NULL;
	g_autoptr(GTask) task = NULL;
#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec G_GNUC_UNUSED = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	task = g_task_new (plugin_loader, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_loader_setup_async);

	/* If setup is already complete, return immediately. */
	if (plugin_loader->setup_complete) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* Setup data closure. */
	setup_data = setup_data_owned = g_new0 (SetupData, 1);
	setup_data->allowlist = g_strdupv ((gchar **) allowlist);
	setup_data->blocklist = g_strdupv ((gchar **) blocklist);
#ifdef HAVE_SYSPROF
	setup_data->setup_begin_time_nsec = begin_time_nsec;
#endif

	g_task_set_task_data (task, g_steal_pointer (&setup_data_owned), (GDestroyNotify) setup_data_free);

	/* Connect to D-Bus if connections haven’t been provided at construction
	 * time. */
	if (plugin_loader->session_bus_connection == NULL)
		g_bus_get (G_BUS_TYPE_SESSION, cancellable, get_session_bus_cb, g_object_ref (task));
	if (plugin_loader->system_bus_connection == NULL)
		g_bus_get (G_BUS_TYPE_SYSTEM, cancellable, get_system_bus_cb, g_object_ref (task));

	finish_setup_get_bus (task);
}

static void
get_session_bus_cb (GObject      *object,
                    GAsyncResult *result,
                    gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginLoader *plugin_loader = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	plugin_loader->session_bus_connection = g_bus_get_finish (result, &local_error);
	if (plugin_loader->session_bus_connection == NULL) {
		notify_setup_complete (plugin_loader);
		g_prefix_error_literal (&local_error, "Error getting session bus: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_SESSION_BUS_CONNECTION]);

	finish_setup_get_bus (task);
}

static void
get_system_bus_cb (GObject      *object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginLoader *plugin_loader = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	plugin_loader->system_bus_connection = g_bus_get_finish (result, &local_error);
	if (plugin_loader->system_bus_connection == NULL) {
		notify_setup_complete (plugin_loader);
		g_prefix_error_literal (&local_error, "Error getting system bus: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_SYSTEM_BUS_CONNECTION]);

	finish_setup_get_bus (task);
}

static void
finish_setup_get_bus (GTask *task)
{
	SetupData *data = g_task_get_task_data (task);
	GsPluginLoader *plugin_loader = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	const gchar *plugin_name;
	gboolean changes;
	GPtrArray *deps;
	GsPlugin *dep;
	GsPlugin *plugin;
	guint dep_loop_check = 0;
	guint i;
	guint j;
	g_autoptr(GPtrArray) locations = NULL;
	g_autoptr(GError) local_error = NULL;

	/* Wait until we’ve got all the buses we need. */
	if (plugin_loader->session_bus_connection == NULL ||
	    plugin_loader->system_bus_connection == NULL)
		return;

	/* use the default, but this requires a 'make install' */
	if (plugin_loader->locations->len == 0) {
		g_autofree gchar *filename = NULL;
		filename = g_strdup_printf ("plugins-%s", GS_PLUGIN_API_VERSION);
		locations = g_ptr_array_new_with_free_func (g_free);
		g_ptr_array_add (locations, g_build_filename (LIBDIR, "lingmo-appstore", filename, NULL));
	} else {
		locations = g_ptr_array_ref (plugin_loader->locations);
	}

	for (i = 0; i < locations->len; i++) {
		GFileMonitor *monitor;
		const gchar *location = g_ptr_array_index (locations, i);
		g_autoptr(GFile) plugin_dir = g_file_new_for_path (location);
		g_debug ("monitoring plugin location %s", location);
		monitor = g_file_monitor_directory (plugin_dir,
						    G_FILE_MONITOR_NONE,
						    cancellable,
						    &local_error);
		if (monitor == NULL) {
			notify_setup_complete (plugin_loader);
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		g_signal_connect (monitor, "changed",
				  G_CALLBACK (gs_plugin_loader_plugin_dir_changed_cb), plugin_loader);
		g_ptr_array_add (plugin_loader->file_monitors, monitor);
	}

	/* search for plugins */
	for (i = 0; i < locations->len; i++) {
		const gchar *location = g_ptr_array_index (locations, i);
		g_autoptr(GPtrArray) fns = NULL;

		/* search in the plugin directory for plugins */
		g_debug ("searching for plugins in %s", location);
		fns = gs_plugin_loader_find_plugins (location, &local_error);
		if (fns == NULL) {
			notify_setup_complete (plugin_loader);
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		for (j = 0; j < fns->len; j++) {
			const gchar *fn = g_ptr_array_index (fns, j);
			gs_plugin_loader_open_plugin (plugin_loader, fn);
		}
	}

	/* optional allowlist */
	if (data->allowlist != NULL) {
		for (i = 0; i < plugin_loader->plugins->len; i++) {
			gboolean ret;
			plugin = g_ptr_array_index (plugin_loader->plugins, i);
			if (!gs_plugin_get_enabled (plugin))
				continue;
			ret = g_strv_contains ((const gchar * const *) data->allowlist,
					       gs_plugin_get_name (plugin));
			if (!ret) {
				g_debug ("%s not in allowlist, disabling",
					 gs_plugin_get_name (plugin));
			}
			gs_plugin_set_enabled (plugin, ret);
		}
	}

	/* optional blocklist */
	if (data->blocklist != NULL) {
		for (i = 0; i < plugin_loader->plugins->len; i++) {
			gboolean ret;
			plugin = g_ptr_array_index (plugin_loader->plugins, i);
			if (!gs_plugin_get_enabled (plugin))
				continue;
			ret = g_strv_contains ((const gchar * const *) data->blocklist,
					       gs_plugin_get_name (plugin));
			if (ret)
				gs_plugin_set_enabled (plugin, FALSE);
		}
	}

	/* order by deps */
	do {
		changes = FALSE;
		for (i = 0; i < plugin_loader->plugins->len; i++) {
			plugin = g_ptr_array_index (plugin_loader->plugins, i);
			deps = gs_plugin_get_rules (plugin, GS_PLUGIN_RULE_RUN_AFTER);
			for (j = 0; j < deps->len && !changes; j++) {
				plugin_name = g_ptr_array_index (deps, j);
				dep = gs_plugin_loader_find_plugin (plugin_loader,
								    plugin_name);
				if (dep == NULL) {
					g_debug ("cannot find plugin '%s' "
						 "requested by '%s'",
						 plugin_name,
						 gs_plugin_get_name (plugin));
					continue;
				}
				if (!gs_plugin_get_enabled (dep))
					continue;
				if (gs_plugin_get_order (plugin) <= gs_plugin_get_order (dep)) {
					gs_plugin_set_order (plugin, gs_plugin_get_order (dep) + 1);
					changes = TRUE;
				}
			}
		}
		for (i = 0; i < plugin_loader->plugins->len; i++) {
			plugin = g_ptr_array_index (plugin_loader->plugins, i);
			deps = gs_plugin_get_rules (plugin, GS_PLUGIN_RULE_RUN_BEFORE);
			for (j = 0; j < deps->len && !changes; j++) {
				plugin_name = g_ptr_array_index (deps, j);
				dep = gs_plugin_loader_find_plugin (plugin_loader,
								    plugin_name);
				if (dep == NULL) {
					g_debug ("cannot find plugin '%s' "
						 "requested by '%s'",
						 plugin_name,
						 gs_plugin_get_name (plugin));
					continue;
				}
				if (!gs_plugin_get_enabled (dep))
					continue;
				if (gs_plugin_get_order (plugin) >= gs_plugin_get_order (dep)) {
					gs_plugin_set_order (dep, gs_plugin_get_order (plugin) + 1);
					changes = TRUE;
				}
			}
		}

		/* check we're not stuck */
		if (dep_loop_check++ > 100) {
			notify_setup_complete (plugin_loader);
			g_task_return_new_error (task,
						 GS_PLUGIN_ERROR,
						 GS_PLUGIN_ERROR_PLUGIN_DEPSOLVE_FAILED,
						 "got stuck in dep loop");
			return;
		}
	} while (changes);

	/* check for conflicts */
	for (i = 0; i < plugin_loader->plugins->len; i++) {
		plugin = g_ptr_array_index (plugin_loader->plugins, i);
		if (!gs_plugin_get_enabled (plugin))
			continue;
		deps = gs_plugin_get_rules (plugin, GS_PLUGIN_RULE_CONFLICTS);
		for (j = 0; j < deps->len && !changes; j++) {
			plugin_name = g_ptr_array_index (deps, j);
			dep = gs_plugin_loader_find_plugin (plugin_loader,
							    plugin_name);
			if (dep == NULL)
				continue;
			if (!gs_plugin_get_enabled (dep))
				continue;
			g_debug ("disabling %s as conflicts with %s",
				 gs_plugin_get_name (dep),
				 gs_plugin_get_name (plugin));
			gs_plugin_set_enabled (dep, FALSE);
		}
	}

	/* sort by order */
	g_ptr_array_sort (plugin_loader->plugins,
			  gs_plugin_loader_plugin_sort_fn);

	/* assign priority values */
	do {
		changes = FALSE;
		for (i = 0; i < plugin_loader->plugins->len; i++) {
			plugin = g_ptr_array_index (plugin_loader->plugins, i);
			deps = gs_plugin_get_rules (plugin, GS_PLUGIN_RULE_BETTER_THAN);
			for (j = 0; j < deps->len && !changes; j++) {
				plugin_name = g_ptr_array_index (deps, j);
				dep = gs_plugin_loader_find_plugin (plugin_loader,
								    plugin_name);
				if (dep == NULL) {
					g_debug ("cannot find plugin '%s' "
						 "requested by '%s'",
						 plugin_name,
						 gs_plugin_get_name (plugin));
					continue;
				}
				if (!gs_plugin_get_enabled (dep))
					continue;
				if (gs_plugin_get_priority (plugin) <= gs_plugin_get_priority (dep)) {
					gs_plugin_set_priority (plugin, gs_plugin_get_priority (dep) + 1);
					changes = TRUE;
				}
			}
		}

		/* check we're not stuck */
		if (dep_loop_check++ > 100) {
			notify_setup_complete (plugin_loader);
			g_task_return_new_error (task,
						 GS_PLUGIN_ERROR,
						 GS_PLUGIN_ERROR_PLUGIN_DEPSOLVE_FAILED,
						 "got stuck in priority loop");
			return;
		}
	} while (changes);

	/* run setup */
	data->n_pending = 1;  /* incremented until all operations have been started */
#ifdef HAVE_SYSPROF
	data->plugins_begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	for (i = 0; i < plugin_loader->plugins->len; i++) {
		plugin = GS_PLUGIN (plugin_loader->plugins->pdata[i]);

		if (!gs_plugin_get_enabled (plugin))
			continue;

		if (GS_PLUGIN_GET_CLASS (plugin)->setup_async != NULL) {
			data->n_pending++;
			GS_PLUGIN_GET_CLASS (plugin)->setup_async (plugin, cancellable,
								   plugin_setup_cb, g_object_ref (task));
		}
	}

	finish_setup_op (task);
}

static void
plugin_setup_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;
#ifdef HAVE_SYSPROF
	SetupData *data = g_task_get_task_data (task);
#endif

	g_assert (GS_PLUGIN_GET_CLASS (plugin)->setup_finish != NULL);

	if (!GS_PLUGIN_GET_CLASS (plugin)->setup_finish (plugin, result, &local_error)) {
		g_debug ("disabling %s as setup failed: %s",
			 gs_plugin_get_name (plugin),
			 local_error->message);
		gs_plugin_set_enabled (plugin, FALSE);
	}

	GS_PROFILER_ADD_MARK (PluginLoader,
			      data->plugins_begin_time_nsec,
			      "setup-plugin", NULL);

	/* Indicate this plugin has finished setting up. */
	finish_setup_op (task);
}

static void
finish_setup_op (GTask *task)
{
	SetupData *data = g_task_get_task_data (task);
	GsPluginLoader *plugin_loader = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GsAppList) install_queue = NULL;
	g_autoptr(GError) local_error = NULL;

	g_assert (data->n_pending > 0);
	data->n_pending--;

	if (data->n_pending > 0)
		return;

	/* now we can load the install-queue */
	install_queue = load_install_queue (plugin_loader, &local_error);
	if (install_queue == NULL) {
		notify_setup_complete (plugin_loader);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Mark setup as complete as it’s now safe for other jobs to be
	 * processed. Indeed, the final step in setup is to refine the install
	 * queue apps, which requires @setup_complete to be %TRUE. */
	notify_setup_complete (plugin_loader);

	GS_PROFILER_ADD_MARK (PluginLoader, data->setup_begin_time_nsec, "setup", NULL);

	/* Refine the install queue. */
	if (gs_app_list_length (install_queue) > 0) {
		g_autoptr(GsPluginJob) refine_job = NULL;

		/* Require ID and Origin to get complete unique IDs */
		refine_job = gs_plugin_job_refine_new (install_queue, GS_PLUGIN_REFINE_FLAGS_REQUIRE_ID |
								      GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN |
								      GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
		gs_plugin_loader_job_process_async (plugin_loader, refine_job,
						    cancellable,
						    finish_setup_install_queue_cb,
						    g_object_ref (task));
	} else {
		g_task_return_boolean (task, TRUE);
	}
}

static void gs_plugin_loader_maybe_flush_pending_install_queue (GsPluginLoader *plugin_loader);

static void
finish_setup_install_queue_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GsAppList) new_list = NULL;
	g_autoptr(GError) local_error = NULL;

	new_list = gs_plugin_loader_job_process_finish (plugin_loader, result, &local_error);
	if (new_list == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
	} else {
		g_autoptr(GsAppList) old_pending_apps = NULL;
		gboolean has_pending_apps = FALSE;
		gboolean changed;
		g_mutex_lock (&plugin_loader->pending_apps_mutex);
		changed = plugin_loader->pending_apps != NULL;
		/* Merge the existing and newly-loaded lists, in case pending apps were added
		   while the install-queue file was being loaded */
		old_pending_apps = g_steal_pointer (&plugin_loader->pending_apps);
		if (old_pending_apps != NULL && gs_app_list_length (new_list) > 0) {
			g_autoptr(GHashTable) expected_unique_ids = g_hash_table_new (g_str_hash, g_str_equal);
			for (guint i = 0; i < gs_app_list_length (old_pending_apps); i++) {
				GsApp *app = gs_app_list_index (old_pending_apps, i);
				if (gs_app_get_unique_id (app) != NULL)
					g_hash_table_add (expected_unique_ids, (gpointer) gs_app_get_unique_id (app));
			}
			for (guint i = 0; i < gs_app_list_length (new_list); i++) {
				GsApp *app = gs_app_list_index (new_list, i);
				if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE &&
				    gs_app_get_unique_id (app) != NULL &&
				    g_hash_table_contains (expected_unique_ids, gs_app_get_unique_id (app))) {
					if (plugin_loader->pending_apps == NULL)
						plugin_loader->pending_apps = gs_app_list_new ();
					gs_app_set_state (app, GS_APP_STATE_QUEUED_FOR_INSTALL);
					gs_app_set_pending_action (app, GS_PLUGIN_ACTION_INSTALL);
					gs_app_list_add (plugin_loader->pending_apps, app);
				}
			}
			has_pending_apps = plugin_loader->pending_apps != NULL;
			changed = TRUE;
		}
		g_mutex_unlock (&plugin_loader->pending_apps_mutex);
		g_task_return_boolean (task, TRUE);

		if (changed)
			save_install_queue (plugin_loader);
		if (has_pending_apps)
			gs_plugin_loader_maybe_flush_pending_install_queue (plugin_loader);
	}
}

/**
 * gs_plugin_loader_setup_finish:
 * @plugin_loader: a #GsPluginLoader
 * @result: result of the asynchronous operation
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous setup operation started with
 * gs_plugin_loader_setup_async().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_plugin_loader_setup_finish (GsPluginLoader  *plugin_loader,
                               GAsyncResult    *result,
                               GError         **error)
{
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, plugin_loader), FALSE);
	g_return_val_if_fail (g_async_result_is_tagged (result, gs_plugin_loader_setup_async), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

void
gs_plugin_loader_dump_state (GsPluginLoader *plugin_loader)
{
	g_autoptr(GString) str_enabled = g_string_new (NULL);
	g_autoptr(GString) str_disabled = g_string_new (NULL);

	/* print what the priorities are if verbose */
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		GString *str = gs_plugin_get_enabled (plugin) ? str_enabled : str_disabled;
		g_string_append_printf (str, "%s, ", gs_plugin_get_name (plugin));
		g_debug ("[%s]\t%u\t->\t%s",
			 gs_plugin_get_enabled (plugin) ? "enabled" : "disabld",
			 gs_plugin_get_order (plugin),
			 gs_plugin_get_name (plugin));
	}
	if (str_enabled->len > 2)
		g_string_truncate (str_enabled, str_enabled->len - 2);
	if (str_disabled->len > 2)
		g_string_truncate (str_disabled, str_disabled->len - 2);
	g_info ("enabled plugins: %s", str_enabled->str);
	g_info ("disabled plugins: %s", str_disabled->str);
}

static void
gs_plugin_loader_get_property (GObject *object, guint prop_id,
			       GValue *value, GParamSpec *pspec)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (object);

	switch ((GsPluginLoaderProperty) prop_id) {
	case PROP_EVENTS:
		g_value_set_pointer (value, plugin_loader->events_by_id);
		break;
	case PROP_ALLOW_UPDATES:
		g_value_set_boolean (value, gs_plugin_loader_get_allow_updates (plugin_loader));
		break;
	case PROP_NETWORK_AVAILABLE:
		g_value_set_boolean (value, gs_plugin_loader_get_network_available (plugin_loader));
		break;
	case PROP_NETWORK_METERED:
		g_value_set_boolean (value, gs_plugin_loader_get_network_metered (plugin_loader));
		break;
	case PROP_SESSION_BUS_CONNECTION:
		g_value_set_object (value, plugin_loader->session_bus_connection);
		break;
	case PROP_SYSTEM_BUS_CONNECTION:
		g_value_set_object (value, plugin_loader->system_bus_connection);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_loader_set_property (GObject *object, guint prop_id,
			       const GValue *value, GParamSpec *pspec)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (object);

	switch ((GsPluginLoaderProperty) prop_id) {
	case PROP_EVENTS:
	case PROP_ALLOW_UPDATES:
	case PROP_NETWORK_AVAILABLE:
	case PROP_NETWORK_METERED:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_SESSION_BUS_CONNECTION:
		/* Construct only */
		g_assert (plugin_loader->session_bus_connection == NULL);
		plugin_loader->session_bus_connection = g_value_dup_object (value);
		break;
	case PROP_SYSTEM_BUS_CONNECTION:
		/* Construct only */
		g_assert (plugin_loader->system_bus_connection == NULL);
		plugin_loader->system_bus_connection = g_value_dup_object (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_loader_dispose (GObject *object)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (object);

	g_cancellable_cancel (plugin_loader->pending_apps_cancellable);

	if (plugin_loader->plugins != NULL) {
		/* Shut down all the plugins first. */
		gs_plugin_loader_shutdown (plugin_loader, NULL);

		g_clear_pointer (&plugin_loader->plugins, g_ptr_array_unref);
	}
	if (plugin_loader->updates_changed_id != 0) {
		g_source_remove (plugin_loader->updates_changed_id);
		plugin_loader->updates_changed_id = 0;
	}
	if (plugin_loader->network_changed_handler != 0) {
		g_signal_handler_disconnect (plugin_loader->network_monitor,
					     plugin_loader->network_changed_handler);
		plugin_loader->network_changed_handler = 0;
	}
	if (plugin_loader->network_available_notify_handler != 0) {
		g_signal_handler_disconnect (plugin_loader->network_monitor,
					     plugin_loader->network_available_notify_handler);
		plugin_loader->network_available_notify_handler = 0;
	}
	if (plugin_loader->network_metered_notify_handler != 0) {
		g_signal_handler_disconnect (plugin_loader->network_monitor,
					     plugin_loader->network_metered_notify_handler);
		plugin_loader->network_metered_notify_handler = 0;
	}
	if (plugin_loader->queued_ops_pool != NULL) {
		/* stop accepting more requests and wait until any currently
		 * running ones are finished */
		g_thread_pool_free (plugin_loader->queued_ops_pool, TRUE, TRUE);
		plugin_loader->queued_ops_pool = NULL;
	}
	g_clear_object (&plugin_loader->network_monitor);
	g_clear_object (&plugin_loader->power_profile_monitor);
	g_clear_object (&plugin_loader->settings);
	g_clear_object (&plugin_loader->pending_apps);
	g_clear_object (&plugin_loader->job_manager);
	g_clear_object (&plugin_loader->category_manager);
	g_clear_object (&plugin_loader->odrs_provider);
	g_clear_object (&plugin_loader->setup_complete_cancellable);
	g_clear_object (&plugin_loader->pending_apps_cancellable);

	g_clear_object (&plugin_loader->session_bus_connection);
	g_clear_object (&plugin_loader->system_bus_connection);

	G_OBJECT_CLASS (gs_plugin_loader_parent_class)->dispose (object);
}

static void
gs_plugin_loader_finalize (GObject *object)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (object);

	g_thread_pool_free (plugin_loader->old_api_thread_pool, TRUE, FALSE);
	plugin_loader->old_api_thread_pool = NULL;

	g_strfreev (plugin_loader->compatible_projects);
	g_ptr_array_unref (plugin_loader->locations);
	g_free (plugin_loader->language);
	g_ptr_array_unref (plugin_loader->file_monitors);
	g_hash_table_unref (plugin_loader->events_by_id);
	g_hash_table_unref (plugin_loader->disallow_updates);

	g_mutex_clear (&plugin_loader->pending_apps_mutex);
	g_mutex_clear (&plugin_loader->events_by_id_mutex);

	G_OBJECT_CLASS (gs_plugin_loader_parent_class)->finalize (object);
}

static void
gs_plugin_loader_class_init (GsPluginLoaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_plugin_loader_get_property;
	object_class->set_property = gs_plugin_loader_set_property;
	object_class->dispose = gs_plugin_loader_dispose;
	object_class->finalize = gs_plugin_loader_finalize;

	/**
	 * GsPluginLoader:events:
	 *
	 * Events added on the plugin loader using gs_plugin_loader_add_event().
	 */
	obj_props[PROP_EVENTS] =
		g_param_spec_string ("events", NULL, NULL,
				     NULL,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginLoader:allow-updates:
	 *
	 * Whether updates and upgrades are managed by lingmo-appstore.
	 *
	 * If not, the updates UI should be hidden and no automatic updates
	 * performed.
	 */
	obj_props[PROP_ALLOW_UPDATES] =
		g_param_spec_boolean ("allow-updates", NULL, NULL,
				      TRUE,
				      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginLoader:network-available:
	 *
	 * Whether the network is considered available.
	 *
	 * This has the same semantics as #GNetworkMonitor:network-available.
	 */
	obj_props[PROP_NETWORK_AVAILABLE] =
		g_param_spec_boolean ("network-available", NULL, NULL,
				      FALSE,
				      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginLoader:network-metered:
	 *
	 * Whether the network is considered metered.
	 *
	 * This has the same semantics as #GNetworkMonitor:network-metered.
	 */
	obj_props[PROP_NETWORK_METERED] =
		g_param_spec_boolean ("network-metered", NULL, NULL,
				      FALSE,
				      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginLoader:session-bus-connection: (nullable)
	 *
	 * A connection to the D-Bus session bus.
	 *
	 * This may be %NULL at construction time. If so, the default session
	 * bus connection will be used (and returned as the value of this
	 * property) after gs_plugin_loader_setup_async() is called.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SESSION_BUS_CONNECTION] =
		g_param_spec_object ("session-bus-connection", NULL, NULL,
				     G_TYPE_DBUS_CONNECTION,
				     G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginLoader:system-bus-connection: (not nullable)
	 *
	 * A connection to the D-Bus system bus.
	 *
	 * This may be %NULL at construction time. If so, the default system
	 * bus connection will be used (and returned as the value of this
	 * property) after gs_plugin_loader_setup_async() is called.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SYSTEM_BUS_CONNECTION] =
		g_param_spec_object ("system-bus-connection", NULL, NULL,
				     G_TYPE_DBUS_CONNECTION,
				     G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	signals [SIGNAL_STATUS_CHANGED] =
		g_signal_new ("status-changed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_generic,
			      G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_UINT);
	signals [SIGNAL_PENDING_APPS_CHANGED] =
		g_signal_new ("pending-apps-changed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	signals [SIGNAL_UPDATES_CHANGED] =
		g_signal_new ("updates-changed",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	signals [SIGNAL_RELOAD] =
		g_signal_new ("reload",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	signals [SIGNAL_BASIC_AUTH_START] =
		g_signal_new ("basic-auth-start",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_generic,
			      G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
	signals [SIGNAL_ASK_UNTRUSTED] =
		g_signal_new ("ask-untrusted",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_generic,
			      G_TYPE_BOOLEAN, 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
}

static void
gs_plugin_loader_allow_updates_recheck (GsPluginLoader *plugin_loader)
{
	gboolean changed;

	if (g_settings_get_boolean (plugin_loader->settings, "allow-updates")) {
		changed = g_hash_table_remove (plugin_loader->disallow_updates, plugin_loader);
	} else {
		changed = g_hash_table_insert (plugin_loader->disallow_updates,
					       (gpointer) plugin_loader,
					       (gpointer) "GSettings");
	}

	if (changed)
		g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_ALLOW_UPDATES]);
}

static void
gs_plugin_loader_settings_changed_cb (GSettings *settings,
				      const gchar *key,
				      GsPluginLoader *plugin_loader)
{
	if (g_strcmp0 (key, "allow-updates") == 0)
		gs_plugin_loader_allow_updates_recheck (plugin_loader);
}

static gint
get_max_parallel_ops (void)
{
	guint mem_total = gs_utils_get_memory_total ();
	if (mem_total == 0)
		return 8;
	/* allow 1 op per GB of memory */
	return (gint) MAX (round((gdouble) mem_total / 1024), 1.0);
}

static void
gs_plugin_loader_init (GsPluginLoader *plugin_loader)
{
	const gchar *tmp;
	gchar *match;
	gchar **projects;
	guint i;
	g_autofree gchar *review_server = NULL;
	g_autofree gchar *user_hash = NULL;
	g_autoptr(GError) local_error = NULL;
	const guint64 odrs_review_max_cache_age_secs = 237000;  /* 1 week */
	const guint odrs_review_n_results_max = 20;
	const gchar *locale;

	plugin_loader->setup_complete_cancellable = g_cancellable_new ();
	plugin_loader->scale = 1;
	plugin_loader->plugins = g_ptr_array_new_with_free_func (g_object_unref);
	plugin_loader->pending_apps = NULL;
	plugin_loader->queued_ops_pool = g_thread_pool_new (gs_plugin_loader_process_in_thread_pool_cb,
						   plugin_loader,
						   get_max_parallel_ops (),
						   FALSE,
						   NULL);
	plugin_loader->file_monitors = g_ptr_array_new_with_free_func (g_object_unref);
	plugin_loader->locations = g_ptr_array_new_with_free_func (g_free);
	plugin_loader->settings = g_settings_new ("org.lingmo.appstore");
	g_signal_connect (plugin_loader->settings, "changed",
			  G_CALLBACK (gs_plugin_loader_settings_changed_cb), plugin_loader);
	plugin_loader->events_by_id = g_hash_table_new_full ((GHashFunc) as_utils_data_id_hash,
							     (GEqualFunc) as_utils_data_id_equal,
							     g_free,
							     (GDestroyNotify) g_object_unref);

	/* Set up a thread pool for running old-style jobs
	 * FIXME: This will eventually disappear when all jobs are ported to
	 * be subclasses of #GsPluginJob. */
	plugin_loader->old_api_thread_pool = g_thread_pool_new_full (gs_plugin_loader_process_old_api_job_cb,
								     plugin_loader,
								     (GDestroyNotify) g_object_unref,
								     20,
								     FALSE,
								     NULL);

	/* get the job manager */
	plugin_loader->job_manager = gs_job_manager_new ();

	/* get the category manager */
	plugin_loader->category_manager = gs_category_manager_new ();

	/* set up the ODRS provider */

	/* get the machine+user ID hash value */
	user_hash = gs_utils_get_user_hash (&local_error);
	if (user_hash == NULL) {
		g_warning ("Failed to get machine+user hash: %s", local_error->message);
		plugin_loader->odrs_provider = NULL;
	} else {
		review_server = g_settings_get_string (plugin_loader->settings, "review-server");

		if (review_server != NULL && *review_server != '\0') {
			const gchar *distro = NULL;
			g_autoptr(GsOsRelease) os_release = NULL;
			g_autoptr(SoupSession) odrs_soup_session = NULL;

			/* get the distro name (e.g. 'Fedora') but allow a fallback */
			os_release = gs_os_release_new (&local_error);
			if (os_release != NULL) {
				distro = gs_os_release_get_name (os_release);
				if (distro == NULL)
					g_warning ("no distro name specified");
			} else {
				g_warning ("failed to get distro name: %s", local_error->message);
			}

			/* Fallback */
			if (distro == NULL)
				distro = C_("Distribution name", "Unknown");

			odrs_soup_session = gs_build_soup_session ();
			plugin_loader->odrs_provider = gs_odrs_provider_new (review_server,
									     user_hash,
									     distro,
									     odrs_review_max_cache_age_secs,
									     odrs_review_n_results_max,
									     odrs_soup_session);
		}
	}

	/* the settings key sets the initial override */
	plugin_loader->disallow_updates = g_hash_table_new (g_direct_hash, g_direct_equal);
	gs_plugin_loader_allow_updates_recheck (plugin_loader);

	/* get the language from the locale (i.e. strip the territory, codeset
	 * and modifier) */
	locale = setlocale (LC_MESSAGES, NULL);
	plugin_loader->language = g_strdup (locale);
	match = strpbrk (plugin_loader->language, "._@");
	if (match != NULL)
		*match = '\0';

	g_debug ("Using locale = %s, language = %s", locale, plugin_loader->language);

	g_mutex_init (&plugin_loader->pending_apps_mutex);
	g_mutex_init (&plugin_loader->events_by_id_mutex);

	/* monitor the network as the many UI operations need the network */
	gs_plugin_loader_monitor_network (plugin_loader);

	plugin_loader->power_profile_monitor = g_power_profile_monitor_dup_default ();

	/* by default we only show project-less apps or compatible projects */
	tmp = g_getenv ("LINGMO_SOFTWARE_COMPATIBLE_PROJECTS");
	if (tmp == NULL) {
		projects = g_settings_get_strv (plugin_loader->settings,
						"compatible-projects");
	} else {
		projects = g_strsplit (tmp, ",", -1);
	}
	for (i = 0; projects[i] != NULL; i++)
		g_debug ("compatible-project: %s", projects[i]);
	plugin_loader->compatible_projects = projects;
}

/**
 * gs_plugin_loader_new:
 * @session_bus_connection: (nullable) (transfer none): a D-Bus session bus
 *   connection to use, or %NULL to use the default
 * @system_bus_connection: (nullable) (transfer none): a D-Bus system bus
 *   connection to use, or %NULL to use the default
 *
 * Create a new #GsPluginLoader.
 *
 * The D-Bus connection arguments should typically be %NULL, and only be
 * non-%NULL when doing unit tests.
 *
 * Return value: (transfer full) (not nullable): a new #GsPluginLoader
 * Since: 43
 **/
GsPluginLoader *
gs_plugin_loader_new (GDBusConnection *session_bus_connection,
                      GDBusConnection *system_bus_connection)
{
	g_return_val_if_fail (session_bus_connection == NULL || G_IS_DBUS_CONNECTION (session_bus_connection), NULL);
	g_return_val_if_fail (system_bus_connection == NULL || G_IS_DBUS_CONNECTION (system_bus_connection), NULL);

	return g_object_new (GS_TYPE_PLUGIN_LOADER,
			     "session-bus-connection", session_bus_connection,
			     "system-bus-connection", system_bus_connection,
			     NULL);
}

static void
gs_plugin_loader_app_installed_cb (GObject *source,
				   GAsyncResult *res,
				   gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = GS_APP (user_data);

	ret = gs_plugin_loader_job_action_finish (plugin_loader,
						   res,
						   &error);
	remove_app_from_install_queue (plugin_loader, app);
	if (!ret) {
		gs_app_set_state_recover (app);
		g_warning ("failed to install %s: %s",
			   gs_app_get_unique_id (app), error->message);
	}
}

gboolean
gs_plugin_loader_get_network_available (GsPluginLoader *plugin_loader)
{
	if (plugin_loader->network_monitor == NULL) {
		g_debug ("no network monitor, so returning network-available=TRUE");
		return TRUE;
	}
	return g_network_monitor_get_network_available (plugin_loader->network_monitor);
}

gboolean
gs_plugin_loader_get_network_metered (GsPluginLoader *plugin_loader)
{
	if (plugin_loader->network_monitor == NULL) {
		g_debug ("no network monitor, so returning network-metered=FALSE");
		return FALSE;
	}
	return g_network_monitor_get_network_metered (plugin_loader->network_monitor);
}

gboolean
gs_plugin_loader_get_power_saver (GsPluginLoader *plugin_loader)
{
	return plugin_loader->power_profile_monitor != NULL &&
	       g_power_profile_monitor_get_power_saver_enabled (plugin_loader->power_profile_monitor);
}

gboolean
gs_plugin_loader_get_game_mode (GsPluginLoader *plugin_loader)
{
	g_autoptr(GDBusProxy) proxy = NULL;
	g_autoptr(GVariant) val = NULL;

	/* This supports https://github.com/FeralInteractive/gamemode ;
	   it's okay when it's not installed, nor running. */
	proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
					       G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
#if GLIB_CHECK_VERSION(2, 72, 0)
					       G_DBUS_PROXY_FLAGS_NO_MATCH_RULE |
#endif
					       G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
					       NULL,
					       "com.feralinteractive.GameMode",
					       "/com/feralinteractive/GameMode",
					       "com.feralinteractive.GameMode",
					       NULL,
					       NULL);
	if (proxy == NULL)
		return FALSE;

	val = g_dbus_proxy_get_cached_property (proxy, "ClientCount");
	if (val != NULL)
		return g_variant_get_int32 (val) > 0;

	return FALSE;
}

static void
gs_plugin_loader_pending_apps_refined_cb (GObject      *source,
                                          GAsyncResult *res,
                                          gpointer      user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	g_autoptr(GsAppList) old_queue = GS_APP_LIST (user_data);
	g_autoptr(GsAppList) refined_queue = NULL;
	g_autoptr(GError) error = NULL;

	refined_queue = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);

	if (refined_queue == NULL) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
			g_debug ("failed to refine pending apps: %s", error->message);

			g_mutex_lock (&plugin_loader->pending_apps_mutex);
			g_clear_object (&plugin_loader->pending_apps);
			g_mutex_unlock (&plugin_loader->pending_apps_mutex);

			save_install_queue (plugin_loader);
		}
		return;
	}

	for (guint i = 0; i < gs_app_list_length (old_queue); i++) {
		GsApp *app = gs_app_list_index (old_queue, i);

		if (gs_app_get_unique_id (app) == NULL ||
		    gs_app_list_lookup (refined_queue, gs_app_get_unique_id (app)) == NULL)
			remove_app_from_install_queue (plugin_loader, app);
	}

	for (guint i = 0; i < gs_app_list_length (refined_queue); i++) {
		GsApp *app = gs_app_list_index (refined_queue, i);
		g_autoptr(GsPluginJob) plugin_job = NULL;

		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_REPOSITORY) {
			plugin_job = gs_plugin_job_manage_repository_new (app,
									  GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INTERACTIVE |
									  GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL);
		} else {
			/* The 'interactive' is needed for credentials prompt, otherwise it just fails */
			plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_INSTALL,
							 "app", app,
							 "interactive", TRUE,
							 NULL);
		}

		gs_plugin_loader_job_process_async (plugin_loader, plugin_job,
						    gs_app_get_cancellable (app),
						    gs_plugin_loader_app_installed_cb,
						    g_object_ref (app));
	}

	g_clear_object (&plugin_loader->pending_apps_cancellable);
}

static void
gs_plugin_loader_maybe_flush_pending_install_queue (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppList) obsolete = NULL;
	g_autoptr(GsAppList) queue = NULL;

	if (!gs_plugin_loader_get_network_available (plugin_loader) ||
	    gs_plugin_loader_get_network_metered (plugin_loader)) {
		/* Print the debug message only when had anything to skip */
		g_mutex_lock (&plugin_loader->pending_apps_mutex);
		if (plugin_loader->pending_apps != NULL) {
			g_debug ("Cannot flush pending install queue, because is %sonline and is %smetered",
				 !gs_plugin_loader_get_network_available (plugin_loader) ? "not " : "",
				 gs_plugin_loader_get_network_metered (plugin_loader) ? "" : "not ");
		}
		g_mutex_unlock (&plugin_loader->pending_apps_mutex);
		return;
	}

	/* Already flushing pending queue */
	if (plugin_loader->pending_apps_cancellable)
		return;

	queue = gs_app_list_new ();
	obsolete = gs_app_list_new ();
	g_mutex_lock (&plugin_loader->pending_apps_mutex);
	for (guint i = 0; plugin_loader->pending_apps != NULL && i < gs_app_list_length (plugin_loader->pending_apps); i++) {
		GsApp *app = gs_app_list_index (plugin_loader->pending_apps, i);
		if (gs_app_get_state (app) == GS_APP_STATE_QUEUED_FOR_INSTALL) {
			gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
			gs_app_list_add (queue, app);
		} else {
			gs_app_list_add (obsolete, app);
		}
	}
	g_mutex_unlock (&plugin_loader->pending_apps_mutex);
	for (guint i = 0; i < gs_app_list_length (obsolete); i++) {
		GsApp *app = gs_app_list_index (obsolete, i);
		remove_app_from_install_queue (plugin_loader, app);
	}

	plugin_loader->pending_apps_cancellable = g_cancellable_new ();

	plugin_job = gs_plugin_job_refine_new (queue, GS_PLUGIN_REFINE_FLAGS_NONE);
	gs_plugin_loader_job_process_async (plugin_loader, plugin_job,
					    plugin_loader->pending_apps_cancellable,
					    gs_plugin_loader_pending_apps_refined_cb,
					    g_steal_pointer (&queue));
}

static void
gs_plugin_loader_network_changed_cb (GNetworkMonitor *monitor,
				     gboolean available,
				     GsPluginLoader *plugin_loader)
{
	gboolean metered = g_network_monitor_get_network_metered (plugin_loader->network_monitor);

	g_debug ("network status change: %s [%s]",
		 available ? "online" : "offline",
		 metered ? "metered" : "unmetered");

	g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_NETWORK_AVAILABLE]);
	g_object_notify_by_pspec (G_OBJECT (plugin_loader), obj_props[PROP_NETWORK_METERED]);

	gs_plugin_loader_maybe_flush_pending_install_queue (plugin_loader);
}

static void
gs_plugin_loader_network_available_notify_cb (GObject    *obj,
					      GParamSpec *pspec,
					      gpointer    user_data)
{
	GNetworkMonitor *monitor = G_NETWORK_MONITOR (obj);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);

	gs_plugin_loader_network_changed_cb (monitor, g_network_monitor_get_network_available (monitor), plugin_loader);
}

static void
gs_plugin_loader_network_metered_notify_cb (GObject    *obj,
					    GParamSpec *pspec,
					    gpointer    user_data)
{
	GNetworkMonitor *monitor = G_NETWORK_MONITOR (obj);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);

	gs_plugin_loader_network_changed_cb (monitor, g_network_monitor_get_network_available (monitor), plugin_loader);
}

static void
gs_plugin_loader_monitor_network (GsPluginLoader *plugin_loader)
{
	GNetworkMonitor *network_monitor;

	network_monitor = g_network_monitor_get_default ();
	if (network_monitor == NULL || plugin_loader->network_changed_handler != 0)
		return;
	plugin_loader->network_monitor = g_object_ref (network_monitor);

	plugin_loader->network_changed_handler =
		g_signal_connect (plugin_loader->network_monitor, "network-changed",
				  G_CALLBACK (gs_plugin_loader_network_changed_cb), plugin_loader);
	plugin_loader->network_available_notify_handler =
		g_signal_connect (plugin_loader->network_monitor, "notify::network-available",
				  G_CALLBACK (gs_plugin_loader_network_available_notify_cb), plugin_loader);
	plugin_loader->network_metered_notify_handler =
		g_signal_connect (plugin_loader->network_monitor, "notify::network-metered",
				  G_CALLBACK (gs_plugin_loader_network_metered_notify_cb), plugin_loader);

	gs_plugin_loader_network_changed_cb (plugin_loader->network_monitor,
			    g_network_monitor_get_network_available (plugin_loader->network_monitor),
			    plugin_loader);
}

/******************************************************************************/

static void
gs_plugin_loader_inherit_list_props (GsAppList *des_list,
				     GsAppList *src_list)
{
	if (gs_app_list_has_flag (src_list, GS_APP_LIST_FLAG_IS_TRUNCATED))
		gs_app_list_add_flag (des_list, GS_APP_LIST_FLAG_IS_TRUNCATED);

	gs_app_list_set_size_peak (des_list, gs_app_list_get_size_peak (src_list));
}

static void
gs_plugin_loader_process_old_api_job_cb (gpointer task_data,
                                         gpointer user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&task_data);
	GError *error = NULL;
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsPluginLoaderHelper *helper = (GsPluginLoaderHelper *) g_task_get_task_data (task);
	GsAppListFilterFlags dedupe_flags;
	g_autoptr(GsAppList) list = g_object_ref (gs_plugin_job_get_list (helper->plugin_job));
	GsPluginAction action = gs_plugin_job_get_action (helper->plugin_job);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);
	gboolean add_to_pending_array = FALSE;
	g_autoptr(GMainContext) context = g_main_context_new ();
	g_autoptr(GMainContextPusher) pusher = g_main_context_pusher_new (context);
	g_autofree gchar *sysprof_name = NULL;
	g_autofree gchar *sysprof_message = NULL;
	g_autofree gchar *job_debug = NULL;

	sysprof_name = g_strconcat ("process-thread:", gs_plugin_action_to_string (action), NULL);
	sysprof_message = gs_plugin_job_to_string (helper->plugin_job);

	GS_PROFILER_BEGIN_SCOPED (PluginLoader, sysprof_name, sysprof_message);

	/* these change the pending count on the installed panel */
	switch (action) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_REMOVE:
		add_to_pending_array = TRUE;
		break;
	default:
		break;
	}

	/* add to pending list */
	if (add_to_pending_array)
		gs_plugin_loader_pending_apps_add (plugin_loader, helper);

	/* run each plugin */
	if (!GS_IS_PLUGIN_JOB_REFINE (helper->plugin_job)) {
		if (!gs_plugin_loader_run_results (helper, cancellable, &error)) {
			if (add_to_pending_array) {
				gs_app_set_state_recover (gs_plugin_job_get_app (helper->plugin_job));
				gs_plugin_loader_pending_apps_remove (plugin_loader, helper);
			}
			gs_utils_error_convert_gio (&error);
			g_task_return_error (task, error);
			gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
			return;
		}

		if (action == GS_PLUGIN_ACTION_URL_TO_APP) {
			const gchar *search = gs_plugin_job_get_search (helper->plugin_job);
			if (search && g_ascii_strncasecmp (search, "file://", 7) == 0 && (
			    gs_plugin_job_get_list (helper->plugin_job) == NULL ||
			    gs_app_list_length (gs_plugin_job_get_list (helper->plugin_job)) == 0)) {
				g_autoptr(GError) local_error = NULL;
				g_autoptr(GFile) file = NULL;
				file = g_file_new_for_uri (search);
				gs_plugin_job_set_action (helper->plugin_job, GS_PLUGIN_ACTION_FILE_TO_APP);
				gs_plugin_job_set_file (helper->plugin_job, file);
				helper->function_name = gs_plugin_action_to_function_name (GS_PLUGIN_ACTION_FILE_TO_APP);
				if (gs_plugin_loader_run_results (helper, cancellable, &local_error)) {
					for (guint j = 0; j < gs_app_list_length (list); j++) {
						GsApp *app = gs_app_list_index (list, j);
						if (gs_app_get_local_file (app) == NULL)
							gs_app_set_local_file (app, gs_plugin_job_get_file (helper->plugin_job));
					}
				} else {
					g_debug ("Failed to convert file:// URI to app using file-to-app action: %s", local_error->message);
				}
				gs_plugin_job_set_action (helper->plugin_job, GS_PLUGIN_ACTION_URL_TO_APP);
				gs_plugin_job_set_file (helper->plugin_job, NULL);
			}
		}
	}

	/* remove from pending list */
	if (add_to_pending_array) {
		GsApp *app = gs_plugin_job_get_app (helper->plugin_job);
		/* The plugin can left the app queued for install when there is no network available,
		   in which case the app cannot be removed from the install queue. */
		if (action != GS_PLUGIN_ACTION_INSTALL ||
		    gs_app_get_state (app) != GS_APP_STATE_QUEUED_FOR_INSTALL) {
			g_autoptr(GsAppList) addons = NULL;

			gs_plugin_loader_pending_apps_remove (plugin_loader, helper);

			/* unstage addons */
			addons = gs_app_dup_addons (gs_plugin_job_get_app (helper->plugin_job));
			for (guint i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
				GsApp *addon = gs_app_list_index (addons, i);
				if (gs_app_get_to_be_installed (addon))
					gs_app_set_to_be_installed (addon, FALSE);
			}
		}
	}

	/* some functions are really required for proper operation */
	switch (action) {
	case GS_PLUGIN_ACTION_GET_UPDATES:
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_LAUNCH:
	case GS_PLUGIN_ACTION_REMOVE:
		if (!helper->anything_ran) {
			g_set_error (&error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no plugin could handle %s",
				     gs_plugin_action_to_string (action));
			g_task_return_error (task, error);
			gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
			return;
		}
		break;
	default:
		if (!helper->anything_ran && !GS_IS_PLUGIN_JOB_REFINE (helper->plugin_job)) {
			g_debug ("no plugin could handle %s",
				 gs_plugin_action_to_string (action));
		}
		break;
	}

	/* filter to reduce to a sane set */
	gs_plugin_loader_job_sorted_truncation (helper->plugin_job, list);

	/* set the local file on any of the returned results */
	switch (action) {
	case GS_PLUGIN_ACTION_FILE_TO_APP:
		for (guint j = 0; j < gs_app_list_length (list); j++) {
			GsApp *app = gs_app_list_index (list, j);
			if (gs_app_get_local_file (app) == NULL)
				gs_app_set_local_file (app, gs_plugin_job_get_file (helper->plugin_job));
		}
	default:
		break;
	}

	/* pick up new source id */
	switch (action) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_REMOVE:
		gs_plugin_job_add_refine_flags (helper->plugin_job,
		                                GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN |
		                                GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION);
		break;
	default:
		break;
	}

	/* run refine() on each one if required */
	if (gs_plugin_job_get_refine_flags (helper->plugin_job) != 0 &&
	    list != NULL &&
	    gs_app_list_length (list) > 0) {
		g_autoptr(GsPluginJob) refine_job = NULL;
		g_autoptr(GAsyncResult) refine_result = NULL;
		g_autoptr(GsAppList) new_list = NULL;

		refine_job = gs_plugin_job_refine_new (list, gs_plugin_job_get_refine_flags (helper->plugin_job) | GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
		gs_plugin_loader_job_process_async (plugin_loader, refine_job,
						    cancellable,
						    async_result_cb,
						    &refine_result);

		/* FIXME: Make this sync until the enclosing function is
		 * refactored to be async. */
		while (refine_result == NULL)
			g_main_context_iteration (g_main_context_get_thread_default (), TRUE);

		new_list = gs_plugin_loader_job_process_finish (plugin_loader, refine_result, &error);
		if (new_list == NULL) {
			gs_utils_error_convert_gio (&error);
			g_task_return_error (task, g_steal_pointer (&error));
			gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
			return;
		}

		gs_plugin_loader_inherit_list_props (new_list, list);

		/* Update the app list in case the refine resolved any wildcards. */
		g_set_object (&list, new_list);
	} else {
		g_debug ("no refine flags set for transaction");
	}

	/* check the local files have an icon set */
	switch (action) {
	case GS_PLUGIN_ACTION_URL_TO_APP:
	case GS_PLUGIN_ACTION_FILE_TO_APP: {
		g_autoptr(GsPluginJob) refine_job = NULL;
		g_autoptr(GAsyncResult) refine_result = NULL;
		g_autoptr(GsAppList) new_list = NULL;

		for (guint j = 0; j < gs_app_list_length (list); j++) {
			GsApp *app = gs_app_list_index (list, j);
			if (!gs_app_has_icons (app)) {
				g_autoptr(GIcon) ic = NULL;
				const gchar *icon_name;
				if (gs_app_has_quirk (app, GS_APP_QUIRK_HAS_SOURCE))
					icon_name = "x-package-repository";
				else
					icon_name = "system-component-application";
				ic = g_themed_icon_new (icon_name);
				gs_app_add_icon (app, ic);
			}
		}

		refine_job = gs_plugin_job_refine_new (list, GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON | GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
		gs_plugin_loader_job_process_async (plugin_loader, refine_job,
						    cancellable,
						    async_result_cb,
						    &refine_result);

		/* FIXME: Make this sync until the enclosing function is
		 * refactored to be async. */
		while (refine_result == NULL)
			g_main_context_iteration (g_main_context_get_thread_default (), TRUE);

		new_list = gs_plugin_loader_job_process_finish (plugin_loader, refine_result, &error);
		if (new_list == NULL) {
			gs_utils_error_convert_gio (&error);
			g_task_return_error (task, g_steal_pointer (&error));
			gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
			return;
		}

		gs_plugin_loader_inherit_list_props (new_list, list);

		/* Update the app list in case the refine resolved any wildcards. */
		g_set_object (&list, new_list);

		break;
	}
	default:
		break;
	}

	/* filter package list */
	switch (action) {
	case GS_PLUGIN_ACTION_URL_TO_APP:
		gs_app_list_filter (list, gs_plugin_loader_app_is_valid_filter, helper);
		break;
	case GS_PLUGIN_ACTION_GET_UPDATES:
		gs_app_list_filter (list, gs_plugin_loader_app_is_valid_updatable, helper);
		break;
	default:
		break;
	}

	/* only allow one result */
	if (action == GS_PLUGIN_ACTION_URL_TO_APP ||
	    action == GS_PLUGIN_ACTION_FILE_TO_APP) {
		if (gs_app_list_length (list) == 0) {
			g_autofree gchar *str = gs_plugin_job_to_string (helper->plugin_job);
			g_autoptr(GError) error_local = NULL;
			g_set_error (&error_local,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no application was created for %s", str);
			if (!gs_plugin_job_get_propagate_error (helper->plugin_job))
				gs_plugin_loader_claim_job_error (plugin_loader, NULL, helper->plugin_job, error_local);
			g_task_return_error (task, g_steal_pointer (&error_local));
			gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
			return;
		}
		if (gs_app_list_length (list) > 1) {
			g_autofree gchar *str = gs_plugin_job_to_string (helper->plugin_job);
			g_debug ("more than one application was created for %s", str);
		}
	}

	/* filter duplicates with priority, taking into account the source name
	 * & version, so we combine available updates with the installed app */
	dedupe_flags = gs_plugin_job_get_dedupe_flags (helper->plugin_job);
	if (dedupe_flags != GS_APP_LIST_FILTER_FLAG_NONE)
		gs_app_list_filter_duplicates (list, dedupe_flags);

	GS_PROFILER_END_SCOPED (PluginLoader);

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (helper->plugin_job);
	g_debug ("%s", job_debug);

	/* success */
	g_task_return_pointer (task, g_object_ref (list), (GDestroyNotify) g_object_unref);
	gs_job_manager_remove_job (plugin_loader->job_manager, helper->plugin_job);
}

static void
gs_plugin_loader_process_in_thread_pool_cb (gpointer data,
					    gpointer user_data)
{
	GTask *task = data;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);
	GsPluginLoaderHelper *helper = g_task_get_task_data (task);
	GsApp *app = gs_plugin_job_get_app (helper->plugin_job);
	GsPluginAction action = gs_plugin_job_get_action (helper->plugin_job);

	gs_ioprio_set (G_PRIORITY_LOW);

	gs_plugin_loader_process_old_api_job_cb (g_object_ref (task), plugin_loader);

	/* Clear any pending action set in gs_plugin_loader_schedule_task() */
	if (app != NULL && gs_app_get_pending_action (app) == action)
		gs_app_set_pending_action (app, GS_PLUGIN_ACTION_UNKNOWN);

	g_object_unref (task);
}

static void
gs_plugin_loader_cancelled_cb (GCancellable *cancellable,
                               gpointer      user_data)
{
	GCancellable *child_cancellable = G_CANCELLABLE (user_data);

	/* just proxy this forward */
	g_debug ("Cancelling job with cancellable %p", child_cancellable);
	g_cancellable_cancel (child_cancellable);
}

static void
gs_plugin_loader_schedule_task (GsPluginLoader *plugin_loader,
				GTask *task)
{
	GsPluginLoaderHelper *helper = g_task_get_task_data (task);
	GsApp *app = gs_plugin_job_get_app (helper->plugin_job);

	if (app != NULL) {
		/* set the pending-action to the app */
		GsPluginAction action = gs_plugin_job_get_action (helper->plugin_job);
		gs_app_set_pending_action (app, action);

		if (action == GS_PLUGIN_ACTION_INSTALL &&
		    gs_app_get_state (app) != GS_APP_STATE_AVAILABLE_LOCAL)
			add_app_to_install_queue (plugin_loader, app);
	}
	g_thread_pool_push (plugin_loader->queued_ops_pool, g_object_ref (task), NULL);
}

static void
run_job_cb (GObject      *source_object,
            GAsyncResult *result,
            gpointer      user_data)
{
	GsPluginJob *plugin_job = GS_PLUGIN_JOB (source_object);
	GsPluginJobClass *job_class;
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	GS_PROFILER_ADD_MARK_TAKE (PluginLoader,
				   GPOINTER_TO_SIZE (g_task_get_task_data (task)),
				   g_strdup_printf ("process-thread:%s", G_OBJECT_TYPE_NAME (plugin_job)),
				   gs_plugin_job_to_string (plugin_job));

	/* FIXME: This will eventually go away when
	 * gs_plugin_loader_job_process_finish() is removed. */
	job_class = GS_PLUGIN_JOB_GET_CLASS (plugin_job);

	g_assert (job_class->run_finish != NULL);

	if (!job_class->run_finish (plugin_job, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (GS_IS_PLUGIN_JOB_REFINE (plugin_job)) {
		GsAppList *list = gs_plugin_job_refine_get_result_list (GS_PLUGIN_JOB_REFINE (plugin_job));
		g_task_return_pointer (task, g_object_ref (list), (GDestroyNotify) g_object_unref);
		return;
	} else if (GS_IS_PLUGIN_JOB_LIST_APPS (plugin_job)) {
		GsAppList *list = gs_plugin_job_list_apps_get_result_list (GS_PLUGIN_JOB_LIST_APPS (plugin_job));
		g_task_return_pointer (task, g_object_ref (list), (GDestroyNotify) g_object_unref);
		return;
	} else if (GS_IS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (plugin_job)) {
		GsAppList *list = gs_plugin_job_list_distro_upgrades_get_result_list (GS_PLUGIN_JOB_LIST_DISTRO_UPGRADES (plugin_job));
		g_task_return_pointer (task, g_object_ref (list), (GDestroyNotify) g_object_unref);
		return;
	} else if (GS_IS_PLUGIN_JOB_REFRESH_METADATA (plugin_job)) {
		/* FIXME: For some reason, existing callers of refresh jobs
		 * expect a #GsAppList instance back, even though it’s empty and
		 * they don’t use its contents. It’s just used to distinguish
		 * against returning an error. This will go away when
		 * job_process_async() does. */
		g_task_return_pointer (task, gs_app_list_new (), g_object_unref);
		return;
	} else if (GS_IS_PLUGIN_JOB_MANAGE_REPOSITORY (plugin_job) ||
		   GS_IS_PLUGIN_JOB_LIST_CATEGORIES (plugin_job) ||
		   GS_IS_PLUGIN_JOB_UPDATE_APPS (plugin_job)) {
		/* FIXME: The gs_plugin_loader_job_action_finish() expects a #GsAppList
		 * pointer on success, thus return it. */
		g_task_return_pointer (task, gs_app_list_new (), g_object_unref);
		return;
	}

	g_assert_not_reached ();
}

typedef struct {
	GWeakRef parent_cancellable_weak;
	gulong handler_id;
} CancellableData;

static void
cancellable_data_free (CancellableData *data)
{
	g_autoptr(GCancellable) parent_cancellable = g_weak_ref_get (&data->parent_cancellable_weak);

	if (parent_cancellable != NULL)
		g_cancellable_disconnect (parent_cancellable, data->handler_id);

	g_weak_ref_clear (&data->parent_cancellable_weak);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CancellableData, cancellable_data_free)

static void
plugin_loader_task_freed_cb (gpointer user_data,
			     GObject *freed_object)
{
	g_autoptr(GsPluginLoader) plugin_loader = user_data;
	if (g_atomic_int_dec_and_test (&plugin_loader->active_jobs)) {
		/* if the plugin used updates-changed during its job, actually schedule
		 * the signal emission now */
		if (plugin_loader->updates_changed_cnt > 0)
			gs_plugin_loader_updates_changed (plugin_loader);
	}
}

static gboolean job_process_setup_complete_cb (GCancellable *cancellable,
                                               gpointer      user_data);
static void job_process_cb (GTask *task);

/**
 * gs_plugin_loader_job_process_async:
 * @plugin_loader: A #GsPluginLoader
 * @plugin_job: job to process
 * @cancellable: a #GCancellable, or %NULL
 * @callback: function to call when complete
 * @user_data: user data to pass to @callback
 *
 * This method calls all plugins.
 *
 * If the #GsPluginLoader is still being set up, this function will wait until
 * setup is complete before running.
 **/
void
gs_plugin_loader_job_process_async (GsPluginLoader *plugin_loader,
				    GsPluginJob *plugin_job,
				    GCancellable *cancellable,
				    GAsyncReadyCallback callback,
				    gpointer user_data)
{
	GsPluginJobClass *job_class;
	GsPluginAction action;
	g_autoptr(GTask) task = NULL;
	g_autoptr(GCancellable) cancellable_job = NULL;
	g_autofree gchar *task_name = NULL;

	g_return_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader));
	g_return_if_fail (GS_IS_PLUGIN_JOB (plugin_job));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	job_class = GS_PLUGIN_JOB_GET_CLASS (plugin_job);
	action = gs_plugin_job_get_action (plugin_job);

	if (job_class->run_async != NULL) {
		task_name = g_strdup_printf ("%s %s", G_STRFUNC, G_OBJECT_TYPE_NAME (plugin_job));
		cancellable_job = (cancellable != NULL) ? g_object_ref (cancellable) : NULL;
	} else {
		task_name = g_strdup_printf ("%s %s", G_STRFUNC, gs_plugin_action_to_string (action));
		cancellable_job = g_cancellable_new ();

		/* Old-style jobs always have a valid cancellable, so proxy the caller */
		g_debug ("Chaining cancellation from %p to %p", cancellable, cancellable_job);
		if (cancellable != NULL) {
			g_autoptr(CancellableData) cancellable_data = NULL;

			cancellable_data = g_new0 (CancellableData, 1);
			g_weak_ref_init (&cancellable_data->parent_cancellable_weak, cancellable);
			cancellable_data->handler_id = g_cancellable_connect (cancellable,
									      G_CALLBACK (gs_plugin_loader_cancelled_cb),
									      cancellable_job, NULL);

			g_object_set_data_full (G_OBJECT (cancellable_job),
						"gs-cancellable-chain",
						g_steal_pointer (&cancellable_data),
						(GDestroyNotify) cancellable_data_free);
		}
	}

	gs_job_manager_add_job (plugin_loader->job_manager, plugin_job);

	task = g_task_new (plugin_loader, cancellable_job, callback, user_data);
	g_task_set_name (task, task_name);
	g_task_set_task_data (task, g_object_ref (plugin_job), (GDestroyNotify) g_object_unref);

	g_atomic_int_inc (&plugin_loader->active_jobs);
	g_object_weak_ref (G_OBJECT (task),
		plugin_loader_task_freed_cb, g_object_ref (plugin_loader));

	/* Wait until the plugin has finished setting up.
	 *
	 * Do this using a #GCancellable. While we’re not using the #GCancellable
	 * to cancel anything, it is a reliable way to signal between threads
	 * without polling, waking up all waiting #GMainContexts when it’s
	 * ‘cancelled’. */
	if (plugin_loader->setup_complete) {
		job_process_cb (task);
	} else {
		g_autoptr(GSource) cancellable_source = g_cancellable_source_new (plugin_loader->setup_complete_cancellable);
		g_task_attach_source (task, cancellable_source, G_SOURCE_FUNC (job_process_setup_complete_cb));
	}
}

static gboolean
job_process_setup_complete_cb (GCancellable *cancellable,
                               gpointer      user_data)
{
	GTask *task = G_TASK (user_data);

	job_process_cb (task);

	return G_SOURCE_REMOVE;
}

static void
job_process_cb (GTask *task)
{
	g_autoptr(GsPluginJob) plugin_job = g_object_ref (g_task_get_task_data (task));
	GsPluginLoader *plugin_loader = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsPluginJobClass *job_class;
	GsPluginAction action;
	GsPluginLoaderHelper *helper;

	job_class = GS_PLUGIN_JOB_GET_CLASS (plugin_job);
	action = gs_plugin_job_get_action (plugin_job);

	gs_plugin_job_set_cancellable (plugin_job, cancellable);

	/* If the job provides a more specific async run function, use that.
	 *
	 * FIXME: This will eventually go away when
	 * gs_plugin_loader_job_process_async() is removed. */

	if (job_class->run_async != NULL) {
#ifdef HAVE_SYSPROF
		gint64 begin_time_nsec G_GNUC_UNUSED = SYSPROF_CAPTURE_CURRENT_TIME;

		g_task_set_task_data (task, GSIZE_TO_POINTER (begin_time_nsec), NULL);
#endif

		job_class->run_async (plugin_job, plugin_loader, cancellable,
				      run_job_cb, g_object_ref (task));
		return;
	}

	/* check job has valid action */
	if (action == GS_PLUGIN_ACTION_UNKNOWN) {
		g_autofree gchar *job_str = gs_plugin_job_to_string (plugin_job);
		g_task_return_new_error (task,
					 GS_PLUGIN_ERROR,
					 GS_PLUGIN_ERROR_NOT_SUPPORTED,
					 "job has no valid action: %s", job_str);
		return;
	}

	/* deal with the install queue */
	if (action == GS_PLUGIN_ACTION_REMOVE) {
		if (remove_app_from_install_queue (plugin_loader, gs_plugin_job_get_app (plugin_job))) {
			GsAppList *list = gs_plugin_job_get_list (plugin_job);
			g_task_return_pointer (task, g_object_ref (list), (GDestroyNotify) g_object_unref);
			return;
		}
	}

	/* FIXME: the plugins should specify this, rather than hardcoding */
	if (gs_plugin_job_has_refine_flags (plugin_job,
					    GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_UI)) {
		gs_plugin_job_add_refine_flags (plugin_job,
						GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN);
	}
	if (gs_plugin_job_has_refine_flags (plugin_job,
					    GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME)) {
		gs_plugin_job_add_refine_flags (plugin_job,
						GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN);
	}
	if (gs_plugin_job_has_refine_flags (plugin_job,
					    GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE)) {
		gs_plugin_job_add_refine_flags (plugin_job,
						GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME);
	}

	/* FIXME: this is probably a bug */
	if (action == GS_PLUGIN_ACTION_GET_SOURCES) {
		gs_plugin_job_add_refine_flags (plugin_job,
						GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION);
	}

	/* check required args */
	switch (action) {
	case GS_PLUGIN_ACTION_URL_TO_APP:
		if (gs_plugin_job_get_search (plugin_job) == NULL) {
			g_task_return_new_error (task,
						 GS_PLUGIN_ERROR,
						 GS_PLUGIN_ERROR_NOT_SUPPORTED,
						 "no valid search terms");
			return;
		}
		break;
	default:
		break;
	}

	/* save helper */
	helper = gs_plugin_loader_helper_new (plugin_loader, plugin_job);
	g_task_set_task_data (task, helper, (GDestroyNotify) gs_plugin_loader_helper_free);

	/* let the task cancel itself */
	g_task_set_check_cancellable (task, FALSE);
	g_task_set_return_on_cancel (task, FALSE);

	switch (action) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD:
		/* these actions must be performed by the thread pool because we
		 * want to limit the number of them running in parallel */
		gs_plugin_loader_schedule_task (plugin_loader, task);
		return;
	default:
		/* run in an unrestricted thread pool thread */
		g_thread_pool_push (plugin_loader->old_api_thread_pool,
				    g_object_ref (task), NULL);
		return;
	}
}

/******************************************************************************/

/**
 * gs_plugin_loader_get_plugin_supported:
 * @plugin_loader: A #GsPluginLoader
 * @function_name: a function name
 *
 * This function returns TRUE if the symbol is found in any enabled plugin.
 */
gboolean
gs_plugin_loader_get_plugin_supported (GsPluginLoader *plugin_loader,
				       const gchar *function_name)
{
	for (guint i = 0; i < plugin_loader->plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugin_loader->plugins, i);
		if (gs_plugin_get_symbol (plugin, function_name) != NULL)
			return TRUE;
	}
	return FALSE;
}

/**
 * gs_plugin_loader_get_plugins:
 * @plugin_loader: a #GsPluginLoader
 *
 * Get the set of currently loaded plugins.
 *
 * This includes disabled plugins, which should be checked for using
 * gs_plugin_get_enabled().
 *
 * This is intended to be used by internal lingmo-appstore code. Plugin and UI
 * code should typically use #GsPluginJob to run operations.
 *
 * Returns: (transfer none) (element-type GsPlugin): list of #GsPlugins
 * Since: 42
 */
GPtrArray *
gs_plugin_loader_get_plugins (GsPluginLoader *plugin_loader)
{
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);

	return plugin_loader->plugins;
}

static void app_create_cb (GObject      *source_object,
                           GAsyncResult *result,
                           gpointer      user_data);

/**
 * gs_plugin_loader_app_create_async:
 * @plugin_loader: a #GsPluginLoader
 * @unique_id: a unique_id
 * @cancellable: a #GCancellable, or %NULL
 * @callback: function to call when complete
 * @user_data: user data to pass to @callback
 *
 * Create a #GsApp identified by @unique_id asynchronously.
 * Finish the call with gs_plugin_loader_app_create_finish().
 *
 * If the #GsPluginLoader is still being set up, this function will wait until
 * setup is complete before running.
 *
 * Since: 41
 **/
void
gs_plugin_loader_app_create_async (GsPluginLoader *plugin_loader,
				   const gchar *unique_id,
				   GCancellable *cancellable,
				   GAsyncReadyCallback callback,
				   gpointer user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GsPluginJob) refine_job = NULL;

	g_return_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader));
	g_return_if_fail (unique_id != NULL);
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (plugin_loader, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_loader_app_create_async);
	g_task_set_task_data (task, g_strdup (unique_id), g_free);

	/* use the plugin loader to convert a wildcard app */
	app = gs_app_new (NULL);
	gs_app_add_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_set_from_unique_id (app, unique_id, AS_COMPONENT_KIND_UNKNOWN);
	gs_app_list_add (list, app);

	/* Refine the wildcard app. */
	refine_job = gs_plugin_job_refine_new (list, GS_PLUGIN_REFINE_FLAGS_REQUIRE_ID | GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
	gs_plugin_loader_job_process_async (plugin_loader, refine_job,
					    cancellable,
					    app_create_cb,
					    g_steal_pointer (&task));
}

static void
app_create_cb (GObject      *source_object,
               GAsyncResult *result,
               gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (g_task_get_source_object (task));
	const gchar *unique_id = g_task_get_task_data (task);
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) local_error = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader, result, &local_error);
	if (list == NULL) {
		g_prefix_error (&local_error, "Failed to refine '%s': ", unique_id);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* return the matching GsApp */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app_tmp = gs_app_list_index (list, i);
		if (g_strcmp0 (unique_id, gs_app_get_unique_id (app_tmp)) == 0) {
			g_task_return_pointer (task, g_object_ref (app_tmp), g_object_unref);
			return;
		}
	}

	/* return the first returned app that's not a wildcard */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app_tmp = gs_app_list_index (list, i);
		if (!gs_app_has_quirk (app_tmp, GS_APP_QUIRK_IS_WILDCARD)) {
			g_debug ("returning imperfect match: %s != %s",
				 unique_id, gs_app_get_unique_id (app_tmp));
			g_task_return_pointer (task, g_object_ref (app_tmp), g_object_unref);
			return;
		}
	}

	/* does not exist */
	g_task_return_new_error (task,
				 GS_PLUGIN_ERROR,
				 GS_PLUGIN_ERROR_FAILED,
				 "Failed to create an app for '%s'", unique_id);
}

/**
 * gs_plugin_loader_app_create_finish:
 * @plugin_loader: a #GsPluginLoader
 * @res: a #GAsyncResult
 * @error: A #GError, or %NULL
 *
 * Finishes call to gs_plugin_loader_app_create_async().
 *
 * Returns: (transfer full): a #GsApp, or %NULL on error.
 *
 * Since: 41
 **/
GsApp *
gs_plugin_loader_app_create_finish (GsPluginLoader *plugin_loader,
				    GAsyncResult *res,
				    GError **error)
{
	GsApp *app;

	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);
	g_return_val_if_fail (G_IS_TASK (res), NULL);
	g_return_val_if_fail (g_task_is_valid (res, plugin_loader), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	app = g_task_propagate_pointer (G_TASK (res), error);
	gs_utils_error_convert_gio (error);
	return app;
}

/**
 * gs_plugin_loader_get_system_app_async:
 * @plugin_loader: a #GsPluginLoader
 * @cancellable: a #GCancellable, or %NULL
 * @callback: function to call when complete
 * @user_data: user data to pass to @callback
 *
 * Get the application that represents the currently installed OS
 * asynchronously. Finish the call with gs_plugin_loader_get_system_app_finish().
 *
 * Since: 41
 **/
void
gs_plugin_loader_get_system_app_async (GsPluginLoader *plugin_loader,
				       GCancellable *cancellable,
				       GAsyncReadyCallback callback,
				       gpointer user_data)
{
	gs_plugin_loader_app_create_async (plugin_loader, "*/*/*/system/*", cancellable, callback, user_data);
}

/**
 * gs_plugin_loader_get_system_app_finish:
 * @plugin_loader: a #GsPluginLoader
 * @res: a #GAsyncResult
 * @error: A #GError, or %NULL
 *
 * Finishes call to gs_plugin_loader_get_system_app_async().
 *
 * Returns: (transfer full): a #GsApp, which represents
 *    the currently installed OS, or %NULL on error.
 *
 * Since: 41
 **/
GsApp *
gs_plugin_loader_get_system_app_finish (GsPluginLoader *plugin_loader,
					GAsyncResult *res,
					GError **error)
{
	return gs_plugin_loader_app_create_finish (plugin_loader, res, error);
}

/**
 * gs_plugin_loader_get_odrs_provider:
 * @plugin_loader: a #GsPluginLoader
 *
 * Get the singleton #GsOdrsProvider which provides access to ratings and
 * reviews data from ODRS.
 *
 * Returns: (transfer none) (nullable): a #GsOdrsProvider, or %NULL if disabled
 * Since: 41
 */
GsOdrsProvider *
gs_plugin_loader_get_odrs_provider (GsPluginLoader *plugin_loader)
{
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);

	return plugin_loader->odrs_provider;
}

/**
 * gs_plugin_loader_set_max_parallel_ops:
 * @plugin_loader: a #GsPluginLoader
 * @max_ops: the maximum number of parallel operations
 *
 * Sets the number of maximum number of queued operations (install/update/upgrade-download)
 * to be processed at a time. If @max_ops is 0, then it will set the default maximum number.
 */
void
gs_plugin_loader_set_max_parallel_ops (GsPluginLoader *plugin_loader,
				       guint max_ops)
{
	g_autoptr(GError) error = NULL;
	if (max_ops == 0)
		max_ops = get_max_parallel_ops ();
	if (!g_thread_pool_set_max_threads (plugin_loader->queued_ops_pool, max_ops, &error))
		g_warning ("Failed to set the maximum number of ops in parallel: %s",
			   error->message);
}

/**
 * gs_plugin_loader_get_job_manager:
 * @plugin_loader: a #GsPluginLoader
 *
 * Get the job manager singleton.
 *
 * Returns: (transfer none): a job manager
 * Since: 44
 */
GsJobManager *
gs_plugin_loader_get_job_manager (GsPluginLoader *plugin_loader)
{
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);

	return plugin_loader->job_manager;
}

/**
 * gs_plugin_loader_get_category_manager:
 * @plugin_loader: a #GsPluginLoader
 *
 * Get the category manager singleton.
 *
 * Returns: (transfer none): a category manager
 * Since: 40
 */
GsCategoryManager *
gs_plugin_loader_get_category_manager (GsPluginLoader *plugin_loader)
{
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), NULL);

	return plugin_loader->category_manager;
}

/**
 * gs_plugin_loader_emit_updates_changed:
 * @self: a #GsPluginLoader
 *
 * Emits the #GsPluginLoader:updates-changed signal in the nearest
 * idle in the main thread.
 *
 * Since: 43
 **/
void
gs_plugin_loader_emit_updates_changed (GsPluginLoader *self)
{
	g_return_if_fail (GS_IS_PLUGIN_LOADER (self));

	if (self->updates_changed_id != 0)
		g_source_remove (self->updates_changed_id);

	self->updates_changed_id =
		g_idle_add_full (G_PRIORITY_HIGH_IDLE,
				 gs_plugin_loader_job_updates_changed_delay_cb,
				 g_object_ref (self), g_object_unref);
}
