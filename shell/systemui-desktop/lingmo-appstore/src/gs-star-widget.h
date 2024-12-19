/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_STAR_WIDGET (gs_star_widget_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsStarWidget, gs_star_widget, GS, STAR_WIDGET, GtkWidget)

struct _GsStarWidgetClass
{
	GtkWidgetClass	 parent_class;

	void			(*rating_changed)		(GsStarWidget	*star);
};

GtkWidget	*gs_star_widget_new			(void);
gint		 gs_star_widget_get_rating		(GsStarWidget	*star);
void		 gs_star_widget_set_rating		(GsStarWidget	*star,
							 gint		 rating);
void		 gs_star_widget_set_icon_size		(GsStarWidget	*star,
							 guint		 pixel_size);
void		 gs_star_widget_set_interactive		(GsStarWidget	*star,
							 gboolean	 interactive);

G_END_DECLS
