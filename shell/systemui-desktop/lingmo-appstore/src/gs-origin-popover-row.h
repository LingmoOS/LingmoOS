/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "lingmo-appstore-private.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_ORIGIN_POPOVER_ROW (gs_origin_popover_row_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsOriginPopoverRow, gs_origin_popover_row, GS, ORIGIN_POPOVER_ROW, GtkListBoxRow)

struct _GsOriginPopoverRowClass
{
	GtkListBoxRowClass	  parent_class;
};

GtkWidget	*gs_origin_popover_row_new		(GsApp			*app);
GsApp		*gs_origin_popover_row_get_app		(GsOriginPopoverRow	*row);
void		 gs_origin_popover_row_set_selected	(GsOriginPopoverRow	*row,
							 gboolean		 selected);

G_END_DECLS
