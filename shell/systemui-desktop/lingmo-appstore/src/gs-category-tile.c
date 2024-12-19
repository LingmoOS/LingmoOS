/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * SECTION:gs-category-tile
 * @short_description: A UI tile for presenting a category
 *
 * #GsCategoryTile is a UI widget to show a category to the user. It’s generally
 * aimed to be used in a list box, to provide navigation options to all the
 * categories.
 *
 * It will display the category’s name, and potentially a background image which
 * is styled to match the category’s content.
 *
 * Since: 41
 */

#include "config.h"

#include "gs-category-tile.h"
#include "gs-common.h"

struct _GsCategoryTile
{
	GtkFlowBoxChild	 parent_instance;

	GsCategory	*category;  /* (owned) (not nullable) */
	GtkWidget	*label;
	GtkWidget	*image;
	GtkBox		*box;
};

G_DEFINE_TYPE (GsCategoryTile, gs_category_tile, GTK_TYPE_FLOW_BOX_CHILD)

typedef enum {
	PROP_CATEGORY = 1,
} GsCategoryTileProperty;

static GParamSpec *obj_props[PROP_CATEGORY + 1] = { NULL, };

static void
gs_category_tile_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsCategoryTile *self = GS_CATEGORY_TILE (object);

	switch ((GsCategoryTileProperty) prop_id) {
	case PROP_CATEGORY:
		g_value_set_object (value, gs_category_tile_get_category (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_category_tile_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsCategoryTile *self = GS_CATEGORY_TILE (object);

	switch ((GsCategoryTileProperty) prop_id) {
	case PROP_CATEGORY:
		gs_category_tile_set_category (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * gs_category_tile_get_category:
 * @tile: a #GsCategoryTile
 *
 * Get the value of #GsCategoryTile:category.
 *
 * Returns: (transfer none) (not nullable): a category
 * Since: 41
 */
GsCategory *
gs_category_tile_get_category (GsCategoryTile *tile)
{
	g_return_val_if_fail (GS_IS_CATEGORY_TILE (tile), NULL);

	return tile->category;
}

static void
gs_category_tile_refresh (GsCategoryTile *tile)
{
	const gchar *icon_name = gs_category_get_icon_name (tile->category);

	/* set labels */
	gtk_label_set_label (GTK_LABEL (tile->label),
			     gs_category_get_name (tile->category));

	gtk_image_set_from_icon_name (GTK_IMAGE (tile->image), icon_name);
	gtk_widget_set_visible (tile->image, icon_name != NULL);

	/* Update the icon class. */
	if (icon_name != NULL)
		gtk_widget_remove_css_class (GTK_WIDGET (tile), "category-tile-iconless");
	else
		gtk_widget_add_css_class (GTK_WIDGET (tile), "category-tile-iconless");

	/* The label should be left-aligned for iconless categories and centred otherwise. */
	gtk_widget_set_halign (GTK_WIDGET (tile->box),
			       (icon_name != NULL) ? GTK_ALIGN_CENTER : GTK_ALIGN_START);
}

/**
 * gs_category_tile_set_category:
 * @tile: a #GsCategoryTile
 * @cat: (transfer none) (not nullable): a #GsCategory
 *
 * Set the value of #GsCategoryTile:category to @cat.
 *
 * Since: 41
 */
void
gs_category_tile_set_category (GsCategoryTile *tile, GsCategory *cat)
{
	g_return_if_fail (GS_IS_CATEGORY_TILE (tile));
	g_return_if_fail (GS_IS_CATEGORY (cat));

	/* Remove the old category ID. */
	if (tile->category != NULL) {
		g_autofree gchar *class_name = g_strdup_printf ("category-%s", gs_category_get_id (tile->category));
		gtk_widget_remove_css_class (GTK_WIDGET (tile), class_name);
	}

	if (g_set_object (&tile->category, cat)) {
		g_autofree gchar *class_name = g_strdup_printf ("category-%s", gs_category_get_id (tile->category));

		/* Add the new category’s ID as a CSS class, to get
		 * category-specific styling. */
		gtk_widget_add_css_class (GTK_WIDGET (tile), class_name);

		gs_category_tile_refresh (tile);
		g_object_notify_by_pspec (G_OBJECT (tile), obj_props[PROP_CATEGORY]);
	}
}

static void
gs_category_tile_dispose (GObject *object)
{
	GsCategoryTile *tile = GS_CATEGORY_TILE (object);

	g_clear_object (&tile->category);

	G_OBJECT_CLASS (gs_category_tile_parent_class)->dispose (object);
}

static void
gs_category_tile_init (GsCategoryTile *tile)
{
	gtk_widget_init_template (GTK_WIDGET (tile));
}

static void
gs_category_tile_class_init (GsCategoryTileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_category_tile_get_property;
	object_class->set_property = gs_category_tile_set_property;
	object_class->dispose = gs_category_tile_dispose;

	/**
	 * GsCategoryTile:category: (not nullable)
	 *
	 * The category to display in this tile.
	 *
	 * This must not be %NULL.
	 *
	 * Since: 41
	 */
	obj_props[PROP_CATEGORY] =
		g_param_spec_object ("category", NULL, NULL,
				     GS_TYPE_CATEGORY,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-category-tile.ui");

	gtk_widget_class_bind_template_child (widget_class, GsCategoryTile, label);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryTile, image);
	gtk_widget_class_bind_template_child (widget_class, GsCategoryTile, box);
}

/**
 * gs_category_tile_new:
 * @cat: (transfer none) (not nullable): a #GsCategory
 *
 * Create a new #GsCategoryTile to represent @cat.
 *
 * Returns: (transfer full) (type GsCategoryTile): a new #GsCategoryTile
 * Since: 41
 */
GtkWidget *
gs_category_tile_new (GsCategory *cat)
{
	return g_object_new (GS_TYPE_CATEGORY_TILE,
			     "category", cat,
			     NULL);
}
