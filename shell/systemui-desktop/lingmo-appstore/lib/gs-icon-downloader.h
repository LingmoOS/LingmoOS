/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2023 Endless OS Foundation LLC
 *
 * Authors:
 *  - Georges Basile Stavracas Neto <georges@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <gio/gio.h>

#include <libsoup/soup.h>

#include "gs-app.h"

G_BEGIN_DECLS

#define GS_TYPE_ICON_DOWNLOADER (gs_icon_downloader_get_type ())

G_DECLARE_FINAL_TYPE (GsIconDownloader, gs_icon_downloader, GS, ICON_DOWNLOADER, GObject)

GsIconDownloader	*gs_icon_downloader_new			(SoupSession		*soup_session,
								 guint			 maximum_size);

void			 gs_icon_downloader_queue_app		(GsIconDownloader	*self,
								 GsApp			*app,
								 gboolean		 interactive);

void		 	gs_icon_downloader_shutdown_async	(GsIconDownloader	*self,
								 GCancellable		*cancellable,
								 GAsyncReadyCallback	 callback,
								 gpointer		 user_data);

gboolean	 	gs_icon_downloader_shutdown_finish	(GsIconDownloader  *self,
								 GAsyncResult    *result,
								 GError         **error);

G_END_DECLS
