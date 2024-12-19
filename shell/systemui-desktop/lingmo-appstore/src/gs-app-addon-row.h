/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_ADDON_ROW (gs_app_addon_row_get_type ())

G_DECLARE_FINAL_TYPE (GsAppAddonRow, gs_app_addon_row, GS, APP_ADDON_ROW, GtkListBoxRow)

GtkWidget	*gs_app_addon_row_new			(GsApp		*app);
void		 gs_app_addon_row_refresh		(GsAppAddonRow	*row);
void		 gs_app_addon_row_activate		(GsAppAddonRow	*row);
GsApp		*gs_app_addon_row_get_addon		(GsAppAddonRow	*row);

G_END_DECLS
