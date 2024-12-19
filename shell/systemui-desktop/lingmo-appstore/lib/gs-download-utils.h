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
#include <libsoup/soup.h>

G_BEGIN_DECLS

SoupSession *gs_build_soup_session (void);

/**
 * GsDownloadProgressCallback:
 * @bytes_downloaded: number of bytes downloaded so far
 * @total_download_size: the total size of the download, in bytes
 * @user_data: data passed to the calling function
 *
 * A progress callback to indicate how far a download has progressed.
 *
 * @total_download_size may be zero (for example, at the start of the download),
 * so implementations of this callback must be careful to avoid division by zero
 * errors.
 *
 * @total_download_size is guaranteed to always be greater than or equal to
 * @bytes_downloaded.
 *
 * Since: 42
 */
typedef void (*GsDownloadProgressCallback) (gsize    bytes_downloaded,
                                            gsize    total_download_size,
                                            gpointer user_data);

/**
 * GsExternalAppstreamError:
 * @GS_DOWNLOAD_ERROR_NOT_MODIFIED: The ETag matches that of the server file.
 *
 * Error codes for download operations.
 *
 * Since: 44
 */
typedef enum {
	GS_DOWNLOAD_ERROR_NOT_MODIFIED,
} GsDownloadError;

#define GS_DOWNLOAD_ERROR gs_download_error_quark ()
GQuark		 gs_download_error_quark (void);

void		gs_download_stream_async	(SoupSession                *soup_session,
						 const gchar                *uri,
						 GOutputStream              *output_stream,
						 const gchar                *last_etag,
						 GDateTime                  *last_modified_date,
						 int                         io_priority,
						 GsDownloadProgressCallback  progress_callback,
						 gpointer                    progress_user_data,
						 GCancellable               *cancellable,
						 GAsyncReadyCallback         callback,
						 gpointer                    user_data);
gboolean	gs_download_stream_finish	(SoupSession   *soup_session,
						 GAsyncResult  *result,
						 gchar        **new_etag_out,
						 GDateTime    **new_last_modified_date_out,
						 GError       **error);

void		gs_download_file_async		(SoupSession                *soup_session,
						 const gchar                *uri,
						 GFile                      *output_file,
						 int                         io_priority,
						 GsDownloadProgressCallback  progress_callback,
						 gpointer                    progress_user_data,
						 GCancellable               *cancellable,
						 GAsyncReadyCallback         callback,
						 gpointer                    user_data);
gboolean	gs_download_file_finish		(SoupSession   *soup_session,
						 GAsyncResult  *result,
						 GError       **error);

void		 gs_download_rewrite_resource_async	(const gchar		*resource,
							 GCancellable		*cancellable,
							 GAsyncReadyCallback	 callback,
							 gpointer		 user_data);
gchar		*gs_download_rewrite_resource_finish	(GAsyncResult		 *result,
							 GError			**error);

G_END_DECLS
