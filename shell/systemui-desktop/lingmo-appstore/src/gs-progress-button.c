/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2014 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-app.h"
#include "gs-common.h"
#include "gs-progress-button.h"

struct _GsProgressButton
{
	GtkButton	 parent_instance;

	GtkWidget	*image;
	GtkWidget	*label;
	GtkWidget	*stack;

	GtkCssProvider	*css_provider;
	char		*label_text;
	char		*icon_name;
	gboolean	 show_icon;
};

G_DEFINE_TYPE (GsProgressButton, gs_progress_button, GTK_TYPE_BUTTON)

typedef enum {
	PROP_ICON_NAME = 1,
	PROP_SHOW_ICON,
	/* Overrides: */
	PROP_LABEL,
} GsProgressButtonProperty;

static GParamSpec *obj_props[PROP_SHOW_ICON + 1] = { NULL, };

void
gs_progress_button_set_progress (GsProgressButton *button, guint percentage)
{
	gchar tmp[64]; /* Large enough to hold the string below. */
	const gchar *css;

	if (percentage == GS_APP_PROGRESS_UNKNOWN) {
		css = "  background-size: 25%;\n"
		      "  animation: install-progress-unknown-move infinite linear 2s;";
	} else {
		percentage = MIN (percentage, 100); /* No need to clamp an unsigned to 0, it produces errors. */
		g_assert ((gsize) g_snprintf (tmp, sizeof (tmp), "background-size: %u%%;", percentage) < sizeof (tmp));
		css = tmp;
	}

	gs_utils_widget_set_css (GTK_WIDGET (button), &button->css_provider, css);
}

void
gs_progress_button_set_show_progress (GsProgressButton *button, gboolean show_progress)
{
	if (show_progress)
		gtk_widget_add_css_class (GTK_WIDGET (button), "install-progress");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (button), "install-progress");
}

/**
 * gs_progress_button_get_label:
 * @button: a #GsProgressButton
 *
 * Get the label of @button.
 *
 * It should be used rather than gtk_button_get_label() as it can only retrieve
 * the text from the label set by gtk_button_set_label(), which also cannot be
 * used.
 *
 * Returns: the label of @button
 *
 * Since: 41
 */
const gchar *
gs_progress_button_get_label (GsProgressButton *button)
{
	g_return_val_if_fail (GS_IS_PROGRESS_BUTTON (button), NULL);

	return button->label_text;
}

/**
 * gs_progress_button_set_label:
 * @button: a #GsProgressButton
 * @label: a string
 *
 * Set the label of @button.
 *
 * It should be used rather than gtk_button_set_label() as it will replace the
 * content of @button by a new label, breaking it.
 *
 * Since: 41
 */
void
gs_progress_button_set_label (GsProgressButton *button, const gchar *label)
{
	g_return_if_fail (GS_IS_PROGRESS_BUTTON (button));

	if (g_strcmp0 (button->label_text, label) == 0)
		return;

	g_free (button->label_text);
	button->label_text = g_strdup (label);

	g_object_notify (G_OBJECT (button), "label");
}

/**
 * gs_progress_button_get_icon_name:
 * @button: a #GsProgressButton
 *
 * Get the value of #GsProgressButton:icon-name.
 *
 * Returns: (nullable): the name of the icon
 *
 * Since: 41
 */
const gchar *
gs_progress_button_get_icon_name (GsProgressButton *button)
{
	g_return_val_if_fail (GS_IS_PROGRESS_BUTTON (button), NULL);

	return button->icon_name;
}

/**
 * gs_progress_button_set_icon_name:
 * @button: a #GsProgressButton
 * @icon_name: (nullable): the name of the icon
 *
 * Set the value of #GsProgressButton:icon-name.
 *
 * Since: 41
 */
void
gs_progress_button_set_icon_name (GsProgressButton *button, const gchar *icon_name)
{
	g_return_if_fail (GS_IS_PROGRESS_BUTTON (button));

	if (g_strcmp0 (button->icon_name, icon_name) == 0)
		return;

	g_free (button->icon_name);
	button->icon_name = g_strdup (icon_name);

	g_object_notify_by_pspec (G_OBJECT (button), obj_props[PROP_ICON_NAME]);
}

/**
 * gs_progress_button_get_show_icon:
 * @button: a #GsProgressButton
 *
 * Get the value of #GsProgressButton:show-icon.
 *
 * Returns: %TRUE if the icon is shown, %FALSE if the label is shown
 *
 * Since: 41
 */
gboolean
gs_progress_button_get_show_icon (GsProgressButton *button)
{
	g_return_val_if_fail (GS_IS_PROGRESS_BUTTON (button), FALSE);

	return button->show_icon;
}

/**
 * gs_progress_button_set_show_icon:
 * @button: a #GsProgressButton
 * @show_icon: %TRUE to set show the icon, %FALSE to show the label
 *
 * Set the value of #GsProgressButton:show-icon.
 *
 * Since: 41
 */
void
gs_progress_button_set_show_icon (GsProgressButton *button, gboolean show_icon)
{
	g_return_if_fail (GS_IS_PROGRESS_BUTTON (button));

	show_icon = !!show_icon;

	if (button->show_icon == show_icon)
		return;

	button->show_icon = show_icon;

	if (show_icon) {
		gtk_stack_set_visible_child (GTK_STACK (button->stack), button->image);
		gtk_widget_remove_css_class (GTK_WIDGET (button), "text-button");
		gtk_widget_add_css_class (GTK_WIDGET (button), "image-button");
	} else {
		gtk_stack_set_visible_child (GTK_STACK (button->stack), button->label);
		gtk_widget_remove_css_class (GTK_WIDGET (button), "image-button");
		gtk_widget_add_css_class (GTK_WIDGET (button), "text-button");
	}

	g_object_notify_by_pspec (G_OBJECT (button), obj_props[PROP_SHOW_ICON]);
}

/**
 * gs_progress_button_set_size_groups:
 * @button: a #GsProgressButton
 * @label: the #GtkSizeGroup for the label
 * @image: the #GtkSizeGroup for the image
 *
 * Groups the size of different buttons while keeping adaptiveness.
 *
 * Since: 41
 */
void
gs_progress_button_set_size_groups (GsProgressButton *button, GtkSizeGroup *label, GtkSizeGroup *image)
{
	g_return_if_fail (GS_IS_PROGRESS_BUTTON (button));

	if (label != NULL)
		gtk_size_group_add_widget (label, button->label);
	if (image != NULL)
		gtk_size_group_add_widget (image, button->image);
}

static void
gs_progress_button_page_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsProgressButton *self = GS_PROGRESS_BUTTON (object);

	switch ((GsProgressButtonProperty) prop_id) {
	case PROP_LABEL:
		g_value_set_string (value, gs_progress_button_get_label (self));
		break;
	case PROP_ICON_NAME:
		g_value_set_string (value, gs_progress_button_get_icon_name (self));
		break;
	case PROP_SHOW_ICON:
		g_value_set_boolean (value, gs_progress_button_get_show_icon (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_progress_button_page_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsProgressButton *self = GS_PROGRESS_BUTTON (object);

	switch ((GsProgressButtonProperty) prop_id) {
	case PROP_LABEL:
		gs_progress_button_set_label (self, g_value_get_string (value));
		break;
	case PROP_ICON_NAME:
		gs_progress_button_set_icon_name (self, g_value_get_string (value));
		break;
	case PROP_SHOW_ICON:
		gs_progress_button_set_show_icon (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_progress_button_dispose (GObject *object)
{
	GsProgressButton *button = GS_PROGRESS_BUTTON (object);

	g_clear_object (&button->css_provider);

	G_OBJECT_CLASS (gs_progress_button_parent_class)->dispose (object);
}

static void
gs_progress_button_finalize (GObject *object)
{
	GsProgressButton *button = GS_PROGRESS_BUTTON (object);

	g_clear_pointer (&button->label_text, g_free);
	g_clear_pointer (&button->icon_name, g_free);

	G_OBJECT_CLASS (gs_progress_button_parent_class)->finalize (object);
}

static void
gs_progress_button_init (GsProgressButton *button)
{
	gtk_widget_init_template (GTK_WIDGET (button));
}

static void
gs_progress_button_class_init (GsProgressButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_progress_button_page_get_property;
	object_class->set_property = gs_progress_button_page_set_property;
	object_class->dispose = gs_progress_button_dispose;
	object_class->finalize = gs_progress_button_finalize;

	/**
	 * GsProgressButton:icon-name: (nullable):
	 *
	 * The name of the icon for the button.
	 *
	 * Since: 41
	 */
	obj_props[PROP_ICON_NAME] =
		g_param_spec_string ("icon-name", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsProgressButton:show-icon:
	 *
	 * Whether to show the icon in place of the label.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SHOW_ICON] =
		g_param_spec_boolean ("show-icon", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	g_object_class_override_property (object_class, PROP_LABEL, "label");

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-progress-button.ui");

	gtk_widget_class_bind_template_child (widget_class, GsProgressButton, image);
	gtk_widget_class_bind_template_child (widget_class, GsProgressButton, label);
	gtk_widget_class_bind_template_child (widget_class, GsProgressButton, stack);
}

GtkWidget *
gs_progress_button_new (void)
{
	return g_object_new (GS_TYPE_PROGRESS_BUTTON, NULL);
}
