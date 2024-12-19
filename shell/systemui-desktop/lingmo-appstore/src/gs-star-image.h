/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_STAR_IMAGE		(gs_star_image_get_type ())

G_DECLARE_FINAL_TYPE (GsStarImage, gs_star_image, GS, STAR_IMAGE, GtkWidget)

GtkWidget *	gs_star_image_new		(void);
void		gs_star_image_set_fraction	(GsStarImage *self,
						 gdouble fraction);
gdouble		gs_star_image_get_fraction	(GsStarImage *self);
void		gs_star_image_set_pixel_size	(GsStarImage *self,
						 gint fraction);
gint		gs_star_image_get_pixel_size	(GsStarImage *self);

G_END_DECLS
