/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_DETAILS_PAGE (gs_app_details_page_get_type ())

G_DECLARE_FINAL_TYPE (GsAppDetailsPage, gs_app_details_page, GS, APP_DETAILS_PAGE, AdwNavigationPage)

GtkWidget	*gs_app_details_page_new			(GsPluginLoader		*plugin_loader);
GsPluginLoader	*gs_app_details_page_get_plugin_loader		(GsAppDetailsPage	*page);
GsApp		*gs_app_details_page_get_app			(GsAppDetailsPage	*page);
void		 gs_app_details_page_set_app			(GsAppDetailsPage	*page,
								 GsApp			*app);

G_END_DECLS
