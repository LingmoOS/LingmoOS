/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <adwaita.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gs-review-dialog.h"
#include "gs-star-widget.h"

#define DESCRIPTION_LENGTH_MAX		3000	/* chars */
#define DESCRIPTION_LENGTH_MIN		15	/* chars */
#define SUMMARY_LENGTH_MAX		70	/* chars */
#define SUMMARY_LENGTH_MIN		3	/* chars */
#define WRITING_TIME_MIN		5	/* seconds */

struct _GsReviewDialog
{
	AdwWindow	 parent_instance;

	GtkWidget       *toast_overlay;
	GtkWidget	*star;
	GtkWidget	*label_rating_desc;
	GtkWidget	*summary_entry;
	GtkWidget	*cancel_button;
	GtkWidget	*post_button;
	GtkWidget	*text_view;
	guint		 timer_id;
};

G_DEFINE_TYPE (GsReviewDialog, gs_review_dialog, ADW_TYPE_WINDOW)

enum {
	SIGNAL_SEND,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST] = { 0 };

gint
gs_review_dialog_get_rating (GsReviewDialog *dialog)
{
	return gs_star_widget_get_rating (GS_STAR_WIDGET (dialog->star));
}

void
gs_review_dialog_set_rating (GsReviewDialog *dialog, gint rating)
{
	gs_star_widget_set_rating (GS_STAR_WIDGET (dialog->star), rating);
}

const gchar *
gs_review_dialog_get_summary (GsReviewDialog *dialog)
{
	return gtk_editable_get_text (GTK_EDITABLE (dialog->summary_entry));
}

gchar *
gs_review_dialog_get_text (GsReviewDialog *dialog)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

static void
gs_review_dialog_update_review_comment (GsReviewDialog *dialog)
{
	const gchar *msg = NULL;
	gint perc;

	/* update the rating description */
	perc = gs_star_widget_get_rating (GS_STAR_WIDGET (dialog->star));
	if (perc == 20) {
		/* TRANSLATORS: lighthearted star rating description;
		 *		A really bad app */
		msg = _("Hate it");
	} else if (perc == 40) {
		/* TRANSLATORS: lighthearted star rating description;
		 *		Not a great app */
		msg = _("Don’t like it");
	} else if (perc == 60) {
		/* TRANSLATORS: lighthearted star rating description;
		 *		A fairly-good app */
		msg = _("It’s OK");
	} else if (perc == 80) {
		/* TRANSLATORS: lighthearted star rating description;
		 *		A good app */
		msg = _("Like it");
	} else if (perc == 100) {
		/* TRANSLATORS: lighthearted star rating description;
		 *		A really awesome app */
		msg = _("Love it");
	} else {
		/* TRANSLATORS: lighthearted star rating description;
		 *		No star has been clicked yet */
		msg = _("Select a Star to Leave a Rating");
	}
	gtk_label_set_label (GTK_LABEL (dialog->label_rating_desc), msg);
}

/* (nullable) - when NULL, all is okay */
static const gchar *
gs_review_dialog_validate (GsReviewDialog *dialog)
{
	GtkTextBuffer *buffer;
	const gchar *msg = NULL;
	glong summary_length;

	/* require rating, summary and long review */
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
	summary_length = g_utf8_strlen (gtk_editable_get_text (GTK_EDITABLE (dialog->summary_entry)), -1);
	if (dialog->timer_id != 0) {
		/* TRANSLATORS: the review can't just be copied and pasted */
		msg = _("Please take more time writing the review");
	} else if (gs_star_widget_get_rating (GS_STAR_WIDGET (dialog->star)) == 0) {
		/* TRANSLATORS: the review is not acceptable */
		msg = _("Please choose a star rating");
	} else if (summary_length < SUMMARY_LENGTH_MIN) {
		/* TRANSLATORS: the review is not acceptable */
		msg = _("The summary is too short");
	} else if (summary_length > SUMMARY_LENGTH_MAX) {
		/* TRANSLATORS: the review is not acceptable */
		msg = _("The summary is too long");
	} else if (gtk_text_buffer_get_char_count (buffer) < DESCRIPTION_LENGTH_MIN) {
		/* TRANSLATORS: the review is not acceptable */
		msg = _("The description is too short");
	} else if (gtk_text_buffer_get_char_count (buffer) > DESCRIPTION_LENGTH_MAX) {
		/* TRANSLATORS: the review is not acceptable */
		msg = _("The description is too long");
	}

	return msg;
}

static void
gs_review_dialog_changed_cb (GsReviewDialog *dialog)
{
	const gchar *error_text;

	/* update review text */
	gs_review_dialog_update_review_comment (dialog);

	error_text = gs_review_dialog_validate (dialog);

	/* tell the user what's happening */
	gtk_widget_set_tooltip_text (dialog->post_button, error_text);

	/* can the user submit this? */
	gtk_widget_set_receives_default (dialog->post_button, error_text == NULL);
	if (error_text == NULL)
		gtk_widget_add_css_class (dialog->post_button, "suggested-action");
	else
		gtk_widget_remove_css_class (dialog->post_button, "suggested-action");
}

static gboolean
gs_review_dialog_timeout_cb (gpointer user_data)
{
	GsReviewDialog *dialog = GS_REVIEW_DIALOG (user_data);

	dialog->timer_id = 0;

	gs_review_dialog_changed_cb (dialog);

	return FALSE;
}

static void
gs_review_dialog_post_button_clicked_cb (GsReviewDialog *self)
{
	const gchar *error_text = gs_review_dialog_validate (self);

	if (error_text != NULL) {
		gs_review_dialog_set_error_text (self, error_text);
	} else {
		g_signal_emit (self, signals[SIGNAL_SEND], 0, NULL);
	}
}

static void
gs_review_dialog_init (GsReviewDialog *dialog)
{
	GtkTextBuffer *buffer;

	g_type_ensure (GS_TYPE_STAR_WIDGET);

	gtk_widget_init_template (GTK_WIDGET (dialog));

	/* require the user to spend at least 30 seconds on writing a review */
	dialog->timer_id = g_timeout_add_seconds (WRITING_TIME_MIN,
						  gs_review_dialog_timeout_cb,
						  dialog);

	/* update UI */
	g_signal_connect_swapped (dialog->star, "rating-changed",
				  G_CALLBACK (gs_review_dialog_changed_cb), dialog);
	g_signal_connect_swapped (dialog->summary_entry, "notify::text",
				  G_CALLBACK (gs_review_dialog_changed_cb), dialog);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->text_view));
	g_signal_connect_swapped (buffer, "changed",
				  G_CALLBACK (gs_review_dialog_changed_cb), dialog);
	g_signal_connect_swapped (dialog->cancel_button, "clicked",
				  G_CALLBACK (gtk_window_destroy), dialog);
	g_signal_connect_swapped (dialog->post_button, "clicked",
				  G_CALLBACK (gs_review_dialog_post_button_clicked_cb), dialog);

	gs_review_dialog_changed_cb (dialog);
}

static void
gs_review_row_dispose (GObject *object)
{
	GsReviewDialog *dialog = GS_REVIEW_DIALOG (object);
	if (dialog->timer_id > 0) {
		g_source_remove (dialog->timer_id);
		dialog->timer_id = 0;
	}
	G_OBJECT_CLASS (gs_review_dialog_parent_class)->dispose (object);
}

static void
gs_review_dialog_class_init (GsReviewDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = gs_review_row_dispose;

	/**
	 * GsReviewDialog::send:
	 * @self: the #GsReviewDialog
	 *
	 * Emitted when the user clicks on the Send button to send the review.
	 *
	 * Since: 45
	 */
	signals[SIGNAL_SEND] =
		g_signal_new ("send",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0, G_TYPE_NONE);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-review-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, toast_overlay);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, star);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, label_rating_desc);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, summary_entry);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, text_view);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, cancel_button);
	gtk_widget_class_bind_template_child (widget_class, GsReviewDialog, post_button);
}

GtkWidget *
gs_review_dialog_new (void)
{
	return GTK_WIDGET (g_object_new (GS_TYPE_REVIEW_DIALOG,
					 NULL));
}

void
gs_review_dialog_set_error_text (GsReviewDialog *dialog,
				 const gchar *error_text)
{
	AdwToast *toast;

	g_return_if_fail (GS_IS_REVIEW_DIALOG (dialog));
	g_return_if_fail (error_text != NULL);

	toast = adw_toast_new (error_text);

	adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (dialog->toast_overlay), toast);
}
