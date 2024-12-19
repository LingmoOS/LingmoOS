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
#include "gs-info-window.h"
#include "gs-lozenge.h"

G_BEGIN_DECLS

#define GS_TYPE_AGE_RATING_CONTEXT_DIALOG (gs_age_rating_context_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsAgeRatingContextDialog, gs_age_rating_context_dialog, GS, AGE_RATING_CONTEXT_DIALOG, GsInfoWindow)

GsAgeRatingContextDialog	*gs_age_rating_context_dialog_new		(GsApp				*app);

GsApp				*gs_age_rating_context_dialog_get_app		(GsAgeRatingContextDialog	*self);
void				 gs_age_rating_context_dialog_set_app		(GsAgeRatingContextDialog	*self,
										 GsApp				*app);

gchar *gs_age_rating_context_dialog_format_age_short (AsContentRatingSystem system,
                                                      guint                 age);
void gs_age_rating_context_dialog_update_lozenge (GsApp     *app,
                                                  GsLozenge *lozenge,
                                                  gboolean  *is_unknown_out);


typedef void (*GsAgeRatingContextDialogAttributeFunc) (const gchar          *attribute,
                                                       AsContentRatingValue  value,
                                                       gpointer              user_data);

void gs_age_rating_context_dialog_process_attributes (AsContentRating                       *content_rating,
                                                      gboolean                               show_worst_only,
                                                      GsAgeRatingContextDialogAttributeFunc  callback,
                                                      gpointer                               user_data);

G_END_DECLS
