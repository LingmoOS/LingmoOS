/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2019 Endless Mobile, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-metered
 * @title: Metered Data Utilities
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Utility functions to help with metered data handling
 *
 * Metered data handling is provided by Mogwai, which implements a download
 * scheduler to control when, and in which order, large downloads happen on
 * the system.
 *
 * All large downloads from #GsPlugin.update_apps_async()
 * calls should be scheduled using Mogwai, which will notify lingmo-appstore
 * when those downloads can start and stop, according to system policy.
 *
 * The functions in this file make interacting with the scheduling daemon a
 * little simpler. Since all #GsPlugin method calls happen in worker threads,
 * typically without a #GMainContext, all interaction with the scheduler should
 * be blocking. libmogwai-schedule-client was designed to be asynchronous; so
 * these helpers make it synchronous.
 *
 * Since: 3.34
 */

#include "config.h"

#include <glib.h>

#ifdef HAVE_MOGWAI
#include <libmogwai-schedule-client/scheduler.h>
#endif

#include "gs-metered.h"
#include "gs-utils.h"


static void
async_result_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GAsyncResult **result_out = user_data;

	g_assert (*result_out == NULL);
	*result_out = g_object_ref (result);
	g_main_context_wakeup (g_main_context_get_thread_default ());
}

/**
 * gs_metered_block_on_download_scheduler:
 * @parameters: (nullable): a #GVariant of type `a{sv}` specifying parameters
 *    for the schedule entry, or %NULL to pass no parameters
 * @schedule_entry_handle_out: (out) (not optional): return location for a
 *    handle to the resulting schedule entry
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Create a schedule entry with the given @parameters, and block until
 * permission is given to download.
 *
 * FIXME: This will currently ignore later revocations of that download
 * permission, and does not support creating a schedule entry per app.
 * The schedule entry must later be removed from the schedule by passing
 * the handle from @schedule_entry_handle_out to
 * gs_metered_remove_from_download_scheduler(), otherwise resources will leak.
 * This is an opaque handle and should not be inspected.
 *
 * If a schedule entry cannot be created, or if @cancellable is cancelled,
 * an error will be set and %FALSE returned.
 *
 * The keys understood by @parameters are listed in the documentation for
 * mwsc_scheduler_schedule_async().
 *
 * This function will likely be called from a #GsPluginLoader worker thread.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 3.38
 */
gboolean
gs_metered_block_on_download_scheduler (GVariant      *parameters,
                                        gpointer      *schedule_entry_handle_out,
                                        GCancellable  *cancellable,
                                        GError       **error)
{
	g_autoptr(GMainContext) context = NULL;
	g_autoptr(GMainContextPusher) pusher = NULL;
	g_autoptr(GAsyncResult) result = NULL;

	context = g_main_context_new ();
	pusher = g_main_context_pusher_new (context);

	gs_metered_block_on_download_scheduler_async (parameters, cancellable, async_result_cb, &result);
	while (result == NULL)
		g_main_context_iteration (context, TRUE);

	return gs_metered_block_on_download_scheduler_finish (result, schedule_entry_handle_out, error);
}

#ifdef HAVE_MOGWAI
typedef struct
{
	MwscScheduleEntry *schedule_entry;  /* (owned) (not nullable) */
	gulong notify_id;
	gulong invalidated_id;
	gulong cancelled_id;
} BlockData;

static void
block_data_free (BlockData *data)
{
	g_clear_object (&data->schedule_entry);

	/* These should already have been disconnected. */
	g_assert (data->notify_id == 0);
	g_assert (data->invalidated_id == 0);
	g_assert (data->cancelled_id == 0);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (BlockData, block_data_free)

static void block_scheduler_new_cb (GObject      *source_object,
                                    GAsyncResult *result,
                                    gpointer      user_data);
static void block_scheduler_schedule_cb (GObject      *source_object,
                                         GAsyncResult *result,
                                         gpointer      user_data);
static void download_now_cb (GObject    *obj,
                             GParamSpec *pspec,
                             gpointer    user_data);
static void invalidated_cb (MwscScheduleEntry *entry,
                            const GError      *error,
                            gpointer           user_data);
static void cancelled_cb (GCancellable *cancellable,
                          gpointer      user_data);
static void block_check_cb (GTask        *task,
                            const GError *invalidated_error);
#endif  /* HAVE_MOGWAI */

/**
 * gs_metered_block_on_download_scheduler_async:
 * @parameters: (nullable): a #GVariant of type `a{sv}` specifying parameters
 *    for the schedule entry, or %NULL to pass no parameters
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback to call when the operation is finished
 * @user_data: data to pass to @callback
 *
 * Asynchronous version of gs_metered_block_on_download_scheduler().
 *
 * Since: 44
 */
void
gs_metered_block_on_download_scheduler_async (GVariant            *parameters,
                                              GCancellable        *cancellable,
                                              GAsyncReadyCallback  callback,
                                              gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(GVariant) parameters_owned = (parameters != NULL) ? g_variant_ref_sink (parameters) : NULL;
#ifdef HAVE_MOGWAI
	g_autofree gchar *parameters_str = NULL;
#endif

	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	task = g_task_new (NULL, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_metered_block_on_download_scheduler_async);

#ifdef HAVE_MOGWAI
	parameters_str = (parameters != NULL) ? g_variant_print (parameters, TRUE) : g_strdup ("(none)");
	g_debug ("%s: Waiting with parameters: %s", G_STRFUNC, parameters_str);

	g_task_set_task_data (task, g_steal_pointer (&parameters_owned), (GDestroyNotify) g_variant_unref);

	/* Wait until the download can be scheduled.
	 * FIXME: In future, downloads could be split up by app, so they can all
	 * be scheduled separately and, for example, higher priority ones could
	 * be scheduled with a higher priority. This would have to be aware of
	 * dependencies. */
	mwsc_scheduler_new_async (cancellable, block_scheduler_new_cb, g_steal_pointer (&task));
#else
	g_debug ("%s: Allowed to download (Mogwai support compiled out)", G_STRFUNC);
	g_task_return_pointer (task, NULL, NULL);
#endif
}

#ifdef HAVE_MOGWAI
static void
block_scheduler_new_cb (GObject      *source_object,
                        GAsyncResult *result,
                        gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GVariant *parameters = g_task_get_task_data (task);
	g_autoptr(MwscScheduler) scheduler = NULL;
	g_autoptr(GError) local_error = NULL;

	scheduler = mwsc_scheduler_new_finish (result, &local_error);
	if (scheduler == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Create a schedule entry for the group of downloads.
	 * FIXME: The underlying OSTree code supports resuming downloads
	 * (at a granularity of individual objects), so it should be
	 * possible to plumb through here. */
	mwsc_scheduler_schedule_async (scheduler, parameters, cancellable, block_scheduler_schedule_cb, g_steal_pointer (&task));
}

static void
block_scheduler_schedule_cb (GObject      *source_object,
                             GAsyncResult *result,
                             gpointer      user_data)
{
	MwscScheduler *scheduler = MWSC_SCHEDULER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(MwscScheduleEntry) schedule_entry = NULL;
	g_autoptr(BlockData) data = NULL;
	g_autoptr(GError) local_error = NULL;

	schedule_entry = mwsc_scheduler_schedule_finish (scheduler, result, &local_error);
	if (schedule_entry == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Wait until the download is allowed to proceed. */
	data = g_new0 (BlockData, 1);
	data->schedule_entry = g_object_ref (schedule_entry);
	data->notify_id = g_signal_connect_object (schedule_entry, "notify::download-now",
						   G_CALLBACK (download_now_cb), task, G_CONNECT_DEFAULT);
	data->invalidated_id = g_signal_connect_object (schedule_entry, "invalidated",
							G_CALLBACK (invalidated_cb), task, G_CONNECT_DEFAULT);
	data->cancelled_id = g_cancellable_connect (cancellable,
						    G_CALLBACK (cancelled_cb), g_object_ref (task), g_object_unref);
	g_task_set_task_data (task, g_steal_pointer (&data), (GDestroyNotify) block_data_free);

	/* Do the initial check. */
	block_check_cb (task, NULL);
}

static void
download_now_cb (GObject    *obj,
                 GParamSpec *pspec,
                 gpointer    user_data)
{
	block_check_cb (G_TASK (user_data), NULL);
}

static void
invalidated_cb (MwscScheduleEntry *entry,
                const GError      *error,
                gpointer           user_data)
{
	block_check_cb (G_TASK (user_data), error);
}

static void
cancelled_cb (GCancellable *cancellable,
              gpointer      user_data)
{
	block_check_cb (G_TASK (user_data), NULL);
}

static void
block_check_cb (GTask        *task_unowned,
                const GError *invalidated_error)
{
	g_autoptr(GTask) task = g_object_ref (task_unowned);
	BlockData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	gboolean download_now = FALSE;
	g_autoptr(GError) local_error = NULL;

	download_now = mwsc_schedule_entry_get_download_now (data->schedule_entry);

	/* Ignore spurious wakeups. */
	if (!download_now && invalidated_error == NULL &&
	    !g_cancellable_is_cancelled (cancellable))
		return;

	/* At this point, either the download is permitted, the
	 * #MwscScheduleEntry has been invalidated, or the operation has been
	 * cancelled. */
	g_signal_handler_disconnect (data->schedule_entry, data->invalidated_id);
	data->invalidated_id = 0;
	g_signal_handler_disconnect (data->schedule_entry, data->notify_id);
	data->notify_id = 0;
	g_cancellable_disconnect (cancellable, data->cancelled_id);
	data->cancelled_id = 0;

	if (!download_now && invalidated_error != NULL) {
		/* no need to remove the schedule entry as it’s been
		 * invalidated */
		g_task_return_error (task, g_error_copy (invalidated_error));
		return;
	} else if (!download_now && g_cancellable_set_error_if_cancelled (cancellable, &local_error)) {
		/* remove the schedule entry and fail */
		gs_metered_remove_from_download_scheduler_async (data->schedule_entry, NULL, NULL, NULL);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_assert (download_now);

	g_task_return_pointer (task, g_object_ref (data->schedule_entry), (GDestroyNotify) g_object_unref);

	g_debug ("%s: Allowed to download", G_STRFUNC);
}
#endif  /* HAVE_MOGWAI */

/**
 * gs_metered_block_on_download_scheduler_finish:
 * @result: result of the async operation
 * @schedule_entry_handle_out: (out) (not optional): return location for a
 *    handle to the resulting schedule entry
 * @error: return location for a #GError, or %NULL
 *
 * Finish function for gs_metered_block_on_download_scheduler_async().
 *
 * See gs_metered_block_on_download_scheduler().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 44
 */
gboolean
gs_metered_block_on_download_scheduler_finish (GAsyncResult  *result,
                                               gpointer      *schedule_entry_handle_out,
                                               GError       **error)
{
	g_autoptr(GError) local_error = NULL;

	g_return_val_if_fail (g_task_is_valid (result, NULL), FALSE);
	g_return_val_if_fail (schedule_entry_handle_out != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	*schedule_entry_handle_out = g_task_propagate_pointer (G_TASK (result), &local_error);

	if (local_error != NULL) {
		g_propagate_error (error, g_steal_pointer (&local_error));
		return FALSE;
	}

	return TRUE;
}

/**
 * gs_metered_remove_from_download_scheduler:
 * @schedule_entry_handle: (transfer full) (nullable): schedule entry handle as
 *    returned by gs_metered_block_on_download_scheduler()
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Remove a schedule entry previously created by
 * gs_metered_block_on_download_scheduler(). This must be called after
 * gs_metered_block_on_download_scheduler() has successfully returned, or
 * resources will leak. It should be called once the corresponding download is
 * complete.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 3.38
 */
gboolean
gs_metered_remove_from_download_scheduler (gpointer       schedule_entry_handle,
                                           GCancellable  *cancellable,
                                           GError       **error)
{
#ifdef HAVE_MOGWAI
	g_autoptr(MwscScheduleEntry) schedule_entry = schedule_entry_handle;
#endif

	g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_debug ("Removing schedule entry handle %p", schedule_entry_handle);

	if (schedule_entry_handle == NULL)
		return TRUE;

#ifdef HAVE_MOGWAI
	return mwsc_schedule_entry_remove (schedule_entry, cancellable, error);
#else
	return TRUE;
#endif
}

#ifdef HAVE_MOGWAI
static void remove_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data);
#endif

/**
 * gs_metered_remove_from_download_scheduler_async:
 * @schedule_entry_handle: (transfer full) (nullable): schedule entry handle as
 *    returned by gs_metered_block_on_download_scheduler()
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: callback to call when the operation is finished
 * @user_data: data to pass to @callback
 *
 * Asynchronous version of gs_metered_remove_from_download_scheduler().
 *
 * Since: 44
 */
void
gs_metered_remove_from_download_scheduler_async (gpointer             schedule_entry_handle,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
#ifdef HAVE_MOGWAI
	g_autoptr(MwscScheduleEntry) schedule_entry = schedule_entry_handle;
#endif

	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));

	g_debug ("Removing schedule entry handle %p", schedule_entry_handle);

	task = g_task_new (schedule_entry_handle, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_metered_remove_from_download_scheduler_async);

	if (schedule_entry_handle == NULL) {
		g_task_return_boolean (task, TRUE);
		return;
	}

#ifdef HAVE_MOGWAI
	mwsc_schedule_entry_remove_async (schedule_entry, cancellable, remove_cb, g_steal_pointer (&task));
#else
	g_task_return_boolean (task, TRUE);
#endif
}

#ifdef HAVE_MOGWAI
static void
remove_cb (GObject      *source_object,
           GAsyncResult *result,
           gpointer      user_data)
{
	MwscScheduleEntry *schedule_entry = MWSC_SCHEDULE_ENTRY (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!mwsc_schedule_entry_remove_finish (schedule_entry, result, &local_error))
		g_task_return_error (task, g_steal_pointer (&local_error));
	else
		g_task_return_boolean (task, TRUE);
}
#endif

/**
 * gs_metered_remove_from_download_scheduler_finish:
 * @schedule_entry_handle: (transfer full) (nullable): schedule entry handle as
 *    returned by gs_metered_block_on_download_scheduler()
 * result: result of the async operation
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous remove operation started with
 * gs_metered_remove_from_download_scheduler_async().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 44
 */
gboolean
gs_metered_remove_from_download_scheduler_finish (gpointer       schedule_entry_handle,
                                                  GAsyncResult  *result,
                                                  GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * gs_metered_build_scheduler_parameters_for_app:
 * @app: a #GsApp to get the scheduler parameters from
 *
 * Build a #GVariant of scheduler parameters for downloading @app.
 *
 * This is suitable to pass to gs_metered_block_on_download_scheduler() or
 * gs_metered_block_on_download_scheduler_async().
 *
 * Returns: (transfer floating) (not nullable): scheduler parameters for @app
 * Since: 44
 */
GVariant *
gs_metered_build_scheduler_parameters_for_app (GsApp *app)
{
	g_auto(GVariantDict) parameters_dict = G_VARIANT_DICT_INIT (NULL);
	guint64 download_size;

	/* Currently no plugins support resumable downloads. This may change in
	 * future, in which case this parameter should be refactored. */
	g_variant_dict_insert (&parameters_dict, "resumable", "b", FALSE);

	if (gs_app_get_size_download (app, &download_size) == GS_SIZE_TYPE_VALID) {
		g_variant_dict_insert (&parameters_dict, "size-minimum", "t", download_size);
		g_variant_dict_insert (&parameters_dict, "size-maximum", "t", download_size);
	}

	return g_variant_dict_end (&parameters_dict);
}

/**
 * gs_metered_block_app_list_on_download_scheduler:
 * @app_list: a #GsAppList to get the scheduler parameters from
 * @schedule_entry_handle_out: (out) (not optional): return location for a
 *    handle to the resulting schedule entry
 * @cancellable: a #GCancellable, or %NULL
 * @error: return location for a #GError, or %NULL
 *
 * Version of gs_metered_block_on_download_scheduler() which extracts the
 * download parameters from the apps in the given @app_list.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 3.38
 */
gboolean
gs_metered_block_app_list_on_download_scheduler (GsAppList     *app_list,
                                                 gpointer      *schedule_entry_handle_out,
                                                 GCancellable  *cancellable,
                                                 GError       **error)
{
	g_auto(GVariantDict) parameters_dict = G_VARIANT_DICT_INIT (NULL);
	g_autoptr(GVariant) parameters = NULL;

	/* Currently no plugins support resumable downloads. This may change in
	 * future, in which case this parameter should be refactored. */
	g_variant_dict_insert (&parameters_dict, "resumable", "b", FALSE);

	/* FIXME: Currently this creates a single Mogwai schedule entry for the
	 * entire app list. Eventually, we probably want one schedule entry per
	 * app being downloaded, so that they can be individually prioritised.
	 * However, that requires much deeper integration into the download
	 * code, and Mogwai does not currently support that level of
	 * prioritisation, so go with this simple implementation for now. */
	parameters = g_variant_ref_sink (g_variant_dict_end (&parameters_dict));

	return gs_metered_block_on_download_scheduler (parameters, schedule_entry_handle_out, cancellable, error);
}
