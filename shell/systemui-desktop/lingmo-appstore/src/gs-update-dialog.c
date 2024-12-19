/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-update-dialog.h"
#include "gs-app-details-page.h"
#include "gs-app-row.h"
#include "gs-os-update-page.h"
#include "gs-update-list.h"
#include "gs-common.h"

struct _GsUpdateDialog
{
	AdwWindow	 parent_instance;

	GCancellable	*cancellable;
	GsPluginLoader	*plugin_loader;
	GsApp		*app;
	GtkWidget	*navigation_view;
	GtkWidget	*list_box_installed_updates;
	GtkWidget	*spinner;
	GtkWidget	*stack;
	AdwWindowTitle	*window_title;
	AdwNavigationPage	*default_page;
	gboolean	 showing_installed_updates;
};

G_DEFINE_TYPE (GsUpdateDialog, gs_update_dialog, ADW_TYPE_WINDOW)

typedef enum {
	PROP_PLUGIN_LOADER = 1,
	PROP_APP,
} GsUpdateDialogProperty;

static GParamSpec *obj_props[PROP_APP + 1]  = { NULL, };

static void gs_update_dialog_show_installed_updates (GsUpdateDialog *dialog);
static void gs_update_dialog_show_update_details (GsUpdateDialog *dialog, GsApp *app);

static void
installed_updates_row_activated_cb (GsUpdateList *update_list,
				    GsApp *app,
				    GsUpdateDialog *dialog)
{
	gs_update_dialog_show_update_details (dialog, app);
}

static void
get_installed_updates_cb (GsPluginLoader *plugin_loader,
                          GAsyncResult *res,
                          GsUpdateDialog *dialog)
{
	guint i;
	guint64 install_date;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) error = NULL;

	/* get the results */
	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);

	/* if we're in teardown, short-circuit and return immediately without
	 * dereferencing priv variables */
	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
	    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) ||
	    dialog->spinner == NULL) {
		g_debug ("get installed updates cancelled");
		return;
	}

	gtk_spinner_stop (GTK_SPINNER (dialog->spinner));

	/* error */
	if (list == NULL) {
		g_warning ("failed to get installed updates: %s", error->message);
		gtk_stack_set_visible_child_name (GTK_STACK (dialog->stack), "empty");
		return;
	}

	/* no results */
	if (gs_app_list_length (list) == 0) {
		g_debug ("no installed updates to show");
		gtk_stack_set_visible_child_name (GTK_STACK (dialog->stack), "empty");
		return;
	}

	/* set the header title using any one of the apps */
	install_date = gs_app_get_install_date (gs_app_list_index (list, 0));
	if (install_date > 0) {
		g_autoptr(GDateTime) date = NULL;
		g_autofree gchar *date_str = NULL;
		g_autofree gchar *subtitle = NULL;

		date = g_date_time_new_from_unix_utc ((gint64) install_date);
		date_str = g_date_time_format (date, "%x");

		/* TRANSLATORS: this is the subtitle of the installed updates dialog window.
		   %s will be replaced by the date when the updates were installed.
		   The date format is defined by the locale's preferred date representation
		   ("%x" in strftime.) */
		subtitle = g_strdup_printf (_("Installed on %s"), date_str);
		adw_window_title_set_subtitle (dialog->window_title, subtitle);
	}

	gtk_stack_set_visible_child_name (GTK_STACK (dialog->stack), "installed-updates-list");

	gs_update_list_remove_all (GS_UPDATE_LIST (dialog->list_box_installed_updates));
	for (i = 0; i < gs_app_list_length (list); i++) {
		gs_update_list_add_app (GS_UPDATE_LIST (dialog->list_box_installed_updates),
					gs_app_list_index (list, i));
	}
}

static void
gs_update_dialog_show_installed_updates (GsUpdateDialog *dialog)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;

	dialog->showing_installed_updates = TRUE;

	/* TRANSLATORS: this is the title of the installed updates dialog window */
	adw_navigation_page_set_title (dialog->default_page, _("Installed Updates"));

	gtk_spinner_start (GTK_SPINNER (dialog->spinner));
	gtk_stack_set_visible_child_name (GTK_STACK (dialog->stack), "spinner");

	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_GET_UPDATES_HISTORICAL,
					 "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY |
							 GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
							 GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION,
					 NULL);
	gs_plugin_loader_job_process_async (dialog->plugin_loader, plugin_job,
	                                    dialog->cancellable,
	                                    (GAsyncReadyCallback) get_installed_updates_cb,
	                                    dialog);
}

static void
unset_focus (GtkWidget *widget)
{
	GtkWidget *focus;

	focus = gtk_window_get_focus (GTK_WINDOW (widget));
	if (GTK_IS_LABEL (focus))
		gtk_label_select_region (GTK_LABEL (focus), 0, 0);
}

static void
app_activated_cb (GtkWidget *widget, GsApp *app, GsUpdateDialog *page)
{
	gs_update_dialog_show_update_details (page, app);
}

static void
gs_update_dialog_show_update_details (GsUpdateDialog *dialog, GsApp *app)
{
	GtkWidget *page;
	AsComponentKind kind;
	g_autofree gchar *str = NULL;

	/* debug */
	str = gs_app_to_string (app);
	g_debug ("%s", str);

	/* workaround a gtk+ issue where the dialog comes up with a label selected,
	 * https://bugzilla.lingmo.org/show_bug.cgi?id=734033 */
	unset_focus (GTK_WIDGET (dialog));

	/* set update description */
	kind = gs_app_get_kind (app);
	if (kind == AS_COMPONENT_KIND_GENERIC &&
	    gs_app_get_special_kind (app) == GS_APP_SPECIAL_KIND_OS_UPDATE) {
		page = gs_os_update_page_new ();
		gs_os_update_page_set_app (GS_OS_UPDATE_PAGE (page), app);
		g_signal_connect (page, "app-activated",
				  G_CALLBACK (app_activated_cb), dialog);
	} else {
		page = gs_app_details_page_new (dialog->plugin_loader);
		gs_app_details_page_set_app (GS_APP_DETAILS_PAGE (page), app);
	}

	adw_navigation_view_push (ADW_NAVIGATION_VIEW (dialog->navigation_view), ADW_NAVIGATION_PAGE (page));
}

static void
gs_update_dialog_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	GsUpdateDialog *dialog = GS_UPDATE_DIALOG (object);

	switch ((GsUpdateDialogProperty) prop_id) {
	case PROP_PLUGIN_LOADER:
		g_value_set_object (value, dialog->plugin_loader);
		break;
	case PROP_APP:
		g_value_set_object (value, dialog->app);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_update_dialog_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	GsUpdateDialog *dialog = GS_UPDATE_DIALOG (object);

	switch ((GsUpdateDialogProperty) prop_id) {
	case PROP_PLUGIN_LOADER:
		dialog->plugin_loader = g_object_ref (g_value_get_object (value));
		break;
	case PROP_APP:
		dialog->app = g_value_get_object (value);
		if (dialog->app != NULL)
			g_object_ref (dialog->app);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_update_dialog_constructed (GObject *object)
{
	GsUpdateDialog *dialog = GS_UPDATE_DIALOG (object);

	g_assert (dialog->plugin_loader);

	if (dialog->app) {
		adw_navigation_view_replace (ADW_NAVIGATION_VIEW (dialog->navigation_view), NULL, 0);

		gs_update_dialog_show_update_details (dialog, dialog->app);
	} else {
		gs_update_dialog_show_installed_updates (dialog);
	}

	G_OBJECT_CLASS (gs_update_dialog_parent_class)->constructed (object);
}

static void
gs_update_dialog_dispose (GObject *object)
{
	GsUpdateDialog *dialog = GS_UPDATE_DIALOG (object);

	g_cancellable_cancel (dialog->cancellable);
	g_clear_object (&dialog->cancellable);

	g_clear_object (&dialog->plugin_loader);
	g_clear_object (&dialog->app);

	G_OBJECT_CLASS (gs_update_dialog_parent_class)->dispose (object);
}

static void
gs_update_dialog_init (GsUpdateDialog *dialog)
{
	g_type_ensure (GS_TYPE_UPDATE_LIST);

	gtk_widget_init_template (GTK_WIDGET (dialog));

	dialog->cancellable = g_cancellable_new ();

	g_signal_connect (dialog->list_box_installed_updates, "show-update",
			  G_CALLBACK (installed_updates_row_activated_cb), dialog);

	g_signal_connect_after (dialog, "show", G_CALLBACK (unset_focus), NULL);
}

static void
gs_update_dialog_class_init (GsUpdateDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_update_dialog_get_property;
	object_class->set_property = gs_update_dialog_set_property;
	object_class->constructed = gs_update_dialog_constructed;
	object_class->dispose = gs_update_dialog_dispose;

	/**
	 * GsUpdateDialog:plugin-loader
	 *
	 * The plugin loader of the dialog.
	 *
	 * Since: 41
	 */
	obj_props[PROP_PLUGIN_LOADER] =
		g_param_spec_object ("plugin-loader", NULL, NULL,
				     GS_TYPE_PLUGIN_LOADER,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsUpdateDialog:app: (nullable)
	 *
	 * The app whose details to display.
	 *
	 * If none is set, the intalled updates will be displayed.
	 *
	 * Since: 41
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", NULL, NULL,
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-update-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, navigation_view);
	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, list_box_installed_updates);
	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, spinner);
	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, stack);
	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, window_title);
	gtk_widget_class_bind_template_child (widget_class, GsUpdateDialog, default_page);

	gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Escape, 0, "window.close", NULL);
}

GtkWidget *
gs_update_dialog_new (GsPluginLoader *plugin_loader)
{
	return GTK_WIDGET (g_object_new (GS_TYPE_UPDATE_DIALOG,
					 "plugin-loader", plugin_loader,
					 NULL));
}

GtkWidget *
gs_update_dialog_new_for_app (GsPluginLoader *plugin_loader, GsApp *app)
{
	return GTK_WIDGET (g_object_new (GS_TYPE_UPDATE_DIALOG,
					 "plugin-loader", plugin_loader,
					 "app", app,
					 NULL));
}
