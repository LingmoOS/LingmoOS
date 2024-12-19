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
 * SECTION:gs-plugin-job-list-categories
 * @short_description: A plugin job to list categories
 *
 * #GsPluginJobListCategories is a #GsPluginJob representing an operation to
 * list categories.
 *
 * All results will be refined using the given set of refine flags, similarly to
 * how #GsPluginJobRefine refines apps.
 *
 * This class is a wrapper around #GsPluginClass.refine_categories_async,
 * calling it for all loaded plugins on the list of categories exposed by a
 * #GsCategoryManager.
 *
 * Retrieve the resulting #GPtrArray of #GsCategory objects using
 * gs_plugin_job_list_categories_get_result_list().
 *
 * See also: #GsPluginClass.refine_categories_async
 * Since: 43
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-category.h"
#include "gs-category-private.h"
#include "gs-enums.h"
#include "gs-plugin-job.h"
#include "gs-plugin-job-list-categories.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-private.h"
#include "gs-plugin-types.h"
#include "gs-profiler.h"
#include "gs-utils.h"

struct _GsPluginJobListCategories
{
	GsPluginJob parent;

	/* Input arguments. */
	GsPluginRefineCategoriesFlags flags;

	/* In-progress data. */
	GPtrArray *category_list;  /* (element-type GsCategory) (owned) (nullable) */
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;

	/* Results. */
	GPtrArray *result_list;  /* (element-type GsCategory) (owned) (nullable) */

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
};

G_DEFINE_TYPE (GsPluginJobListCategories, gs_plugin_job_list_categories, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_FLAGS = 1,
} GsPluginJobListCategoriesProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

static void
gs_plugin_job_list_categories_dispose (GObject *object)
{
	GsPluginJobListCategories *self = GS_PLUGIN_JOB_LIST_CATEGORIES (object);

	g_assert (self->category_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	g_clear_pointer (&self->result_list, g_ptr_array_unref);

	G_OBJECT_CLASS (gs_plugin_job_list_categories_parent_class)->dispose (object);
}

static void
gs_plugin_job_list_categories_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
	GsPluginJobListCategories *self = GS_PLUGIN_JOB_LIST_CATEGORIES (object);

	switch ((GsPluginJobListCategoriesProperty) prop_id) {
	case PROP_FLAGS:
		g_value_set_flags (value, self->flags);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_job_list_categories_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	GsPluginJobListCategories *self = GS_PLUGIN_JOB_LIST_CATEGORIES (object);

	switch ((GsPluginJobListCategoriesProperty) prop_id) {
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

static void plugin_refine_categories_cb (GObject      *source_object,
                                         GAsyncResult *result,
                                         gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);

static void
gs_plugin_job_list_categories_run_async (GsPluginJob         *job,
                                         GsPluginLoader      *plugin_loader,
                                         GCancellable        *cancellable,
                                         GAsyncReadyCallback  callback,
                                         gpointer             user_data)
{
	GsPluginJobListCategories *self = GS_PLUGIN_JOB_LIST_CATEGORIES (job);
	g_autoptr(GTask) task = NULL;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean anything_ran = FALSE;
	GsCategory * const *categories = NULL;
	gsize n_categories;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_list_categories_run_async);
	g_task_set_task_data (task, g_object_ref (plugin_loader), (GDestroyNotify) g_object_unref);

	/* get the categories */
	categories = gs_category_manager_get_categories (gs_plugin_loader_get_category_manager (plugin_loader), &n_categories);
	self->category_list = g_ptr_array_new_full (n_categories, (GDestroyNotify) g_object_unref);

	for (gsize i = 0; i < n_categories; i++) {
		/* reset the sizes to 0, because the plugins just increment the current value */
		gs_category_set_size (categories[i], 0);

		g_ptr_array_add (self->category_list, g_object_ref (categories[i]));
	}

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
		if (plugin_class->refine_categories_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* run the plugin */
		self->n_pending_ops++;
		plugin_class->refine_categories_async (plugin, self->category_list, self->flags, cancellable, plugin_refine_categories_cb, g_object_ref (task));
	}

	if (!anything_ran)
		g_debug ("no plugin could handle listing categories");

	finish_op (task, g_steal_pointer (&local_error));
}

static void
plugin_refine_categories_cb (GObject      *source_object,
                             GAsyncResult *result,
                             gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GTask) task = G_TASK (user_data);
	g_autoptr(GError) local_error = NULL;
#ifdef HAVE_SYSPROF
	GsPluginJobListCategories *self = g_task_get_source_object (task);
#endif

	GS_PROFILER_ADD_MARK_TAKE (PluginJobListCategories,
				   self->begin_time_nsec,
				   g_strdup_printf ("%s:%s",
						    G_OBJECT_TYPE_NAME (self),
						    gs_plugin_get_name (plugin)),
				   NULL);

	if (!plugin_class->refine_categories_finish (plugin, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("plugin '%s' failed to refine categories: %s",
			 gs_plugin_get_name (plugin),
			 local_error->message);
		g_clear_error (&local_error);
	}

	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	finish_op (task, g_steal_pointer (&local_error));
}

static gint
category_sort_cb (gconstpointer a,
                  gconstpointer b)
{
	GsCategory *category_a = GS_CATEGORY (*(GsCategory **) a);
	GsCategory *category_b = GS_CATEGORY (*(GsCategory **) b);
	gint score_a = gs_category_get_score (category_a);
	gint score_b = gs_category_get_score (category_b);

	if (score_a != score_b)
		return score_b - score_a;
	return gs_utils_sort_strcmp (gs_category_get_name (category_a),
				     gs_category_get_name (category_b));
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	GsPluginJobListCategories *self = g_task_get_source_object (task);
	g_autoptr(GPtrArray) category_list = NULL;
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	g_autofree gchar *job_debug = NULL;

	if (error_owned != NULL && self->saved_error == NULL)
		self->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while listing categories: %s", error_owned->message);

	g_assert (self->n_pending_ops > 0);
	self->n_pending_ops--;

	if (self->n_pending_ops > 0)
		return;

	/* Get the results of the parallel ops. */
	category_list = g_steal_pointer (&self->category_list);

	if (self->saved_error != NULL) {
		g_task_return_error (task, g_steal_pointer (&self->saved_error));
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		return;
	}

	/* sort by name */
	g_ptr_array_sort (category_list, category_sort_cb);
	for (guint i = 0; i < category_list->len; i++) {
		GsCategory *category = GS_CATEGORY (g_ptr_array_index (category_list, i));
		gs_category_sort_children (category);
	}

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	/* Check the intermediate working values are all cleared. */
	g_assert (self->category_list == NULL);
	g_assert (self->saved_error == NULL);
	g_assert (self->n_pending_ops == 0);

	/* success */
	self->result_list = g_ptr_array_ref (category_list);
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
gs_plugin_job_list_categories_run_finish (GsPluginJob   *self,
                                          GAsyncResult  *result,
                                          GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_list_categories_class_init (GsPluginJobListCategoriesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_list_categories_dispose;
	object_class->get_property = gs_plugin_job_list_categories_get_property;
	object_class->set_property = gs_plugin_job_list_categories_set_property;

	job_class->run_async = gs_plugin_job_list_categories_run_async;
	job_class->run_finish = gs_plugin_job_list_categories_run_finish;

	/**
	 * GsPluginJobListCategories:flags:
	 *
	 * Flags to specify how the operation should run.
	 *
	 * Since: 43
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to specify how the operation should run.",
				    GS_TYPE_PLUGIN_REFINE_CATEGORIES_FLAGS,
				    GS_PLUGIN_REFINE_CATEGORIES_FLAGS_NONE,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_job_list_categories_init (GsPluginJobListCategories *self)
{
}

/**
 * gs_plugin_job_list_categories_new:
 * @flags: flags affecting how the operation runs
 *
 * Create a new #GsPluginJobListCategories for listing categories.
 *
 * Returns: (transfer full): a new #GsPluginJobListCategories
 * Since: 43
 */
GsPluginJob *
gs_plugin_job_list_categories_new (GsPluginRefineCategoriesFlags flags)
{
	return g_object_new (GS_TYPE_PLUGIN_JOB_LIST_CATEGORIES,
			     "flags", flags,
			     NULL);
}

/**
 * gs_plugin_job_list_categories_get_result_list:
 * @self: a #GsPluginJobListCategories
 *
 * Get the full list of categories.
 *
 * If this is called before the job is complete, %NULL will be returned.
 *
 * Returns: (transfer none) (nullable) (element-type GsCategory): the job
 *   results, or %NULL on error or if called before the job has completed
 * Since: 43
 */
GPtrArray *
gs_plugin_job_list_categories_get_result_list (GsPluginJobListCategories *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_LIST_CATEGORIES (self), NULL);

	return self->result_list;
}
