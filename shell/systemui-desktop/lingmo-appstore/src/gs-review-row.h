/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

/**
 * GsReviewAction:
 * @GS_REVIEW_ACTION_UPVOTE: Add a vote to the review.
 * @GS_REVIEW_ACTION_DOWNVOTE: Remove a vote from the review.
 * @GS_REVIEW_ACTION_DISMISS: Dismiss (ignore) the review when moderating.
 * @GS_REVIEW_ACTION_REPORT: Report the review for inappropriate content.
 * @GS_REVIEW_ACTION_REMOVE: Remove one of your own reviews.
 *
 * Actions which can be performed on a review.
 *
 * Since: 41
 */
typedef enum
{
	GS_REVIEW_ACTION_UPVOTE,
	GS_REVIEW_ACTION_DOWNVOTE,
	GS_REVIEW_ACTION_DISMISS,
	GS_REVIEW_ACTION_REPORT,
	GS_REVIEW_ACTION_REMOVE,
} GsReviewAction;

#define GS_TYPE_REVIEW_ROW (gs_review_row_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsReviewRow, gs_review_row, GS, REVIEW_ROW, GtkListBoxRow)

struct _GsReviewRowClass
{
	GtkListBoxRowClass	 parent_class;
	void			(*button_clicked)	(GsReviewRow	*review_row,
							 GsPluginAction	 action);
};

GtkWidget	*gs_review_row_new		(AsReview	*review);
AsReview	*gs_review_row_get_review	(GsReviewRow	*review_row);
void		 gs_review_row_set_actions	(GsReviewRow	*review_row,
						 guint64	 actions);
void		 gs_review_row_set_network_available	(GsReviewRow	*review_row,
							 gboolean	 network_available);
void		 gs_review_row_refresh		(GsReviewRow	*row);

G_END_DECLS
