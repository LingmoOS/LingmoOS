/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2019 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_TILE (gs_app_tile_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsAppTile, gs_app_tile, GS, APP_TILE, GtkFlowBoxChild)

struct _GsAppTileClass
{
	GtkFlowBoxChildClass		parent_class;
	void			 (*refresh)		(GsAppTile	*self);
};

GsApp		*gs_app_tile_get_app	(GsAppTile	*self);
void		 gs_app_tile_set_app	(GsAppTile	*self,
					 GsApp		*app);

G_END_DECLS
