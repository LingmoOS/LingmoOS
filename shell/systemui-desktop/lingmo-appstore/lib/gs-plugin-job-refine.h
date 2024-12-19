/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

#include "gs-plugin-job.h"

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_JOB_REFINE (gs_plugin_job_refine_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobRefine, gs_plugin_job_refine, GS, PLUGIN_JOB_REFINE, GsPluginJob)

GsPluginJob	*gs_plugin_job_refine_new_for_app	(GsApp               *app,
							 GsPluginRefineFlags  flags);
GsPluginJob	*gs_plugin_job_refine_new		(GsAppList           *app_list,
							 GsPluginRefineFlags  flags);

GsAppList	*gs_plugin_job_refine_get_result_list	(GsPluginJobRefine   *self);

G_END_DECLS
