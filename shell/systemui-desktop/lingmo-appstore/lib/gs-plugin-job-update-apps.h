/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022, 2023 Endless OS Foundation LLC
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

#define GS_TYPE_PLUGIN_JOB_UPDATE_APPS (gs_plugin_job_update_apps_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobUpdateApps, gs_plugin_job_update_apps, GS, PLUGIN_JOB_UPDATE_APPS, GsPluginJob)

GsPluginJob		*gs_plugin_job_update_apps_new		(GsAppList               *apps,
								 GsPluginUpdateAppsFlags  flags);

GsAppList		*gs_plugin_job_update_apps_get_apps	(GsPluginJobUpdateApps	*self);
GsPluginUpdateAppsFlags	 gs_plugin_job_update_apps_get_flags	(GsPluginJobUpdateApps	*self);

G_END_DECLS
