/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat (www.redhat.com)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_LOZENGE (gs_lozenge_get_type ())
G_DECLARE_FINAL_TYPE (GsLozenge, gs_lozenge, GS, LOZENGE, GtkBox)

GtkWidget *	gs_lozenge_new			(void);
gboolean	gs_lozenge_get_circular		(GsLozenge *self);
void		gs_lozenge_set_circular		(GsLozenge *self,
						 gboolean value);
const gchar *	gs_lozenge_get_icon_name	(GsLozenge *self);
void		gs_lozenge_set_icon_name	(GsLozenge *self,
						 const gchar *value);
gint		gs_lozenge_get_pixel_size	(GsLozenge *self);
void		gs_lozenge_set_pixel_size	(GsLozenge *self,
						 gint value);
gboolean	gs_lozenge_get_use_markup	(GsLozenge *self);
const gchar *	gs_lozenge_get_text		(GsLozenge *self);
void		gs_lozenge_set_text		(GsLozenge *self,
						 const gchar *value);
const gchar *	gs_lozenge_get_markup		(GsLozenge *self);
void		gs_lozenge_set_markup		(GsLozenge *self,
						 const gchar *value);

G_END_DECLS
