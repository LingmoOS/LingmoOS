/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2020 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-desription-box
 * @title: GsDescriptionBox
 * @stability: Stable
 * @short_description: Show description text in a way that can show more/less lines
 *
 * Show a description in an expandable form with "Show More" button when
 * there are too many lines to be shown. The button is hidden when
 * the description is short enough. The button changes to "Show Less"
 * to be able to collapse it.
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-description-box.h"

#define MAX_COLLAPSED_LINES 4

/* How many lines should be hidden at least, to not "save"
   less space than the button height. */
#define MIN_HIDDEN_LINES 3

struct _GsDescriptionBox {
	GtkWidget parent;
	GtkWidget *box;
	GtkLabel *label;
	GtkButton *button;
	gchar *text;
	gboolean is_collapsed;
	gboolean always_expanded;
	gboolean needs_recalc;
	gint last_width;
	gint last_height;
	guint idle_update_id;
};

G_DEFINE_TYPE (GsDescriptionBox, gs_description_box, GTK_TYPE_WIDGET)

typedef enum {
	PROP_ALWAYS_EXPANDED = 1,
	PROP_COLLAPSED,
	PROP_TEXT,
} GsDescriptionBoxProperty;

static GParamSpec *obj_props[PROP_TEXT + 1] = { NULL, };

static void
gs_description_box_update_content (GsDescriptionBox *box)
{
	gint width, height;
	PangoLayout *layout;
	gint n_lines;
	gboolean visible;
	const gchar *text;

	if (!box->text || !*(box->text)) {
		gtk_widget_set_visible (GTK_WIDGET (box), FALSE);
		box->needs_recalc = TRUE;
		return;
	}

	width = gtk_widget_get_width (GTK_WIDGET (box));
	height = gtk_widget_get_height (GTK_WIDGET (box));

	if (!box->needs_recalc && box->last_width == width && box->last_height == height)
		return;

	if ((!gtk_widget_get_visible (GTK_WIDGET (box->button))) == (!box->always_expanded))
		gtk_widget_set_visible (GTK_WIDGET (box->button), !box->always_expanded);

	box->needs_recalc = width <= 1 || height <= 1;
	box->last_width = width;
	box->last_height = height;

	if (box->always_expanded) {
		gtk_widget_set_visible (GTK_WIDGET (box->button), FALSE);
		gtk_label_set_markup (box->label, box->text);
		gtk_label_set_lines (box->label, -1);
		gtk_label_set_ellipsize (box->label, PANGO_ELLIPSIZE_NONE);
		return;
	}

	text = box->is_collapsed ? _("_Show More") : _("_Show Less");
	/* FIXME: Work around a flickering issue in GTK:
	 * https://github.com/LingmoOS/gtk/-/merge_requests/3949 */
	if (g_strcmp0 (text, gtk_button_get_label (box->button)) != 0)
		gtk_button_set_label (box->button, text);

	gtk_label_set_markup (box->label, box->text);
	gtk_label_set_lines (box->label, -1);
	gtk_label_set_ellipsize (box->label, PANGO_ELLIPSIZE_NONE);

	layout = gtk_label_get_layout (box->label);
	n_lines = pango_layout_get_line_count (layout);
	visible = n_lines > MAX_COLLAPSED_LINES && n_lines - MAX_COLLAPSED_LINES >= MIN_HIDDEN_LINES;

	gtk_widget_set_visible (GTK_WIDGET (box->button), visible);

	if (box->is_collapsed && visible) {
		PangoLayoutLine *line;
		GString *str;
		GSList *opened_markup = NULL;
		gint start_index, line_index, in_markup = 0;

		line = pango_layout_get_line_readonly (layout, MAX_COLLAPSED_LINES);

		line_index = line->start_index;

		/* Pango does not count markup in the text, thus calculate the position manually */
		for (start_index = 0; box->text[start_index] && line_index > 0; start_index++) {
			if (box->text[start_index] == '<') {
				if (box->text[start_index + 1] == '/') {
					g_autofree gchar *value = opened_markup->data;
					opened_markup = g_slist_remove (opened_markup, value);
				} else {
					const gchar *end = strchr (box->text + start_index, '>');
					opened_markup = g_slist_prepend (opened_markup, g_strndup (box->text + start_index + 1, end - (box->text + start_index) - 1));
				}
				in_markup++;
			} else if (box->text[start_index] == '>') {
				g_warn_if_fail (in_markup > 0);
				in_markup--;
			} else if (!in_markup) {
				/* Encoded characters count as one */
				if (box->text[start_index] == '&') {
					const gchar *end = strchr (box->text + start_index, ';');
					if (end)
						start_index += end - box->text - start_index;
				}

				line_index--;
			}
		}
		str = g_string_sized_new (start_index);
		g_string_append_len (str, box->text, start_index);

		/* Cut white spaces from the end of the string, thus it doesn't look bad when it's ellipsized. */
		while (str->len > 0 && strchr ("\r\n\t ", str->str[str->len - 1])) {
			str->len--;
		}

		str->str[str->len] = '\0';

		/* Close any opened tags after cutting the text */
		for (GSList *link = opened_markup; link; link = g_slist_next (link)) {
			const gchar *tag = link->data;
			g_string_append_printf (str, "</%s>", tag);
		}

		gtk_label_set_lines (box->label, MAX_COLLAPSED_LINES);
		gtk_label_set_ellipsize (box->label, PANGO_ELLIPSIZE_END);
		gtk_label_set_markup (box->label, str->str);

		g_slist_free_full (opened_markup, g_free);
		g_string_free (str, TRUE);
	}
}

static void
gs_description_box_read_button_clicked_cb (GtkButton *button,
					   gpointer user_data)
{
	GsDescriptionBox *box = user_data;

	g_return_if_fail (GS_IS_DESCRIPTION_BOX (box));

	box->is_collapsed = !box->is_collapsed;
	box->needs_recalc = TRUE;

	gs_description_box_update_content (box);
}

static gboolean
update_description_in_idle_cb (gpointer data)
{
	GsDescriptionBox *box = GS_DESCRIPTION_BOX (data);

	gs_description_box_update_content (box);
	box->idle_update_id = 0;

	return G_SOURCE_REMOVE;
}

static void
gs_description_box_measure (GtkWidget      *widget,
                            GtkOrientation  orientation,
                            gint            for_size,
                            gint           *minimum,
                            gint           *natural,
                            gint           *minimum_baseline,
                            gint           *natural_baseline)
{
	GsDescriptionBox *box = GS_DESCRIPTION_BOX (widget);

	gtk_widget_measure (box->box, orientation,
			    for_size,
			    minimum, natural,
			    minimum_baseline,
			    natural_baseline);

	if (!box->idle_update_id)
		box->idle_update_id = g_idle_add (update_description_in_idle_cb, box);
}

static void
gs_description_box_size_allocate (GtkWidget *widget,
                                  int        width,
                                  int        height,
                                  int        baseline)
{
	GsDescriptionBox *box = GS_DESCRIPTION_BOX (widget);
	GtkAllocation allocation;

	allocation.x = 0;
	allocation.y = 0;
	allocation.width = width;
	allocation.height = height;

	gtk_widget_size_allocate (box->box, &allocation, baseline);

	if (!box->idle_update_id)
		box->idle_update_id = g_idle_add (update_description_in_idle_cb, box);
}

static GtkSizeRequestMode
gs_description_box_get_request_mode (GtkWidget *widget)
{
	return gtk_widget_get_request_mode (GS_DESCRIPTION_BOX (widget)->box);
}

static void
gs_description_box_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	GsDescriptionBox *self = GS_DESCRIPTION_BOX (object);

	switch ((GsDescriptionBoxProperty) prop_id) {
	case PROP_ALWAYS_EXPANDED:
		g_value_set_boolean (value, gs_description_box_get_always_expanded (self));
		break;
	case PROP_COLLAPSED:
		g_value_set_boolean (value, gs_description_box_get_collapsed (self));
		break;
	case PROP_TEXT:
		g_value_set_string (value, gs_description_box_get_text (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_description_box_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	GsDescriptionBox *self = GS_DESCRIPTION_BOX (object);

	switch ((GsDescriptionBoxProperty) prop_id) {
	case PROP_ALWAYS_EXPANDED:
		gs_description_box_set_always_expanded (self, g_value_get_boolean (value));
		break;
	case PROP_COLLAPSED:
		gs_description_box_set_collapsed (self, g_value_get_boolean (value));
		break;
	case PROP_TEXT:
		gs_description_box_set_text (self, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_description_box_dispose (GObject *object)
{
	GsDescriptionBox *box = GS_DESCRIPTION_BOX (object);

	g_clear_handle_id (&box->idle_update_id, g_source_remove);
	g_clear_pointer (&box->box, gtk_widget_unparent);

	G_OBJECT_CLASS (gs_description_box_parent_class)->dispose (object);
}

static void
gs_description_box_finalize (GObject *object)
{
	GsDescriptionBox *box = GS_DESCRIPTION_BOX (object);

	g_clear_pointer (&box->text, g_free);

	G_OBJECT_CLASS (gs_description_box_parent_class)->finalize (object);
}

static void
gs_description_box_init (GsDescriptionBox *box)
{
	GtkWidget *widget;

	box->is_collapsed = TRUE;
	box->always_expanded = FALSE;

	box->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
	gtk_widget_set_parent (GTK_WIDGET (box->box), GTK_WIDGET (box));

	gtk_widget_add_css_class (GTK_WIDGET (box), "application-details-description");

	widget = gtk_label_new ("");
	g_object_set (G_OBJECT (widget),
		"hexpand", TRUE,
		"halign", GTK_ALIGN_FILL,
		"vexpand", FALSE,
		"valign", GTK_ALIGN_START,
		"visible", TRUE,
		"max-width-chars", 40,
		"selectable", TRUE,
		"wrap", TRUE,
		"xalign", 0.0,
		NULL);

	gtk_box_append (GTK_BOX (box->box), widget);

	gtk_widget_add_css_class (GTK_WIDGET (box), "label");

	box->label = GTK_LABEL (widget);

	widget = gtk_button_new_with_mnemonic (_("_Show More"));

	g_object_set (G_OBJECT (widget),
		"hexpand", FALSE,
		"halign", GTK_ALIGN_CENTER,
		"vexpand", FALSE,
		"valign", GTK_ALIGN_CENTER,
		"visible", TRUE,
		NULL);

	gtk_box_append (GTK_BOX (box->box), widget);

	gtk_widget_add_css_class (widget, "button");
	gtk_widget_add_css_class (widget, "circular");

	box->button = GTK_BUTTON (widget);

	g_signal_connect (box->button, "clicked",
		G_CALLBACK (gs_description_box_read_button_clicked_cb), box);
}

static void
gs_description_box_class_init (GsDescriptionBoxClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = gs_description_box_get_property;
	object_class->set_property = gs_description_box_set_property;
	object_class->dispose = gs_description_box_dispose;
	object_class->finalize = gs_description_box_finalize;

	widget_class = GTK_WIDGET_CLASS (klass);
	widget_class->get_request_mode = gs_description_box_get_request_mode;
	widget_class->measure = gs_description_box_measure;
	widget_class->size_allocate = gs_description_box_size_allocate;

	/**
	 * GsDescriptionBox:always-expanded:
	 *
	 * If always expanded, the ‘Show More’ button will be hidden, and the box’s
	 * content will not be truncated. It will all always be shown.
	 *
	 * This property is useful to allow a single widget tree using #GsDescriptionBox
	 * to be used in situations where sometimes its expanding/truncating behaviour
	 * isn’t needed.
	 *
	 * The text is not shown as always expanded by default.
	 *
	 * Since: 44
	 */
	obj_props[PROP_ALWAYS_EXPANDED] =
		g_param_spec_boolean ("always-expanded", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsDescriptionBox:collapsed:
	 *
	 * Whether the text is currently collapsed. When being collapsed,
	 * and the text is long enough, there's a "Show More" button shown.
	 *
	 * The text is collapsed by default.
	 *
	 * Since: 44
	 */
	obj_props[PROP_COLLAPSED] =
		g_param_spec_boolean ("collapsed", NULL, NULL,
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsDescriptionBox:text:
	 *
	 * Text shown in the description box. It's interpreted
	 * as a markup, not as a plain text.
	 *
	 * Since: 44
	 */
	obj_props[PROP_TEXT] =
		g_param_spec_string ("text", NULL, NULL,
				      NULL,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

GtkWidget *
gs_description_box_new (void)
{
	return g_object_new (GS_TYPE_DESCRIPTION_BOX, NULL);
}

const gchar *
gs_description_box_get_text (GsDescriptionBox *box)
{
	g_return_val_if_fail (GS_IS_DESCRIPTION_BOX (box), NULL);

	return box->text;
}

void
gs_description_box_set_text (GsDescriptionBox *box,
			     const gchar *text)
{
	g_return_if_fail (GS_IS_DESCRIPTION_BOX (box));

	if (g_strcmp0 (text, box->text) != 0) {
		g_free (box->text);
		box->text = g_strdup (text);
		box->needs_recalc = TRUE;

		gtk_widget_set_visible (GTK_WIDGET (box), text && *text);

		/* Set the text and everything immediately, to avoid screen flickering
		   when no button will be shown anyway */
		if (box->always_expanded)
			gs_description_box_update_content (box);

		gtk_widget_queue_resize (GTK_WIDGET (box));

		g_object_notify_by_pspec (G_OBJECT (box), obj_props[PROP_TEXT]);
	}
}

gboolean
gs_description_box_get_collapsed (GsDescriptionBox *box)
{
	g_return_val_if_fail (GS_IS_DESCRIPTION_BOX (box), FALSE);

	return box->is_collapsed;
}

void
gs_description_box_set_collapsed (GsDescriptionBox *box,
				  gboolean collapsed)
{
	g_return_if_fail (GS_IS_DESCRIPTION_BOX (box));

	if ((!collapsed) != (!box->is_collapsed)) {
		box->is_collapsed = collapsed;
		box->needs_recalc = TRUE;

		gtk_widget_queue_resize (GTK_WIDGET (box));

		g_object_notify_by_pspec (G_OBJECT (box), obj_props[PROP_COLLAPSED]);
	}
}

gboolean
gs_description_box_get_always_expanded (GsDescriptionBox *box)
{
	g_return_val_if_fail (GS_IS_DESCRIPTION_BOX (box), FALSE);

	return box->always_expanded;
}

/**
 * gs_description_box_set_always_expanded:
 * @box: a #GsDescriptionBox
 * @always_expanded: %TRUE to always expand the box, %FALSE otherwise
 *
 * Set whether to always expand the box.
 *
 * If always expanded, the ‘Show More’ button will be hidden, and the box’s
 * content will not be truncated. It will all always be shown.
 *
 * This property is useful to allow a single widget tree using #GsDescriptionBox
 * to be used in situations where sometimes its expanding/truncating behaviour
 * isn’t needed.
 *
 * Since: 44
 */
void
gs_description_box_set_always_expanded (GsDescriptionBox *box,
					gboolean always_expanded)
{
	g_return_if_fail (GS_IS_DESCRIPTION_BOX (box));

	if ((!always_expanded) != (!box->always_expanded)) {
		box->always_expanded = always_expanded;
		box->needs_recalc = TRUE;

		/* Hide the button immediately, because the rest is loaded on resize,
		   which shows it in the GUI otherwise */
		if (box->always_expanded)
			gtk_widget_set_visible (GTK_WIDGET (box->button), FALSE);

		gtk_widget_queue_resize (GTK_WIDGET (box));
		g_object_notify_by_pspec (G_OBJECT (box), obj_props[PROP_ALWAYS_EXPANDED]);
	}
}
