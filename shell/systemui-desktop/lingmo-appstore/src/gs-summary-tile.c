/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2019 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-summary-tile.h"
#include "gs-layout-manager.h"
#include "gs-common.h"

#define GS_TYPE_SUMMARY_TILE_LAYOUT (gs_summary_tile_layout_get_type ())
G_DECLARE_FINAL_TYPE (GsSummaryTileLayout, gs_summary_tile_layout, GS, SUMMARY_TILE_LAYOUT, GsLayoutManager)

struct _GsSummaryTileLayout
{
	GsLayoutManager parent_instance;

	gint		preferred_width;
};

G_DEFINE_TYPE (GsSummaryTileLayout, gs_summary_tile_layout, GS_TYPE_LAYOUT_MANAGER)

static void
gs_summary_tile_layout_measure (GtkLayoutManager *layout_manager,
				GtkWidget        *widget,
				GtkOrientation    orientation,
				gint              for_size,
				gint             *minimum,
				gint             *natural,
				gint             *minimum_baseline,
				gint             *natural_baseline)
{
	GsSummaryTileLayout *self = GS_SUMMARY_TILE_LAYOUT (layout_manager);

	GTK_LAYOUT_MANAGER_CLASS (gs_summary_tile_layout_parent_class)->measure (layout_manager,
		widget, orientation, for_size, minimum, natural, minimum_baseline, natural_baseline);

	/* Limit the natural width */
	if (self->preferred_width > 0 && orientation == GTK_ORIENTATION_HORIZONTAL)
		*natural = MAX (*minimum, self->preferred_width);
}

static void
gs_summary_tile_layout_class_init (GsSummaryTileLayoutClass *klass)
{
	GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);
	layout_manager_class->measure = gs_summary_tile_layout_measure;
}

static void
gs_summary_tile_layout_init (GsSummaryTileLayout *self)
{
}

/* ********************************************************************* */

struct _GsSummaryTile
{
	GsAppTile	 parent_instance;

	GtkWidget	*image;
	GtkWidget	*image_stack;
	GtkWidget	*name;
	GtkWidget	*summary;
	GtkWidget	*bin;
	GtkWidget	*stack;
	gint		 preferred_width;

	GsAppIconsState	 current_app_icons_state;
};

G_DEFINE_TYPE (GsSummaryTile, gs_summary_tile, GS_TYPE_APP_TILE)

typedef enum {
	PROP_PREFERRED_WIDTH = 1,
} GsSummaryTileProperty;

static GParamSpec *obj_props[PROP_PREFERRED_WIDTH + 1] = { NULL, };

static void
gs_summary_tile_refresh (GsAppTile *self)
{
	GsSummaryTile *tile = GS_SUMMARY_TILE (self);
	GsAppIconsState app_icons_state;
	GsApp *app = gs_app_tile_get_app (self);
	gboolean installed;
	g_autofree gchar *name = NULL;
	const gchar *summary;

	if (app == NULL)
		return;

	gtk_image_set_pixel_size (GTK_IMAGE (tile->image), 64);
	gtk_stack_set_visible_child_name (GTK_STACK (tile->stack), "content");

	/* set name */
	gtk_label_set_label (GTK_LABEL (tile->name), gs_app_get_name (app));

	summary = gs_app_get_summary (app);
	gtk_label_set_label (GTK_LABEL (tile->summary), summary);
	gtk_widget_set_visible (tile->summary, summary && summary[0]);

	app_icons_state = gs_app_get_icons_state (app);
	if (tile->current_app_icons_state != app_icons_state) {
		g_autoptr(GIcon) icon = NULL;

		switch (app_icons_state) {
		case GS_APP_ICONS_STATE_AVAILABLE:
			icon = gs_app_get_icon_for_size (app,
							 gtk_image_get_pixel_size (GTK_IMAGE (tile->image)),
							 gtk_widget_get_scale_factor (tile->image),
							 "application-x-executable");
			gtk_image_set_from_gicon (GTK_IMAGE (tile->image), icon);
			gtk_stack_set_visible_child_name (GTK_STACK (tile->image_stack), "image");
			break;
		case GS_APP_ICONS_STATE_UNKNOWN:
		case GS_APP_ICONS_STATE_PENDING_DOWNLOAD:
		case GS_APP_ICONS_STATE_DOWNLOADING:
		default:
			gtk_stack_set_visible_child_name (GTK_STACK (tile->image_stack), "loading");
			break;
		}

		tile->current_app_icons_state = app_icons_state;
	}

	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_UPDATABLE_LIVE:
		installed = TRUE;
		name = g_strdup_printf (_("%s (Installed)"),
					gs_app_get_name (app));
		break;
	case GS_APP_STATE_INSTALLING:
		installed = FALSE;
		name = g_strdup_printf (_("%s (Installing)"),
					gs_app_get_name (app));
		break;
	case GS_APP_STATE_DOWNLOADING:
		installed = FALSE;
		name = g_strdup_printf (_("%s (Downloading)"),
					gs_app_get_name (app));
		break;
	case GS_APP_STATE_REMOVING:
		installed = TRUE;
		name = g_strdup_printf (_("%s (Removing)"),
					gs_app_get_name (app));
		break;
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
	case GS_APP_STATE_AVAILABLE:
	default:
		installed = FALSE;
		name = g_strdup (gs_app_get_name (app));
		break;
	}

	gtk_widget_set_visible (tile->bin, installed);

	if (name != NULL) {
		gtk_accessible_update_property (GTK_ACCESSIBLE (tile),
						GTK_ACCESSIBLE_PROPERTY_LABEL, name,
						GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, gs_app_get_summary (app),
						-1);
	}
}

static void
gs_summary_tile_init (GsSummaryTile *tile)
{
	tile->current_app_icons_state = GS_APP_ICONS_STATE_UNKNOWN;
	tile->preferred_width = -1;
	gtk_widget_init_template (GTK_WIDGET (tile));
}

static void
gs_summary_tile_notify (GObject    *object,
                        GParamSpec *pspec)
{
	GsSummaryTile *self = GS_SUMMARY_TILE (object);

	/* If the app of this tile changes, we have to reload its icon */
	if (g_strcmp0 (pspec->name, "app") == 0)
		self->current_app_icons_state = GS_APP_ICONS_STATE_UNKNOWN;

	if (G_OBJECT_CLASS (gs_summary_tile_parent_class)->notify)
		G_OBJECT_CLASS (gs_summary_tile_parent_class)->notify (object, pspec);
}

static void
gs_summary_tile_get_property (GObject *object,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *pspec)
{
	GsSummaryTile *app_tile = GS_SUMMARY_TILE (object);

	switch ((GsSummaryTileProperty) prop_id) {
	case PROP_PREFERRED_WIDTH:
		g_value_set_int (value, app_tile->preferred_width);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_summary_tile_set_property (GObject *object,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *pspec)
{
	GsSummaryTile *app_tile = GS_SUMMARY_TILE (object);
	GtkLayoutManager *layout_manager;

	switch ((GsSummaryTileProperty) prop_id) {
	case PROP_PREFERRED_WIDTH:
		app_tile->preferred_width = g_value_get_int (value);
		layout_manager = gtk_widget_get_layout_manager (GTK_WIDGET (app_tile));
		GS_SUMMARY_TILE_LAYOUT (layout_manager)->preferred_width = app_tile->preferred_width;
		gtk_layout_manager_layout_changed (layout_manager);
		g_object_notify_by_pspec (object, obj_props[PROP_PREFERRED_WIDTH]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_summary_tile_class_init (GsSummaryTileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GsAppTileClass *tile_class = GS_APP_TILE_CLASS (klass);

	object_class->get_property = gs_summary_tile_get_property;
	object_class->set_property = gs_summary_tile_set_property;
	object_class->notify = gs_summary_tile_notify;

	tile_class->refresh = gs_summary_tile_refresh;

	/**
	 * GsAppTile:preferred-width:
	 *
	 * The only purpose of this property is to be retrieved as the
	 * natural width by gtk_widget_get_preferred_width() fooling the
	 * parent #GtkFlowBox container and making it switch to more columns
	 * (children per row) if it is able to place n+1 children in a row
	 * having this specified width.  If this value is less than a minimum
	 * width of this app tile then the minimum is returned instead.  Set
	 * this property to -1 to turn off this feature and return the default
	 * natural width instead.
	 */
	obj_props[PROP_PREFERRED_WIDTH] =
		g_param_spec_int ("preferred-width",
				  "Preferred width",
				  "The preferred width of this widget, its only purpose is to trick the parent container",
				  -1, G_MAXINT, -1,
				  G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-summary-tile.ui");
	gtk_widget_class_set_layout_manager_type (widget_class, GS_TYPE_SUMMARY_TILE_LAYOUT);
	/* Override the 'button' class name, to be able to turn off hover states */
	gtk_widget_class_set_css_name (widget_class, "gs-summary-tile");

	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      image);
	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      image_stack);
	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      name);
	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      summary);
	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      bin);
	gtk_widget_class_bind_template_child (widget_class, GsSummaryTile,
					      stack);
}

GtkWidget *
gs_summary_tile_new (GsApp *app)
{
	return g_object_new (GS_TYPE_SUMMARY_TILE,
			     "app", app,
			     NULL);
}
