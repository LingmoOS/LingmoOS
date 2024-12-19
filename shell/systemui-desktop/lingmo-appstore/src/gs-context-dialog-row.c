/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-context-dialog-row
 * @short_description: A list box row for context dialogs
 *
 * #GsContextDialogRow is a #GtkListBox row designed to be used in context
 * dialogs such as #GsHardwareSupportContextDialog. Each row indicates how well
 * the app supports a certain feature, attribute or permission. Each row
 * contains an image in a lozenge, a title, a description, and has an
 * ‘importance’ which is primarily indicated through the colour of the image.
 *
 * Since: 41
 */

#include "config.h"

#include <adwaita.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gs-context-dialog-row.h"
#include "gs-lozenge.h"
#include "gs-enums.h"

struct _GsContextDialogRow
{
	AdwActionRow			 parent_instance;

	GsContextDialogRowImportance	 importance;

	GsLozenge			*lozenge;  /* (unowned) */
};

G_DEFINE_TYPE (GsContextDialogRow, gs_context_dialog_row, ADW_TYPE_ACTION_ROW)

typedef enum {
	PROP_ICON_NAME = 1,
	PROP_CONTENT,
	PROP_IMPORTANCE,
} GsContextDialogRowProperty;

static GParamSpec *obj_props[PROP_IMPORTANCE + 1] = { NULL, };

/* These match the CSS classes from gtk-style.css. */
static const gchar *
css_class_for_importance (GsContextDialogRowImportance importance)
{
	switch (importance) {
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL:
		return "grey";
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT:
		return "green";
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION:
		return "yellow";
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING:
		return "orange";
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT:
		return "red";
	default:
		g_assert_not_reached ();
	}
}

static void
gs_context_dialog_row_init (GsContextDialogRow *self)
{
	g_type_ensure (GS_TYPE_LOZENGE);

	gtk_widget_init_template (GTK_WIDGET (self));

#if ADW_CHECK_VERSION(1,2,0)
	adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (self), FALSE);
#endif
}

static void
gs_context_dialog_row_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
	GsContextDialogRow *self = GS_CONTEXT_DIALOG_ROW (object);

	switch ((GsContextDialogRowProperty) prop_id) {
	case PROP_ICON_NAME:
		g_value_set_string (value, gs_context_dialog_row_get_icon_name (self));
		break;
	case PROP_CONTENT:
		g_value_set_string (value, gs_context_dialog_row_get_content (self));
		break;
	case PROP_IMPORTANCE:
		g_value_set_enum (value, gs_context_dialog_row_get_importance (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_context_dialog_row_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
	GsContextDialogRow *self = GS_CONTEXT_DIALOG_ROW (object);

	switch ((GsContextDialogRowProperty) prop_id) {
	case PROP_ICON_NAME:
		gs_lozenge_set_icon_name (self->lozenge, g_value_get_string (value));
		break;
	case PROP_CONTENT:
		gs_lozenge_set_text (self->lozenge, g_value_get_string (value));
		break;
	case PROP_IMPORTANCE: {
		const gchar *css_class;

		self->importance = g_value_get_enum (value);
		css_class = css_class_for_importance (self->importance);

		gtk_widget_remove_css_class (GTK_WIDGET (self->lozenge), "green");
		gtk_widget_remove_css_class (GTK_WIDGET (self->lozenge), "yellow");
		gtk_widget_remove_css_class (GTK_WIDGET (self->lozenge), "red");
		gtk_widget_remove_css_class (GTK_WIDGET (self->lozenge), "grey");

		gtk_widget_add_css_class (GTK_WIDGET (self->lozenge), css_class);
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_context_dialog_row_class_init (GsContextDialogRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_context_dialog_row_get_property;
	object_class->set_property = gs_context_dialog_row_set_property;

	/**
	 * GsContextDialogRow:icon-name: (nullable)
	 *
	 * Name of the icon to display in the row.
	 *
	 * This must be %NULL if #GsContextDialogRow:content is set,
	 * and non-%NULL otherwise.
	 *
	 * Since: 41
	 */
	obj_props[PROP_ICON_NAME] =
		g_param_spec_string ("icon-name", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsContextDialogRow:content: (nullable)
	 *
	 * Text content to display in the row.
	 *
	 * This must be %NULL if #GsContextDialogRow:icon-name is set,
	 * and non-%NULL otherwise.
	 *
	 * Since: 41
	 */
	obj_props[PROP_CONTENT] =
		g_param_spec_string ("content", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsContextDialogRow:importance:
	 *
	 * Importance of the information in the row to the user’s decision
	 * making about an app. This is primarily represented as the row’s
	 * colour.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IMPORTANCE] =
		g_param_spec_enum ("importance", NULL, NULL,
				   GS_TYPE_CONTEXT_DIALOG_ROW_IMPORTANCE, GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL,
				   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	/* This uses the same CSS name as a standard #GtkListBoxRow in order to
	 * get the default styling from GTK. */
	gtk_widget_class_set_css_name (widget_class, "row");
	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-context-dialog-row.ui");

	gtk_widget_class_bind_template_child (widget_class, GsContextDialogRow, lozenge);
}

/**
 * gs_context_dialog_row_new:
 * @icon_name: (not nullable): name of the icon for the row
 * @importance: importance of the information in the row
 * @title: (not nullable): title for the row
 * @description: (not nullable): description for the row
 *
 * Create a new #GsContextDialogRow with an icon inside the lozenge.
 *
 * Returns: (transfer full): a new #GsContextDialogRow
 * Since: 41
 */
GtkListBoxRow *
gs_context_dialog_row_new (const gchar                  *icon_name,
                           GsContextDialogRowImportance  importance,
                           const gchar                  *title,
                           const gchar                  *description)
{
	g_return_val_if_fail (icon_name != NULL, NULL);
	g_return_val_if_fail (title != NULL, NULL);
	g_return_val_if_fail (description != NULL, NULL);

	return g_object_new (GS_TYPE_CONTEXT_DIALOG_ROW,
			     "icon-name", icon_name,
			     "importance", importance,
			     "title", title,
			     "subtitle", description,
			     NULL);
}

/**
 * gs_context_dialog_row_new_text:
 * @content: (not nullable): text to put in the lozenge
 * @importance: importance of the information in the row
 * @title: (not nullable): title for the row
 * @description: (not nullable): description for the row
 *
 * Create a new #GsContextDialogRow with text inside the lozenge.
 *
 * Returns: (transfer full): a new #GsContextDialogRow
 * Since: 41
 */
GtkListBoxRow *
gs_context_dialog_row_new_text (const gchar                  *content,
                                GsContextDialogRowImportance  importance,
                                const gchar                  *title,
                                const gchar                  *description)
{
	g_return_val_if_fail (content != NULL, NULL);
	g_return_val_if_fail (title != NULL, NULL);
	g_return_val_if_fail (description != NULL, NULL);

	return g_object_new (GS_TYPE_CONTEXT_DIALOG_ROW,
			     "content", content,
			     "importance", importance,
			     "title", title,
			     "subtitle", description,
			     NULL);
}

/**
 * gs_context_dialog_row_get_icon_name:
 * @self: a #GsContextDialogRow
 *
 * Get the value of #GsContextDialogRow:icon-name.
 *
 * Returns: the name of the icon used in the row
 * Since: 41
 */
const gchar *
gs_context_dialog_row_get_icon_name (GsContextDialogRow *self)
{
	g_return_val_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self), NULL);

	return gs_lozenge_get_icon_name (self->lozenge);
}

/**
 * gs_context_dialog_row_get_content:
 * @self: a #GsContextDialogRow
 *
 * Get the value of #GsContextDialogRow:content.
 *
 * Returns: the text content used in the row
 * Since: 41
 */
const gchar *
gs_context_dialog_row_get_content (GsContextDialogRow *self)
{
	g_return_val_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self), NULL);

	return gs_lozenge_get_text (self->lozenge);
}

/**
 * gs_context_dialog_row_get_content_is_markup:
 * @self: a #GsContextDialogRow
 *
 * Get whether the #GsContextDialogRow:content is markup.
 *
 * Returns: %TRUE when then content text is markup
 * Since: 43
 */
gboolean
gs_context_dialog_row_get_content_is_markup (GsContextDialogRow *self)
{
	g_return_val_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self), FALSE);

	return gs_lozenge_get_use_markup (self->lozenge);
}

/**
 * gs_context_dialog_row_set_content_markup:
 * @self: a #GsContextDialogRow
 * @markup: markup to set
 *
 * Set the @markup content as markup.
 *
 * Since: 43
 */
void
gs_context_dialog_row_set_content_markup (GsContextDialogRow *self,
					  const gchar *markup)
{
	g_return_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self));

	gs_lozenge_set_markup (self->lozenge, markup);
}

/**
 * gs_context_dialog_row_get_importance:
 * @self: a #GsContextDialogRow
 *
 * Get the value of #GsContextDialogRow:importance.
 *
 * Returns: the importance of the information in the row
 * Since: 41
 */
GsContextDialogRowImportance
gs_context_dialog_row_get_importance (GsContextDialogRow *self)
{
	g_return_val_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self), GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL);

	return self->importance;
}

/**
 * gs_context_dialog_row_set_size_groups:
 * @self: a #GsContextDialogRow
 * @lozenge: (nullable) (transfer none): a #GtkSizeGroup for the lozenge, or %NULL
 * @title: (nullable) (transfer none): a #GtkSizeGroup for the title, or %NULL
 * @description: (nullable) (transfer none): a #GtkSizeGroup for the description, or %NULL
 *
 * Add widgets from the #GsContextDialogRow to the given size groups. If a size
 * group is %NULL, the corresponding widget will not be changed.
 *
 * Since: 41
 */
void
gs_context_dialog_row_set_size_groups (GsContextDialogRow *self,
                                       GtkSizeGroup       *lozenge,
                                       GtkSizeGroup       *title,
                                       GtkSizeGroup       *description)
{
	g_return_if_fail (GS_IS_CONTEXT_DIALOG_ROW (self));
	g_return_if_fail (lozenge == NULL || GTK_IS_SIZE_GROUP (lozenge));
	g_return_if_fail (title == NULL || GTK_IS_SIZE_GROUP (title));
	g_return_if_fail (description == NULL || GTK_IS_SIZE_GROUP (description));

	if (lozenge != NULL)
		gtk_size_group_add_widget (lozenge, GTK_WIDGET (self->lozenge));
}
