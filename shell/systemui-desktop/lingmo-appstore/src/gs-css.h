 /* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GS_TYPE_CSS (gs_css_get_type ())

G_DECLARE_FINAL_TYPE (GsCss, gs_css, GS, CSS, GObject)

typedef gchar	*(*GsCssRewriteFunc)		(gpointer	 user_data,
						 const gchar	*markup,
						 GError		**error);

GsCss		*gs_css_new			(void);
const gchar	*gs_css_get_markup_for_id	(GsCss		*self,
						 const gchar	*id);
gboolean	 gs_css_parse			(GsCss		*self,
						 const gchar	*markup,
						 GError		**error);
gboolean	 gs_css_validate		(GsCss		*self,
						 GError		**error);
void		 gs_css_set_rewrite_func	(GsCss		*self,
						 GsCssRewriteFunc func,
						 gpointer	 user_data);

G_END_DECLS
