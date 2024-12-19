/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat (www.redhat.com)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-lozenge.h"
#include "gs-layout-manager.h"

#define GS_TYPE_LOZENGE_LAYOUT (gs_lozenge_layout_get_type ())
G_DECLARE_FINAL_TYPE (GsLozengeLayout, gs_lozenge_layout, GS, LOZENGE_LAYOUT, GsLayoutManager)

struct _GsLozengeLayout
{
	GsLayoutManager		 parent_instance;

	gboolean		 circular;
};

G_DEFINE_TYPE (GsLozengeLayout, gs_lozenge_layout, GS_TYPE_LAYOUT_MANAGER)

static void
gs_lozenge_layout_measure (GtkLayoutManager *layout_manager,
			   GtkWidget	    *widget,
			   GtkOrientation    orientation,
			   gint		     for_size,
			   gint		    *minimum,
			   gint		    *natural,
			   gint		    *minimum_baseline,
			   gint		    *natural_baseline)
{
	GsLozengeLayout *self = GS_LOZENGE_LAYOUT (layout_manager);

	GTK_LAYOUT_MANAGER_CLASS (gs_lozenge_layout_parent_class)->measure (layout_manager,
		widget, orientation, for_size, minimum, natural, minimum_baseline, natural_baseline);

	if (self->circular) {
		*minimum = MAX (for_size, *minimum);
		*natural = *minimum;
		*natural_baseline = *minimum_baseline;
	}

	if (*natural_baseline > *natural)
		*natural_baseline = *natural;
	if (*minimum_baseline > *minimum)
		*minimum_baseline = *minimum;
}

static void
gs_lozenge_layout_class_init (GsLozengeLayoutClass *klass)
{
	GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

	layout_manager_class->measure = gs_lozenge_layout_measure;
}

static void
gs_lozenge_layout_init (GsLozengeLayout *self)
{
}

/* ********************************************************************* */

struct _GsLozenge
{
	GtkBox		 parent_instance;

	GtkWidget	*image; /* (unowned) */
	GtkWidget	*label; /* (unowned) */

	gchar		*icon_name;
	gchar		*text;
	gchar		*markup;
	gboolean	 circular;
	gint		 pixel_size;
};

G_DEFINE_TYPE (GsLozenge, gs_lozenge, GTK_TYPE_BOX)

typedef enum {
	PROP_CIRCULAR = 1,
	PROP_ICON_NAME,
	PROP_PIXEL_SIZE,
	PROP_TEXT,
	PROP_MARKUP
} GsLozengeProperty;

static GParamSpec *obj_props[PROP_MARKUP + 1] = { NULL, };

static void
gs_lozenge_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	GsLozenge *self = GS_LOZENGE (object);

	switch ((GsLozengeProperty) prop_id) {
	case PROP_CIRCULAR:
		g_value_set_boolean (value, gs_lozenge_get_circular (self));
		break;
	case PROP_ICON_NAME:
		g_value_set_string (value, gs_lozenge_get_icon_name (self));
		break;
	case PROP_PIXEL_SIZE:
		g_value_set_int (value, gs_lozenge_get_pixel_size (self));
		break;
	case PROP_TEXT:
		g_value_set_string (value, gs_lozenge_get_text (self));
		break;
	case PROP_MARKUP:
		g_value_set_string (value, gs_lozenge_get_markup (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_lozenge_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GsLozenge *self = GS_LOZENGE (object);

	switch ((GsLozengeProperty) prop_id) {
	case PROP_CIRCULAR:
		gs_lozenge_set_circular (self, g_value_get_boolean (value));
		break;
	case PROP_ICON_NAME:
		gs_lozenge_set_icon_name (self, g_value_get_string (value));
		break;
	case PROP_PIXEL_SIZE:
		gs_lozenge_set_pixel_size (self, g_value_get_int (value));
		break;
	case PROP_TEXT:
		gs_lozenge_set_text (self, g_value_get_string (value));
		break;
	case PROP_MARKUP:
		gs_lozenge_set_markup (self, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_lozenge_dispose (GObject *object)
{
	GsLozenge *self = GS_LOZENGE (object);

	g_clear_pointer (&self->icon_name, g_free);
	g_clear_pointer (&self->text, g_free);
	g_clear_pointer (&self->markup, g_free);

	G_OBJECT_CLASS (gs_lozenge_parent_class)->dispose (object);
}

static void
gs_lozenge_class_init (GsLozengeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_lozenge_get_property;
	object_class->set_property = gs_lozenge_set_property;
	object_class->dispose = gs_lozenge_dispose;

	/**
	 * GsLozenge:circular:
	 *
	 * Whether the lozenge should be circular/square widget.
	 *
	 * Since: 43
	 */
	obj_props[PROP_CIRCULAR] =
		g_param_spec_boolean ("circular", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsLozenge:icon-name:
	 *
	 * An icon name for the lozenge. Setting this property turns
	 * the lozenge into the icon mode, which mean showing the icon,
	 * not the markup.
	 *
	 * Since: 43
	 */
	obj_props[PROP_ICON_NAME] =
		g_param_spec_string ("icon-name", NULL, NULL,
				      NULL,
				      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsLozenge:pixel-size:
	 *
	 * An icon pixel size for the lozenge.
	 *
	 * Since: 43
	 */
	obj_props[PROP_PIXEL_SIZE] =
		g_param_spec_int ("pixel-size", NULL, NULL,
				  0, G_MAXINT, 16,
				  G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsLozenge:text:
	 *
	 * A plain text for the lozenge. Setting this property turns
	 * the lozenge into the text mode, which mean showing the text,
	 * not the icon.
	 *
	 * Since: 43
	 */
	obj_props[PROP_TEXT] =
		g_param_spec_string ("text", NULL, NULL,
				      NULL,
				      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsLozenge:markup:
	 *
	 * A markup text for the lozenge. Setting this property turns
	 * the lozenge into the text mode, which mean showing the markup,
	 * not the icon.
	 *
	 * Since: 43
	 */
	obj_props[PROP_MARKUP] =
		g_param_spec_string ("markup", NULL, NULL,
				      NULL,
				      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_layout_manager_type (widget_class, GS_TYPE_LOZENGE_LAYOUT);
	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-lozenge.ui");

	gtk_widget_class_bind_template_child (widget_class, GsLozenge, image);
	gtk_widget_class_bind_template_child (widget_class, GsLozenge, label);
}

static void
gs_lozenge_init (GsLozenge *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));

	self->pixel_size = 16;
}

/**
 * gs_lozenge_new:
 *
 * Returns: (transfer full): a new #GsLozenge
 *
 * Since: 43
 **/
GtkWidget *
gs_lozenge_new (void)
{
	return g_object_new (GS_TYPE_LOZENGE, NULL);
}

const gchar *
gs_lozenge_get_icon_name (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), NULL);

	return self->icon_name;
}

gboolean
gs_lozenge_get_circular (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), FALSE);

	return self->circular;
}

void
gs_lozenge_set_circular (GsLozenge *self,
			 gboolean value)
{
	GtkLayoutManager *layout_manager;

	g_return_if_fail (GS_IS_LOZENGE (self));

	if ((!self->circular) == (!value))
		return;

	self->circular = value;

	layout_manager = gtk_widget_get_layout_manager (GTK_WIDGET (self));
	GS_LOZENGE_LAYOUT (layout_manager)->circular = self->circular;
	gtk_layout_manager_layout_changed (layout_manager);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_CIRCULAR]);
}

void
gs_lozenge_set_icon_name (GsLozenge *self,
			  const gchar *value)
{
	g_return_if_fail (GS_IS_LOZENGE (self));

	if (value != NULL && *value == '\0')
		value = NULL;

	if (g_strcmp0 (self->icon_name, value) == 0)
		return;

	g_clear_pointer (&self->icon_name, g_free);
	self->icon_name = g_strdup (value);

	if (self->icon_name == NULL) {
		gtk_widget_set_visible (self->image, FALSE);
		gtk_widget_set_visible (self->label, TRUE);
	} else {
		gtk_image_set_from_icon_name (GTK_IMAGE (self->image), self->icon_name);
		gtk_widget_set_visible (self->label, FALSE);
		gtk_widget_set_visible (self->image, TRUE);
	}

	/* Clean up the other properties before notifying of the changed property name */

	if (self->text != NULL) {
		g_clear_pointer (&self->text, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_TEXT]);
	}

	if (self->markup != NULL) {
		g_clear_pointer (&self->markup, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_MARKUP]);
	}

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_ICON_NAME]);
}

gint
gs_lozenge_get_pixel_size (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), 0);

	return self->pixel_size;
}

void
gs_lozenge_set_pixel_size (GsLozenge *self,
			   gint value)
{
	g_return_if_fail (GS_IS_LOZENGE (self));

	if (self->pixel_size == value)
		return;

	self->pixel_size = value;

	gtk_image_set_pixel_size (GTK_IMAGE (self->image), self->pixel_size);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_PIXEL_SIZE]);
}

gboolean
gs_lozenge_get_use_markup (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), FALSE);
	return gtk_label_get_use_markup (GTK_LABEL (self->label));
}

const gchar *
gs_lozenge_get_text (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), NULL);

	return self->text;
}

void
gs_lozenge_set_text (GsLozenge *self,
		     const gchar *value)
{
	g_return_if_fail (GS_IS_LOZENGE (self));

	if (value != NULL && *value == '\0')
		value = NULL;

	if (g_strcmp0 (self->text, value) == 0)
		return;

	g_clear_pointer (&self->text, g_free);
	self->text = g_strdup (value);

	if (self->text == NULL) {
		gtk_widget_set_visible (self->label, FALSE);
		gtk_widget_set_visible (self->image, TRUE);
	} else {
		gtk_label_set_text (GTK_LABEL (self->label), self->text);
		gtk_widget_set_visible (self->image, FALSE);
		gtk_widget_set_visible (self->label, TRUE);
	}

	/* Clean up the other properties before notifying of the changed property name */

	if (self->icon_name != NULL) {
		g_clear_pointer (&self->icon_name, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_ICON_NAME]);
	}

	if (self->markup != NULL) {
		g_clear_pointer (&self->markup, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_MARKUP]);
	}

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_TEXT]);
}

const gchar *
gs_lozenge_get_markup (GsLozenge *self)
{
	g_return_val_if_fail (GS_IS_LOZENGE (self), NULL);

	return self->markup;
}

void
gs_lozenge_set_markup (GsLozenge *self,
		       const gchar *value)
{
	g_return_if_fail (GS_IS_LOZENGE (self));

	if (value != NULL && *value == '\0')
		value = NULL;

	if (g_strcmp0 (self->markup, value) == 0)
		return;

	g_clear_pointer (&self->markup, g_free);
	self->markup = g_strdup (value);

	if (self->markup == NULL) {
		gtk_widget_set_visible (self->label, FALSE);
		gtk_widget_set_visible (self->image, TRUE);
	} else {
		gtk_label_set_markup (GTK_LABEL (self->label), self->markup);
		gtk_widget_set_visible (self->image, FALSE);
		gtk_widget_set_visible (self->label, TRUE);
	}

	/* Clean up the other properties before notifying of the changed property name */

	if (self->icon_name != NULL) {
		g_clear_pointer (&self->icon_name, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_ICON_NAME]);
	}

	if (self->text != NULL) {
		g_clear_pointer (&self->text, g_free);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_TEXT]);
	}

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_MARKUP]);
}
