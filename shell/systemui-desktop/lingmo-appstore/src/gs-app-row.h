/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_ROW (gs_app_row_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsAppRow, gs_app_row, GS, APP_ROW, GtkListBoxRow)

struct _GsAppRowClass
{
	GtkListBoxRowClass	 parent_class;
	void			(*button_clicked)	(GsAppRow	*app_row);
	void			(*unrevealed)		(GsAppRow	*app_row);
};

GtkWidget	*gs_app_row_new				(GsApp		*app);
void		 gs_app_row_unreveal			(GsAppRow	*app_row);
void		 gs_app_row_set_colorful		(GsAppRow	*app_row,
							 gboolean	 colorful);
void		 gs_app_row_set_show_buttons		(GsAppRow	*app_row,
							 gboolean	 show_buttons);
void		 gs_app_row_set_show_rating		(GsAppRow	*app_row,
							 gboolean	 show_rating);
gboolean	 gs_app_row_get_show_description	(GsAppRow	*app_row);
void		 gs_app_row_set_show_description	(GsAppRow	*app_row,
							 gboolean	 show_description);
void		 gs_app_row_set_show_source		(GsAppRow	*app_row,
							 gboolean	 show_source);
void		 gs_app_row_set_show_update		(GsAppRow	*app_row,
							 gboolean	 show_update);
void		 gs_app_row_set_show_installed		(GsAppRow	*app_row,
							 gboolean	 show_installed);
GsApp		*gs_app_row_get_app			(GsAppRow	*app_row);
void		 gs_app_row_set_size_groups		(GsAppRow	*app_row,
							 GtkSizeGroup	*name,
							 GtkSizeGroup	*button_label,
							 GtkSizeGroup	*button_image);
void		 gs_app_row_set_show_installed_size	(GsAppRow	*app_row,
							 gboolean	 show_size);
gboolean	 gs_app_row_get_is_narrow		(GsAppRow	*app_row);
void		 gs_app_row_set_is_narrow		(GsAppRow	*app_row,
							 gboolean	 is_narrow);

G_END_DECLS
