/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2012-2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-app-addon-row.h"

struct _GsAppAddonRow
{
	GtkListBoxRow	 parent_instance;

	GsApp		*app;
	GtkWidget	*name_label;
	GtkWidget	*description_label;
	GtkWidget	*label;
	GtkWidget	*buttons_stack;
	GtkWidget	*button_install;
	GtkWidget	*button_remove;
};

G_DEFINE_TYPE (GsAppAddonRow, gs_app_addon_row, GTK_TYPE_LIST_BOX_ROW)

enum {
	SIGNAL_INSTALL_BUTTON_CLICKED,
	SIGNAL_REMOVE_BUTTON_CLICKED,
	SIGNAL_LAST
};

static guint signals [SIGNAL_LAST] = { 0 };

static void
app_addon_install_button_cb (GtkWidget *widget, GsAppAddonRow *row)
{
	g_signal_emit (row, signals[SIGNAL_INSTALL_BUTTON_CLICKED], 0);
}

static void
app_addon_remove_button_cb (GtkWidget *widget, GsAppAddonRow *row)
{
	g_signal_emit (row, signals[SIGNAL_REMOVE_BUTTON_CLICKED], 0);
}

/**
 * gs_app_addon_row_get_summary:
 *
 * Return value: PangoMarkup
 **/
static GString *
gs_app_addon_row_get_summary (GsAppAddonRow *row)
{
	const gchar *tmp = NULL;
	g_autofree gchar *escaped = NULL;

	/* try all these things in order */
	if (gs_app_get_state (row->app) == GS_APP_STATE_UNAVAILABLE)
		tmp = gs_app_get_summary_missing (row->app);
	if (tmp == NULL || (tmp != NULL && tmp[0] == '\0'))
		tmp = gs_app_get_summary (row->app);
	if (tmp == NULL || (tmp != NULL && tmp[0] == '\0'))
		tmp = gs_app_get_description (row->app);

	escaped = g_markup_escape_text (tmp, -1);
	return g_string_new (escaped);
}

void
gs_app_addon_row_refresh (GsAppAddonRow *row)
{
	g_autoptr(GString) str = NULL;
	gboolean show_install = FALSE, show_remove = FALSE;

	if (row->app == NULL)
		return;

	/* join the lines */
	str = gs_app_addon_row_get_summary (row);
	gs_utils_gstring_replace (str, "\n", " ");
	gtk_label_set_markup (GTK_LABEL (row->description_label), str->str);
	gtk_label_set_label (GTK_LABEL (row->name_label),
			     gs_app_get_name (row->app));

	/* update the state label */
	switch (gs_app_get_state (row->app)) {
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Pending"));
		break;
	case GS_APP_STATE_PENDING_INSTALL:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Pending install"));
		break;
	case GS_APP_STATE_PENDING_REMOVE:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Pending remove"));
		break;
	case GS_APP_STATE_INSTALLING:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Installing"));
		break;
	case GS_APP_STATE_REMOVING:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Removing"));
		break;
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (row->label, TRUE);
		gtk_label_set_label (GTK_LABEL (row->label), _("Downloading"));
		break;
	default:
		gtk_widget_set_visible (row->label, FALSE);
		break;
	}

	/* update the checkbox, remove button, and activatable state */
	switch (gs_app_get_state (row->app)) {
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		show_install = FALSE;
		show_remove = !gs_app_has_quirk (row->app, GS_APP_QUIRK_COMPULSORY);
		gtk_widget_set_sensitive (row->button_remove, !gs_app_has_quirk (row->app, GS_APP_QUIRK_COMPULSORY));
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);
		break;
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
		show_install = TRUE;
		show_remove = FALSE;
		gtk_widget_set_sensitive (row->button_install, TRUE);
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);
		break;
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_UPDATABLE_LIVE:
	case GS_APP_STATE_INSTALLED:
		show_install = FALSE;
		show_remove = !gs_app_has_quirk (row->app, GS_APP_QUIRK_COMPULSORY);
		gtk_widget_set_sensitive (row->button_remove, !gs_app_has_quirk (row->app, GS_APP_QUIRK_COMPULSORY));
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);
		break;
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_DOWNLOADING:
		show_install = FALSE;
		show_remove = TRUE;
		gtk_widget_set_sensitive (row->button_remove, FALSE);
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);
		break;
	default:
		show_install = TRUE;
		show_remove = FALSE;
		gtk_widget_set_sensitive (row->button_install, FALSE);
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);
		break;
	}

	g_assert (!(show_install && show_remove));

	gtk_widget_set_visible (row->buttons_stack, show_install || show_remove);
	gtk_stack_set_visible_child (GTK_STACK (row->buttons_stack), show_install ? row->button_install : row->button_remove);
}

GsApp *
gs_app_addon_row_get_addon (GsAppAddonRow *row)
{
	g_return_val_if_fail (GS_IS_APP_ADDON_ROW (row), NULL);
	return row->app;
}

static gboolean
gs_app_addon_row_refresh_idle (gpointer user_data)
{
	GsAppAddonRow *row = GS_APP_ADDON_ROW (user_data);

	gs_app_addon_row_refresh (row);

	g_object_unref (row);
	return G_SOURCE_REMOVE;
}

static void
gs_app_addon_row_notify_props_changed_cb (GsApp *app,
					  GParamSpec *pspec,
					  GsAppAddonRow *row)
{
	g_idle_add (gs_app_addon_row_refresh_idle, g_object_ref (row));
}

static void
gs_app_addon_row_set_addon (GsAppAddonRow *row, GsApp *app)
{
	row->app = g_object_ref (app);

	g_signal_connect_object (row->app, "notify::state",
				 G_CALLBACK (gs_app_addon_row_notify_props_changed_cb),
				 row, 0);
	gs_app_addon_row_refresh (row);
}

static void
gs_app_addon_row_dispose (GObject *object)
{
	GsAppAddonRow *row = GS_APP_ADDON_ROW (object);

	if (row->app)
		g_signal_handlers_disconnect_by_func (row->app, gs_app_addon_row_notify_props_changed_cb, row);

	g_clear_object (&row->app);

	G_OBJECT_CLASS (gs_app_addon_row_parent_class)->dispose (object);
}

static void
gs_app_addon_row_class_init (GsAppAddonRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = gs_app_addon_row_dispose;

	signals [SIGNAL_INSTALL_BUTTON_CLICKED] =
		g_signal_new ("install-button-clicked",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals [SIGNAL_REMOVE_BUTTON_CLICKED] =
		g_signal_new ("remove-button-clicked",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-addon-row.ui");

	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, name_label);
	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, description_label);
	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, label);
	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, buttons_stack);
	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, button_install);
	gtk_widget_class_bind_template_child (widget_class, GsAppAddonRow, button_remove);
}

static void
gs_app_addon_row_init (GsAppAddonRow *row)
{
	gtk_widget_init_template (GTK_WIDGET (row));

	g_signal_connect (row->button_install, "clicked",
			  G_CALLBACK (app_addon_install_button_cb), row);
	g_signal_connect (row->button_remove, "clicked",
			  G_CALLBACK (app_addon_remove_button_cb), row);
}

void
gs_app_addon_row_activate (GsAppAddonRow *row)
{
	GtkWidget *button;

	g_return_if_fail (GS_IS_APP_ADDON_ROW (row));

	if (!gtk_widget_get_visible (row->buttons_stack))
		return;

	button = gtk_stack_get_visible_child (GTK_STACK (row->buttons_stack));
	if (gtk_widget_get_sensitive (button))
		gtk_widget_activate (button);
}

GtkWidget *
gs_app_addon_row_new (GsApp *app)
{
	GtkWidget *row;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	row = g_object_new (GS_TYPE_APP_ADDON_ROW, NULL);
	gs_app_addon_row_set_addon (GS_APP_ADDON_ROW (row), app);
	return row;
}
