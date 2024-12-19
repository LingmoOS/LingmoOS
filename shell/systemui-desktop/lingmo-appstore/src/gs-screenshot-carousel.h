/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Adrien Plazas <adrien.plazas@puri.sm>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>
#include "gs-app.h"

G_BEGIN_DECLS

#define GS_TYPE_SCREENSHOT_CAROUSEL (gs_screenshot_carousel_get_type ())

G_DECLARE_FINAL_TYPE (GsScreenshotCarousel, gs_screenshot_carousel, GS, SCREENSHOT_CAROUSEL, GtkWidget)

GsScreenshotCarousel	*gs_screenshot_carousel_new	(void);
void			 gs_screenshot_carousel_load_screenshots	(GsScreenshotCarousel *self,
									 GsApp                *app,
									 gboolean              is_online,
									 GCancellable         *cancellable);
gboolean		 gs_screenshot_carousel_get_has_screenshots	(GsScreenshotCarousel *self);

G_END_DECLS
