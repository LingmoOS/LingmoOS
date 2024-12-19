/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-plugin-loader-sync.h"

/* tiny helper to help us do the async operation */
typedef struct {
	GAsyncResult    *res;
	GMainContext    *context;
	GMainLoop	*loop;
} GsPluginLoaderHelper;

static void
_helper_finish_sync (GObject *source_object,
		     GAsyncResult *res,
		     gpointer user_data)
{
	GsPluginLoaderHelper *helper = user_data;
	helper->res = g_object_ref (res);
	g_main_loop_quit (helper->loop);
}

GsAppList *
gs_plugin_loader_job_process (GsPluginLoader *plugin_loader,
			      GsPluginJob *plugin_job,
			      GCancellable *cancellable,
			      GError **error)
{
	GsPluginLoaderHelper helper;
	GsAppList *list;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job,
					    cancellable,
					    _helper_finish_sync,
					    &helper);
	g_main_loop_run (helper.loop);
	list = gs_plugin_loader_job_process_finish (plugin_loader,
	                                            helper.res,
	                                            error);

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return list;
}

gboolean
gs_plugin_loader_job_action (GsPluginLoader *plugin_loader,
			     GsPluginJob *plugin_job,
			     GCancellable *cancellable,
			     GError **error)
{
	GsPluginLoaderHelper helper;
	gboolean ret;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job,
					    cancellable,
					    _helper_finish_sync,
					    &helper);
	g_main_loop_run (helper.loop);
	ret = gs_plugin_loader_job_action_finish (plugin_loader,
	                                          helper.res,
	                                          error);

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return ret;
}

GsApp *
gs_plugin_loader_job_process_app (GsPluginLoader *plugin_loader,
				  GsPluginJob *plugin_job,
				  GCancellable *cancellable,
				  GError **error)
{
	GsPluginLoaderHelper helper;
	g_autoptr(GsAppList) list = NULL;
	GsApp *app = NULL;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job,
					    cancellable,
					    _helper_finish_sync,
					    &helper);
	g_main_loop_run (helper.loop);
	list = gs_plugin_loader_job_process_finish (plugin_loader,
	                                            helper.res,
	                                            error);
	if (list != NULL)
		app = g_object_ref (gs_app_list_index (list, 0));

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return app;
}

GsApp *
gs_plugin_loader_app_create (GsPluginLoader *plugin_loader,
			     const gchar *unique_id,
			     GCancellable *cancellable,
			     GError **error)
{
	GsPluginLoaderHelper helper;
	GsApp *app;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_app_create_async (plugin_loader,
					   unique_id,
					   cancellable,
					   _helper_finish_sync,
					   &helper);
	g_main_loop_run (helper.loop);
	app = gs_plugin_loader_app_create_finish (plugin_loader,
						  helper.res,
						  error);

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return app;
}

GsApp *
gs_plugin_loader_get_system_app (GsPluginLoader *plugin_loader,
				 GCancellable *cancellable,
				 GError **error)
{
	GsPluginLoaderHelper helper;
	GsApp *app;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_get_system_app_async (plugin_loader,
					       cancellable,
					       _helper_finish_sync,
					       &helper);
	g_main_loop_run (helper.loop);
	app = gs_plugin_loader_get_system_app_finish (plugin_loader,
						      helper.res,
						      error);

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return app;
}

gboolean
gs_plugin_loader_setup (GsPluginLoader       *plugin_loader,
                        const gchar * const  *allowlist,
                        const gchar * const  *blocklist,
                        GCancellable         *cancellable,
                        GError              **error)
{
	GsPluginLoaderHelper helper;
	gboolean retval;

	/* create temp object */
	helper.res = NULL;
	helper.context = g_main_context_new ();
	helper.loop = g_main_loop_new (helper.context, FALSE);

	g_main_context_push_thread_default (helper.context);

	/* run async method */
	gs_plugin_loader_setup_async (plugin_loader,
				      allowlist,
				      blocklist,
				      cancellable,
				      _helper_finish_sync,
				      &helper);
	g_main_loop_run (helper.loop);
	retval = gs_plugin_loader_setup_finish (plugin_loader,
						helper.res,
						error);

	g_main_context_pop_thread_default (helper.context);

	g_main_loop_unref (helper.loop);
	g_main_context_unref (helper.context);
	if (helper.res != NULL)
		g_object_unref (helper.res);

	return retval;
}
