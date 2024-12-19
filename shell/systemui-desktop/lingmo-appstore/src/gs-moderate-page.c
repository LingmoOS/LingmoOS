/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2016-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>

#include "gs-app-row.h"
#include "gs-review-row.h"
#include "gs-shell.h"
#include "gs-moderate-page.h"
#include "gs-common.h"

struct _GsModeratePage
{
	GsPage			 parent_instance;

	GsPluginLoader		*plugin_loader;
	GCancellable		*cancellable;
	GtkSizeGroup		*sizegroup_name;
	GtkSizeGroup		*sizegroup_button_label;
	GtkSizeGroup		*sizegroup_button_image;
	GsShell			*shell;
	GsOdrsProvider		*odrs_provider;

	GtkWidget		*list_box_install;
	GtkWidget		*scrolledwindow_install;
	GtkWidget		*spinner_install;
	GtkWidget		*stack_install;
};

G_DEFINE_TYPE (GsModeratePage, gs_moderate_page, GS_TYPE_PAGE)

typedef enum {
	PROP_ODRS_PROVIDER = 1,
} GsModeratePageProperty;

static GParamSpec *obj_props[PROP_ODRS_PROVIDER + 1]  = { NULL, };

static void
gs_moderate_page_perhaps_hide_app_row (GsModeratePage *self, GsApp *app)
{
	GtkWidget *child;
	GsAppRow *app_row = NULL;
	gboolean is_visible = FALSE;

	for (child = gtk_widget_get_first_child (self->list_box_install);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		if (!gtk_widget_get_visible (child))
			continue;
		if (GS_IS_APP_ROW (child)) {
			GsApp *app_tmp = gs_app_row_get_app (GS_APP_ROW (child));
			if (g_strcmp0 (gs_app_get_id (app),
				       gs_app_get_id (app_tmp)) == 0) {
				app_row = GS_APP_ROW (child);
				continue;
			}
		}
		if (GS_IS_REVIEW_ROW (child)) {
			GsApp *app_tmp = g_object_get_data (G_OBJECT (child), "GsApp");
			if (g_strcmp0 (gs_app_get_id (app),
				       gs_app_get_id (app_tmp)) == 0) {
				is_visible = TRUE;
				break;
			}
		}
	}
	if (!is_visible && app_row != NULL)
		gs_app_row_unreveal (app_row);
}

static void
gs_moderate_page_review_clicked_cb (GsReviewRow *row,
                                    GsReviewAction action,
                                    GsModeratePage *self)
{
	GsApp *app = g_object_get_data (G_OBJECT (row), "GsApp");
	AsReview *review = gs_review_row_get_review (row);
	g_autoptr(GError) local_error = NULL;

	g_assert (self->odrs_provider != NULL);

	/* FIXME: Make this async */
	switch (action) {
	case GS_REVIEW_ACTION_UPVOTE:
		gs_odrs_provider_upvote_review (self->odrs_provider, app,
						review, self->cancellable,
						&local_error);
		break;
	case GS_REVIEW_ACTION_DOWNVOTE:
		gs_odrs_provider_downvote_review (self->odrs_provider, app,
						  review, self->cancellable,
						  &local_error);
		break;
	case GS_REVIEW_ACTION_REPORT:
		gs_odrs_provider_report_review (self->odrs_provider, app,
						review, self->cancellable,
						&local_error);
		break;
	case GS_REVIEW_ACTION_DISMISS:
		gs_odrs_provider_dismiss_review (self->odrs_provider, app,
						 review, self->cancellable,
						 &local_error);
		break;
	case GS_REVIEW_ACTION_REMOVE:
		gs_odrs_provider_remove_review (self->odrs_provider, app,
						review, self->cancellable,
						&local_error);
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_widget_set_visible (GTK_WIDGET (row), FALSE);

	/* if there are no more visible rows, hide the app */
	gs_moderate_page_perhaps_hide_app_row (self, app);

	if (local_error != NULL) {
		g_warning ("failed to set review on %s: %s",
			   gs_app_get_id (app), local_error->message);
		return;
	}
}

static void
gs_moderate_page_selection_changed_cb (GtkListBox *listbox,
                                       GsAppRow *app_row,
                                       GsModeratePage *self)
{
	g_autofree gchar *tmp = NULL;
	tmp = gs_app_to_string (gs_app_row_get_app (app_row));
	g_print ("%s", tmp);
}

static void
gs_moderate_page_add_app (GsModeratePage *self, GsApp *app)
{
	GPtrArray *reviews;
	GtkWidget *app_row;
	guint i;

	/* this hides the action button */
	gs_app_add_quirk (app, GS_APP_QUIRK_COMPULSORY);

	/* add top level app */
	app_row = gs_app_row_new (app);
	gs_app_row_set_show_buttons (GS_APP_ROW (app_row), TRUE);
	gtk_list_box_append (GTK_LIST_BOX (self->list_box_install), app_row);
	gs_app_row_set_size_groups (GS_APP_ROW (app_row),
				    self->sizegroup_name,
				    self->sizegroup_button_label,
				    self->sizegroup_button_image);

	/* add reviews */
	reviews = gs_app_get_reviews (app);
	for (i = 0; i < reviews->len; i++) {
		AsReview *review = g_ptr_array_index (reviews, i);
		GtkWidget *row = gs_review_row_new (review);
		gtk_widget_set_margin_start (row, 250);
		gtk_widget_set_margin_end (row, 250);
		gs_review_row_set_actions (GS_REVIEW_ROW (row),
					   1 << GS_REVIEW_ACTION_UPVOTE |
					   1 << GS_REVIEW_ACTION_DOWNVOTE |
					   1 << GS_REVIEW_ACTION_DISMISS |
					   1 << GS_REVIEW_ACTION_REPORT);
		g_signal_connect (row, "button-clicked",
				  G_CALLBACK (gs_moderate_page_review_clicked_cb), self);
		g_object_set_data_full (G_OBJECT (row), "GsApp",
					g_object_ref (app),
					(GDestroyNotify) g_object_unref);
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install), row);
	}
	gtk_widget_set_visible (app_row, TRUE);
}

static void
gs_moderate_page_refine_unvoted_reviews_cb (GObject      *source_object,
                                            GAsyncResult *res,
                                            gpointer      user_data)
{
	guint i;
	GsApp *app;
	GsModeratePage *self = GS_MODERATE_PAGE (user_data);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;

	gtk_spinner_stop (GTK_SPINNER (self->spinner_install));
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_install), "view");

	list = gs_plugin_loader_job_process_finish (plugin_loader,
							    res,
							    &error);
	if (list == NULL) {
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get moderate apps: %s", error->message);
		return;
	}

	/* no results */
	if (gs_app_list_length (list) == 0) {
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_install),
						  "uptodate");
		return;
	}

	for (i = 0; i < gs_app_list_length (list); i++) {
		app = gs_app_list_index (list, i);
		gs_moderate_page_add_app (self, app);
	}
}

static void
gs_moderate_page_load (GsModeratePage *self)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GError) local_error = NULL;

	/* remove old entries */
	gs_widget_remove_all (self->list_box_install, (GsRemoveFunc) gtk_list_box_remove);

	/* get unvoted reviews as apps */
	if (!gs_odrs_provider_add_unvoted_reviews (self->odrs_provider, list,
						   self->cancellable, &local_error)) {
		if (!g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get moderate apps: %s", local_error->message);
		return;
	}

	plugin_job = gs_plugin_job_refine_new (list,
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
					       GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS);
	gs_plugin_job_set_interactive (plugin_job, TRUE);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    gs_moderate_page_refine_unvoted_reviews_cb,
					    self);
	gtk_spinner_start (GTK_SPINNER (self->spinner_install));
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_install), "spinner");
}

static void
gs_moderate_page_reload (GsPage *page)
{
	GsModeratePage *self = GS_MODERATE_PAGE (page);
	if (gs_shell_get_mode (self->shell) == GS_SHELL_MODE_MODERATE)
		gs_moderate_page_load (self);
}

static void
gs_moderate_page_switch_to (GsPage *page)
{
	GsModeratePage *self = GS_MODERATE_PAGE (page);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_MODERATE) {
		g_warning ("Called switch_to(moderate) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}
	if (gs_shell_get_mode (self->shell) == GS_SHELL_MODE_MODERATE)
		gs_grab_focus_when_mapped (self->scrolledwindow_install);
	gs_moderate_page_load (self);
}

static void
gs_moderate_page_list_header_func (GtkListBoxRow *row,
                                   GtkListBoxRow *before,
                                   gpointer user_data)
{
	GtkWidget *header;
	gtk_list_box_row_set_header (row, NULL);
	if (before == NULL)
		return;
	if (GS_IS_REVIEW_ROW (before) && GS_IS_APP_ROW (row)) {
		header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_list_box_row_set_header (row, header);
	}
}

static gboolean
gs_moderate_page_setup (GsPage *page,
                        GsShell *shell,
                        GsPluginLoader *plugin_loader,
                        GCancellable *cancellable,
                        GError **error)
{
	GsModeratePage *self = GS_MODERATE_PAGE (page);

	g_return_val_if_fail (GS_IS_MODERATE_PAGE (self), TRUE);

	self->shell = shell;
	self->plugin_loader = g_object_ref (plugin_loader);
	self->cancellable = g_object_ref (cancellable);

	gtk_list_box_set_header_func (GTK_LIST_BOX (self->list_box_install),
				      gs_moderate_page_list_header_func,
				      self, NULL);

	return TRUE;
}

static void
gs_moderate_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	GsModeratePage *self = GS_MODERATE_PAGE (object);

	switch ((GsModeratePageProperty) prop_id) {
	case PROP_ODRS_PROVIDER:
		g_value_set_object (value, gs_moderate_page_get_odrs_provider (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_moderate_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	GsModeratePage *self = GS_MODERATE_PAGE (object);

	switch ((GsModeratePageProperty) prop_id) {
	case PROP_ODRS_PROVIDER:
		gs_moderate_page_set_odrs_provider (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_moderate_page_dispose (GObject *object)
{
	GsModeratePage *self = GS_MODERATE_PAGE (object);

	g_clear_object (&self->sizegroup_name);
	g_clear_object (&self->sizegroup_button_label);
	g_clear_object (&self->sizegroup_button_image);

	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->odrs_provider);

	G_OBJECT_CLASS (gs_moderate_page_parent_class)->dispose (object);
}

static void
gs_moderate_page_class_init (GsModeratePageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_moderate_page_get_property;
	object_class->set_property = gs_moderate_page_set_property;
	object_class->dispose = gs_moderate_page_dispose;
	page_class->switch_to = gs_moderate_page_switch_to;
	page_class->reload = gs_moderate_page_reload;
	page_class->setup = gs_moderate_page_setup;

	/**
	 * GsModeratePage:odrs-provider: (nullable)
	 *
	 * An ODRS provider to give access to ratings and reviews information
	 * for the apps being displayed.
	 *
	 * If this is %NULL, ratings and reviews will be disabled and the page
	 * will be effectively useless.
	 *
	 * Since: 41
	 */
	obj_props[PROP_ODRS_PROVIDER] =
		g_param_spec_object ("odrs-provider", NULL, NULL,
				     GS_TYPE_ODRS_PROVIDER,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-moderate-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsModeratePage, list_box_install);
	gtk_widget_class_bind_template_child (widget_class, GsModeratePage, scrolledwindow_install);
	gtk_widget_class_bind_template_child (widget_class, GsModeratePage, spinner_install);
	gtk_widget_class_bind_template_child (widget_class, GsModeratePage, stack_install);
}

static void
gs_moderate_page_init (GsModeratePage *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));

	g_signal_connect (self->list_box_install, "row-activated",
			  G_CALLBACK (gs_moderate_page_selection_changed_cb), self);

	self->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_label = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_image = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
}

GsModeratePage *
gs_moderate_page_new (void)
{
	GsModeratePage *self;
	self = g_object_new (GS_TYPE_MODERATE_PAGE, NULL);
	return GS_MODERATE_PAGE (self);
}

/**
 * gs_moderate_page_get_odrs_provider:
 * @self: a #GsModeratePage
 *
 * Get the value of #GsModeratePage:odrs-provider.
 *
 * Returns: (nullable) (transfer none): a #GsOdrsProvider, or %NULL if unset
 * Since: 41
 */
GsOdrsProvider *
gs_moderate_page_get_odrs_provider (GsModeratePage *self)
{
	g_return_val_if_fail (GS_IS_MODERATE_PAGE (self), NULL);

	return self->odrs_provider;
}

/**
 * gs_moderate_page_set_odrs_provider:
 * @self: a #GsModeratePage
 * @odrs_provider: (nullable) (transfer none): new #GsOdrsProvider or %NULL
 *
 * Set the value of #GsModeratePage:odrs-provider.
 *
 * Since: 41
 */
void
gs_moderate_page_set_odrs_provider (GsModeratePage *self,
                                    GsOdrsProvider *odrs_provider)
{
	g_return_if_fail (GS_IS_MODERATE_PAGE (self));
	g_return_if_fail (odrs_provider == NULL || GS_IS_ODRS_PROVIDER (odrs_provider));

	if (g_set_object (&self->odrs_provider, odrs_provider)) {
		gs_moderate_page_reload (GS_PAGE (self));
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_ODRS_PROVIDER]);
	}
}
