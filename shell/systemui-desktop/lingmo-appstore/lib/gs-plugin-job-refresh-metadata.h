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

#define GS_TYPE_PLUGIN_JOB_REFRESH_METADATA (gs_plugin_job_refresh_metadata_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobRefreshMetadata, gs_plugin_job_refresh_metadata, GS, PLUGIN_JOB_REFRESH_METADATA, GsPluginJob)

GsPluginJob	*gs_plugin_job_refresh_metadata_new	(guint64                      cache_age_secs,
							 GsPluginRefreshMetadataFlags flags);

G_END_DECLS
