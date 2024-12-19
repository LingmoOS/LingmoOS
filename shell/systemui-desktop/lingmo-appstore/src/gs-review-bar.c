/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <math.h>

#include "gs-review-bar.h"

struct _GsReviewBar
{
	GtkWidget	 parent_instance;
	gdouble		 fraction;
};

G_DEFINE_TYPE (GsReviewBar, gs_review_bar, GTK_TYPE_WIDGET)

void
gs_review_bar_set_fraction (GsReviewBar *bar, gdouble fraction)
{
	g_return_if_fail (GS_IS_REVIEW_BAR (bar));
	bar->fraction = fraction;
}

static void
gs_review_bar_init (GsReviewBar *bar)
{
}

static void
gs_review_bar_snapshot (GtkWidget   *widget,
                        GtkSnapshot *snapshot)
{
	gdouble bar_width, bar_height;
	GdkRGBA color;

	gtk_widget_get_color (widget, &color);

	bar_width = round (GS_REVIEW_BAR (widget)->fraction * gtk_widget_get_width (widget));
	bar_height = gtk_widget_get_height (widget);

	gtk_snapshot_append_color (snapshot,
				   &color,
				   &GRAPHENE_RECT_INIT (0,
							0,
							bar_width,
							bar_height));

	GTK_WIDGET_CLASS (gs_review_bar_parent_class)->snapshot (widget, snapshot);
}

static void
gs_review_bar_class_init (GsReviewBarClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	widget_class->snapshot = gs_review_bar_snapshot;

	gtk_widget_class_set_css_name (widget_class, "review-bar");
}

GtkWidget *
gs_review_bar_new (void)
{
	GsReviewBar *bar;
	bar = g_object_new (GS_TYPE_REVIEW_BAR, NULL);
	return GTK_WIDGET (bar);
}
