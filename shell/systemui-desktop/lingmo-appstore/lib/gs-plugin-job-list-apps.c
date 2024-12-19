/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-plugin-job-list-apps
 * @short_description: A plugin job to list apps according to a search query
 *
 * #GsPluginJobListApps is a #GsPluginJob representing an operation to
 * list apps which match a given query, from all #GsPlugins.
 *
 * The known properties on the set of apps returned by this operation can be
 * controlled with the #GsAppQuery:refine-flags property of the query. All
 * results will be refined using the given set of refine flags. See
 * #GsPluginJobRefine.
 *
 * This class is a wrapper around #GsPluginClass.list_apps_async,
 * calling it for all loaded plugins, with #GsPluginJobRefine used to refine
 * them.
 *
 * Retrieve the resulting #GsAppList using
 * gs_plugin_job_list_apps_get_result_list().
 *
 * See also: #GsPluginClass.list_apps_async
 * Since: 43
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-app.h"
#include "gs-app-list-private.h"
#include "gs-app-query.h"
#include "gs-enums.h"
#include "gs-plugin-job.h"
#include "gs-plugin-job-list-apps.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-refine.h"
#include "gs-plugin-private.h"
#include "gs-plugin-types.h"
#include "gs-profiler.h"
#include "gs-utils.h"

struct _GsPluginJobListApps
{
	GsPluginJob parent;

	/* Input arguments. */
	GsAppQuery *query;  /* (owned) (nullable) */
	GsPluginListAppsFlags flags;

	/* In-progress data. */
	GsAppList *merged_list;  /* (owned) (nullable) */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;

	/* Results. */
	GsAppList *result_list;  /* (owned) (nullable) */

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
};

G_DEFINE_TYPE (GsPluginJobListApps, gs_plugin_job_list_apps, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_QUERY = 1,
	PROP_FLAGS,
} GsPluginJobListAppsProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

static void
gs_plugin_job_list_apps_dispose (GObject *object)
{
	GsPluginJobListApps *self = GS_PLUGIN_JOB_LIST_APPS (object);

	g_assert (self->merged_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	g_clear_object (&self->result_list);
	g_clear_object (&self->query);

	G_OBJECT_CLASS (gs_plugin_job_list_apps_parent_class)->dispose (object);
}

static void
gs_plugin_job_list_apps_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
	GsPluginJobListApps *self = GS_PLUGIN_JOB_LIST_APPS (object);

	switch ((GsPluginJobListAppsProperty) prop_id) {
	case PROP_QUERY:
		g_value_set_object (value, self->query);
		break;
	case PROP_FLAGS:
		g_value_set_flags (value, self->flags);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_job_list_apps_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
	GsPluginJobListApps *self = GS_PLUGIN_JOB_LIST_APPS (object);

	switch ((GsPluginJobListAppsProperty) prop_id) {
	case PROP_QUERY:
		/* Construct only. */
		g_assert (self->query == NULL);
		self->query = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_FLAGS:
		/* Construct only. */
		g_assert (self->flags == 0);
		self->flags = g_value_get_flags (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
filter_valid_apps (GsApp    *app,
                   gpointer  user_data)
{
	GsPluginJobListApps *self = GS_PLUGIN_JOB_LIST_APPS (user_data);
	GsPluginRefineFlags refine_flags = GS_PLUGIN_REFINE_FLAGS_NONE;

	if (self->query)
		refine_flags = gs_app_query_get_refine_flags (self->query);

	return gs_plugin_loader_app_is_valid (app, refine_flags);
}

static gboolean
filter_freely_licensed_apps (GsApp    *app,
			     gpointer  user_data)
{
	return (gs_app_get_kind (app) != AS_COMPONENT_KIND_GENERIC &&
		gs_app_get_kind (app) != AS_COMPONENT_KIND_DESKTOP_APP &&
		gs_app_get_kind (app) != AS_COMPONENT_KIND_CONSOLE_APP &&
		gs_app_get_kind (app) != AS_COMPONENT_KIND_WEB_APP) ||
	       gs_app_get_state (app) == GS_APP_STATE_INSTALLED ||
	       gs_app_get_state (app) == GS_APP_STATE_UPDATABLE ||
	       gs_app_get_state (app) == GS_APP_STATE_UPDATABLE_LIVE ||
	       gs_app_get_license_is_free (app);
}

static gboolean
filter_developer_verified_apps (GsApp    *app,
				gpointer  user_data)
{
	return gs_app_has_quirk (app, GS_APP_QUIRK_DEVELOPER_VERIFIED);
}

static gboolean
app_filter_qt_for_gtk_and_compatible (GsApp    *app,
                                      gpointer  user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (user_data);

	/* hide the QT versions in preference to the GTK ones */
	if (g_strcmp0 (gs_app_get_id (app), "transmission-qt.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "nntpgrab_qt.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "gimagereader-qt4.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "gimagereader-qt5.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "nntpgrab_server_qt.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "hotot-qt.desktop") == 0) {
		g_debug ("removing QT version of %s",
			 gs_app_get_unique_id (app));
		return FALSE;
	}

	/* hide the KDE version in preference to the GTK one */
	if (g_strcmp0 (gs_app_get_id (app), "qalculate_kde.desktop") == 0) {
		g_debug ("removing KDE version of %s",
			 gs_app_get_unique_id (app));
		return FALSE;
	}

	/* hide the KDE version in preference to the Qt one */
	if (g_strcmp0 (gs_app_get_id (app), "kid3.desktop") == 0 ||
	    g_strcmp0 (gs_app_get_id (app), "kchmviewer.desktop") == 0) {
		g_debug ("removing KDE version of %s",
			 gs_app_get_unique_id (app));
		return FALSE;
	}

	return gs_plugin_loader_app_is_compatible (plugin_loader, app);
}

static void plugin_list_apps_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);
static void refine_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data);
static void finish_task (GTask     *task,
                         GsAppList *merged_list);

static void
gs_plugin_job_list_apps_run_async (GsPluginJob         *job,
                                   GsPluginLoader      *plugin_loader,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginJobListApps *self = GS_PLUGIN_JOB_LIST_APPS (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean anything_ran = FALSE;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_list_apps_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* run each plugin, keeping a counter of pending operations which is
	 * initialised to 1 until all the operations are started */
	self->n_pending_ops = 1;
	self->merged_list = gs_app_list_new ();
	plugins = gs_plugin_loader_get_plugins (plugin_loader);

#ifdef HAVE_SYSPROF
	self->begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->list_apps_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* run the plugin */
		self->n_pending_ops++;
		plugin_class->list_apps_async (plugin, self->query, self->flags, cancellable, plugin_list_apps_cb, g_object_ref (task));
	}

	if (!anything_ran)
		g_debug ("no plugin could handle listing apps");

	finish_op (task, g_steal_pointer (&local_error));
}

static void
plugin_list_apps_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobListApps *self = g_task_get_source_object (task);
	g_autoptr(GsAppList) plugin_apps = NULL;
	g_autoptr(GError) local_error = NULL;

	plugin_apps = plugin_class->list_apps_finish (plugin, result, &local_error);
	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	if (plugin_apps != NULL)
		gs_app_list_add_list (self->merged_list, plugin_apps);

	/* Only log errors from plugins. No need to discard everything when one plugin fails. */
	if (local_error != NULL &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("plugin '%s' failed to list apps: %s",
			 gs_plugin_get_name (plugin),
			 local_error->message);
		g_clear_error (&local_error);
	}

	GS_PROFILER_ADD_MARK_TAKE (PluginJobListApps,
				   self->begin_time_nsec,
				   g_strdup_printf ("%s:%s",
						    G_OBJECT_TYPE_NAME (self),
						    gs_plugin_get_name (plugin)),
				   NULL);

	finish_op (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobListApps *self = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsPluginLoader *plugin_loader = g_task_get_task_data (task);
	g_autoptr(GsAppList) merged_list = NULL;
	GsPluginRefineFlags refine_flags = GS_PLUGIN_REFINE_FLAGS_NONE;
	GsAppQueryLicenseType license_type = GS_APP_QUERY_LICENSE_ANY;
	g_autoptr(GError) error_owned = g_steal_pointer (&error);

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while listing apps: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* Get the results of the parallel ops. */
	merged_list = g_steal_pointer (&self->merged_list);

	if (self->saved_error != NULL) {
		g_task_return_error (task, g_steal_pointer (&self->saved_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	/* run refine() on each one if required */
	if (self->query != NULL) {
		refine_flags = gs_app_query_get_refine_flags (self->query);
		license_type = gs_app_query_get_license_type (self->query);
	}

	if (!(refine_flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE) &&
	    license_type != GS_APP_QUERY_LICENSE_ANY) {
		/* Needs the license information when filtering with it */
		refine_flags |= GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE;
	}

	if (merged_list != NULL &&
	    gs_app_list_length (merged_list) > 0 &&
	    refine_flags != GS_PLUGIN_REFINE_FLAGS_NONE) {
		g_autoptr(GsPluginJob) refine_job = NULL;

		refine_job = gs_plugin_job_refine_new (merged_list,
						       refine_flags |
						       GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING);
		gs_plugin_loader_job_process_async (plugin_loader, refine_job,
						    cancellable,
						    refine_cb,
						    g_object_ref (task));
	} else {
		g_debug ("No apps to refine");
		finish_task (task, merged_list);
	}
}

static void
refine_cb (GObject      *source_object,
           GAsyncResult *result,
           gpointer      user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobListApps *self = g_task_get_source_object (task);
	g_autoptr(GsAppList) new_list = NULL;
	g_autoptr(GError) local_error = NULL;

	new_list = gs_plugin_loader_job_process_finish (plugin_loader, result, &local_error);
	if (new_list == NULL) {
		gs_utils_error_convert_gio (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	finish_task (task, new_list);
}

static void
finish_task (GTask     *task,
             GsAppList *merged_list)
{
	GsPluginJobListApps *self = g_task_get_source_object (task);
	GsPluginLoader *plugin_loader = g_task_get_task_data (task);
	GsAppListFilterFlags dedupe_flags = GS_APP_LIST_FILTER_FLAG_NONE;
	GsAppListSortFunc sort_func = NULL;
	gpointer sort_func_data = NULL;
	GsAppQueryLicenseType license_type = GS_APP_QUERY_LICENSE_ANY;
	GsAppQueryDeveloperVerifiedType developer_verified_type = GS_APP_QUERY_DEVELOPER_VERIFIED_ANY;
	GsAppListFilterFunc filter_func = NULL;
	gpointer filter_func_data = NULL;
	guint max_results = 0;
	g_autofree gchar *job_debug = NULL;

	/* Standard filtering.
	 *
	 * FIXME: It feels like this filter should be done in a different layer. */
	gs_app_list_filter (merged_list, filter_valid_apps, self);
	gs_app_list_filter (merged_list, app_filter_qt_for_gtk_and_compatible, plugin_loader);

	if (self->query != NULL) {
		license_type = gs_app_query_get_license_type (self->query);
		developer_verified_type = gs_app_query_get_developer_verified_type (self->query);
	}

	if (license_type == GS_APP_QUERY_LICENSE_FOSS)
		gs_app_list_filter (merged_list, filter_freely_licensed_apps, self);
	if (developer_verified_type == GS_APP_QUERY_DEVELOPER_VERIFIED_ONLY)
		gs_app_list_filter (merged_list, filter_developer_verified_apps, self);

	/* Caller-specified filtering. */
	if (self->query != NULL)
		filter_func = gs_app_query_get_filter_func (self->query, &filter_func_data);

	if (filter_func != NULL)
		gs_app_list_filter (merged_list, filter_func, filter_func_data);

	/* Filter duplicates with priority, taking into account the source name
	 * & version, so we combine available updates with the installed app */
	if (self->query != NULL)
		dedupe_flags = gs_app_query_get_dedupe_flags (self->query);

	if (dedupe_flags != GS_APP_LIST_FILTER_FLAG_NONE)
		gs_app_list_filter_duplicates (merged_list, dedupe_flags);

	/* Sort the results. The refine may have added useful metadata. */
	if (self->query != NULL)
		sort_func = gs_app_query_get_sort_func (self->query, &sort_func_data);

	if (sort_func != NULL) {
		gs_app_list_sort (merged_list, sort_func, sort_func_data);
	} else {
		g_debug ("no ->sort_func() set, using random!");
		gs_app_list_randomize (merged_list);
	}

	/* Truncate the results if needed. */
	if (self->query != NULL)
		max_results = gs_app_query_get_max_results (self->query);

	if (max_results > 0 && gs_app_list_length (merged_list) > max_results) {
		g_debug ("truncating results from %u to %u",
			 gs_app_list_length (merged_list), max_results);
		gs_app_list_truncate (merged_list, max_results);
	}

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	/* Check the intermediate working values are all cleared. */
	g_assert (self->merged_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* success */
	g_set_object (&self->result_list, merged_list);
	g_task_return_boolean (task, TRUE);
	g_signal_emit_by_name (G_OBJECT (self), "completed");

#ifdef HAVE_SYSPROF
	sysprof_collector_mark (self->begin_time_nsec,
				SYSPROF_CAPTURE_CURRENT_TIME - self->begin_time_nsec,
				"lingmo-appstore",
				G_OBJECT_TYPE_NAME (self),
				NULL);
#endif
}

static gboolean
gs_plugin_job_list_apps_run_finish (GsPluginJob   *self,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_list_apps_class_init (GsPluginJobListAppsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_list_apps_dispose;
	object_class->get_property = gs_plugin_job_list_apps_get_property;
	object_class->set_property = gs_plugin_job_list_apps_set_property;

	job_class->run_async = gs_plugin_job_list_apps_run_async;
	job_class->run_finish = gs_plugin_job_list_apps_run_finish;

	/**
	 * GsPluginJobListApps:query: (nullable)
	 *
	 * A #GsAppQuery defining the query parameters.
	 *
	 * If this is %NULL, all apps will be returned.
	 *
	 * Since: 43
	 */
	props[PROP_QUERY] =
		g_param_spec_object ("query", "Query",
				     "A #GsAppQuery defining the query parameters.",
				     GS_TYPE_APP_QUERY,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobListApps:flags:
	 *
	 * Flags to specify how the operation should run.
	 *
	 * Since: 43
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how the operation should run.",
				    GS_TYPE_PLUGIN_LIST_APPS_FLAGS,
				    GS_PLUGIN_LIST_APPS_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_job_list_apps_init (GsPluginJobListApps *self)
{
}

/**
 * gs_plugin_job_list_apps_new:
 * @query: (nullable) (transfer none): query to affect which apps to return
 * @flags: flags affecting how the operation runs
 *
 * Create a new #GsPluginJobListApps for listing apps according to the given
 * @query.
 *
 * Returns: (transfer full): a new #GsPluginJobListApps
 * Since: 43
 */
GsPluginJob *
gs_plugin_job_list_apps_new (GsAppQuery            *query,
                             GsPluginListAppsFlags  flags)
{
	g_return_val_if_fail (query == NULL || GS_IS_APP_QUERY (query), NULL);

	return g_object_new (GS_TYPE_PLUGIN_JOB_LIST_APPS,
			     "query", query,
			     "flags", flags,
			     NULL);
}

/**
 * gs_plugin_job_list_apps_get_result_list:
 * @self: a #GsPluginJobListApps
 *
 * Get the full list of apps matching the query.
 *
 * If this is called before the job is complete, %NULL will be returned.
 *
 * Returns: (transfer none) (nullable): the job results, or %NULL on error
 *   or if called before the job has completed
 * Since: 43
 */
GsAppList *
gs_plugin_job_list_apps_get_result_list (GsPluginJobListApps *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_LIST_APPS (self), NULL);

	return self->result_list;
}
