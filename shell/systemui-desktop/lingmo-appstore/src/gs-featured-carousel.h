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

#include "gs-app-list.h"

G_BEGIN_DECLS

#define GS_TYPE_FEATURED_CAROUSEL (gs_featured_carousel_get_type ())

G_DECLARE_FINAL_TYPE (GsFeaturedCarousel, gs_featured_carousel, GS, FEATURED_CAROUSEL, GtkBox)

GtkWidget	*gs_featured_carousel_new	(GsAppList		*apps);

GsAppList	*gs_featured_carousel_get_apps	(GsFeaturedCarousel	*self);
void		 gs_featured_carousel_set_apps	(GsFeaturedCarousel	*self,
						 GsAppList		*apps);

G_END_DECLS
