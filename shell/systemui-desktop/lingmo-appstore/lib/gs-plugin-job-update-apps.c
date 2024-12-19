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
 * SECTION:gs-plugin-job-update-apps
 * @short_description: A plugin job to update apps or the OS
 *
 * #GsPluginJobUpdateApps is a #GsPluginJob representing an operation to
 * download and/or apply updates to apps or the OS.
 *
 * This class is a wrapper around #GsPluginClass.update_apps_async(),
 * calling it for all loaded plugins.
 *
 * Unless %GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD is specified, the first step
 * of this job downloads the app and any dependencies, ready to be installed or
 * updated.
 *
 * Plugins are expected to schedule downloads using the system download
 * scheduler if appropriate (if the download is not guaranteed to be under a few
 * hundred kilobytes, for example), so that the user’s metered data preferences
 * are honoured.
 *
 * Plugins are expected to send progress notifications to the UI by calling the
 * provided #GsPluginProgressCallback function. Plugins may also call
 * gs_app_set_progress() on apps as they are updated, but this method will
 * eventually be removed as it cannot represent progress in multiple ongoing
 * operations.
 *
 * Callbacks from this job will be executed in the #GMainContext which was
 * thread-default at the time when #GsPlugin.run_async() was called on the
 * #GsPluginJobUpdateApps. For plugins, this means that callbacks must be
 * executed in the same #GMainContext which called
 * #GsPlugin.update_apps_async().
 *
 * If the app is already downloaded, the first step of this job is a no-op.
 *
 * Unless %GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY is specified, the second step of
 * this job applies the app and any dependencies (updating the app, and updating
 * dependencies or installing missing dependencies).
 *
 * Once that is completed, the apps will typically be set to the state
 * %GS_APP_STATE_INSTALLED, or %GS_APP_STATE_UNKNOWN.
 *
 * On failure the error message returned will usually only be shown on the
 * console, but they can also be retrieved using gs_plugin_loader_get_events().
 *
 * See also: #GsPluginClass.update_apps_async
 * Since: 44
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#ifdef HAVE_SYSPROF
#include <sysprof-capture.h>
#endif

#include "gs-enums.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-update-apps.h"
#include "gs-plugin-types.h"
#include "gs-profiler.h"
#include "gs-utils.h"

struct _GsPluginJobUpdateApps
{
	GsPluginJob parent;

	/* Input arguments. */
	GsAppList *apps;
	GsPluginUpdateAppsFlags flags;

	/* In-progress data. */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;
	GHashTable *plugins_progress;  /* (element-type GsPlugin guint) (owned) (nullable) */
	GSource *progress_source;  /* (owned) (nullable) */

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
};

G_DEFINE_TYPE (GsPluginJobUpdateApps, gs_plugin_job_update_apps, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_APPS = 1,
	PROP_FLAGS,
} GsPluginJobUpdateAppsProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

typedef enum {
	SIGNAL_APP_NEEDS_USER_ACTION,
	SIGNAL_PROGRESS,
} GsPluginJobUpdateAppsSignal;

static guint signals[SIGNAL_PROGRESS + 1] = { 0, };

static void
gs_plugin_job_update_apps_dispose (GObject *object)
{
	GsPluginJobUpdateApps *self = GS_PLUGIN_JOB_UPDATE_APPS (object);

	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* Progress reporting should have been stopped by now. */
	if (self->progress_source != NULL) {
		g_assert (g_source_is_destroyed (self->progress_source));
		g_clear_pointer (&self->progress_source, g_source_unref);
	}

	g_clear_pointer (&self->plugins_progress, g_hash_table_unref);
	g_clear_object (&self->apps);

	G_OBJECT_CLASS (gs_plugin_job_update_apps_parent_class)->dispose (object);
}

static void
gs_plugin_job_update_apps_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
	GsPluginJobUpdateApps *self = GS_PLUGIN_JOB_UPDATE_APPS (object);

	switch ((GsPluginJobUpdateAppsProperty) prop_id) {
	case PROP_APPS:
		g_value_set_object (value, self->apps);
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
gs_plugin_job_update_apps_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
	GsPluginJobUpdateApps *self = GS_PLUGIN_JOB_UPDATE_APPS (object);

	switch ((GsPluginJobUpdateAppsProperty) prop_id) {
	case PROP_APPS:
		/* Construct only. */
		g_assert (self->apps == NULL);
		self->apps = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_FLAGS:
		/* Construct only. */
		g_assert (self->flags == 0);
		self->flags = g_value_get_flags (value);

		/* Perhaps we could eventually allow both of these to be
		 * specified at the same time, but for now it would over
		 * complicate the implementation of plugins, for no benefit. */
		g_assert (!(self->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD) ||
			  !(self->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY));

		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
app_needs_user_action_cb (GsPlugin     *plugin,
                          GsApp        *app,
                          AsScreenshot *action_screenshot,
                          gpointer      user_data)
{
	GTask *task = G_TASK (user_data);
	GsPluginJobUpdateApps *self = g_task_get_source_object (task);

	g_assert (g_main_context_is_owner (g_task_get_context (task)));
	g_signal_emit (self, signals[SIGNAL_APP_NEEDS_USER_ACTION], 0, app, action_screenshot);
}

static void plugin_progress_cb (GsPlugin *plugin,
                                guint     progress,
                                gpointer  user_data);
static gboolean progress_cb (gpointer user_data);
static void plugin_update_apps_cb (GObject      *source_object,
                                   GAsyncResult *result,
                                   gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);

static void
gs_plugin_job_update_apps_run_async (GsPluginJob         *job,
                                     GsPluginLoader      *plugin_loader,
                                     GCancellable        *cancellable,
                                     GAsyncReadyCallback  callback,
                                     gpointer             user_data)
{
	GsPluginJobUpdateApps *self = GS_PLUGIN_JOB_UPDATE_APPS (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean any_plugins_ran = FALSE;
	g_autoptr(GError) local_error = NULL;

	/* Chosen to allow a few UI updates per second without updating the
	 * progress label so often it’s unreadable. */
	const guint progress_update_period_ms = 300;

	/* check required args */
	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_update_apps_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* Set up the progress timeout. This periodically sums up the progress
	 * tuples in `self->*_progress` and reports them to the calling
	 * function via the #GsPluginJobUpdateApps::progress signal, giving
	 * an overall progress for all the parallel operations. */
	self->plugins_progress = g_hash_table_new (g_direct_hash, g_direct_equal);
	self->progress_source = g_timeout_source_new (progress_update_period_ms);
	g_source_set_callback (self->progress_source, progress_cb, g_object_ref (self), g_object_unref);
	g_source_attach (self->progress_source, g_main_context_get_thread_default ());

	/* run each plugin, keeping a counter of pending operations which is
	 * initialised to 1 until all the operations are started */
	self->n_pending_ops = 1;
	plugins = gs_plugin_loader_get_plugins (plugin_loader);

#ifdef HAVE_SYSPROF
	self->begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->update_apps_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		any_plugins_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* Set up progress reporting for this plugin. */
		g_hash_table_insert (self->plugins_progress, plugin, GUINT_TO_POINTER (0));

		/* run the plugin */
		self->n_pending_ops++;
		plugin_class->update_apps_async (plugin,
						 self->apps,
						 self->flags,
						 plugin_progress_cb,
						 task,
						 app_needs_user_action_cb,
						 task,
						 cancellable,
						 plugin_update_apps_cb,
						 g_object_ref (task));
	}

	/* some functions are really required for proper operation */
	if (!any_plugins_ran) {
		g_set_error_literal (&local_error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_NOT_SUPPORTED,
				     "no plugin could handle updating apps");
	}

	finish_op (task, g_steal_pointer (&local_error));
}

/* Called in the same thread as gs_plugin_job_update_apps_run_async(), to
 * report the progress for the given plugin. */
static void
plugin_progress_cb (GsPlugin *plugin,
                    guint     progress,
                    gpointer  user_data)
{
	GTask *task = G_TASK (user_data);
	GsPluginJobUpdateApps *self = g_task_get_source_object (task);

	g_assert (g_main_context_is_owner (g_task_get_context (task)));
	g_hash_table_replace (self->plugins_progress, plugin, GUINT_TO_POINTER (progress));
}

static gboolean
progress_cb (gpointer user_data)
{
	GsPluginJobUpdateApps *self = GS_PLUGIN_JOB_UPDATE_APPS (user_data);
	gdouble progress;
	guint n_portions;
	GHashTableIter iter;
	gpointer plugin_progress_ptr;
	gboolean all_unknown = TRUE;

	/* Sum up the progress for all parallel operations.
	 *
	 * Allocate each operation an equal portion of 100 percentage points. In
	 * this context, an operation is a call to a plugin’s
	 * update_apps_async() vfunc. */
	n_portions = g_hash_table_size (self->plugins_progress);
	progress = 0.0;
	g_hash_table_iter_init (&iter, self->plugins_progress);

	while (g_hash_table_iter_next (&iter, NULL, &plugin_progress_ptr)) {
		guint plugin_progress = GPOINTER_TO_UINT (plugin_progress_ptr);

		if (plugin_progress == GS_APP_PROGRESS_UNKNOWN)
			continue;
		else
			all_unknown = FALSE;

		progress += (100.0 / n_portions) * ((gdouble) plugin_progress / 100.0);
	}

	if (all_unknown)
		progress = GS_APP_PROGRESS_UNKNOWN;

	/* Report progress via signal emission. */
	/* FIXME: In future we could add explicit signals to notify that a
	 * download operation is blocked on waiting for metered data permission
	 * to download, so the UI can represent that better. */
	g_signal_emit (self, signals[SIGNAL_PROGRESS], 0, (guint) progress);

	return G_SOURCE_CONTINUE;
}

static void
plugin_update_apps_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobUpdateApps *self = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	/* Forward cancellation errors, but ignore all other errors so
	 * that other plugins don’t get blocked.
	 *
	 * If plugins produce errors which should be reported to the user, they
	 * should report them directly by calling gs_plugin_report_event().
	 * #GsPluginJobUpdateApps cannot do this as it doesn’t know which errors
	 * are interesting to the user and which are useless. */
	if (!plugin_class->update_apps_finish (plugin, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("Plugin ‘%s’ failed to update apps: %s",
			 gs_plugin_get_name (plugin), local_error->message);
		g_clear_error (&local_error);
	}

	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	GS_PROFILER_ADD_MARK_TAKE (PluginJobUpdateApps,
				   self->begin_time_nsec,
				   g_strdup_printf ("%s:%s",
						    G_OBJECT_TYPE_NAME (self),
						    gs_plugin_get_name (plugin)),
				   NULL);

	/* Update progress reporting. */
	g_hash_table_replace (self->plugins_progress, plugin, GUINT_TO_POINTER (100));

	finish_op (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobUpdateApps *self = g_task_get_source_object (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	g_autofree gchar *job_debug = NULL;

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while updating apps: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* Emit one final progress update, then stop any further ones.
	 * Ensure the emission is in the right #GMainContext. */
	g_assert (g_main_context_is_owner (g_task_get_context (task)));
	progress_cb (self);
	g_source_destroy (self->progress_source);
	g_clear_pointer (&self->plugins_progress, g_hash_table_unref);

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

	GS_PROFILER_ADD_MARK (PluginJobUpdateApps,
			      self->begin_time_nsec,
			      G_OBJECT_TYPE_NAME (self),
			      NULL);
}

static gboolean
gs_plugin_job_update_apps_run_finish (GsPluginJob   *self,
                                      GAsyncResult  *result,
                                      GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_update_apps_class_init (GsPluginJobUpdateAppsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_update_apps_dispose;
	object_class->get_property = gs_plugin_job_update_apps_get_property;
	object_class->set_property = gs_plugin_job_update_apps_set_property;

	job_class->run_async = gs_plugin_job_update_apps_run_async;
	job_class->run_finish = gs_plugin_job_update_apps_run_finish;

	/**
	 * GsPluginJobUpdateApps:apps:
	 *
	 * List of apps to update.
	 *
	 * Since: 44
	 */
	props[PROP_APPS] =
		g_param_spec_object ("apps", "Apps",
				     "List of apps to update.",
				     GS_TYPE_APP_LIST,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobUpdateApps:flags:
	 *
	 * Flags to specify how the update job should behave.
	 *
	 * Currently, it is forbidden to specify both
	 * %GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD and
	 * %GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY at the same time.
	 *
	 * Since: 44
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how the update job should behave.",
				    GS_TYPE_PLUGIN_UPDATE_APPS_FLAGS, GS_PLUGIN_UPDATE_APPS_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);

	/**
	 * GsPluginJobUpdateApps::app-needs-user-action:
	 * @app: (not nullable): the app which needs user action
	 * @action_screenshot: (not nullable): an image and caption explaining what action is needed
	 *
	 * Emitted during #GsPluginJob.run_async() if an app needs user action
	 * to upgrade.
	 *
	 * This is typically used for firmware upgrades where a piece of
	 * hardware needs user interaction to accept a firmware upgrade, such as
	 * being turned on and off, or having a button pressed.
	 *
	 * The image in @action_screenshot should explain to the user what to do
	 * to the device.
	 *
	 * It’s emitted in the thread which is running the #GMainContext which
	 * was the thread-default context when #GsPluginJob.run_async() was
	 * called.
	 *
	 * Since: 44
	 */
	signals[SIGNAL_APP_NEEDS_USER_ACTION] =
		g_signal_new ("app-needs-user-action",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_generic,
			      G_TYPE_NONE, 2, GS_TYPE_APP, AS_TYPE_SCREENSHOT);

	/**
	 * GsPluginJobUpdateApps::progress:
	 * @progress_percent: percentage completion of the job, [0, 100], or
	 *   %G_MAXUINT to indicate that progress is unknown
	 *
	 * Emitted during #GsPluginJob.run_async() when progress is made.
	 *
	 * It’s emitted in the thread which is running the #GMainContext which
	 * was the thread-default context when #GsPluginJob.run_async() was
	 * called.
	 *
	 * Since: 44
	 */
	signals[SIGNAL_PROGRESS] =
		g_signal_new ("progress",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__UINT,
			      G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void
gs_plugin_job_update_apps_init (GsPluginJobUpdateApps *self)
{
}

/**
 * gs_plugin_job_update_apps_new:
 * @apps: (transfer none) (not nullable): list of apps to update
 * @flags: flags to affect the update
 *
 * Create a new #GsPluginJobUpdateApps for updating apps, or pre-downloading
 * updates to apps.
 *
 * Returns: (transfer full): a new #GsPluginJobUpdateApps
 * Since: 44
 */
GsPluginJob *
gs_plugin_job_update_apps_new (GsAppList               *apps,
                               GsPluginUpdateAppsFlags  flags)
{
	return g_object_new (GS_TYPE_PLUGIN_JOB_UPDATE_APPS,
			     "apps", apps,
			     "flags", flags,
			     NULL);
}

/**
 * gs_plugin_job_update_apps_get_apps:
 * @self: a #GsPluginJobUpdateApps
 *
 * Get the set of apps being updated by this #GsPluginJobUpdateApps.
 *
 * Returns: apps being updated
 * Since: 44
 */
GsAppList *
gs_plugin_job_update_apps_get_apps (GsPluginJobUpdateApps *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_UPDATE_APPS (self), NULL);

	return self->apps;
}

/**
 * gs_plugin_job_update_apps_get_flags:
 * @self: a #GsPluginJobUpdateApps
 *
 * Get the flags affecting the behaviour of this #GsPluginJobUpdateApps.
 *
 * Returns: flags for the job
 * Since: 44
 */
GsPluginUpdateAppsFlags
gs_plugin_job_update_apps_get_flags (GsPluginJobUpdateApps *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_UPDATE_APPS (self), GS_PLUGIN_UPDATE_APPS_FLAGS_NONE);

	return self->flags;
}
