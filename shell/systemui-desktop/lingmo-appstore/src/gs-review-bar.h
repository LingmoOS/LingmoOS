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

#define GS_TYPE_REVIEW_BAR (gs_review_bar_get_type ())

G_DECLARE_FINAL_TYPE (GsReviewBar, gs_review_bar, GS, REVIEW_BAR, GtkWidget)

GtkWidget	*gs_review_bar_new		(void);

void		 gs_review_bar_set_fraction	(GsReviewBar	*bar,
						 gdouble	 fraction);

G_END_DECLS
