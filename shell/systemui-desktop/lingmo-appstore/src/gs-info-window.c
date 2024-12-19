/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Purism SPC
 *
 * Author: Adrien Plazas <adrien.plazas@puri.sm>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-info-window
 * @short_description: A minimalist window designed to present information
 *
 * #GsInfoWindow is a window with floating window buttons which can be closed
 * by pressing the Escape key. It is intended to present information and to not
 * give the user many interaction possibilities.
 *
 * Since: 42
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adwaita.h>
#include <locale.h>

#include "gs-common.h"
#include "gs-info-window.h"

typedef struct
{
	GtkWidget	*view;
} GsInfoWindowPrivate;

static void gs_info_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (GsInfoWindow, gs_info_window, ADW_TYPE_WINDOW,
			 G_ADD_PRIVATE (GsInfoWindow)
			 G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, gs_info_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
gs_info_window_init (GsInfoWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
}

static void
gs_info_window_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const char   *type)
{
	GsInfoWindow *self = GS_INFO_WINDOW (buildable);
	GsInfoWindowPrivate *priv = gs_info_window_get_instance_private (self);

	if (!priv->view)
		parent_buildable_iface->add_child (buildable, builder, child, type);
	else if (GTK_IS_WIDGET (child))
		gs_info_window_set_child (self, GTK_WIDGET (child));
	else
		GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
}

static void
gs_info_window_buildable_init (GtkBuildableIface *iface)
{
	parent_buildable_iface = g_type_interface_peek_parent (iface);

	iface->add_child = gs_info_window_buildable_add_child;

}

static void
gs_info_window_class_init (GsInfoWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-info-window.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GsInfoWindow, view);

	gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Escape, 0, "window.close", NULL);
}

/**
 * gs_info_window_new:
 *
 * Create a new #GsInfoWindow.
 *
 * Returns: (transfer full): a new #GsInfoWindow
 * Since: 42
 */
GsInfoWindow *
gs_info_window_new (void)
{
	return g_object_new (GS_TYPE_INFO_WINDOW, NULL);
}

/**
 * gs_info_window_set_child:
 * @self: a #GsInfoWindow
 * @widget: (nullable): the new child of @self
 *
 * Create a new #GsInfoWindow.
 *
 * Since: 42
 */
void
gs_info_window_set_child (GsInfoWindow *self,
                          GtkWidget    *widget)
{
	GsInfoWindowPrivate *priv;
	g_return_if_fail (GS_IS_INFO_WINDOW (self));
	g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

	priv = gs_info_window_get_instance_private (self);
	adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (priv->view), widget);
}
