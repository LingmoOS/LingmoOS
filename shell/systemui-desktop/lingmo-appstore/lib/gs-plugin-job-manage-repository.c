/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-plugin-job-manage-repository
 * @short_description: A plugin job on a repository
 *
 * #GsPluginJobManageRepository is a #GsPluginJob representing an operation on
 * a repository, like install, remove, enable and disable it.
 *
 * This class is a wrapper around #GsPluginClass.install_repository_async,
 * #GsPluginClass.remove_repository_async, #GsPluginClass.enable_repository_async
 * and #GsPluginClass.disable_repository_async calling it for all loaded plugins.
 *
 * Since: 43
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-app.h"
#include "gs-app-collation.h"
#include "gs-enums.h"
#include "gs-plugin-job.h"
#include "gs-plugin-job-manage-repository.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-types.h"

struct _GsPluginJobManageRepository
{
	GsPluginJob parent;

	/* Input arguments. */
	GsApp *repository;  /* (owned) (not nullable) */
	GsPluginManageRepositoryFlags flags;

	/* In-progress data. */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;
};

G_DEFINE_TYPE (GsPluginJobManageRepository, gs_plugin_job_manage_repository, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_FLAGS = 1,
	PROP_REPOSITORY,
} GsPluginJobManageRepositoryProperty;

static GParamSpec *props[PROP_REPOSITORY + 1] = { NULL, };

static void
gs_plugin_job_manage_repository_dispose (GObject *object)
{
	GsPluginJobManageRepository *self = GS_PLUGIN_JOB_MANAGE_REPOSITORY (object);

	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	g_clear_object (&self->repository);

	G_OBJECT_CLASS (gs_plugin_job_manage_repository_parent_class)->dispose (object);
}

static void
gs_plugin_job_manage_repository_get_property (GObject    *object,
					      guint       prop_id,
					      GValue     *value,
					      GParamSpec *pspec)
{
	GsPluginJobManageRepository *self = GS_PLUGIN_JOB_MANAGE_REPOSITORY (object);

	switch ((GsPluginJobManageRepositoryProperty) prop_id) {
	case PROP_FLAGS:
		g_value_set_flags (value, self->flags);
		break;
	case PROP_REPOSITORY:
		g_value_set_object (value, self->repository);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_job_manage_repository_set_property (GObject      *object,
					      guint         prop_id,
					      const GValue *value,
					      GParamSpec   *pspec)
{
	GsPluginJobManageRepository *self = GS_PLUGIN_JOB_MANAGE_REPOSITORY (object);

	switch ((GsPluginJobManageRepositoryProperty) prop_id) {
	case PROP_FLAGS:
		/* Construct only. */
		g_assert (self->flags == 0);
		self->flags = g_value_get_flags (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_REPOSITORY:
		/* Construct only. */
		g_assert (self->repository == NULL);
		self->repository = g_value_dup_object (value);
		g_assert (self->repository != NULL);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void plugin_repository_func_cb (GObject      *source_object,
				       GAsyncResult *result,
				       gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);

static void
gs_plugin_job_manage_repository_run_async (GsPluginJob         *job,
					   GsPluginLoader      *plugin_loader,
					   GCancellable        *cancellable,
					   GAsyncReadyCallback  callback,
					   gpointer             user_data)
{
	GsPluginJobManageRepository *self = GS_PLUGIN_JOB_MANAGE_REPOSITORY (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean anything_ran = FALSE;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_manage_repository_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* run each plugin, keeping a counter of pending operations which is
	 * initialised to 1 until all the operations are started */
	self->n_pending_ops = 1;
	plugins = gs_plugin_loader_get_plugins (plugin_loader);

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
		void (* repository_func_async) (GsPlugin *plugin,
						GsApp *repository,
						GsPluginManageRepositoryFlags flags,
						GCancellable *cancellable,
						GAsyncReadyCallback callback,
						gpointer user_data) = NULL;

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL) != 0)
			repository_func_async = plugin_class->install_repository_async;
		else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE) != 0)
			repository_func_async = plugin_class->remove_repository_async;
		else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE) != 0)
			repository_func_async = plugin_class->enable_repository_async;
		else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE) != 0)
			repository_func_async = plugin_class->disable_repository_async;
		else
			g_assert_not_reached ();

		if (repository_func_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* run the plugin */
		self->n_pending_ops++;
		repository_func_async (plugin, self->repository, self->flags, cancellable, plugin_repository_func_cb, g_object_ref (task));
	}

	if (!anything_ran)
		g_debug ("no plugin could handle repository operation");

	finish_op (task, g_steal_pointer (&local_error));
}

static void
plugin_repository_func_cb (GObject      *source_object,
			   GAsyncResult *result,
			   gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginJobManageRepository *self = g_task_get_source_object (task);
	gboolean success;
	g_autoptr(GError) local_error = NULL;
	gboolean (* repository_func_finish) (GsPlugin *plugin,
					     GAsyncResult *result,
					     GError **error) = NULL;

	if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL) != 0)
		repository_func_finish = plugin_class->install_repository_finish;
	else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE) != 0)
		repository_func_finish = plugin_class->remove_repository_finish;
	else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE) != 0)
		repository_func_finish = plugin_class->enable_repository_finish;
	else if ((self->flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE) != 0)
		repository_func_finish = plugin_class->disable_repository_finish;
	else
		g_assert_not_reached ();

	success = repository_func_finish (plugin, result, &local_error);
	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	g_assert (success || local_error != NULL);

	finish_op (task, g_steal_pointer (&local_error));
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

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobManageRepository *self = g_task_get_source_object (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	g_autofree gchar *job_debug = NULL;

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while managing repository: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	reset_app_progress (self->repository);

	if (self->saved_error != NULL)
		g_task_return_error (task, g_steal_pointer (&self->saved_error));
	else
		g_task_return_boolean (task, TRUE);
	g_signal_emit_by_name (G_OBJECT (self), "completed");
}

static gboolean
gs_plugin_job_manage_repository_run_finish (GsPluginJob   *self,
					    GAsyncResult  *result,
					    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_manage_repository_class_init (GsPluginJobManageRepositoryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_manage_repository_dispose;
	object_class->get_property = gs_plugin_job_manage_repository_get_property;
	object_class->set_property = gs_plugin_job_manage_repository_set_property;

	job_class->run_async = gs_plugin_job_manage_repository_run_async;
	job_class->run_finish = gs_plugin_job_manage_repository_run_finish;

	/**
	 * GsPluginJobManageRepository:repository: (not nullable)
	 *
	 * A #GsApp describing the repository to run the operation on.
	 *
	 * Since: 43
	 */
	props[PROP_REPOSITORY] =
		g_param_spec_object ("repository", "Repository",
				     "A #GsApp describing the repository to run the operation on.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobManageRepository:flags:
	 *
	 * Flags to specify how and which the operation should run.
	 * Only one of the %GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL,
	 * %GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE, %GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE and
	 * %GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE can be specified.
	 *
	 * Since: 43
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how and which the operation should run.",
				    GS_TYPE_PLUGIN_MANAGE_REPOSITORY_FLAGS,
				    GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_job_manage_repository_init (GsPluginJobManageRepository *self)
{
}

/**
 * gs_plugin_job_manage_repository_new:
 * @repository: (not nullable) (transfer none): a repository to run the operation on
 * @flags: flags affecting how the operation runs
 *
 * Create a new #GsPluginJobManageRepository to manage the given @repository.
 *
 * Returns: (transfer full): a new #GsPluginJobManageRepository
 * Since: 43
 */
GsPluginJob *
gs_plugin_job_manage_repository_new (GsApp			   *repository,
				     GsPluginManageRepositoryFlags  flags)
{
	guint nops = 0;

	g_return_val_if_fail (GS_IS_APP (repository), NULL);

	if ((flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_INSTALL) != 0)
		nops++;
	if ((flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_REMOVE) != 0)
		nops++;
	if ((flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_ENABLE) != 0)
		nops++;
	if ((flags & GS_PLUGIN_MANAGE_REPOSITORY_FLAGS_DISABLE) != 0)
		nops++;

	g_return_val_if_fail (nops == 1, NULL);

	return g_object_new (GS_TYPE_PLUGIN_JOB_MANAGE_REPOSITORY,
			     "repository", repository,
			     "flags", flags,
			     NULL);
}
