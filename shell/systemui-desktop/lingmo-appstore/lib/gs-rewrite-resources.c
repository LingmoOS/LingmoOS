/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2023 Endless OS Foundation, LLC
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <lingmo-appstore.h>

#include "gs-profiler.h"
#include "gs-rewrite-resources.h"

/*
 * SECTION:gs-rewrite-resources
 * @short_description: Rewrites CSS metadata for apps to refer to locally downloaded resources.
 *
 * This set of functions rewrites the CSS of apps to refer to locally cached
 * resources, rather than HTTP/HTTPS URIs for images (for example).
 *
 * Resources are downloaded asynchronously and in parallel, and are cached
 * locally automatically.
 *
 * This code is designed to be used by #GsPluginJobRefine.
 *
 * Since: 45
 */

typedef struct {
	GError *saved_error;  /* (owned) (nullable) */
	guint n_pending_ops;

#ifdef HAVE_SYSPROF
	gint64 begin_time_nsec;
#endif
} RewriteResourcesData;

static void
rewrite_resources_data_free (RewriteResourcesData *data)
{
	g_assert (data->saved_error == NULL);
	g_assert (data->n_pending_ops == 0);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (RewriteResourcesData, rewrite_resources_data_free)

typedef struct {
	GTask *task;  /* (owned) (not nullable) */
	GsApp *app;  /* (owned) (not nullable) */
	const gchar *key;  /* (not nullable) */
} OpData;

static void
op_data_free (OpData *data)
{
	g_clear_object (&data->task);
	g_clear_object (&data->app);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (OpData, op_data_free)

static void rewrite_resource_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);
static void finish_op (GTask  *task,
                       GError *error);

/**
 * gs_rewrite_resources_async:
 * @list: a list of apps to download resources for
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once the operation is complete
 * @user_data: data to pass to @callback
 *
 * Downloads remote resources for the apps in @list, caches those downloads
 * locally and rewrites the apps’ metadata to refer to the local copies.
 *
 * This currently acts on the following app metadata keys:
 *  - `LingmoAppstore::FeatureTile-css`
 *  - `LingmoAppstore::UpgradeBanner-css`
 *
 * Since: 45
 */
void
gs_rewrite_resources_async (GsAppList           *list,
                            GCancellable        *cancellable,
                            GAsyncReadyCallback  callback,
                            gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	RewriteResourcesData *data;
	g_autoptr(RewriteResourcesData) data_owned = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (NULL, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_rewrite_resources_async);

	data = data_owned = g_new0 (RewriteResourcesData, 1);
	data->n_pending_ops = 1;  /* count setup as an operation */

	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) rewrite_resources_data_free);

#ifdef HAVE_SYSPROF
	data->begin_time_nsec = SYSPROF_CAPTURE_CURRENT_TIME;
#endif

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		const gchar *keys[] = {
			"LingmoAppstore::FeatureTile-css",
			"LingmoAppstore::UpgradeBanner-css",
			NULL
		};

		/* Handle cancellation */
		if (g_cancellable_set_error_if_cancelled (cancellable, &local_error))
			break;

		/* rewrite URIs */
		for (gsize j = 0; keys[j] != NULL; j++) {
			const gchar *css = gs_app_get_metadata_item (app, keys[j]);
			g_autoptr(OpData) op_data = NULL;

			if (css == NULL)
				continue;

			op_data = g_new0 (OpData, 1);
			op_data->task = g_object_ref (task);
			op_data->app = g_object_ref (app);
			op_data->key = keys[j];

			data->n_pending_ops++;
			gs_download_rewrite_resource_async (css,
							    cancellable,
							    rewrite_resource_cb,
							    g_steal_pointer (&op_data));
		}
	}

	finish_op (task, g_steal_pointer (&local_error));
}

static void
rewrite_resource_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	g_autoptr(OpData) op_data = g_steal_pointer (&user_data);
	GTask *task = op_data->task;
	g_autoptr(GError) local_error = NULL;
	const gchar *css_old;
	g_autofree gchar *css_new = NULL;

	css_new = gs_download_rewrite_resource_finish (result, &local_error);

	/* Successfully rewritten? */
	css_old = gs_app_get_metadata_item (op_data->app, op_data->key);

	if (css_new != NULL && g_strcmp0 (css_old, css_new) != 0) {
		gs_app_set_metadata (op_data->app, op_data->key, NULL);
		gs_app_set_metadata (op_data->app, op_data->key, css_new);
	}

	finish_op (task, g_steal_pointer (&local_error));
}

/* @error is (transfer full) if non-%NULL */
static void
finish_op (GTask  *task,
           GError *error)
{
	g_autoptr(GError) error_owned = g_steal_pointer (&error);
	RewriteResourcesData *data = g_task_get_task_data (task);

	if (error_owned != NULL && data->saved_error == NULL)
		data->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while rewriting resources: %s", error_owned->message);

	g_assert (data->n_pending_ops > 0);
	data->n_pending_ops--;

	if (data->n_pending_ops > 0)
		return;

	if (data->saved_error != NULL) {
		g_task_return_error (task, g_steal_pointer (&data->saved_error));
		return;
	}

	g_assert (data->saved_error == NULL);
	g_assert (data->n_pending_ops == 0);

	/* success */
	g_task_return_boolean (task, TRUE);

	GS_PROFILER_ADD_MARK (RewriteResources,
			      data->begin_time_nsec,
			      "RewriteResources",
			      NULL);
}

/**
 * gs_rewrite_resources_finish:
 * @result: a #GAsyncResult
 * @error: return location for a #GError, or %NULL
 *
 * Finish an asynchronous rewrite operation started with
 * gs_rewrite_resources_async().
 *
 * Returns: %TRUE on success, %FALSE otherwise
 * Since: 45
 */
gboolean
gs_rewrite_resources_finish (GAsyncResult  *result,
                             GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}
