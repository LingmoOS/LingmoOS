/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Matthew Leeds <mwleeds@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-app-version-history-dialog.h"

#include "lingmo-appstore-private.h"
#include "gs-common.h"
#include "gs-app-version-history-row.h"
#include <glib/gi18n.h>

struct _GsAppVersionHistoryDialog
{
	AdwWindow	 parent_instance;
	GsApp		*app;
	GtkWidget	*listbox;
};

G_DEFINE_TYPE (GsAppVersionHistoryDialog, gs_app_version_history_dialog, ADW_TYPE_WINDOW)

static void
populate_version_history (GsAppVersionHistoryDialog *dialog,
			  GsApp			    *app)
{
	g_autoptr(GPtrArray) version_history = NULL;

	/* remove previous */
	gs_widget_remove_all (dialog->listbox, (GsRemoveFunc) gtk_list_box_remove);

	/* grab focus so first row text is not selected by default */
	gtk_widget_grab_focus (dialog->listbox);

	version_history = gs_app_get_version_history (app);
	if (version_history == NULL || version_history->len == 0) {
		GtkWidget *row;
		row = gs_app_version_history_row_new ();
		gs_app_version_history_row_set_always_expanded (GS_APP_VERSION_HISTORY_ROW (row), TRUE);
		gs_app_version_history_row_set_info (GS_APP_VERSION_HISTORY_ROW (row),
						     gs_app_get_version (app),
						     gs_app_get_release_date (app), NULL);
		gtk_list_box_append (GTK_LIST_BOX (dialog->listbox), row);
		gtk_widget_set_visible (row, TRUE);
		return;
	}

	/* add each */
	for (guint i = 0; i < version_history->len; i++) {
		GtkWidget *row;
		AsRelease *version = g_ptr_array_index (version_history, i);

		row = gs_app_version_history_row_new ();
		gs_app_version_history_row_set_always_expanded (GS_APP_VERSION_HISTORY_ROW (row), TRUE);
		gs_app_version_history_row_set_info (GS_APP_VERSION_HISTORY_ROW (row),
						     as_release_get_version (version),
						     as_release_get_timestamp (version),
						     as_release_get_description (version));

		gtk_list_box_append (GTK_LIST_BOX (dialog->listbox), row);
		gtk_widget_set_visible (row, TRUE);
	}
}

static void
gs_app_version_history_dialog_init (GsAppVersionHistoryDialog *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void
gs_app_version_history_dialog_class_init (GsAppVersionHistoryDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-version-history-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsAppVersionHistoryDialog, listbox);

	gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Escape, 0, "window.close", NULL);
}

GtkWidget *
gs_app_version_history_dialog_new (GtkWindow *parent, GsApp *app)
{
	GsAppVersionHistoryDialog *dialog;

	dialog = g_object_new (GS_TYPE_APP_VERSION_HISTORY_DIALOG,
			       "transient-for", parent,
			       "modal", TRUE,
			       NULL);
	populate_version_history (dialog, app);

	return GTK_WIDGET (dialog);
}
