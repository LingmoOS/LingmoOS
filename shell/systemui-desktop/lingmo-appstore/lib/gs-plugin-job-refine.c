/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-plugin-job-refine
 * @short_description: A plugin job to refine #GsApps and add more data
 *
 * #GsPluginJobRefine is a #GsPluginJob representing a refine operation.
 *
 * It’s used to query and add more data to a set of #GsApps. The data to be set
 * is controlled by the #GsPluginRefineFlags, and is looked up for all the apps
 * in a #GsAppList by the loaded plugins.
 *
 * This class is a wrapper around #GsPluginClass.refine_async, calling it for
 * all loaded plugins, with some additional refinements done on the results.
 *
 * In particular, if an app in the #GsAppList has %GS_APP_QUIRK_IS_WILDCARD,
 * refining it will replace it with zero or more non-wildcard #GsApps in the
 * #GsAppList, all of which are candidates for what the wildcard represents.
 * For example, they may have the same ID as the wildcard, or match its name.
 * Refining is the canonical process for resolving wildcards.
 *
 * This means that the #GsAppList at the end of the refine operation may not
 * match the #GsAppList passed in as input. Retrieve the final #GsAppList using
 * gs_plugin_job_refine_get_result_list(). The #GsAppList which was passed
 * into the job will not be modified.
 *
 * Internally, the #GsPluginClass.refine_async() functions are called on all
 * the plugins in series, and in series with calls to
 * gs_odrs_provider_refine_async() and gs_rewrite_resources_async().
 * Once all of those calls are finished,
 * zero or more recursive calls to run_refine_internal_async() are made in
 * parallel to do a similar refine process on the addons, runtime and related
 * components for all the components in the input #GsAppList. The refine job is
 * complete once all these recursive calls complete.
 *
 * The call to gs_rewrite_resources_async() will rewrite the CSS of apps to
 * refer to locally cached resources, rather than HTTP/HTTPS URIs for images
 * (for example).
 *
 * FIXME: Ideally, the #GsPluginClass.refine_async() calls would happen in
 * parallel, but this cannot be the case until the results of the refine_async()
 * call in one plugin don’t depend on the results of refine_async() in another.
 * This still happens with several pairs of plugins.
 *
 * ```
 *                                    run_async()
 *                                         |
 *                                         v
 *           /-----------------------+-------------+----------------+------------------------------\
 *           |                       |             |                |                              |
 * plugin->refine_async()            |             |                |                              |
 *           v             plugin->refine_async()  |                |                              |
 *           |                       v             …                |                              |
 *           |                       |             v  gs_odrs_provider_refine_async()              |
 *           |                       |             |                v                  gs_rewrite_resources_async()
 *           |                       |             |                |                              v
 *           |                       |             |                |                              |
 *           \-----------------------+-------------+----------------+------------------------------/
 *                                         |
 *                            finish_refine_internal_op()
 *                                         |
 *                                         v
 *            /----------------------------+-----------------\
 *            |                            |                 |
 * run_refine_internal_async()  run_refine_internal_async()  …
 *            |                            |                 |
 *            v                            v                 v
 *            \----------------------------+-----------------/
 *                                         |
 *                         finish_refine_internal_recursion()
 * ```
 *
 * See also: #GsPluginClass.refine_async
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include "gs-app.h"
#include "gs-app-collation.h"
#include "gs-app-private.h"
#include "gs-app-list-private.h"
#include "gs-enums.h"
#include "gs-plugin-private.h"
#include "gs-plugin-job-private.h"
#include "gs-plugin-job-refine.h"
#include "gs-profiler.h"
#include "gs-utils.h"

struct _GsPluginJobRefine
{
	GsPluginJob parent;

	/* Input data. */
	GsAppList *app_list;  /* (owned) */
	GsPluginRefineFlags flags;

	/* Output data. */
	GsAppList *result_list;  /* (owned) (nullable) */

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
};

G_DEFINE_TYPE (GsPluginJobRefine, gs_plugin_job_refine, GS_TYPE_PLUGIN_JOB)

typedef enum {
	PROP_APP_LIST = 1,
	PROP_FLAGS,
} GsPluginJobRefineProperty;

static GParamSpec *props[PROP_FLAGS + 1] = { NULL, };

static void
gs_plugin_job_refine_dispose (GObject *object)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (object);

	g_clear_object (&self->app_list);
	g_clear_object (&self->result_list);

	G_OBJECT_CLASS (gs_plugin_job_refine_parent_class)->dispose (object);
}

static void
gs_plugin_job_refine_constructed (GObject *object)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (object);

	G_OBJECT_CLASS (gs_plugin_job_refine_parent_class)->constructed (object);

	/* FIXME: the plugins should specify this, rather than hardcoding */
	if (self->flags & (GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_UI |
			   GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME))
		self->flags |= GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN;
	if (self->flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE)
		self->flags |= GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME;
}

static void
gs_plugin_job_refine_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (object);

	switch ((GsPluginJobRefineProperty) prop_id) {
	case PROP_APP_LIST:
		g_value_set_object (value, self->app_list);
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
gs_plugin_job_refine_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (object);

	switch ((GsPluginJobRefineProperty) prop_id) {
	case PROP_APP_LIST:
		/* Construct only. */
		g_assert (self->app_list == NULL);
		self->app_list = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[PROP_APP_LIST]);
		break;
	case PROP_FLAGS:
		/* Construct only. */
		g_assert (self->flags == 0);
		self->flags = g_value_get_flags (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
app_is_valid_filter (GsApp    *app,
                     gpointer  user_data)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (user_data);

	return gs_plugin_loader_app_is_valid (app, self->flags);
}

static gint
review_score_sort_cb (gconstpointer a, gconstpointer b)
{
	AsReview *ra = *((AsReview **) a);
	AsReview *rb = *((AsReview **) b);
	if (as_review_get_priority (ra) < as_review_get_priority (rb))
		return 1;
	if (as_review_get_priority (ra) > as_review_get_priority (rb))
		return -1;
	return 0;
}

static gboolean
app_is_non_wildcard (GsApp *app, gpointer user_data)
{
	return !gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
}

static void plugin_refine_cb (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data);
static void odrs_provider_refine_cb (GObject      *source_object,
                                     GAsyncResult *result,
                                     gpointer      user_data);
static void rewrite_resources_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data);
static void finish_refine_internal_op (GTask  *task,
                                       GError *error);
static void recursive_internal_refine_cb (GObject      *source_object,
                                          GAsyncResult *result,
                                          gpointer      user_data);
static void finish_refine_internal_recursion (GTask  *task,
                                              GError *error);
static gboolean run_refine_internal_finish (GsPluginJobRefine  *self,
                                            GAsyncResult       *result,
                                            GError            **error);

typedef struct {
	/* Input data. */
	GsPluginLoader *plugin_loader;  /* (not nullable) (owned) */
	GsAppList *list;  /* (not nullable) (owned) */
	GsPluginRefineFlags flags;

	/* In-progress data. */
	guint n_pending_ops;
	guint n_pending_recursions;
	guint next_plugin_index;
	guint next_plugin_order;

#ifdef HAVE_SYSPROF
	gint64 plugin_begin_time_nsec;
#endif

	/* Output data. */
	GError *error;  /* (nullable) (owned) */
} RefineInternalData;

static void
refine_internal_data_free (RefineInternalData *data)
{
	g_clear_object (&data->plugin_loader);
	g_clear_object (&data->list);

	g_assert (data->n_pending_ops == 0);
	g_assert (data->n_pending_recursions == 0);

	/* If an error occurred, it should have been stolen to pass to
	 * g_task_return_error() by now. */
	g_assert (data->error == NULL);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (RefineInternalData, refine_internal_data_free)

static void
run_refine_internal_async (GsPluginJobRefine   *self,
                           GsPluginLoader      *plugin_loader,
                           GsAppList           *list,
                           GsPluginRefineFlags  flags,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	g_autoptr(GTask) task = NULL;
	RefineInternalData *data;
	g_autoptr(RefineInternalData) data_owned = NULL;
	gboolean anything_ran = FALSE;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, run_refine_internal_async);

	data = data_owned = g_new0 (RefineInternalData, 1);
	data->plugin_loader = g_object_ref (plugin_loader);
	data->list = g_object_ref (list);
	data->flags = flags;
#ifdef HAVE_SYSPROF
	data->plugin_begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) refine_internal_data_free);

	/* try to adopt each app with a plugin */
	gs_plugin_loader_run_adopt (plugin_loader, list);

	data->n_pending_ops = 0;
	data->next_plugin_order = 0;

	/* run each plugin
	 *
	 * FIXME: For now, we have to run these vfuncs sequentially rather than
	 * all in parallel. This is because there are still dependencies between
	 * some of the plugins, where the code to refine an app in one plugin
	 * depends on the results of refining it in another plugin first.
	 *
	 * Eventually, the plugins should all be changed/removed so that they
	 * can operate independently. At that point, this code can be reverted
	 * so that the refine_async() vfuncs are called in parallel. */
	plugins = gs_plugin_loader_get_plugins (plugin_loader);

	for (guint i = 0; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (gs_plugin_get_order (plugin) > data->next_plugin_order) {
			if (!anything_ran)
				data->next_plugin_order = gs_plugin_get_order (plugin);
			else
				return;
		}

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->refine_async == NULL)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* FIXME: The next refine_async() call is made in
		 * finish_refine_internal_op(). */
		data->next_plugin_index = i + 1;

		/* run the batched plugin symbol */
		data->n_pending_ops++;
		plugin_class->refine_async (plugin, list, flags,
					    cancellable, plugin_refine_cb, g_object_ref (task));
	}

	if (!anything_ran)
		g_debug ("no plugin could handle refining apps");

	data->n_pending_ops++;
	finish_refine_internal_op (task, g_steal_pointer (&local_error));
}

static void
plugin_refine_cb (GObject      *source_object,
                  GAsyncResult *result,
                  gpointer      user_data)
{
	GsPlugin *plugin = GS_PLUGIN (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
	g_autoptr(GError) local_error = NULL;
#ifdef HAVE_SYSPROF
	GsPluginJobRefine *self = g_task_get_source_object (task);
	RefineInternalData *data = g_task_get_task_data (task);
#endif

	GS_PROFILER_ADD_MARK_TAKE (PluginJobRefine,
				   data->plugin_begin_time_nsec,
				   g_strdup_printf ("%s:%s",
						    G_OBJECT_TYPE_NAME (self),
						    gs_plugin_get_name (plugin)),
				   NULL);

	if (!plugin_class->refine_finish (plugin, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("plugin '%s' failed to refine apps: %s",
			 gs_plugin_get_name (plugin),
			 local_error->message);
		g_clear_error (&local_error);
	}

	gs_plugin_status_update (plugin, NULL, GS_PLUGIN_STATUS_FINISHED);

	finish_refine_internal_op (task, NULL);
}

static void
odrs_provider_refine_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data)
{
	GsOdrsProvider *odrs_provider = GS_ODRS_PROVIDER (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_odrs_provider_refine_finish (odrs_provider, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("ODRS provider failed to refine apps: %s",
			 local_error->message);
		g_clear_error (&local_error);
	}
	finish_refine_internal_op (task, g_steal_pointer (&local_error));
}

static void
rewrite_resources_cb (GObject      *source_object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!gs_rewrite_resources_finish (result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("Rewriting resources failed when refine apps: %s",
			 local_error->message);
		g_clear_error (&local_error);
	}
	finish_refine_internal_op (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-NULL */
static void
finish_refine_internal_op (GTask  *task,
                           GError *error)
{
	GsPluginJobRefine *self = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	RefineInternalData *data = g_task_get_task_data (task);
	GsPluginLoader *plugin_loader = data->plugin_loader;
	GsAppList *list = data->list;
	GsPluginRefineFlags flags = data->flags;
	GsOdrsProvider *odrs_provider;
	GsOdrsProviderRefineFlags odrs_refine_flags = 0;
	GPtrArray *plugins;  /* (element-type GsPlugin) */
	gboolean anything_ran = FALSE;

	if (data->error == NULL && error_owned != NULL) {
		data->error = g_steal_pointer (&error_owned);
	} else if (error_owned != NULL) {
		g_debug ("Additional error while refining: %s", error_owned->message);
	}

	g_assert (data->n_pending_ops > 0);
	data->n_pending_ops--;

#ifdef HAVE_SYSPROF
	data->plugin_begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	if (data->n_pending_ops > 0)
		return;

	/* We reach this line after all plugins of a certain order ran, and now
	 * we need to run the next set of plugins. */
	data->next_plugin_order++;

	plugins = gs_plugin_loader_get_plugins (plugin_loader);

	for (guint i = data->next_plugin_index; i < plugins->len; i++) {
		GsPlugin *plugin = g_ptr_array_index (plugins, i);
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);

		if (gs_plugin_get_order (plugin) > data->next_plugin_order) {
			if (!anything_ran)
				data->next_plugin_order = gs_plugin_get_order (plugin);
			else
				return;
		}

		if (!gs_plugin_get_enabled (plugin))
			continue;
		if (plugin_class->refine_async == NULL)
			continue;
		if (gs_plugin_get_order (plugin) < data->next_plugin_order)
			continue;

		/* at least one plugin supports this vfunc */
		anything_ran = TRUE;

		/* FIXME: The next refine_async() call is made in
		 * finish_refine_internal_op(). */
		data->next_plugin_index = i + 1;

		/* run the batched plugin symbol */
		data->n_pending_ops++;
		plugin_class->refine_async (plugin, list, flags,
					    cancellable, plugin_refine_cb, g_object_ref (task));
	}

	if (data->next_plugin_index == plugins->len) {
		/* Avoid the ODRS and rewrite refines being run multiple times. */
		data->next_plugin_index++;

		/* Add ODRS data if needed */
		odrs_provider = gs_plugin_loader_get_odrs_provider (plugin_loader);

		if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS)
			odrs_refine_flags |= GS_ODRS_PROVIDER_REFINE_FLAGS_GET_REVIEWS;
		if (flags & (GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS |
			     GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING))
			odrs_refine_flags |= GS_ODRS_PROVIDER_REFINE_FLAGS_GET_RATINGS;

		if (odrs_provider != NULL && odrs_refine_flags != 0) {
			data->n_pending_ops++;
			gs_odrs_provider_refine_async (odrs_provider, list, odrs_refine_flags,
						       cancellable, odrs_provider_refine_cb, g_object_ref (task));
		}

		/* Rewrite app CSS if needed. */
		data->n_pending_ops++;
		gs_rewrite_resources_async (list, cancellable, rewrite_resources_cb, g_object_ref (task));
	}

	if (data->n_pending_ops > 0)
		return;

	/* At this point, all the plugin->refine() calls are complete and the
	 * gs_odrs_provider_refine_async() and gs_rewrite_resources_async()
	 * calls are also complete. If an error
	 * occurred during those calls, return with it now rather than
	 * proceeding to the recursive calls below. */
	if (data->error != NULL) {
		g_task_return_error (task, g_steal_pointer (&data->error));
		return;
	}

	/* filter any wildcard apps left in the list */
	gs_app_list_filter (list, app_is_non_wildcard, NULL);

	/* ensure these are sorted by score */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS) {
		GPtrArray *reviews;
		for (guint i = 0; i < gs_app_list_length (list); i++) {
			GsApp *app = gs_app_list_index (list, i);
			reviews = gs_app_get_reviews (app);
			g_ptr_array_sort (reviews, review_score_sort_cb);
		}
	}

	/* Now run several recursive calls to run_refine_internal_async() in
	 * parallel, to refine related components. */
	data->n_pending_recursions = 1;

	/* refine addons one layer deep */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS) {
		g_autoptr(GsAppList) addons_list = gs_app_list_new ();
		GsPluginRefineFlags addons_flags = flags;

		addons_flags &= ~(GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS |
				  GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS |
				  GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS);

		for (guint i = 0; i < gs_app_list_length (list); i++) {
			GsApp *app = gs_app_list_index (list, i);
			g_autoptr(GsAppList) addons = gs_app_dup_addons (app);

			for (guint j = 0; addons != NULL && j < gs_app_list_length (addons); j++) {
				GsApp *addon = gs_app_list_index (addons, j);
				g_debug ("refining app %s addon %s",
					 gs_app_get_id (app),
					 gs_app_get_id (addon));
				gs_app_list_add (addons_list, addon);
			}
		}

		if (gs_app_list_length (addons_list) > 0 && addons_flags != 0) {
			data->n_pending_recursions++;
			run_refine_internal_async (self, plugin_loader,
						   addons_list, addons_flags,
						   cancellable, recursive_internal_refine_cb,
						   g_object_ref (task));
		}
	}

	/* also do runtime */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME) {
		g_autoptr(GsAppList) runtimes_list = gs_app_list_new ();
		GsPluginRefineFlags runtimes_flags = flags;

		runtimes_flags &= ~GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME;

		for (guint i = 0; i < gs_app_list_length (list); i++) {
			GsApp *app = gs_app_list_index (list, i);
			GsApp *runtime = gs_app_get_runtime (app);

			if (runtime != NULL)
				gs_app_list_add (runtimes_list, runtime);
		}

		if (gs_app_list_length (runtimes_list) > 0 && runtimes_flags != 0) {
			data->n_pending_recursions++;
			run_refine_internal_async (self, plugin_loader,
						   runtimes_list, runtimes_flags,
						   cancellable, recursive_internal_refine_cb,
						   g_object_ref (task));
		}
	}

	/* also do related packages one layer deep */
	if (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_RELATED) {
		g_autoptr(GsAppList) related_list = gs_app_list_new ();
		GsPluginRefineFlags related_flags = flags;

		related_flags &= ~GS_PLUGIN_REFINE_FLAGS_REQUIRE_RELATED;

		for (guint i = 0; i < gs_app_list_length (list); i++) {
			GsApp *app = gs_app_list_index (list, i);
			GsAppList *related = gs_app_get_related (app);
			for (guint j = 0; j < gs_app_list_length (related); j++) {
				GsApp *app2 = gs_app_list_index (related, j);
				g_debug ("refining related: %s[%s]",
					 gs_app_get_id (app2),
					 gs_app_get_source_default (app2));
				gs_app_list_add (related_list, app2);
			}
		}

		if (gs_app_list_length (related_list) > 0 && related_flags != 0) {
			data->n_pending_recursions++;
			run_refine_internal_async (self, plugin_loader,
						   related_list, related_flags,
						   cancellable, recursive_internal_refine_cb,
						   g_object_ref (task));
		}
	}

	finish_refine_internal_recursion (task, NULL);
}

static void
recursive_internal_refine_cb (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GError) local_error = NULL;

	if (!run_refine_internal_finish (self, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
	    !g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
		g_debug ("failed to recursive-refine: %s", local_error->message);
		g_clear_error (&local_error);
	}

	finish_refine_internal_recursion (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-NULL */
static void
finish_refine_internal_recursion (GTask  *task,
                                  GError *error)
{
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	RefineInternalData *data = g_task_get_task_data (task);

	if (data->error == NULL && error_owned != NULL) {
		data->error = g_steal_pointer (&error_owned);
	} else if (error_owned != NULL) {
		g_debug ("Additional error while refining: %s", error_owned->message);
	}

	g_assert (data->n_pending_recursions > 0);
	data->n_pending_recursions--;

	if (data->n_pending_recursions > 0)
		return;

	/* The entire refine operation (and all its sub-operations and
	 * recursions) is complete. */
	if (data->error != NULL)
		g_task_return_error (task, g_steal_pointer (&data->error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
run_refine_internal_finish (GsPluginJobRefine  *self,
                            GAsyncResult       *result,
                            GError            **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static gboolean
app_thaw_notify_idle (gpointer data)
{
	GsApp *app = GS_APP (data);
	g_object_thaw_notify (G_OBJECT (app));
	g_object_unref (app);
	return G_SOURCE_REMOVE;
}

static void run_cb (GObject      *source_object,
                    GAsyncResult *result,
                    gpointer      user_data);
static void finish_run (GTask     *task,
                        GsAppList *result_list);

static void
gs_plugin_job_refine_run_async (GsPluginJob         *job,
                                GsPluginLoader      *plugin_loader,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (job);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GsAppList) result_list = NULL;

	/* check required args */
	task = g_task_new (job, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_job_refine_run_async);

	/* Operate on a copy of the input list so we don’t modify it when
	 * resolving wildcards. */
	result_list = gs_app_list_copy (self->app_list);
	g_task_set_task_data (task, g_object_ref (result_list), (GDestroyNotify) g_object_unref);

	/* nothing to do */
	if (self->flags == 0 ||
	    gs_app_list_length (result_list) == 0) {
		g_debug ("no refine flags set for transaction or app list is empty");
		finish_run (task, result_list);
		return;
	}

	/* freeze all apps */
	for (guint i = 0; i < gs_app_list_length (self->app_list); i++) {
		GsApp *app = gs_app_list_index (self->app_list, i);
		g_object_freeze_notify (G_OBJECT (app));
	}

#ifdef HAVE_SYSPROF
	self->begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	/* Start refining the apps. */
	run_refine_internal_async (self, plugin_loader, result_list,
				   self->flags, cancellable,
				   run_cb, g_steal_pointer (&task));
}

static void
run_cb (GObject      *source_object,
        GAsyncResult *result,
        gpointer      user_data)
{
	GsPluginJobRefine *self = GS_PLUGIN_JOB_REFINE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsAppList *result_list = g_task_get_task_data (task);
	g_autoptr(GError) local_error = NULL;

	if (run_refine_internal_finish (self, result, &local_error)) {
		/* remove any addons that have the same source as the parent app */
		for (guint i = 0; i < gs_app_list_length (result_list); i++) {
			g_autoptr(GPtrArray) to_remove = g_ptr_array_new ();
			GsApp *app = gs_app_list_index (result_list, i);
			g_autoptr(GsAppList) addons = gs_app_dup_addons (app);

			/* find any apps with the same source */
			const gchar *pkgname_parent = gs_app_get_source_default (app);
			if (pkgname_parent == NULL)
				continue;
			for (guint j = 0; addons != NULL && j < gs_app_list_length (addons); j++) {
				GsApp *addon = gs_app_list_index (addons, j);
				if (g_strcmp0 (gs_app_get_source_default (addon),
					       pkgname_parent) == 0) {
					g_debug ("%s has the same pkgname of %s as %s",
						 gs_app_get_unique_id (app),
						 pkgname_parent,
						 gs_app_get_unique_id (addon));
					g_ptr_array_add (to_remove, addon);
				}
			}

			/* remove any addons with the same source */
			for (guint j = 0; j < to_remove->len; j++) {
				GsApp *addon = g_ptr_array_index (to_remove, j);
				gs_app_remove_addon (app, addon);
			}
		}
	}

	/* now emit all the changed signals */
	for (guint i = 0; i < gs_app_list_length (self->app_list); i++) {
		GsApp *app = gs_app_list_index (self->app_list, i);
		g_idle_add (app_thaw_notify_idle, g_object_ref (app));
	}

	/* Delayed error handling. */
	if (local_error != NULL) {
		gs_utils_error_convert_gio (&local_error);
		g_signal_emit_by_name (G_OBJECT (self), "completed");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	finish_run (task, result_list);
}

static void
finish_run (GTask     *task,
            GsAppList *result_list)
{
	GsPluginJobRefine *self = g_task_get_source_object (task);
	g_autofree gchar *job_debug = NULL;

	/* Internal calls to #GsPluginJobRefine may want to do their own
	 * filtering, typically if the refine is being done as part of another
	 * plugin job. If so, only filter to remove wildcards. Wildcards should
	 * always be removed, as they should have been resolved as part of the
	 * refine; any remaining wildcards will never be resolved.
	 *
	 * If the flag is not specified, filter by a variety of indicators of
	 * what a ‘valid’ app is. */
	if (self->flags & GS_PLUGIN_REFINE_FLAGS_DISABLE_FILTERING)
		gs_app_list_filter (result_list, app_is_non_wildcard, NULL);
	else
		gs_app_list_filter (result_list, app_is_valid_filter, self);

	/* show elapsed time */
	job_debug = gs_plugin_job_to_string (GS_PLUGIN_JOB (self));
	g_debug ("%s", job_debug);

	/* success */
	g_set_object (&self->result_list, result_list);
	g_task_return_boolean (task, TRUE);
	g_signal_emit_by_name (G_OBJECT (self), "completed");
}

static gboolean
gs_plugin_job_refine_run_finish (GsPluginJob   *self,
                                 GAsyncResult  *result,
                                 GError       **error)
{
	GS_PROFILER_ADD_MARK (PluginJobRefine,
			      GS_PLUGIN_JOB_REFINE (self)->begin_time_nsec,
			      G_OBJECT_TYPE_NAME (self),
			      NULL);

	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_job_refine_class_init (GsPluginJobRefineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginJobClass *job_class = GS_PLUGIN_JOB_CLASS (klass);

	object_class->dispose = gs_plugin_job_refine_dispose;
	object_class->constructed = gs_plugin_job_refine_constructed;
	object_class->get_property = gs_plugin_job_refine_get_property;
	object_class->set_property = gs_plugin_job_refine_set_property;

	job_class->run_async = gs_plugin_job_refine_run_async;
	job_class->run_finish = gs_plugin_job_refine_run_finish;

	/**
	 * GsPluginJobRefine:app-list:
	 *
	 * List of #GsApps to refine.
	 *
	 * This will not change during the course of the operation.
	 *
	 * Since: 42
	 */
	props[PROP_APP_LIST] =
		g_param_spec_object ("app-list", "App List",
				     "List of GsApps to refine.",
				     GS_TYPE_APP_LIST,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginJobRefine:flags:
	 *
	 * Flags to control what to refine.
	 *
	 * Since: 42
	 */
	props[PROP_FLAGS] =
		g_param_spec_flags ("flags", "Flags",
				    "Flags to control what to refine.",
				     GS_TYPE_PLUGIN_REFINE_FLAGS, GS_PLUGIN_REFINE_FLAGS_NONE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_job_refine_init (GsPluginJobRefine *self)
{
}

/**
 * gs_plugin_job_refine_new:
 * @app_list: the list of #GsApps to refine
 * @flags: flags to affect what is refined
 *
 * Create a new #GsPluginJobRefine for refining the given @app_list.
 *
 * Returns: (transfer full): a new #GsPluginJobRefine
 * Since: 42
 */
GsPluginJob *
gs_plugin_job_refine_new (GsAppList           *app_list,
                          GsPluginRefineFlags  flags)
{
	return g_object_new (GS_TYPE_PLUGIN_JOB_REFINE,
			     "app-list", app_list,
			     "flags", flags,
			     NULL);
}

/**
 * gs_plugin_job_refine_new_for_app:
 * @app: the #GsApp to refine
 * @flags: flags to affect what is refined
 *
 * Create a new #GsPluginJobRefine for refining the given @app.
 *
 * Returns: (transfer full): a new #GsPluginJobRefine
 * Since: 42
 */
GsPluginJob *
gs_plugin_job_refine_new_for_app (GsApp               *app,
                                  GsPluginRefineFlags  flags)
{
	g_autoptr(GsAppList) list = gs_app_list_new ();
	gs_app_list_add (list, app);

	return gs_plugin_job_refine_new (list, flags);
}

/**
 * gs_plugin_job_refine_get_result_list:
 * @self: a #GsPluginJobRefine
 *
 * Get the full list of refined #GsApps. This includes apps created in place of
 * wildcards, if wildcards were provided in the #GsAppList passed to
 * gs_plugin_job_refine_new().
 *
 * If this is called before the job is complete, %NULL will be returned.
 *
 * Returns: (transfer none) (nullable): the job results, or %NULL on error
 *   or if called before the job has completed
 * Since: 42
 */
GsAppList *
gs_plugin_job_refine_get_result_list (GsPluginJobRefine *self)
{
	g_return_val_if_fail (GS_IS_PLUGIN_JOB_REFINE (self), NULL);

	return self->result_list;
}
