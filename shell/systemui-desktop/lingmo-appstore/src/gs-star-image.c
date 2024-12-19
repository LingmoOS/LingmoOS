/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-star-image
 * @title: GsStarImage
 * @stability: Unstable
 * @short_description: Draw a star image, which can be partially filled
 *
 * Depending on the #GsStarImage:fraction property, the starred image can be
 * drawn as filled only partially or fully or not at all, with the non-starred
 * image taking the rest of the space up.
 *
 * ## CSS nodes
 *
 * ```
 * star-image
 * ├── image.starred
 * ╰── image.non-starred
 * ```
 *
 * Since: 41
 */

#include "config.h"

#include "gs-common.h"
#include "gs-star-image.h"

#include <adwaita.h>

struct _GsStarImage
{
	GtkWidget parent_instance;

	GtkWidget *starred;
	GtkWidget *non_starred;
	gdouble fraction;
	gdouble pixel_size;
};

G_DEFINE_TYPE (GsStarImage, gs_star_image, GTK_TYPE_WIDGET)

enum {
	PROP_FRACTION = 1,
	PROP_PIXEL_SIZE,
};

/* Floating points are imprecise, we can't use `<= 0.0` and `>= 1.0` */
#define FRACTION_IS_MIN(f) (f < 0.01)
#define FRACTION_IS_MAX(f) (f > 0.99)

static void
gs_star_image_get_property (GObject *object,
			    guint param_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	switch (param_id) {
	case PROP_FRACTION:
		g_value_set_double (value, gs_star_image_get_fraction (GS_STAR_IMAGE (object)));
		break;
	case PROP_PIXEL_SIZE:
		g_value_set_int (value, gs_star_image_get_pixel_size (GS_STAR_IMAGE (object)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gs_star_image_set_property (GObject *object,
			    guint param_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	switch (param_id) {
	case PROP_FRACTION:
		gs_star_image_set_fraction (GS_STAR_IMAGE (object), g_value_get_double (value));
		break;
	case PROP_PIXEL_SIZE:
		gs_star_image_set_pixel_size (GS_STAR_IMAGE (object), g_value_get_int (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gs_star_image_dispose (GObject *object)
{
	gs_widget_remove_all (GTK_WIDGET (object), NULL);

	G_OBJECT_CLASS (gs_star_image_parent_class)->dispose (object);
}

static void
gs_star_image_snapshot (GtkWidget   *widget,
                        GtkSnapshot *snapshot)
{
	GsStarImage *self = GS_STAR_IMAGE (widget);

	if (FRACTION_IS_MIN (self->fraction)) {
		gtk_widget_snapshot_child (widget, self->non_starred, snapshot);
	} else if (FRACTION_IS_MAX (self->fraction)) {
		gtk_widget_snapshot_child (widget, self->starred, snapshot);
	} else {
		int width, height;
		int starred_width;

		width = gtk_widget_get_width (widget);
		height = gtk_widget_get_height (widget);
		starred_width = width * self->fraction;

		if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
			gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT(width - starred_width, 0, starred_width, height));
		else
			gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT(0, 0, starred_width, height));
		gtk_widget_snapshot_child (widget, self->starred, snapshot);
		gtk_snapshot_pop (snapshot);

		if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
			gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT(0, 0, width - starred_width, height));
		else
			gtk_snapshot_push_clip (snapshot, &GRAPHENE_RECT_INIT(starred_width, 0, width - starred_width, height));
		gtk_widget_snapshot_child (widget, self->non_starred, snapshot);
		gtk_snapshot_pop (snapshot);
	}
}

static void
gs_star_image_class_init (GsStarImageClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = gs_star_image_get_property;
	object_class->set_property = gs_star_image_set_property;
	object_class->dispose = gs_star_image_dispose;

	widget_class = GTK_WIDGET_CLASS (klass);
	widget_class->snapshot = gs_star_image_snapshot;

	g_object_class_install_property (object_class,
					 PROP_FRACTION,
					 g_param_spec_double ("fraction", NULL, NULL,
							      0.0, 1.0, 1.0,
							      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY));

	g_object_class_install_property (object_class,
					 PROP_PIXEL_SIZE,
					 g_param_spec_int ("pixel-size", NULL, NULL,
							   -1, G_MAXINT, -1,
							   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY));

	gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_METER);
	gtk_widget_class_set_css_name (widget_class, "star-image");
	gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
gs_star_image_init (GsStarImage *self)
{
	self->starred = gtk_image_new_from_icon_name ("starred-symbolic");
	gtk_widget_set_child_visible (self->starred, TRUE);
	gtk_widget_set_parent (self->starred, GTK_WIDGET (self));
	gtk_widget_add_css_class (self->starred, "starred");

	self->non_starred = gtk_image_new_from_icon_name ("starred-symbolic");
	gtk_widget_set_child_visible (self->non_starred, FALSE);
	gtk_widget_set_parent (self->non_starred, GTK_WIDGET (self));
	gtk_widget_add_css_class (self->non_starred, "non-starred");

	self->fraction = 1.0;
}

GtkWidget *
gs_star_image_new (void)
{
	return g_object_new (GS_TYPE_STAR_IMAGE, NULL);
}

void
gs_star_image_set_fraction (GsStarImage *self,
			    gdouble fraction)
{
	g_return_if_fail (GS_IS_STAR_IMAGE (self));

	if (self->fraction == fraction)
		return;

	self->fraction = fraction;

	gtk_widget_set_child_visible (self->starred, !FRACTION_IS_MIN (self->fraction));
	gtk_widget_set_child_visible (self->non_starred, !FRACTION_IS_MAX (self->fraction));

	g_object_notify (G_OBJECT (self), "fraction");

	gtk_widget_queue_draw (GTK_WIDGET (self));
}

gdouble
gs_star_image_get_fraction (GsStarImage *self)
{
	g_return_val_if_fail (GS_IS_STAR_IMAGE (self), -1.0);

	return self->fraction;
}

void
gs_star_image_set_pixel_size (GsStarImage *self,
			      gint pixel_size)
{
	g_return_if_fail (GS_IS_STAR_IMAGE (self));
	g_return_if_fail (pixel_size >= -1);

	if (self->pixel_size == pixel_size)
		return;

	self->pixel_size = pixel_size;

	gtk_image_set_pixel_size (GTK_IMAGE (self->starred), pixel_size);
	gtk_image_set_pixel_size (GTK_IMAGE (self->non_starred), pixel_size);

	g_object_notify (G_OBJECT (self), "pixel-size");

	gtk_widget_queue_resize (GTK_WIDGET (self));
}

gint
gs_star_image_get_pixel_size (GsStarImage *self)
{
	g_return_val_if_fail (GS_IS_STAR_IMAGE (self), -1);

	return self->pixel_size;
}
