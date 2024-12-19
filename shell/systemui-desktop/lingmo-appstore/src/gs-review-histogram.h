/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_REVIEW_HISTOGRAM (gs_review_histogram_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsReviewHistogram, gs_review_histogram, GS, REVIEW_HISTOGRAM, GtkWidget)

struct _GsReviewHistogramClass
{
	GtkWidgetClass	 parent_class;
};

GtkWidget	*gs_review_histogram_new			(void);

void		 gs_review_histogram_set_ratings		(GsReviewHistogram *histogram,
								 gint rating_percent,
								 GArray *review_ratings);

G_END_DECLS
