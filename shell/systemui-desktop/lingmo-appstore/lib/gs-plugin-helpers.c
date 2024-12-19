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
 * SECTION:gs-plugin-helpers
 * @short_description: Helpers for storing call closures for #GsPlugin vfuncs
 *
 * The helpers in this file each create a context structure to store the
 * arguments passed to a standard #GsPlugin vfunc.
 *
 * These are intended to be used by plugin implementations to easily create
 * #GTasks for handling #GsPlugin vfunc calls, without all having to write the
 * same code to create a structure to wrap the vfunc arguments.
 *
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>

#include "gs-plugin-helpers.h"

/**
 * gs_plugin_refine_data_new:
 * @list: list of #GsApps to refine
 * @flags: refine flags
 *
 * Context data for a call to #GsPluginClass.refine_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 42
 */
GsPluginRefineData *
gs_plugin_refine_data_new (GsAppList           *list,
                           GsPluginRefineFlags  flags)
{
	g_autoptr(GsPluginRefineData) data = g_new0 (GsPluginRefineData, 1);
	data->list = g_object_ref (list);
	data->flags = flags;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_refine_data_new_task:
 * @source_object: task source object
 * @list: list of #GsApps to refine
 * @flags: refine flags
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once asynchronous operation is finished
 * @user_data: data to pass to @callback
 *
 * Create a #GTask for a refine operation with the given arguments. The task
 * data will be set to a #GsPluginRefineData containing the given context.
 *
 * This is essentially a combination of gs_plugin_refine_data_new(),
 * g_task_new() and g_task_set_task_data().
 *
 * Returns: (transfer full): new #GTask with the given context data
 * Since: 42
 */
GTask *
gs_plugin_refine_data_new_task (gpointer             source_object,
                                GsAppList           *list,
                                GsPluginRefineFlags  flags,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
	g_autoptr(GTask) task = g_task_new (source_object, cancellable, callback, user_data);
	g_task_set_task_data (task, gs_plugin_refine_data_new (list, flags), (GDestroyNotify) gs_plugin_refine_data_free);
	return g_steal_pointer (&task);
}

/**
 * gs_plugin_refine_data_free:
 * @data: (transfer full): a #GsPluginRefineData
 *
 * Free the given @data.
 *
 * Since: 42
 */
void
gs_plugin_refine_data_free (GsPluginRefineData *data)
{
	g_clear_object (&data->list);
	g_free (data);
}

/**
 * gs_plugin_refresh_metadata_data_new:
 * @cache_age_secs: maximum allowed age of the cache in order for it to remain valid, in seconds
 * @flags: refresh metadata flags
 *
 * Context data for a call to #GsPluginClass.refresh_metadata_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 42
 */
GsPluginRefreshMetadataData *
gs_plugin_refresh_metadata_data_new (guint64                      cache_age_secs,
                                     GsPluginRefreshMetadataFlags flags)
{
	g_autoptr(GsPluginRefreshMetadataData) data = g_new0 (GsPluginRefreshMetadataData, 1);
	data->cache_age_secs = cache_age_secs;
	data->flags = flags;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_refresh_metadata_data_free:
 * @data: (transfer full): a #GsPluginRefreshMetadataData
 *
 * Free the given @data.
 *
 * Since: 42
 */
void
gs_plugin_refresh_metadata_data_free (GsPluginRefreshMetadataData *data)
{
	g_free (data);
}

/**
 * gs_plugin_list_apps_data_new:
 * @query: (nullable) (transfer none): a query to filter apps, or %NULL for
 *   no filtering
 * @flags: list apps flags
 *
 * Context data for a call to #GsPluginClass.list_apps_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 43
 */
GsPluginListAppsData *
gs_plugin_list_apps_data_new (GsAppQuery            *query,
                              GsPluginListAppsFlags  flags)
{
	g_autoptr(GsPluginListAppsData) data = g_new0 (GsPluginListAppsData, 1);
	data->query = (query != NULL) ? g_object_ref (query) : NULL;
	data->flags = flags;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_list_apps_data_new_task:
 * @source_object: task source object
 * @query: (nullable) (transfer none): a query to filter apps, or %NULL for
 *   no filtering
 * @flags: list apps flags
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once asynchronous operation is finished
 * @user_data: data to pass to @callback
 *
 * Create a #GTask for a list apps operation with the given arguments. The task
 * data will be set to a #GsPluginListAppsData containing the given context.
 *
 * This is essentially a combination of gs_plugin_list_apps_data_new(),
 * g_task_new() and g_task_set_task_data().
 *
 * Returns: (transfer full): new #GTask with the given context data
 * Since: 43
 */
GTask *
gs_plugin_list_apps_data_new_task (gpointer               source_object,
                                   GsAppQuery            *query,
                                   GsPluginListAppsFlags  flags,
                                   GCancellable          *cancellable,
                                   GAsyncReadyCallback    callback,
                                   gpointer               user_data)
{
	g_autoptr(GTask) task = g_task_new (source_object, cancellable, callback, user_data);
	g_task_set_task_data (task, gs_plugin_list_apps_data_new (query, flags), (GDestroyNotify) gs_plugin_list_apps_data_free);
	return g_steal_pointer (&task);
}

/**
 * gs_plugin_list_apps_data_free:
 * @data: (transfer full): a #GsPluginListAppsData
 *
 * Free the given @data.
 *
 * Since: 43
 */
void
gs_plugin_list_apps_data_free (GsPluginListAppsData *data)
{
	g_clear_object (&data->query);
	g_free (data);
}

/**
 * gs_plugin_manage_repository_data_new:
 * @repository: (not-nullable) (transfer none): a repository to manage
 * @flags: manage repository flags
 *
 * Common context data for a call to #GsPluginClass.install_repository_async,
 * #GsPluginClass.remove_repository_async, #GsPluginClass.enable_repository_async
 * and #GsPluginClass.disable_repository_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 43
 */
GsPluginManageRepositoryData *
gs_plugin_manage_repository_data_new (GsApp			   *repository,
				      GsPluginManageRepositoryFlags flags)
{
	g_autoptr(GsPluginManageRepositoryData) data = g_new0 (GsPluginManageRepositoryData, 1);
	data->repository = g_object_ref (repository);
	data->flags = flags;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_manage_repository_data_new_task:
 * @source_object: task source object
 * @repository: (not-nullable) (transfer none): a repository to manage
 * @flags: manage repository flags
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once asynchronous operation is finished
 * @user_data: data to pass to @callback
 *
 * Create a #GTask for a manage repository operation with the given arguments. The task
 * data will be set to a #GsPluginManageRepositoryData containing the given context.
 *
 * This is essentially a combination of gs_plugin_manage_repository_data_new(),
 * g_task_new() and g_task_set_task_data().
 *
 * Returns: (transfer full): new #GTask with the given context data
 * Since: 43
 */
GTask *
gs_plugin_manage_repository_data_new_task (gpointer			 source_object,
					   GsApp			*repository,
					   GsPluginManageRepositoryFlags flags,
					   GCancellable			*cancellable,
					   GAsyncReadyCallback		 callback,
					   gpointer			 user_data)
{
	g_autoptr(GTask) task = g_task_new (source_object, cancellable, callback, user_data);
	g_task_set_task_data (task, gs_plugin_manage_repository_data_new (repository, flags), (GDestroyNotify) gs_plugin_manage_repository_data_free);
	return g_steal_pointer (&task);
}

/**
 * gs_plugin_manage_repository_data_free:
 * @data: (transfer full): a #GsPluginManageRepositoryData
 *
 * Free the given @data.
 *
 * Since: 43
 */
void
gs_plugin_manage_repository_data_free (GsPluginManageRepositoryData *data)
{
	g_clear_object (&data->repository);
	g_free (data);
}

/**
 * gs_plugin_refine_categories_data_new:
 * @list: (element-type GsCategory): list of #GsCategory objects to refine
 * @flags: refine flags
 *
 * Context data for a call to #GsPluginClass.refine_categories_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 43
 */
GsPluginRefineCategoriesData *
gs_plugin_refine_categories_data_new (GPtrArray                     *list,
                                      GsPluginRefineCategoriesFlags  flags)
{
	g_autoptr(GsPluginRefineCategoriesData) data = g_new0 (GsPluginRefineCategoriesData, 1);
	data->list = g_ptr_array_ref (list);
	data->flags = flags;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_refine_categories_data_new_task:
 * @source_object: task source object
 * @list: (element-type GsCategory): list of #GsCategory objects to refine
 * @flags: refine flags
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once asynchronous operation is finished
 * @user_data: data to pass to @callback
 *
 * Create a #GTask for a refine categories operation with the given arguments.
 * The task data will be set to a #GsPluginRefineCategoriesData containing the
 * given context.
 *
 * This is essentially a combination of gs_plugin_refine_categories_data_new(),
 * g_task_new() and g_task_set_task_data().
 *
 * Returns: (transfer full): new #GTask with the given context data
 * Since: 43
 */
GTask *
gs_plugin_refine_categories_data_new_task (gpointer                       source_object,
                                           GPtrArray                     *list,
                                           GsPluginRefineCategoriesFlags  flags,
                                           GCancellable                  *cancellable,
                                           GAsyncReadyCallback            callback,
                                           gpointer                       user_data)
{
	g_autoptr(GTask) task = g_task_new (source_object, cancellable, callback, user_data);
	g_task_set_task_data (task, gs_plugin_refine_categories_data_new (list, flags), (GDestroyNotify) gs_plugin_refine_categories_data_free);
	return g_steal_pointer (&task);
}

/**
 * gs_plugin_refine_categories_data_free:
 * @data: (transfer full): a #GsPluginRefineCategoriesData
 *
 * Free the given @data.
 *
 * Since: 43
 */
void
gs_plugin_refine_categories_data_free (GsPluginRefineCategoriesData *data)
{
	g_clear_pointer (&data->list, g_ptr_array_unref);
	g_free (data);
}

/**
 * gs_plugin_update_apps_data_new:
 * @apps: list of apps to update
 * @flags: update flags
 *
 * Context data for a call to #GsPluginClass.update_apps_async.
 *
 * Returns: (transfer full): context data structure
 * Since: 44
 */
GsPluginUpdateAppsData *
gs_plugin_update_apps_data_new (GsAppList                          *apps,
                                GsPluginUpdateAppsFlags             flags,
                                GsPluginProgressCallback            progress_callback,
                                gpointer                            progress_user_data,
                                GsPluginAppNeedsUserActionCallback  app_needs_user_action_callback,
                                gpointer                            app_needs_user_action_data)
{
	g_autoptr(GsPluginUpdateAppsData) data = g_new0 (GsPluginUpdateAppsData, 1);
	data->apps = g_object_ref (apps);
	data->flags = flags;
	data->progress_callback = progress_callback;
	data->progress_user_data = progress_user_data;
	data->app_needs_user_action_callback = app_needs_user_action_callback;
	data->app_needs_user_action_data = app_needs_user_action_data;

	return g_steal_pointer (&data);
}

/**
 * gs_plugin_update_apps_data_new_task:
 * @source_object: task source object
 * @apps: list of apps to update
 * @flags: update flags
 * @progress_callback: (nullable): function to call to notify of progress
 * @progress_user_data: data to pass to @progress_callback
 * @app_needs_user_action_callback: (nullable): function to call to ask the
 *   user for a decision
 * @app_needs_user_action_data: data to pass to @app_needs_user_action_callback
 * @cancellable: (nullable): a #GCancellable, or %NULL
 * @callback: function to call once asynchronous operation is finished
 * @user_data: data to pass to @callback
 *
 * Create a #GTask for an update apps operation with the given arguments.
 * The task data will be set to a #GsPluginUpdateAppsData containing the
 * given context.
 *
 * This is essentially a combination of gs_plugin_update_apps_data_new(),
 * g_task_new() and g_task_set_task_data().
 *
 * Returns: (transfer full): new #GTask with the given context data
 * Since: 44
 */
GTask *
gs_plugin_update_apps_data_new_task (gpointer                            source_object,
                                     GsAppList                          *apps,
                                     GsPluginUpdateAppsFlags             flags,
                                     GsPluginProgressCallback            progress_callback,
                                     gpointer                            progress_user_data,
                                     GsPluginAppNeedsUserActionCallback  app_needs_user_action_callback,
                                     gpointer                            app_needs_user_action_data,
                                     GCancellable                       *cancellable,
                                     GAsyncReadyCallback                 callback,
                                     gpointer                            user_data)
{
	g_autoptr(GTask) task = g_task_new (source_object, cancellable, callback, user_data);
	g_task_set_task_data (task,
			      gs_plugin_update_apps_data_new (apps,
							      flags,
							      progress_callback,
							      progress_user_data,
							      app_needs_user_action_callback,
							      app_needs_user_action_data),
			      (GDestroyNotify) gs_plugin_update_apps_data_free);
	return g_steal_pointer (&task);
}

/**
 * gs_plugin_update_apps_data_free:
 * @data: (transfer full): a #GsPluginUpdateAppsData
 *
 * Free the given @data.
 *
 * Since: 44
 */
void
gs_plugin_update_apps_data_free (GsPluginUpdateAppsData *data)
{
	g_clear_object (&data->apps);
	g_free (data);
}
