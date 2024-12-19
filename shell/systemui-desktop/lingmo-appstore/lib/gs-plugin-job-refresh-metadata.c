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
 * SECTION:gs-plugin-job-refresh-metadata
 * @short_description: A plugin job to refresh metadata
 *
 * #GsPluginJobRefreshMetadata is a #GsPluginJob representing an operation to
 * refresh metadata inside plugins and about apps.
 *
 * For example, the metadata could be the list of apps available, or
 * the list of updates, or a new set of popular apps to highlight.
 *
 * The maximum cache age should be set using
 * #GsPluginJobRefreshMetadata:cache-age-secs. If this is not a low value, this
 * job is not expected to do much work. Set it to zero to force all caches to be
 * refreshed.
 *
 * This class is a wrapper around #GsPluginClass.refresh_metadata_async(),
 * calling it for all loaded plugins. In addition it will refresh ODRS data on
 * the #GsOdrsProvider set on the #GsPluginLoader.
 *
 * Once the refresh is complete, signals may be asynchronously emitted on
 * plugins, apps and the #GsPluginLoader to indicate what metadata or sets of
 * apps have changed.
 *
 * See also: #GsPluginClass.refresh_metadata_async
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#ifdef HAVE_SYSPROF
#include <sysprof-capture.h>
#endif

#include "gs-enums.h"
#include "gs-external-appstream-utils.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-refresh-metadata.h"
#include "gs-plugin-types.h"
#include "gs-profiler.h"
#include "gs-odrs-provider.h"
#include "gs-utils.h"

/* A tuple to store the last-received progress data for a single download.
 * See progress_cb() for more details. */
typedef struct {
	gsize bytes_downloaded;
	gsize total_download_size;
} ProgressTuple;

struct _GsPluginJobRefreshMetadata
{
	GsPluginJob parent;

	/* Input arguments. */
	guint64 cache_age_secs;
	GsPluginRefreshMetadataFlags flags;

	/* In-progress data. */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;
#ifdef ENABLE_EXTERNAL_APPSTREAM
	ProgressTuple external_appstream_progress;
#endif
	ProgressTuple odrs_progress;
	struct {
		guint n_plugins;
		guint n_plugins_complete;
	} plugins_progress;
	GSource *progress_source;  /* (owned) (nullable) */

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
};

G_DEFINE_TYPE (GsPluginJobRefreshMetadata, gs_plugin_job_refresh_metadata, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_CACHE_AGE_SECS = 1,
	PROP_FLAGS,
} GsPluginJobRefreshMetadataProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

typedef enum {
	SIGNAL_PROGRESS,
} GsPluginJobRefreshMetadataSignal;

static guint signals[SIGNAL_PROGRESS + 1] = { 0, };

static void
gs_plugin_job_refresh_metadata_dispose (GObject *object)
{
	GsPluginJobRefreshMetadata *self = GS_PLUGIN_JOB_REFRESH_METADATA (object);

	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* Progress reporting should have been stopped by now. */
	if (self->progress_source != NULL) {
		g_assert (g_source_is_destroyed (self->progress_source));
		g_clear_pointer (&self->progress_source, g_source_unref);
	}

	G_OBJECT_CLASS (gs_plugin_job_refresh_metadata_parent_class)->dispose (object);
}

static void
gs_plugin_job_refresh_metadata_get_property (GObject    *object,
                                             guint       prop_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
	GsPluginJobRefreshMetadata *self = GS_PLUGIN_JOB_REFRESH_METADATA (object);

	switch ((GsPluginJobRefreshMetadataProperty) prop_id) {
	case PROP_CACHE_AGE_SECS:
		g_value_set_uint64 (value, self->cache_age_secs);
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
gs_plugin_job_refresh_metadata_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
	GsPluginJobRefreshMetadata *self = GS_PLUGIN_JOB_REFRESH_METADATA (object);

	switch ((GsPluginJobRefreshMetadataProperty) prop_id) {
	case PROP_CACHE_AGE_SECS:
		/* Construct only. */
		g_assert (self->cache_age_secs == 0);
		self->cache_age_secs = g_value_get_uint64 (value);
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

static void refresh_progress_tuple_cb (gsize    bytes_downloaded,
                                       gsize    total_download_size,
                                       gpointer user_data);
static gboolean progress_cb (gpointer user_data);
#ifdef ENABLE_EXTERNAL_APPSTREAM
static void external_appstream_refresh_cb (GObject      *source_object,
                                           GAsyncResult *result,
                                           gpointer      user_data);
#endif
static void odrs_provider_refresh_ratings_cb (GObject      *source_object,
                                              GAsyncResult *result,
                                              gpointer      user_data);
static void plugin_refresh_metadata_cb (GObject      *source_object,
                                        GAsyncResult *result,
                                        gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);

static void
gs_plugin_job_refresh_metadata_run_async (GsPluginJob         *job,
                                          GsPluginLoader      *plugin_loader,
                                          GCancellable        *cancellable,
                                          GAsyncReadyCallback  callback,
                                          gpointer             user_data)
{
	GsPluginJobRefreshMetadata *self = GS_PLUGIN_JOB_REFRESH_METADATA (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean any_plugins_ran = FALSE;
	GsOdrsProvider *odrs_provider;
	g_autoptr(GError) local_error = NULL;

	/* Chosen to allow a few UI updates per second without updating the
	 * progress label so often it’s unreadable. */
	const guint progress_update_period_ms = 300;

	/* check required args */
	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_refresh_metadata_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* Set up the progress timeout. This periodically sums up the progress
	 * tuples in `self->*_progress` and reports them to the calling
	 * function via the #GsPluginJobRefreshMetadata::progress signal, giving
	 * an overall progress for all the parallel operations. */
	self->progress_source = g_timeout_source_new (progress_update_period_ms);
	g_source_set_callback (self->progress_source, progress_cb, g_object_ref (self), g_object_unref);
	g_source_attach (self->progress_source, g_main_context_get_thread_default ());

	/* run each plugin, keeping a counter of pending operations which is
	 * initialised to 1 until all the operations are started */
	self->n_pending_ops = 1;
	plugins = gs_plugin_loader_get_plugins (plugin_loader);
	odrs_provider = gs_plugin_loader_get_odrs_provider (plugin_loader);

	/* Start downloading updated external appstream before anything else */
#ifdef ENABLE_EXTERNAL_APPSTREAM
	if (!g_cancellable_is_cancelled (cancellable)) {
		self->n_pending_ops++;
		gs_external_appstream_refresh_async (self->cache_age_secs,
						     refresh_progress_tuple_cb,
						     &self->external_appstream_progress,
						     cancellable,
						     external_appstream_refresh_cb,
						     g_object_ref (task));
	}
#endif

#ifdef HAVE_SYSPROF
	self->begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->refresh_metadata_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		any_plugins_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* Set up progress reporting for this plugin. */
		self->plugins_progress.n_plugins++;

		/* run the plugin */
		self->n_pending_ops++;
		plugin_class->refresh_metadata_async (plugin,
						      self->cache_age_secs,
						      self->flags,
						      cancellable,
						      plugin_refresh_metadata_cb,
						      g_object_ref (task));
	}

	if (odrs_provider != NULL &&
	    !g_cancellable_is_cancelled (cancellable)) {
		self->n_pending_ops++;
		gs_odrs_provider_refresh_ratings_async (odrs_provider,
							self->cache_age_secs,
							refresh_progress_tuple_cb,
							&self->odrs_progress,
							cancellable,
							odrs_provider_refresh_ratings_cb,
							g_object_ref (task));
	}

	/* some functions are really required for proper operation */
	if (!any_plugins_ran) {
		g_set_error_literal (&local_error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no plugin could handle refreshing");
	}

	finish_op (task, g_steal_pointer (&local_error));
}

static void
refresh_progress_tuple_cb (gsize    bytes_downloaded,
                           gsize    total_download_size,
                           gpointer user_data)
{
	ProgressTuple *tuple = user_data;

	tuple->bytes_downloaded = bytes_downloaded;
	tuple->total_download_size = total_download_size;

	/* The timeout callback in progress_cb() periodically sums these. No
	 * need to notify of progress from here. */
}

static gboolean
progress_cb (gpointer user_data)
{
	GsPluginJobRefreshMetadata *self = GS_PLUGIN_JOB_REFRESH_METADATA (user_data);
#ifdef ENABLE_EXTERNAL_APPSTREAM
	gdouble external_appstream_completion = 0.0;
#endif
	gdouble odrs_completion = 0.0;
	gdouble progress;
	guint n_portions;

	/* Sum up the progress for all parallel operations. This is complicated
	 * by the fact that external-appstream and ODRS operations report their
	 * progress in terms of bytes downloaded, but the other operations are
	 * just a counter.
	 *
	 * There is further complication from the fact that external-appstream
	 * support can be compiled out.
	 *
	 * Allocate each operation an equal portion of 100 percentage points. In
	 * this context, an operation is either a call to a plugin’s
	 * refresh_metadata_async() vfunc, or an external-appstream or ODRS
	 * refresh. */
	n_portions = self->plugins_progress.n_plugins;

#ifdef ENABLE_EXTERNAL_APPSTREAM
	if (self->external_appstream_progress.total_download_size > 0)
		external_appstream_completion = (self->external_appstream_progress.bytes_downloaded /
						 self->external_appstream_progress.total_download_size);
	n_portions++;
#endif

	if (self->odrs_progress.total_download_size > 0)
		odrs_completion = (self->odrs_progress.bytes_downloaded /
				   self->odrs_progress.total_download_size);
	n_portions++;

	/* Report progress via signal emission. */
	progress = (100.0 / n_portions) * (self->plugins_progress.n_plugins_complete + odrs_completion);
#ifdef ENABLE_EXTERNAL_APPSTREAM
	progress += (100.0 / n_portions) * external_appstream_completion;
#endif

	g_signal_emit (self, signals[SIGNAL_PROGRESS], 0, (guint) progress);

	return G_SOURCE_CONTINUE;
}

#ifdef ENABLE_EXTERNAL_APPSTREAM
static void
external_appstream_refresh_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data)
{
	g_autoptr(GTask) task = G_TASK (user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_external_appstream_refresh_finish (result, &local_error))
		g_debug ("Failed to refresh external appstream: %s", local_error->message);
	/* Intentionally ignore errors, to not block other plugins */
	finish_op (task, NULL);
}
#endif  /* ENABLE_EXTERNAL_APPSTREAM */

static void
odrs_provider_refresh_ratings_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data)
{
	GsOdrsProvider *odrs_provider = GS_ODRS_PROVIDER (source_object);
	g_autoptr(GTask) task = G_TASK (user_data);
	g_autoptr(GError) local_error = NULL;
#ifdef HAVE_SYSPROF
	GsPluginJobRefreshMetadata *self = g_task_get_source_object (task);
#endif

	if (!gs_odrs_provider_refresh_ratings_finish (odrs_provider, result, &local_error))
		g_debug ("Failed to refresh ratings: %s", local_error->message);

	GS_PROFILER_ADD_MARK_TAKE (PluginJobRefreshMetadata,
				   self->begin_time_nsec,
				   g_strdup_printf ("%s:odrs", G_OBJECT_TYPE_NAME (self)),
				   NULL);

	/* Intentionally ignore errors, to not block other plugins */
	finish_op (task, NULL);
}

static void
plugin_refresh_metadata_cb (GObject      *source_object,
                            GAsyncResult *result,
                            gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobRefreshMetadata *self = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	if (!plugin_class->refresh_metadata_finish (plugin, result, &local_error))
		g_debug ("Failed to refresh plugin '%s': %s", gs_plugin_get_name (plugin), local_error->message);
	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	/* Update progress reporting. */
	self->plugins_progress.n_plugins_complete++;

	GS_PROFILER_ADD_MARK_TAKE (PluginJobRefreshMetadata,
				   self->begin_time_nsec,
				   g_strdup_printf ("%s:%s",
						    G_OBJECT_TYPE_NAME (self),
						    gs_plugin_get_name (plugin)),
				   NULL);

	/* Intentionally ignore errors, to not block other plugins */
	finish_op (task, NULL);
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobRefreshMetadata *self = g_task_get_source_object (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	g_autofree gchar *job_debug = NULL;

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while refreshing metadata: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* Emit one final progress update, then stop any further ones.
	 * Ensure the emission is in the right #GMainContext. */
	g_assert (g_main_context_is_owner (g_task_get_context (task)));
	progress_cb (self);
	g_source_destroy (self->progress_source);

	/* Get the results of the parallel ops. */
	if (self->saved_error != NULL) {
		g_task_return_error (task, g_steal_pointer (&self->saved_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	/* Check the intermediate working values are all cleared. */
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* success */
	g_task_return_boolean (task, TRUE);
	g_signal_emit_by_name (G_OBJECT (self), "completed");

	GS_PROFILER_ADD_MARK (PluginJobRefreshMetadata,
			      self->begin_time_nsec,
			      G_OBJECT_TYPE_NAME (self),
			      NULL);
}

static gboolean
gs_plugin_job_refresh_metadata_run_finish (GsPluginJob   *self,
                                           GAsyncResult  *result,
                                           GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_refresh_metadata_class_init (GsPluginJobRefreshMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_refresh_metadata_dispose;
	object_class->get_property = gs_plugin_job_refresh_metadata_get_property;
	object_class->set_property = gs_plugin_job_refresh_metadata_set_property;

	job_class->run_async = gs_plugin_job_refresh_metadata_run_async;
	job_class->run_finish = gs_plugin_job_refresh_metadata_run_finish;

	/**
	 * GsPluginJobRefreshMetadata:cache-age-secs:
	 *
	 * Maximum age of caches before they are refreshed.
	 *
	 * Since: 42
	 */
	props[PROP_CACHE_AGE_SECS] =
		g_param_spec_uint64 ("cache-age-secs", "Cache Age",
				     "Maximum age of caches before they are refreshed.",
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobRefreshMetadata:flags:
	 *
	 * Flags to specify how the refresh job should behave.
	 *
	 * Since: 42
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how the refresh job should behave.",
				    GS_TYPE_PLUGIN_REFRESH_METADATA_FLAGS, GS_PLUGIN_REFRESH_METADATA_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);

	/**
	 * GsPluginJobRefreshMetadata::progress:
	 * @progress_percent: percentage completion of the job, [0, 100], or
	 *   %G_MAXUINT to indicate that progress is unknown
	 *
	 * Emitted during #GsPluginJob.run_async() when progress is made.
	 *
	 * It’s emitted in the thread which is running the #GMainContext which
	 * was the thread-default context when #GsPluginJob.run_async() was
	 * called.
	 *
	 * Since: 42
	 */
	signals[SIGNAL_PROGRESS] =
		g_signal_new ("progress",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void
gs_plugin_job_refresh_metadata_init (GsPluginJobRefreshMetadata *self)
{
}

/**
 * gs_plugin_job_refresh_metadata_new:
 * @cache_age_secs: maximum allowed cache age, in seconds
 * @flags: flags to affect the refresh
 *
 * Create a new #GsPluginJobRefreshMetadata for refreshing metadata about
 * available apps.
 *
 * Caches will be refreshed if they are older than @cache_age_secs.
 *
 * Returns: (transfer full): a new #GsPluginJobRefreshMetadata
 * Since: 42
 */
GsPluginJob *
gs_plugin_job_refresh_metadata_new (guint64                      cache_age_secs,
                                    GsPluginRefreshMetadataFlags flags)
{
	return g_object_new (GS_TYPE_PLUGIN_JOB_REFRESH_METADATA,
			     "cache-age-secs", cache_age_secs,
			     "flags", flags,
			     NULL);
}
