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

#define GS_TYPE_PLUGIN_JOB_LIST_DISTRO_UPGRADES (gs_plugin_job_list_distro_upgrades_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobListDistroUpgrades, gs_plugin_job_list_distro_upgrades, GS, PLUGIN_JOB_LIST_DISTRO_UPGRADES, GsPluginJob)

GsPluginJob	*gs_plugin_job_list_distro_upgrades_new	(GsPluginListDistroUpgradesFlags flags,
							 GsPluginRefineFlags             refine_flags);

GsAppList	*gs_plugin_job_list_distro_upgrades_get_result_list	(GsPluginJobListDistroUpgrades *self);

G_END_DECLS
