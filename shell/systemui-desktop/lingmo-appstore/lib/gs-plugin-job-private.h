/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-plugin-job.h"

G_BEGIN_DECLS

GsPluginAction		 gs_plugin_job_get_action		(GsPluginJob	*self);
GsAppListFilterFlags	 gs_plugin_job_get_dedupe_flags		(GsPluginJob	*self);
GsPluginRefineFlags	 gs_plugin_job_get_refine_flags		(GsPluginJob	*self);
gboolean		 gs_plugin_job_has_refine_flags		(GsPluginJob	*self,
								 GsPluginRefineFlags refine_flags);
void			 gs_plugin_job_add_refine_flags		(GsPluginJob	*self,
								 GsPluginRefineFlags refine_flags);
void			 gs_plugin_job_remove_refine_flags	(GsPluginJob	*self,
								 GsPluginRefineFlags refine_flags);
gboolean		 gs_plugin_job_get_interactive		(GsPluginJob	*self);
gboolean		 gs_plugin_job_get_propagate_error	(GsPluginJob	*self);
guint			 gs_plugin_job_get_max_results		(GsPluginJob	*self);
const gchar		*gs_plugin_job_get_search		(GsPluginJob	*self);
GsApp			*gs_plugin_job_get_app			(GsPluginJob	*self);
GsAppList		*gs_plugin_job_get_list			(GsPluginJob	*self);
GFile			*gs_plugin_job_get_file			(GsPluginJob	*self);
GsPlugin		*gs_plugin_job_get_plugin		(GsPluginJob	*self);
gchar			*gs_plugin_job_to_string		(GsPluginJob	*self);
void			 gs_plugin_job_set_action		(GsPluginJob	*self,
								 GsPluginAction	 action);
void			 gs_plugin_job_set_cancellable		(GsPluginJob	*self,
								 GCancellable	*cancellable);
void			 gs_plugin_job_cancel			(GsPluginJob	*self);

G_END_DECLS
