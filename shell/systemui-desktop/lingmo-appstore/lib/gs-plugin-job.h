/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017-2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-app-list.h"
#include "gs-plugin-types.h"

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_JOB (gs_plugin_job_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsPluginJob, gs_plugin_job, GS, PLUGIN_JOB, GObject)

#include "gs-plugin-loader.h"

struct _GsPluginJobClass
{
	GObjectClass parent_class;

	void (*run_async) (GsPluginJob         *self,
	                   GsPluginLoader      *plugin_loader,
	                   GCancellable        *cancellable,
	                   GAsyncReadyCallback  callback,
	                   gpointer             user_data);
	gboolean (*run_finish) (GsPluginJob   *self,
	                        GAsyncResult  *result,
	                        GError       **error);
};

void		 gs_plugin_job_set_refine_flags		(GsPluginJob	*self,
							 GsPluginRefineFlags refine_flags);
void		 gs_plugin_job_set_dedupe_flags		(GsPluginJob	*self,
							 GsAppListFilterFlags dedupe_flags);
void		 gs_plugin_job_set_interactive		(GsPluginJob	*self,
							 gboolean	 interactive);
void		 gs_plugin_job_set_propagate_error	(GsPluginJob	*self,
							 gboolean	 propagate_error);
void		 gs_plugin_job_set_max_results		(GsPluginJob	*self,
							 guint		 max_results);
void		 gs_plugin_job_set_search		(GsPluginJob	*self,
							 const gchar	*search);
void		 gs_plugin_job_set_app			(GsPluginJob	*self,
							 GsApp		*app);
void		 gs_plugin_job_set_list			(GsPluginJob	*self,
							 GsAppList	*list);
void		 gs_plugin_job_set_file			(GsPluginJob	*self,
							 GFile		*file);
void		 gs_plugin_job_set_plugin		(GsPluginJob	*self,
							 GsPlugin	*plugin);

#define		 gs_plugin_job_newv(a,...)		GS_PLUGIN_JOB(g_object_new(GS_TYPE_PLUGIN_JOB, "action", a, __VA_ARGS__))

#define		 GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT	(GS_APP_LIST_FILTER_FLAG_KEY_ID | \
							 GS_APP_LIST_FILTER_FLAG_KEY_SOURCE | \
							 GS_APP_LIST_FILTER_FLAG_KEY_VERSION)

G_END_DECLS
