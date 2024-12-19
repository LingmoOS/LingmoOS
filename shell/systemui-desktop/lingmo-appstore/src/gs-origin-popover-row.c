/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-common.h"

#include "gs-origin-popover-row.h"

#include <glib/gi18n.h>

typedef struct
{
	GsApp		*app;
	GtkCssProvider	*css_provider;
	GtkWidget	*name_label;
	GtkWidget	*info_label;
	GtkWidget	*installed_image;
	GtkWidget	*packaging_box;
	GtkWidget	*packaging_image;
	GtkWidget	*packaging_label;
	GtkWidget	*beta_box;
	GtkWidget	*user_scope_box;
	GtkWidget	*selected_image;
} GsOriginPopoverRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GsOriginPopoverRow, gs_origin_popover_row, GTK_TYPE_LIST_BOX_ROW)

static void
refresh_ui (GsOriginPopoverRow *row)
{
	GsOriginPopoverRowPrivate *priv = gs_origin_popover_row_get_instance_private (row);
	const gchar *packaging_base_css_color, *packaging_icon;
	g_autofree gchar *packaging_format = NULL;
	g_autofree gchar *info = NULL;
	g_autofree gchar *css = NULL;
	g_autofree gchar *origin_ui = NULL;
	g_autofree gchar *url = NULL;

	g_assert (GS_IS_ORIGIN_POPOVER_ROW (row));
	g_assert (GS_IS_APP (priv->app));

	origin_ui = gs_app_dup_origin_ui (priv->app, FALSE);
	if (origin_ui != NULL)
		gtk_label_set_text (GTK_LABEL (priv->name_label), origin_ui);

	if (gs_app_get_state (priv->app) == GS_APP_STATE_AVAILABLE_LOCAL ||
	    gs_app_get_local_file (priv->app) != NULL) {
		GFile *local_file = gs_app_get_local_file (priv->app);
		url = g_file_get_basename (local_file);
	} else {
		url = g_strdup (gs_app_get_origin_hostname (priv->app));
	}

	if (gs_app_get_bundle_kind (priv->app) == AS_BUNDLE_KIND_SNAP) {
		const gchar *branch = NULL, *version = NULL;
		const gchar *order[3];
		const gchar *items[7] = { NULL, };
		guint index = 0;

		branch = gs_app_get_branch (priv->app);
		version = gs_app_get_version (priv->app);

		if (gtk_widget_get_direction (GTK_WIDGET (row)) == GTK_TEXT_DIR_RTL) {
			order[0] = version;
			order[1] = branch;
			order[2] = url;
		} else {
			order[0] = url;
			order[1] = branch;
			order[2] = version;
		}

		for (guint ii = 0; ii < G_N_ELEMENTS (order); ii++) {
			const gchar *value = order[ii];

			if (value != NULL) {
				if (index > 0) {
					items[index] = "•";
					index++;
				}
				items[index] = value;
				index++;
			}
		}

		if (index > 0) {
			g_assert (index + 1 < G_N_ELEMENTS (items));
			items[index] = NULL;

			info = g_strjoinv (" ", (gchar **) items);
		}
	} else {
		info = g_steal_pointer (&url);
	}

	if (info != NULL)
		gtk_label_set_text (GTK_LABEL (priv->info_label), info);
	else
		gtk_label_set_text (GTK_LABEL (priv->info_label), _("Unknown source"));

	gtk_widget_set_visible (priv->installed_image, gs_app_is_installed (priv->app));
	gtk_widget_set_visible (priv->beta_box, gs_app_has_quirk (priv->app, GS_APP_QUIRK_DEVELOPMENT_SOURCE));

	if (gs_app_get_bundle_kind (priv->app) == AS_BUNDLE_KIND_FLATPAK &&
	    gs_app_get_scope (priv->app) != AS_COMPONENT_SCOPE_UNKNOWN) {
		AsComponentScope scope = gs_app_get_scope (priv->app);
		gtk_widget_set_visible (priv->user_scope_box, scope == AS_COMPONENT_SCOPE_USER);
	} else {
		gtk_widget_set_visible (priv->user_scope_box, FALSE);
	}

	packaging_base_css_color = gs_app_get_metadata_item (priv->app, "LingmoAppstore::PackagingBaseCssColor");
	packaging_icon = gs_app_get_metadata_item (priv->app, "LingmoAppstore::PackagingIcon");
	packaging_format = gs_app_get_packaging_format (priv->app);

	gtk_label_set_text (GTK_LABEL (priv->packaging_label), packaging_format);

	if (packaging_icon != NULL)
		gtk_image_set_from_icon_name (GTK_IMAGE (priv->packaging_image), packaging_icon);

	if (packaging_base_css_color != NULL)
		css = g_strdup_printf ("   color: @%s;\n", packaging_base_css_color);

	gs_utils_widget_set_css (priv->packaging_box, &priv->css_provider, css);
}

static void
gs_origin_popover_row_set_app (GsOriginPopoverRow *row, GsApp *app)
{
	GsOriginPopoverRowPrivate *priv = gs_origin_popover_row_get_instance_private (row);

	g_assert (priv->app == NULL);

	priv->app = g_object_ref (app);
	refresh_ui (row);
}

GsApp *
gs_origin_popover_row_get_app (GsOriginPopoverRow *row)
{
	GsOriginPopoverRowPrivate *priv = gs_origin_popover_row_get_instance_private (row);
	return priv->app;
}

void
gs_origin_popover_row_set_selected (GsOriginPopoverRow *row, gboolean selected)
{
	GsOriginPopoverRowPrivate *priv = gs_origin_popover_row_get_instance_private (row);

	gtk_widget_set_visible (priv->selected_image, selected);
}

static void
gs_origin_popover_row_dispose (GObject *object)
{
	GsOriginPopoverRow *row = GS_ORIGIN_POPOVER_ROW (object);
	GsOriginPopoverRowPrivate *priv = gs_origin_popover_row_get_instance_private (row);

	g_clear_object (&priv->app);
	g_clear_object (&priv->css_provider);

	G_OBJECT_CLASS (gs_origin_popover_row_parent_class)->dispose (object);
}

static void
gs_origin_popover_row_init (GsOriginPopoverRow *row)
{
	gtk_widget_init_template (GTK_WIDGET (row));
}

static void
gs_origin_popover_row_class_init (GsOriginPopoverRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = gs_origin_popover_row_dispose;

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-origin-popover-row.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, name_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, info_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, installed_image);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, packaging_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, packaging_image);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, packaging_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, beta_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, user_scope_box);
	gtk_widget_class_bind_template_child_private (widget_class, GsOriginPopoverRow, selected_image);
}

GtkWidget *
gs_origin_popover_row_new (GsApp *app)
{
	GsOriginPopoverRow *row = g_object_new (GS_TYPE_ORIGIN_POPOVER_ROW, NULL);
	gs_origin_popover_row_set_app (row, app);
	return GTK_WIDGET (row);
}
