/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_REVIEW_DIALOG (gs_review_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsReviewDialog, gs_review_dialog, GS, REVIEW_DIALOG, AdwWindow)

GtkWidget	*gs_review_dialog_new		(void);
gint		 gs_review_dialog_get_rating	(GsReviewDialog	*dialog);
void		 gs_review_dialog_set_rating	(GsReviewDialog	*dialog,
						 gint		 rating);
const gchar	*gs_review_dialog_get_summary	(GsReviewDialog	*dialog);
gchar		*gs_review_dialog_get_text	(GsReviewDialog	*dialog);
void		 gs_review_dialog_set_error_text(GsReviewDialog	*dialog,
						 const gchar	*error_text);

G_END_DECLS
