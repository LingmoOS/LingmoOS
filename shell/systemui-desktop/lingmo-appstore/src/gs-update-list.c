/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-update-list.h"

#include "gs-app-row.h"
#include "gs-common.h"

typedef struct
{
	GtkSizeGroup		*sizegroup_name;
	GtkListBox		*listbox;
} GsUpdateListPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GsUpdateList, gs_update_list, GTK_TYPE_WIDGET)

enum {
	SIGNAL_SHOW_UPDATE,
};

static guint signals [SIGNAL_SHOW_UPDATE + 1] = { 0 };

static void
installed_updates_row_activated_cb (GtkListBox    *list_box,
                                    GtkListBoxRow *row,
                                    GsUpdateList  *self)
{
	GsApp *app = gs_app_row_get_app (GS_APP_ROW (row));

	g_signal_emit (self, signals[SIGNAL_SHOW_UPDATE], 0, app);
}

static void
gs_update_list_app_state_notify_cb (GsApp *app, GParamSpec *pspec, gpointer user_data)
{
	if (gs_app_get_state (app) == GS_APP_STATE_INSTALLED) {
		GsAppRow *app_row = GS_APP_ROW (user_data);
		gs_app_row_unreveal (app_row);
	}
}

void
gs_update_list_add_app (GsUpdateList *update_list, GsApp *app)
{
	GsUpdateListPrivate *priv = gs_update_list_get_instance_private (update_list);
	GtkWidget *app_row;

	app_row = gs_app_row_new (app);
	gs_app_row_set_show_description (GS_APP_ROW (app_row), FALSE);
	gs_app_row_set_show_update (GS_APP_ROW (app_row), FALSE);
	gs_app_row_set_show_buttons (GS_APP_ROW (app_row), FALSE);
	gs_app_row_set_show_installed (GS_APP_ROW (app_row), FALSE);
	gtk_list_box_append (priv->listbox, app_row);
	gs_app_row_set_size_groups (GS_APP_ROW (app_row),
				    priv->sizegroup_name,
				    NULL,
				    NULL);
	g_signal_connect_object (app, "notify::state",
	                         G_CALLBACK (gs_update_list_app_state_notify_cb),
	                         app_row, 0);
	gtk_widget_set_visible (app_row, TRUE);
}

static gint
list_sort_func (GtkListBoxRow *a,
		GtkListBoxRow *b,
		gpointer user_data)
{
	GsApp *a1 = gs_app_row_get_app (GS_APP_ROW (a));
	GsApp *b1 = gs_app_row_get_app (GS_APP_ROW (b));
	return g_strcmp0 (gs_app_get_name (a1), gs_app_get_name (b1));
}

static void
gs_update_list_dispose (GObject *object)
{
	GsUpdateList *update_list = GS_UPDATE_LIST (object);
	GsUpdateListPrivate *priv = gs_update_list_get_instance_private (update_list);

	if (priv->listbox != NULL) {
		gtk_widget_unparent (GTK_WIDGET (priv->listbox));
		priv->listbox = NULL;
	}

	g_clear_object (&priv->sizegroup_name);

	G_OBJECT_CLASS (gs_update_list_parent_class)->dispose (object);
}

static void
gs_update_list_init (GsUpdateList *update_list)
{
	GsUpdateListPrivate *priv = gs_update_list_get_instance_private (update_list);
	priv->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	priv->listbox = GTK_LIST_BOX (gtk_list_box_new ());
	gtk_list_box_set_selection_mode (priv->listbox, GTK_SELECTION_NONE);
	gtk_widget_set_parent (GTK_WIDGET (priv->listbox), GTK_WIDGET (update_list));
	gtk_list_box_set_sort_func (priv->listbox, list_sort_func, update_list, NULL);
	gtk_widget_add_css_class (GTK_WIDGET (priv->listbox), "boxed-list");

	g_signal_connect (priv->listbox, "row-activated",
			  G_CALLBACK (installed_updates_row_activated_cb), update_list);
}

static void
gs_update_list_class_init (GsUpdateListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = gs_update_list_dispose;

	signals [SIGNAL_SHOW_UPDATE] =
		g_signal_new ("show-update",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, GS_TYPE_APP);

	gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

GtkWidget *
gs_update_list_new (void)
{
	GsUpdateList *update_list;
	update_list = g_object_new (GS_TYPE_UPDATE_LIST, NULL);
	return GTK_WIDGET (update_list);
}

void
gs_update_list_remove_all (GsUpdateList *update_list)
{
	GsUpdateListPrivate *priv;

	g_return_if_fail (GS_IS_UPDATE_LIST (update_list));

	priv = gs_update_list_get_instance_private (update_list);
	gs_widget_remove_all (GTK_WIDGET (priv->listbox), (GsRemoveFunc) gtk_list_box_remove);
}
