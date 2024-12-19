/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

#include "gs-search-page.h"
#include "gs-shell.h"
#include "gs-common.h"
#include "gs-app-row.h"

#define GS_SEARCH_PAGE_MAX_RESULTS	50

struct _GsSearchPage
{
	GsPage			 parent_instance;

	GsPluginLoader		*plugin_loader;
	GCancellable		*cancellable;
	GCancellable		*search_cancellable;
	GtkSizeGroup		*sizegroup_name;
	GtkSizeGroup		*sizegroup_button_label;
	GtkSizeGroup		*sizegroup_button_image;
	GsShell			*shell;
	gchar			*appid_to_show;
	gchar			*value;
	guint			 waiting_id;
	guint			 max_results;
	guint			 stamp;
	gboolean		 changed;

	GtkWidget		*list_box_search;
	GtkWidget		*scrolledwindow_search;
	GtkWidget		*spinner_search;
	GtkWidget		*stack_search;
};

G_DEFINE_TYPE (GsSearchPage, gs_search_page, GS_TYPE_PAGE)

typedef enum {
	PROP_VADJUSTMENT = 1,
} GsSearchPageProperty;

static void
gs_search_page_app_row_clicked_cb (GsAppRow *app_row,
                                   GsSearchPage *self)
{
	GsApp *app;
	app = gs_app_row_get_app (app_row);
	if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE)
		gs_page_install_app (GS_PAGE (self), app, GS_SHELL_INTERACTION_FULL,
				     self->cancellable);
	else if (gs_app_get_state (app) == GS_APP_STATE_INSTALLED)
		gs_page_remove_app (GS_PAGE (self), app, self->cancellable);
	else if (gs_app_get_state (app) == GS_APP_STATE_UNAVAILABLE) {
		if (gs_app_get_url_missing (app) == NULL) {
			gs_page_install_app (GS_PAGE (self), app,
					     GS_SHELL_INTERACTION_FULL,
					     self->cancellable);
			return;
		}
		gs_shell_show_uri (self->shell,
		                   gs_app_get_url_missing (app));
	}
}

static void
gs_search_page_waiting_cancel (GsSearchPage *self)
{
	if (self->waiting_id > 0)
		g_source_remove (self->waiting_id);
	self->waiting_id = 0;
}

static void
gs_search_page_app_to_show_created_cb (GObject *source_object,
				       GAsyncResult *result,
				       gpointer user_data)
{
	GsSearchPage *self = user_data;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;

	app = gs_plugin_loader_app_create_finish (GS_PLUGIN_LOADER (source_object), result, &error);
	if (app != NULL) {
		g_return_if_fail (GS_IS_SEARCH_PAGE (self));

		gs_shell_show_app (self->shell, app);
	}
}

typedef struct {
	GsSearchPage *self;
	guint stamp;
} GetSearchData;

static void
gs_search_page_get_search_cb (GObject *source_object,
                              GAsyncResult *res,
                              gpointer user_data)
{
	guint i;
	g_autofree GetSearchData *search_data = user_data;
	GsApp *app;
	GsSearchPage *self = search_data->self;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	GtkWidget *app_row;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;

	/* different stamps means another search had been started before this one finished */
	if (search_data->stamp != self->stamp)
		return;

	/* don't do the delayed spinner */
	gs_search_page_waiting_cancel (self);

	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_debug ("search cancelled");
			return;
		}
		g_warning ("failed to get search apps: %s", error->message);
		gtk_spinner_stop (GTK_SPINNER (self->spinner_search));
		if (self->value && self->value[0])
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "no-results");
		else
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "no-search");
		return;
	}

	/* no results */
	if (gs_app_list_length (list) == 0) {
		g_debug ("no search results to show");
		if (self->value && self->value[0])
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "no-results");
		else
			gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "no-search");
		return;
	}

	/* remove old entries */
	gs_widget_remove_all (self->list_box_search, (GsRemoveFunc) gtk_list_box_remove);

	gtk_spinner_stop (GTK_SPINNER (self->spinner_search));
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "results");
	for (i = 0; i < gs_app_list_length (list); i++) {
		app = gs_app_list_index (list, i);
		app_row = gs_app_row_new (app);
		gs_app_row_set_show_rating (GS_APP_ROW (app_row), TRUE);
		g_signal_connect (app_row, "button-clicked",
				  G_CALLBACK (gs_search_page_app_row_clicked_cb),
				  self);
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_search), app_row);
		gs_app_row_set_size_groups (GS_APP_ROW (app_row),
					    self->sizegroup_name,
					    self->sizegroup_button_label,
					    self->sizegroup_button_image);
		gtk_widget_set_visible (app_row, TRUE);
	}

	/* too many results */
	if (gs_app_list_has_flag (list, GS_APP_LIST_FLAG_IS_TRUNCATED)) {
		GtkWidget *w = gtk_label_new (NULL);
		g_autofree gchar *str = NULL;

		/* TRANSLATORS: this is when there are too many search results
		 * to show in in the search page */
		str = g_strdup_printf (ngettext("%u more match",
		                                "%u more matches",
		                                gs_app_list_get_size_peak (list) - gs_app_list_length (list)),
		                       gs_app_list_get_size_peak (list) - gs_app_list_length (list));
		gtk_label_set_label (GTK_LABEL (w), str);
		gtk_widget_set_margin_bottom (w, 20);
		gtk_widget_set_margin_top (w, 20);
		gtk_widget_set_margin_start (w, 20);
		gtk_widget_set_margin_end (w, 20);
		gtk_widget_add_css_class (w, "dim-label");
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_search), w);
		gtk_widget_set_visible (w, TRUE);
	} else {
		/* reset to default */
		self->max_results = GS_SEARCH_PAGE_MAX_RESULTS;
	}

	if (self->appid_to_show != NULL) {
		g_autoptr (GsApp) a = NULL;
		if (as_utils_data_id_valid (self->appid_to_show)) {
			gs_plugin_loader_app_create_async (self->plugin_loader, self->appid_to_show, self->cancellable,
				gs_search_page_app_to_show_created_cb, self);
		} else {
			a = gs_app_new (self->appid_to_show);
		}

		if (a)
			gs_shell_show_app (self->shell, a);

		g_clear_pointer (&self->appid_to_show, g_free);
	}
}

static gboolean
gs_search_page_waiting_show_cb (gpointer user_data)
{
	GsSearchPage *self = GS_SEARCH_PAGE (user_data);

	/* show spinner */
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_search), "spinner");
	gtk_spinner_start (GTK_SPINNER (self->spinner_search));
	gs_search_page_waiting_cancel (self);
	return FALSE;
}

static gchar *
gs_search_page_get_app_sort_key (GsApp *app)
{
	GString *key = g_string_sized_new (64);

	/* sort apps before runtimes and extensions */
	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		g_string_append (key, "9:");
		break;
	default:
		g_string_append (key, "1:");
		break;
	}

	/* sort missing codecs before apps */
	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_UNAVAILABLE:
		g_string_append (key, "9:");
		break;
	default:
		g_string_append (key, "1:");
		break;
	}

	/* sort by the search key */
	g_string_append_printf (key, "%05x:", gs_app_get_match_value (app));

	/* sort by rating */
	g_string_append_printf (key, "%03i:", gs_app_get_rating (app));

	/* sort by kudos */
	g_string_append_printf (key, "%03u:", gs_app_get_kudos_percentage (app));

	return g_string_free (key, FALSE);
}

static gint
gs_search_page_sort_cb (GsApp *app1, GsApp *app2, gpointer user_data)
{
	g_autofree gchar *key1 = NULL;
	g_autofree gchar *key2 = NULL;
	key1 = gs_search_page_get_app_sort_key (app1);
	key2 = gs_search_page_get_app_sort_key (app2);
	return g_strcmp0 (key2, key1);
}

static void
gs_search_page_load (GsSearchPage *self)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	const gchar *keywords[2] = { NULL, };
	g_autofree GetSearchData *search_data = NULL;

	self->changed = FALSE;

	/* cancel any pending searches */
	g_cancellable_cancel (self->search_cancellable);
	g_clear_object (&self->search_cancellable);
	self->search_cancellable = g_cancellable_new ();
	self->stamp++;

	/* search for apps */
	gs_search_page_waiting_cancel (self);
	self->waiting_id = g_timeout_add (250, gs_search_page_waiting_show_cb, self);

	search_data = g_new0 (GetSearchData, 1);
	search_data->self = self;
	search_data->stamp = self->stamp;

	keywords[0] = self->value;
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING,
				  "dedupe-flags", GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED |
						  GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES,
				  "max-results", self->max_results,
				  "sort-func", gs_search_page_sort_cb,
				  "sort-user-data", self,
				  "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
				  "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->search_cancellable,
					    gs_search_page_get_search_cb,
					    g_steal_pointer (&search_data));
}

static void
gs_search_page_app_row_activated_cb (GtkListBox *list_box,
                                     GtkListBoxRow *row,
                                     GsSearchPage *self)
{
	GsApp *app;

	/* increase the maximum allowed, and re-request the search */
	if (!GS_IS_APP_ROW (row)) {
		self->max_results *= 4;
		gs_search_page_load (self);
		return;
	}

	app = gs_app_row_get_app (GS_APP_ROW (row));
	gs_shell_show_app (self->shell, app);
}

static void
gs_search_page_reload (GsPage *page)
{
	GsSearchPage *self = GS_SEARCH_PAGE (page);
	if (self->value != NULL)
		gs_search_page_load (self);
}

/**
 * gs_search_page_set_appid_to_show:
 *
 * Switch to the specified app id after loading the search results.
 **/
void
gs_search_page_set_appid_to_show (GsSearchPage *self, const gchar *appid)
{
	if (appid == self->appid_to_show ||
	    g_strcmp0 (appid, self->appid_to_show) == 0)
		return;

	g_free (self->appid_to_show);
	self->appid_to_show = g_strdup (appid);

	self->changed = TRUE;
}

const gchar *
gs_search_page_get_text (GsSearchPage *self)
{
	return self->value;
}

void
gs_search_page_set_text (GsSearchPage *self, const gchar *value)
{
	if (value == self->value ||
	    g_strcmp0 (value, self->value) == 0)
		return;

	g_free (self->value);
	self->value = g_strdup (value);

	/* Load immediately, when the page is active */
	if (self->value && gs_page_is_active (GS_PAGE (self)))
		gs_search_page_load (self);
	else
		self->changed = TRUE;
}

static void
gs_search_page_switch_to (GsPage *page)
{
	GsSearchPage *self = GS_SEARCH_PAGE (page);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_SEARCH) {
		g_warning ("Called switch_to(search) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}

	if (self->value && self->changed)
		gs_search_page_load (self);
}

static void
gs_search_page_switch_from (GsPage *page)
{
	GsSearchPage *self = GS_SEARCH_PAGE (page);

	g_cancellable_cancel (self->search_cancellable);
	g_clear_object (&self->search_cancellable);
}

static void
gs_search_page_cancel_cb (GCancellable *cancellable,
                          GsSearchPage *self)
{
	g_cancellable_cancel (self->search_cancellable);
}

static void
gs_search_page_app_installed (GsPage *page, GsApp *app)
{
	gs_search_page_reload (page);
}

static void
gs_search_page_app_removed (GsPage *page, GsApp *app)
{
	gs_search_page_reload (page);
}

static gboolean
gs_search_page_setup (GsPage *page,
                      GsShell *shell,
                      GsPluginLoader *plugin_loader,
                      GCancellable *cancellable,
                      GError **error)
{
	GsSearchPage *self = GS_SEARCH_PAGE (page);

	g_return_val_if_fail (GS_IS_SEARCH_PAGE (self), TRUE);

	self->plugin_loader = g_object_ref (plugin_loader);
	self->cancellable = g_object_ref (cancellable);
	self->shell = shell;

	/* connect the cancellables */
	g_cancellable_connect (self->cancellable,
			       G_CALLBACK (gs_search_page_cancel_cb),
			       self, NULL);

	/* setup search */
	g_signal_connect (self->list_box_search, "row-activated",
			  G_CALLBACK (gs_search_page_app_row_activated_cb), self);
	return TRUE;
}

static void
gs_search_page_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	GsSearchPage *self = GS_SEARCH_PAGE (object);

	switch ((GsSearchPageProperty) prop_id) {
	case PROP_VADJUSTMENT:
		g_value_set_object (value, gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_search)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_search_page_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	switch ((GsSearchPageProperty) prop_id) {
	case PROP_VADJUSTMENT:
		/* Not supported yet */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_search_page_dispose (GObject *object)
{
	GsSearchPage *self = GS_SEARCH_PAGE (object);

	g_clear_object (&self->sizegroup_name);
	g_clear_object (&self->sizegroup_button_label);
	g_clear_object (&self->sizegroup_button_image);

	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->search_cancellable);

	G_OBJECT_CLASS (gs_search_page_parent_class)->dispose (object);
}

static void
gs_search_page_finalize (GObject *object)
{
	GsSearchPage *self = GS_SEARCH_PAGE (object);

	g_free (self->appid_to_show);
	g_free (self->value);

	G_OBJECT_CLASS (gs_search_page_parent_class)->finalize (object);
}

static void
gs_search_page_class_init (GsSearchPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_search_page_get_property;
	object_class->set_property = gs_search_page_set_property;
	object_class->dispose = gs_search_page_dispose;
	object_class->finalize = gs_search_page_finalize;

	page_class->app_installed = gs_search_page_app_installed;
	page_class->app_removed = gs_search_page_app_removed;
	page_class->switch_to = gs_search_page_switch_to;
	page_class->switch_from = gs_search_page_switch_from;
	page_class->reload = gs_search_page_reload;
	page_class->setup = gs_search_page_setup;

	g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-search-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsSearchPage, list_box_search);
	gtk_widget_class_bind_template_child (widget_class, GsSearchPage, scrolledwindow_search);
	gtk_widget_class_bind_template_child (widget_class, GsSearchPage, spinner_search);
	gtk_widget_class_bind_template_child (widget_class, GsSearchPage, stack_search);
}

static void
gs_search_page_init (GsSearchPage *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));

	self->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_label = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_image = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	self->max_results = GS_SEARCH_PAGE_MAX_RESULTS;
}

GsSearchPage *
gs_search_page_new (void)
{
	GsSearchPage *self;
	self = g_object_new (GS_TYPE_SEARCH_PAGE, NULL);
	return GS_SEARCH_PAGE (self);
}
