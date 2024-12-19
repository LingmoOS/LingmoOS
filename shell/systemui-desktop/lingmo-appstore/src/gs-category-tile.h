/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_CATEGORY_TILE (gs_category_tile_get_type ())

G_DECLARE_FINAL_TYPE (GsCategoryTile, gs_category_tile, GS, CATEGORY_TILE, GtkFlowBoxChild)

GtkWidget	*gs_category_tile_new			(GsCategory *cat);
GsCategory      *gs_category_tile_get_category		(GsCategoryTile	*tile);
void		 gs_category_tile_set_category		(GsCategoryTile	*tile,
							 GsCategory     *cat);

G_END_DECLS
