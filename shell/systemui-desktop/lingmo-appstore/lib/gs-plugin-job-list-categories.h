/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "gs-plugin-job.h"

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_JOB_LIST_CATEGORIES (gs_plugin_job_list_categories_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobListCategories, gs_plugin_job_list_categories, GS, PLUGIN_JOB_LIST_CATEGORIES, GsPluginJob)

GsPluginJob	*gs_plugin_job_list_categories_new		(GsPluginRefineCategoriesFlags flags);

GPtrArray	*gs_plugin_job_list_categories_get_result_list	(GsPluginJobListCategories *self);

G_END_DECLS
