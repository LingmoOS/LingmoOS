/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "gs-app.h"

G_BEGIN_DECLS

#define GS_TYPE_LICENSE_TILE (gs_license_tile_get_type ())

G_DECLARE_FINAL_TYPE (GsLicenseTile, gs_license_tile, GS, LICENSE_TILE, GtkWidget)

GtkWidget	*gs_license_tile_new		(GsApp *app);

GsApp		*gs_license_tile_get_app	(GsLicenseTile *self);
void		 gs_license_tile_set_app	(GsLicenseTile *self,
						 GsApp         *app);

G_END_DECLS
