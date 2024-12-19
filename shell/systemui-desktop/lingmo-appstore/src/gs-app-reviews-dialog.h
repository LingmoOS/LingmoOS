/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Adrien Plazas <adrien.plazas@puri.sm>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_APP_REVIEWS_DIALOG (gs_app_reviews_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsAppReviewsDialog, gs_app_reviews_dialog, GS, APP_REVIEWS_DIALOG, AdwWindow)

GtkWidget	*gs_app_reviews_dialog_new	(GtkWindow	*parent,
						 GsApp		*app,
						 GsOdrsProvider	*odrs_provider,
						 GsPluginLoader	*plugin_loader);

GsApp	*gs_app_reviews_dialog_get_app	(GsAppReviewsDialog	*self);
void	 gs_app_reviews_dialog_set_app	(GsAppReviewsDialog	*self,
					 GsApp			*app);

GsOdrsProvider	*gs_app_reviews_dialog_get_odrs_provider	(GsAppReviewsDialog	*self);
void		 gs_app_reviews_dialog_set_odrs_provider	(GsAppReviewsDialog	*self,
								 GsOdrsProvider		*odrs_provider);

GsPluginLoader	*gs_app_reviews_dialog_get_plugin_loader	(GsAppReviewsDialog	*self);
void		 gs_app_reviews_dialog_set_plugin_loader	(GsAppReviewsDialog	*self,
								 GsPluginLoader		*plugin_loader);

G_END_DECLS
