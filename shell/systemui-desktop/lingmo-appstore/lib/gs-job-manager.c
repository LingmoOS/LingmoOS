/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022, 2023 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-job-manager
 * @short_description: A manager to track ongoing #GsPluginJobs
 *
 * #GsJobManager tracks ongoing #GsPluginJobs and the #GsApps they are
 * affecting.
 *
 * This makes it possible to track all the jobs ongoing in lingmo-appstore, or
 * in a particular backend, or for a particular app at any time.
 *
 * ‘Watches’ can be added to the job manager, which cause callbacks to be
 * invoked when jobs are added or removed which match certain criteria, such as
 * being a certain type of job or referring to a certain application. See
 * gs_job_manager_add_watch() and gs_job_manager_remove_watch().
 *
 * #GsJobManager is safe to use from any thread.
 *
 * See also: #GsPluginJob
 * Since: 44
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include "gs-enums.h"
#include "gs-plugin-job.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-update-apps.h"
#include "gs-plugin-types.h"
#include "gs-utils.h"

/* Data for a single watch, added using gs_job_manager_add_watch().
 *
 * This structure is immutable after creation* which means it can be safely
 * accessed from multiple threads. It might be accessed from multiple threads
 * if operations happen on the #GsJobManager from one thread, but require the
 * @added_handler/@removed_handler callbacks to be called in another thread.
 * They, plus @user_data_free_func, are always called in the thread running
 * @callback_context.
 *
 * * @user_data and @user_data_free_func may actually be temporarily cleared
 * by watch_data_unref() while the structure is temporarily resurrected to pass
 * to watch_free_data_cb(). That doesn’t affect the normal operation of the
 * struct, though, which is immutable.
 */
typedef struct {
	gint ref_count;  /* (atomic) */

	guint watch_id;

	gchar *match_app_unique_id;  /* (nullable) */
	GType match_job_type;

	GsJobManagerJobCallback added_handler;
	GsJobManagerJobCallback removed_handler;
	gpointer user_data;
	GDestroyNotify user_data_free_func;
	GMainContext *callback_context;  /* (owned) */
} WatchData;

static WatchData *
watch_data_ref (WatchData *data)
{
	gint old_value = g_atomic_int_add (&data->ref_count, 1);
	g_assert (old_value > 0);
	return data;
}

static gboolean watch_free_data_cb (gpointer user_data);

static void
watch_data_unref (WatchData *data)
{
	if (g_atomic_int_dec_and_test (&data->ref_count)) {
		if (data->user_data_free_func != NULL) {
			g_autoptr(GSource) idle_source = NULL;
			GMainContext *callback_context = data->callback_context;

			/* Temporarily resurrect @data so it can be used as the
			 * closure for watch_free_data_cb(), so that the
			 * user_data is freed in the right thread. */
			g_atomic_int_inc (&data->ref_count);

			idle_source = g_idle_source_new ();
			g_source_set_priority (idle_source, G_PRIORITY_DEFAULT);
			g_source_set_callback (idle_source,
					       watch_free_data_cb,
					       g_steal_pointer (&data),
					       (GDestroyNotify) watch_data_unref);
			g_source_set_static_name (idle_source, G_STRFUNC);
			g_source_attach (idle_source, callback_context);

			/* Freeing will eventually happen in watch_free_data_cb(). */
			return;
		}

		g_free (data->match_app_unique_id);
		g_main_context_unref (data->callback_context);
		g_free (data);
	}
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (WatchData, watch_data_unref)

static gboolean
watch_free_data_cb (gpointer user_data)
{
	WatchData *data = user_data;

	/* We must hold the last reference to @data, and this callback must be
	 * executed in the right thread. */
	g_assert (g_atomic_int_get (&data->ref_count) == 1);
	g_assert (data->user_data_free_func != NULL);
	g_assert (g_main_context_is_owner (data->callback_context));

	data->user_data_free_func (g_steal_pointer (&data->user_data));
	data->user_data_free_func = NULL;

	/* The callback must not somehow re-reference @data. */
	g_assert (g_atomic_int_get (&data->ref_count) == 1);

	/* Removing the source will drop the last ref on @data */
	return G_SOURCE_REMOVE;
}

static gboolean
job_contains_app_by_unique_id (GsPluginJob *job,
                               const gchar *app_unique_id)
{
	GsAppList *apps = NULL;

	/* FIXME: This could be improved in future by making GsPluginJob subclasses
	 * implement an interface to query which apps they are acting on. */
	if (GS_IS_PLUGIN_JOB_UPDATE_APPS (job))
		apps = gs_plugin_job_update_apps_get_apps (GS_PLUGIN_JOB_UPDATE_APPS (job));

	if (apps == NULL)
		return FALSE;

	return (gs_app_list_lookup (apps, app_unique_id) != NULL);
}

static gboolean
watch_data_matches (const WatchData *data,
                    GsPluginJob     *job)
{
	if (data->match_job_type != G_TYPE_INVALID &&
	    data->match_job_type != G_OBJECT_TYPE (job))
		return FALSE;

	if (data->match_app_unique_id != NULL &&
	    !job_contains_app_by_unique_id (job, data->match_app_unique_id))
		return FALSE;

	return TRUE;
}

/* Data relating to a single invocation of a #GsJobManagerJobCallback, either
 * an @added_handler or a @removed_handler.
 *
 * This is essentially a closure to pass the callback data from the thread where
 * the job is being added/removed to the thread where the callback is invoked.
 *
 * This structure is immutable after creation, so is inherently thread-safe.
 */
typedef struct {
	GsJobManager *job_manager;  /* (owned) (not nullable) */
	WatchData *watch_data;  /* (owned) (not nullable) */
	enum {
		WATCH_CALL_ADDED,
		WATCH_CALL_REMOVED,
	} call_type;
	GsPluginJob *job;  /* (owned) (not nullable) */
} WatchCallHandlerData;

static void
watch_call_handler_data_free (WatchCallHandlerData *data)
{
	g_clear_object (&data->job);
	g_clear_pointer (&data->watch_data, watch_data_unref);
	g_clear_object (&data->job_manager);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (WatchCallHandlerData, watch_call_handler_data_free)

static gboolean
watch_call_handler_cb (gpointer user_data)
{
	WatchCallHandlerData *data = user_data;
	GsJobManagerJobCallback handler;

	/* Must be executed in the right thread. */
	g_assert (g_main_context_is_owner (data->watch_data->callback_context));

	switch (data->call_type) {
	case WATCH_CALL_ADDED:
		handler = data->watch_data->added_handler;
		break;
	case WATCH_CALL_REMOVED:
		handler = data->watch_data->removed_handler;
		break;
	default:
		g_assert_not_reached ();
	}

	handler (data->job_manager, data->job, data->watch_data->user_data);

	return G_SOURCE_REMOVE;
}

struct _GsJobManager
{
	GObject parent;

	GMutex mutex;

	GPtrArray *jobs;  /* (owned) (element-type GsPluginJob) (not nullable), protected by @mutex */

	GPtrArray *watches;  /* (owned) (element-type WatchData) (not nullable), protected by @mutex */
	guint next_watch_id;  /* protected by @mutex */

	GCond shutdown_cond;
	gboolean shut_down; /* set to TRUE when being shut down */
};

G_DEFINE_TYPE (GsJobManager, gs_job_manager, G_TYPE_OBJECT)

static void
gs_job_manager_dispose (GObject *object)
{
	GsJobManager *self = GS_JOB_MANAGER (object);

	/* All jobs should have completed or been cancelled by now. */
	g_assert (self->jobs->len == 0);

	/* All watches should have been removed by now. */
	g_assert (self->watches->len == 0);

	G_OBJECT_CLASS (gs_job_manager_parent_class)->dispose (object);
}

static void
gs_job_manager_finalize (GObject *object)
{
	GsJobManager *self = GS_JOB_MANAGER (object);

	g_clear_pointer (&self->jobs, g_ptr_array_unref);
	g_clear_pointer (&self->watches, g_ptr_array_unref);
	g_cond_clear (&self->shutdown_cond);
	g_mutex_clear (&self->mutex);

	G_OBJECT_CLASS (gs_job_manager_parent_class)->finalize (object);
}

static void
gs_job_manager_class_init (GsJobManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gs_job_manager_dispose;
	object_class->finalize = gs_job_manager_finalize;
}

static void
gs_job_manager_init (GsJobManager *self)
{
	g_mutex_init (&self->mutex);
	g_cond_init (&self->shutdown_cond);
	self->jobs = g_ptr_array_new_with_free_func (g_object_unref);
	self->watches = g_ptr_array_new_with_free_func ((GDestroyNotify) watch_data_unref);
	self->next_watch_id = 1;
}

/**
 * gs_job_manager_new:
 *
 * Create a new #GsJobManager for tracking pending jobs.
 *
 * Returns: (transfer full): a new #GsJobManager
 * Since: 44
 */
GsJobManager *
gs_job_manager_new (void)
{
	return g_object_new (GS_TYPE_JOB_MANAGER, NULL);
}

static void
job_completed_cb (GsPluginJob *job,
		  gpointer user_data)
{
	GsJobManager *self = GS_JOB_MANAGER (user_data);

	gs_job_manager_remove_job (self, job);
}

/**
 * gs_job_manager_add_job:
 * @self: a #GsJobManager
 * @job: a #GsPluginJob to add
 *
 * Add @job to the set of jobs tracked by the #GsJobManager.
 *
 * If @job is already tracked by the job manager, this function is a no-op.
 *
 * Returns: %TRUE if @job was added to the manager, %FALSE if it was already
 *   tracked
 * Since: 44
 */
gboolean
gs_job_manager_add_job (GsJobManager *self,
                        GsPluginJob  *job)
{
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), FALSE);
	g_return_val_if_fail (GS_IS_PLUGIN_JOB (job), FALSE);

	locker = g_mutex_locker_new (&self->mutex);

	if (g_ptr_array_find (self->jobs, job, NULL))
		return FALSE;

	g_ptr_array_add (self->jobs, g_object_ref (job));
	g_signal_connect (job, "completed", G_CALLBACK (job_completed_cb), self);

	/* Dispatch watches for this job. */
	for (guint i = 0; i < self->watches->len; i++) {
		WatchData *data = g_ptr_array_index (self->watches, i);

		if (data->added_handler != NULL &&
		    watch_data_matches (data, job)) {
			g_autoptr(WatchCallHandlerData) idle_data = NULL;
			g_autoptr(GSource) idle_source = NULL;

			idle_data = g_new0 (WatchCallHandlerData, 1);
			idle_data->job_manager = g_object_ref (self);
			idle_data->watch_data = watch_data_ref (data);
			idle_data->call_type = WATCH_CALL_ADDED;
			idle_data->job = g_object_ref (job);

			idle_source = g_idle_source_new ();
			g_source_set_priority (idle_source, G_PRIORITY_DEFAULT);
			g_source_set_callback (idle_source,
					       watch_call_handler_cb,
					       g_steal_pointer (&idle_data),
					       (GDestroyNotify) watch_call_handler_data_free);
			g_source_set_static_name (idle_source, G_STRFUNC);
			g_source_attach (idle_source, data->callback_context);
		}
	}

	if (self->shut_down) {
		g_debug ("Adding job '%s' while being shut down", G_OBJECT_TYPE_NAME (job));
		g_cond_broadcast (&self->shutdown_cond);
	}

	return TRUE;
}

/**
 * gs_job_manager_remove_job:
 * @self: a #GsJobManager
 * @job: a #GsPluginJob to remove
 *
 * Remove @job from the set of jobs tracked by the #GsJobManager.
 *
 * If @job is not already tracked by the job manager, this function is a no-op.
 *
 * Returns: %TRUE if @job was removed from the manager, %FALSE if it was not
 *   already tracked
 * Since: 44
 */
gboolean
gs_job_manager_remove_job (GsJobManager *self,
                           GsPluginJob  *job)
{
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), FALSE);
	g_return_val_if_fail (GS_IS_PLUGIN_JOB (job), FALSE);

	locker = g_mutex_locker_new (&self->mutex);

	if (!g_ptr_array_remove_fast (self->jobs, job))
		return FALSE;

	/* Dispatch watches for this job. */
	for (guint i = 0; i < self->watches->len; i++) {
		WatchData *data = g_ptr_array_index (self->watches, i);

		if (data->removed_handler != NULL &&
		    watch_data_matches (data, job)) {
			g_autoptr(WatchCallHandlerData) idle_data = NULL;
			g_autoptr(GSource) idle_source = NULL;

			idle_data = g_new0 (WatchCallHandlerData, 1);
			idle_data->job_manager = g_object_ref (self);
			idle_data->watch_data = watch_data_ref (data);
			idle_data->call_type = WATCH_CALL_REMOVED;
			idle_data->job = g_object_ref (job);

			idle_source = g_idle_source_new ();
			g_source_set_priority (idle_source, G_PRIORITY_DEFAULT);
			g_source_set_callback (idle_source,
					       watch_call_handler_cb,
					       g_steal_pointer (&idle_data),
					       (GDestroyNotify) watch_call_handler_data_free);
			g_source_set_static_name (idle_source, G_STRFUNC);
			g_source_attach (idle_source, data->callback_context);
		}
	}

	g_signal_handlers_disconnect_by_func (job, job_completed_cb, self);

	if (self->shut_down && self->jobs->len == 0)
		g_cond_broadcast (&self->shutdown_cond);

	return TRUE;
}

static gboolean
job_contains_app (GsPluginJob *job,
                  GsApp       *app)
{
	return job_contains_app_by_unique_id (job, gs_app_get_unique_id (app));
}

/**
 * gs_job_manager_get_pending_jobs_for_app:
 * @self: a #GsJobManager
 * @app: app to get pending jobs for
 *
 * Find the jobs which are ongoing for the given @app.
 *
 * Returns: (element-type GsPluginJob) (transfer container): zero or more
 *   ongoing jobs
 * Since: 44
 */
GPtrArray *
gs_job_manager_get_pending_jobs_for_app (GsJobManager *self,
                                         GsApp        *app)
{
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GPtrArray) jobs_for_app = NULL;

	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), NULL);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&self->mutex);

	jobs_for_app = g_ptr_array_new_with_free_func (g_object_unref);

	for (gsize i = 0; i < self->jobs->len; i++) {
		GsPluginJob *job = g_ptr_array_index (self->jobs, i);

		if (job_contains_app (job, app))
			g_ptr_array_add (jobs_for_app, g_object_ref (job));
	}

	return g_steal_pointer (&jobs_for_app);
}

/**
 * gs_job_manager_app_has_pending_job_type:
 * @self: a #GsJobManager
 * @app: app to query for pending jobs for
 * @pending_job_type: %GS_TYPE_PLUGIN_JOB or one of its subtypes
 *
 * Query whether there is at least one job of type @pending_job_type ongoing for
 * @app.
 *
 * Returns: %TRUE if there is at least one job ongoing for @app, %FALSE
 *   otherwise
 * Since: 44
 */
gboolean
gs_job_manager_app_has_pending_job_type (GsJobManager *self,
                                         GsApp        *app,
                                         GType         pending_job_type)
{
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), FALSE);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	g_return_val_if_fail (g_type_is_a (pending_job_type, GS_TYPE_PLUGIN_JOB), FALSE);

	locker = g_mutex_locker_new (&self->mutex);

	for (gsize i = 0; i < self->jobs->len; i++) {
		GsPluginJob *job = g_ptr_array_index (self->jobs, i);

		if (g_type_is_a (G_OBJECT_TYPE (job), pending_job_type) &&
		    job_contains_app (job, app))
			return TRUE;
	}

	return FALSE;
}

/**
 * gs_job_manager_add_watch:
 * @self: a #GsJobManager
 * @match_app: (nullable) (transfer none): an app to match, or %NULL to not match by app
 * @match_job_type: a job type to match, or %G_TYPE_INVALID to not match by job type
 * @added_handler: (nullable) (scope notified): function to call when a matching
 *   job is added to the manager, or %NULL to ignore
 * @removed_handler: (nullable) (scope notified): function to call when a
 *   matching job is removed from the manager or completed, or %NULL to ignore
 * @user_data: (closure): data to pass to @added_handler and @removed_handler
 * @user_data_free_func: free function for @user_data
 *
 * Add a watch for certain job types or jobs touching a particular app.
 *
 * This will cause @added_handler and @removed_handler to be called whenever a
 * matching job is added to or removed from the #GsJobManager. The callbacks
 * and @user_data_free_func will all be invoked in the #GMainContext which is
 * the thread-default at the time of calling gs_job_manager_add_watch().
 *
 * Jobs are matched against @match_app and @match_job_type, if they are set.
 * Jobs must match both filters if both are set. To match, a job must be of type
 * @match_job_type, and must be operating on @match_app.
 *
 * To remove the watch, call gs_job_manager_remove_watch() using the handle
 * which is returned by this function. All watches must be removed before the
 * #GsJobManager is finalised.
 *
 * It is possible for @added_handler and/or @removed_handler to be invoked after
 * gs_job_manager_remove_watch() is called, if the notifications are already in
 * flight when gs_job_manager_remove_watch() is called (perhaps from another
 * thread). If you need to synchronise on the watch being fully removed, use
 * @user_data_free_func.
 *
 * Returns: a handle for the watch, guaranteed to never be zero
 * Since: 44
 */
guint
gs_job_manager_add_watch (GsJobManager            *self,
                          GsApp                   *match_app,
                          GType                    match_job_type,
                          GsJobManagerJobCallback  added_handler,
                          GsJobManagerJobCallback  removed_handler,
                          gpointer                 user_data,
                          GDestroyNotify           user_data_free_func)
{
	g_autoptr(GMutexLocker) locker = NULL;
	guint watch_id;
	g_autoptr(WatchData) data = NULL;

	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), 0);
	g_return_val_if_fail (match_app == NULL || GS_IS_APP (match_app), 0);
	g_return_val_if_fail (match_job_type == G_TYPE_INVALID || g_type_is_a (match_job_type, GS_TYPE_PLUGIN_JOB), 0);

	locker = g_mutex_locker_new (&self->mutex);

	g_assert (self->next_watch_id < G_MAXUINT);
	watch_id = self->next_watch_id++;

	data = g_new0 (WatchData, 1);
	data->ref_count = 1;
	data->watch_id = watch_id;
	data->match_app_unique_id = (match_app != NULL) ? g_strdup (gs_app_get_unique_id (match_app)) : NULL;
	data->match_job_type = match_job_type;
	data->added_handler = added_handler;
	data->removed_handler = removed_handler;
	data->user_data = user_data;
	data->user_data_free_func = user_data_free_func;
	data->callback_context = g_main_context_ref_thread_default ();

	g_ptr_array_add (self->watches, g_steal_pointer (&data));

	g_assert (watch_id != 0);
	return watch_id;
}

/**
 * gs_job_manager_remove_watch:
 * @self: a #GsJobManager
 * @watch_id: a handle to a watch, returned by gs_job_manager_add_watch()
 *
 * Remove a watch previously added using gs_job_manager_add_watch().
 *
 * It is an error to call this with an invalid @watch_id.
 *
 * Since: 44
 */
void
gs_job_manager_remove_watch (GsJobManager *self,
                             guint         watch_id)
{
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_JOB_MANAGER (self));
	g_return_if_fail (watch_id != 0);

	locker = g_mutex_locker_new (&self->mutex);

	for (guint i = 0; i < self->watches->len; i++) {
		const WatchData *data = g_ptr_array_index (self->watches, i);

		if (data->watch_id == watch_id) {
			g_ptr_array_remove_index_fast (self->watches, i);
			return;
		}
	}

	g_critical ("Unknown watch ID %u in call to gs_job_manager_remove_watch()", watch_id);
}

static gpointer
copy_job_cb (gconstpointer src,
	     gpointer user_data)
{
	return g_object_ref ((gpointer) src);
}

static void
gs_job_manager_shutdown_thread (GTask *task,
				gpointer source_object,
				gpointer task_data,
				GCancellable *cancellable)
{
	GsJobManager *self = source_object;
	g_autoptr(GMutexLocker) locker = NULL;

	locker = g_mutex_locker_new (&self->mutex);

	while (self->jobs->len > 0) {
		g_autoptr(GPtrArray) jobs = g_ptr_array_copy (self->jobs, copy_job_cb, NULL);

		g_clear_pointer (&locker, g_mutex_locker_free);

		for (guint i = 0; i < jobs->len; i++) {
			GsPluginJob *job = g_ptr_array_index (jobs, i);
			gs_plugin_job_cancel (job);
		}

		locker = g_mutex_locker_new (&self->mutex);

		g_clear_pointer (&jobs, g_ptr_array_unref);

		g_cond_wait (&self->shutdown_cond, &self->mutex);
	}

	g_task_return_boolean (task, TRUE);
}

/**
 * gs_job_manager_shutdown_async:
 * @self: a #GsJobManager
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: (scope async): a callback to call when done
 * @user_data: user data for the @callback
 *
 * Shuts down all running jobs. Once called, any following
 * jobs are automatically cancelled too.
 *
 * Finish the call with gs_job_manager_shutdown_finish().
 *
 * Since: 45
 **/
void
gs_job_manager_shutdown_async (GsJobManager *self,
			       GCancellable *cancellable,
			       GAsyncReadyCallback callback,
			       gpointer user_data)
{
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GTask) task = NULL;

	g_return_if_fail (GS_IS_JOB_MANAGER (self));

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_job_manager_shutdown_async);

	locker = g_mutex_locker_new (&self->mutex);

	self->shut_down = TRUE;

	g_task_run_in_thread (task, gs_job_manager_shutdown_thread);
}

/**
 * gs_job_manager_shutdown_finish:
 * @self: a #GsJobManager
 * @result: a #GAsyncResult
 * @error: (out) (nullable): a reference to an #GError, or %NULL
 *
 * Finish the call of gs_job_manager_shutdown_async().
 *
 * Returns: %TRUE, when succeeded, or %FALSE on failure with the @error set
 *
 * Since: 45
 **/
gboolean
gs_job_manager_shutdown_finish (GsJobManager *self,
				GAsyncResult *result,
				GError **error)
{
	g_return_val_if_fail (GS_IS_JOB_MANAGER (self), FALSE);
	g_return_val_if_fail (g_task_is_valid (G_TASK (result), self), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}
