/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Endless Mobile, Inc.
 *
 * Authors: Joaquim Rocha <jrocha@endlessm.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <lingmo-appstore.h>

#define EXTERNAL_APPSTREAM_PREFIX "org.lingmo.Appstore"

/**
 * GsExternalAppstreamError:
 * @GS_EXTERNAL_APPSTREAM_ERROR_DOWNLOADING: Error while downloading external appstream data.
 * @GS_EXTERNAL_APPSTREAM_ERROR_NO_NETWORK: Offline or network unavailable.
 * @GS_EXTERNAL_APPSTREAM_ERROR_INSTALLING_ON_SYSTEM: Error while installing an external AppStream file system-wide.
 *
 * Error codes for external appstream operations.
 *
 * Since: 42
 */
typedef enum {
	GS_EXTERNAL_APPSTREAM_ERROR_DOWNLOADING,
	GS_EXTERNAL_APPSTREAM_ERROR_NO_NETWORK,
	GS_EXTERNAL_APPSTREAM_ERROR_INSTALLING_ON_SYSTEM,
} GsExternalAppstreamError;

#define GS_EXTERNAL_APPSTREAM_ERROR gs_external_appstream_error_quark ()
GQuark		 gs_external_appstream_error_quark (void);

const gchar	*gs_external_appstream_utils_get_system_dir (void);
gchar		*gs_external_appstream_utils_get_file_cache_path (const gchar	*file_name);
gchar		*gs_external_appstream_utils_get_legacy_file_cache_path (const gchar *file_name);

void		 gs_external_appstream_refresh_async (guint64                     cache_age_secs,
						      GsDownloadProgressCallback  progress_callback,
						      gpointer                    progress_user_data,
						      GCancellable               *cancellable,
						      GAsyncReadyCallback         callback,
						      gpointer                    user_data);
gboolean	 gs_external_appstream_refresh_finish (GAsyncResult  *result,
						       GError       **error);
