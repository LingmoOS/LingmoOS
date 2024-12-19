/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation, Inc
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

#define GS_TYPE_REMOTE_ICON (gs_remote_icon_get_type ())

/* FIXME: This is actually derived from GFileIcon, but the GFileIconClass isn’t
 * public, so we use GObjectClass instead (which is what GFileIconClass is — it
 * doesn’t define any vfuncs). See the note in gs-remote-icon.c. */
G_DECLARE_FINAL_TYPE (GsRemoteIcon, gs_remote_icon, GS, REMOTE_ICON, GObject)

GIcon		*gs_remote_icon_new		(const gchar		 *uri);

const gchar	*gs_remote_icon_get_uri		(GsRemoteIcon		 *self);

gboolean	 gs_remote_icon_ensure_cached	(GsRemoteIcon		 *self,
						 SoupSession		 *soup_session,
						 guint			  maximum_icon_size,
						 GCancellable		 *cancellable,
						 GError			**error);

G_END_DECLS
