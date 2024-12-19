/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_LANGUAGE (gs_language_get_type ())

G_DECLARE_FINAL_TYPE (GsLanguage, gs_language, GS, LANGUAGE, GObject)

GsLanguage	*gs_language_new			(void);
gboolean	 gs_language_populate			(GsLanguage	 *language,
							 GError		**error);
gchar		*gs_language_iso639_to_language		(GsLanguage	 *language,
							 const gchar	 *iso639);

G_END_DECLS
