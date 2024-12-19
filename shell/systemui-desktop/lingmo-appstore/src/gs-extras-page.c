/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-extras-page.h"

#include "gs-app-row.h"
#include "gs-application.h"
#include "gs-language.h"
#include "gs-shell.h"
#include "gs-common.h"
#include "gs-utils.h"
#include "gs-vendor.h"

#include <glib/gi18n.h>

typedef enum {
	GS_EXTRAS_PAGE_STATE_LOADING,
	GS_EXTRAS_PAGE_STATE_READY,
	GS_EXTRAS_PAGE_STATE_NO_RESULTS,
	GS_EXTRAS_PAGE_STATE_FAILED
} GsExtrasPageState;

typedef struct {
	gchar		*title;
	gchar		*search;
	GsAppQueryProvidesType search_provides_type;
	gchar		*search_filename;
	gchar		*package_filename;
	gchar		*url_not_found;
	GsExtrasPage	*self;
} SearchData;

struct _GsExtrasPage
{
	GsPage			  parent_instance;

	GsPluginLoader		 *plugin_loader;
	GCancellable		 *search_cancellable;
	GsShell			 *shell;
	GsExtrasPageState	  state;
	GtkSizeGroup		 *sizegroup_name;
	GtkSizeGroup		 *sizegroup_button_label;
	GtkSizeGroup		 *sizegroup_button_image;
	GPtrArray		 *array_search_data;
	GsExtrasPageMode	  mode;
	GsLanguage		 *language;
	GsVendor		 *vendor;
	guint			  pending_search_cnt;
	gchar			 *caller_app_name;
	gchar			 *install_resources_ident;

	GtkWidget		 *label_failed;
	GtkWidget		 *label_no_results;
	GtkWidget		 *list_box_results;
	GtkWidget		 *scrolledwindow;
	GtkWidget		 *spinner;
	GtkWidget		 *stack;
	GtkWidget		 *button_install_all;
};

G_DEFINE_TYPE (GsExtrasPage, gs_extras_page, GS_TYPE_PAGE)

typedef enum {
	PROP_VADJUSTMENT = 1,
	PROP_TITLE,
} GsExtrasPageProperty;

static void
search_data_free (SearchData *search_data)
{
	if (search_data->self != NULL)
		g_object_unref (search_data->self);
	g_free (search_data->title);
	g_free (search_data->search);
	g_free (search_data->search_filename);
	g_free (search_data->package_filename);
	g_free (search_data->url_not_found);
	g_slice_free (SearchData, search_data);
}

static GsExtrasPageMode
gs_extras_page_mode_from_string (const gchar *str)
{
	if (g_strcmp0 (str, "install-package-files") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_FILES;
	if (g_strcmp0 (str, "install-provide-files") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_PROVIDE_FILES;
	if (g_strcmp0 (str, "install-package-names") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_NAMES;
	if (g_strcmp0 (str, "install-mime-types") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_MIME_TYPES;
	if (g_strcmp0 (str, "install-fontconfig-resources") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES;
	if (g_strcmp0 (str, "install-gstreamer-resources") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_GSTREAMER_RESOURCES;
	if (g_strcmp0 (str, "install-plasma-resources") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_PLASMA_RESOURCES;
	if (g_strcmp0 (str, "install-printer-drivers") == 0)
		return GS_EXTRAS_PAGE_MODE_INSTALL_PRINTER_DRIVERS;

	g_assert_not_reached ();
}

const gchar *
gs_extras_page_mode_to_string (GsExtrasPageMode mode)
{
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_FILES)
		return "install-package-files";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_PROVIDE_FILES)
		return "install-provide-files";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_NAMES)
		return "install-package-names";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_MIME_TYPES)
		return "install-mime-types";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES)
		return "install-fontconfig-resources";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_GSTREAMER_RESOURCES)
		return "install-gstreamer-resources";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_PLASMA_RESOURCES)
		return "install-plasma-resources";
	if (mode == GS_EXTRAS_PAGE_MODE_INSTALL_PRINTER_DRIVERS)
		return "install-printer-drivers";

	g_assert_not_reached ();
}

static gchar *
build_comma_separated_list (gchar **items)
{
	guint len;

	len = g_strv_length (items);
	if (len == 2) {
		/* TRANSLATORS: separator for a list of items */
		return g_strjoinv (_(" and "), items);
	} else {
		/* TRANSLATORS: separator for a list of items */
		return g_strjoinv (_(", "), items);
	}
}

static gchar *
build_title (GsExtrasPage *self)
{
	guint i;
	g_autofree gchar *titles = NULL;
	g_autoptr(GPtrArray) title_array = NULL;

	title_array = g_ptr_array_new ();
	for (i = 0; i < self->array_search_data->len; i++) {
		SearchData *search_data;

		search_data = g_ptr_array_index (self->array_search_data, i);
		g_ptr_array_add (title_array, search_data->title);
	}
	g_ptr_array_add (title_array, NULL);

	titles = build_comma_separated_list ((gchar **) title_array->pdata);

	switch (self->mode) {
	case GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES:
		/* TRANSLATORS: App window title for fonts installation.
		   %s will be replaced by name of the script we're searching for. */
		return g_strdup_printf (ngettext ("Available fonts for the %s script",
		                                  "Available fonts for the %s scripts",
		                                  self->array_search_data->len),
		                        titles);
		break;
	default:
		/* TRANSLATORS: App window title for codec installation.
		   %s will be replaced by actual codec name(s) */
		return g_strdup_printf (ngettext ("Available software for %s",
		                                  "Available software for %s",
		                                  self->array_search_data->len),
		                        titles);
		break;
	}
}

static void
gs_extras_page_update_ui_state (GsExtrasPage *self)
{
	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_EXTRAS)
		return;

	/* main spinner */
	switch (self->state) {
	case GS_EXTRAS_PAGE_STATE_LOADING:
		gtk_spinner_start (GTK_SPINNER (self->spinner));
		break;
	case GS_EXTRAS_PAGE_STATE_READY:
	case GS_EXTRAS_PAGE_STATE_NO_RESULTS:
	case GS_EXTRAS_PAGE_STATE_FAILED:
		gtk_spinner_stop (GTK_SPINNER (self->spinner));
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	/* stack */
	switch (self->state) {
	case GS_EXTRAS_PAGE_STATE_LOADING:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack), "spinner");
		break;
	case GS_EXTRAS_PAGE_STATE_READY:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack), "results");
		break;
	case GS_EXTRAS_PAGE_STATE_NO_RESULTS:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack), "no-results");
		break;
	case GS_EXTRAS_PAGE_STATE_FAILED:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack), "failed");
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
gs_extras_page_maybe_emit_installed_resources_done (GsExtrasPage *self)
{
	if (self->install_resources_ident && (
	    self->state == GS_EXTRAS_PAGE_STATE_LOADING ||
	    self->state == GS_EXTRAS_PAGE_STATE_NO_RESULTS ||
	    self->state == GS_EXTRAS_PAGE_STATE_FAILED)) {
		GsApplication *application;
		GError *op_error = NULL;

		/* When called during the LOADING state, it means the package is already installed */
		if (self->state == GS_EXTRAS_PAGE_STATE_NO_RESULTS) {
			g_set_error_literal (&op_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, _("Requested software not found"));
		} else if (self->state == GS_EXTRAS_PAGE_STATE_FAILED) {
			g_set_error_literal (&op_error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Failed to find requested software"));
		}

		application = GS_APPLICATION (g_application_get_default ());
		gs_application_emit_install_resources_done (application, self->install_resources_ident, op_error);

		g_clear_pointer (&self->install_resources_ident, g_free);
		g_clear_error (&op_error);
	}
}

static void
gs_extras_page_set_state (GsExtrasPage *self,
                          GsExtrasPageState state)
{
	if (self->state == state)
		return;

	self->state = state;

	g_object_notify (G_OBJECT (self), "title");
	gs_extras_page_update_ui_state (self);
	gs_extras_page_maybe_emit_installed_resources_done (self);
}

static gboolean
gs_extras_page_can_install_app (GsApp *app)
{
	return gs_app_get_state (app) == GS_APP_STATE_AVAILABLE ||
	       gs_app_get_state (app) == GS_APP_STATE_AVAILABLE_LOCAL ||
	      (gs_app_get_state (app) == GS_APP_STATE_UNAVAILABLE && gs_app_get_url_missing (app) == NULL);
}

static void
app_row_button_clicked_cb (GsAppRow *app_row,
                           GsExtrasPage *self)
{
	GsApp *app = gs_app_row_get_app (app_row);

	if (gs_app_get_state (app) == GS_APP_STATE_UNAVAILABLE &&
	    gs_app_get_url_missing (app) != NULL) {
		gs_shell_show_uri (self->shell,
	                           gs_app_get_url_missing (app));
	} else if (gs_extras_page_can_install_app (app)) {
		gs_page_install_app (GS_PAGE (self), app, GS_SHELL_INTERACTION_FULL,
				     self->search_cancellable);
	} else if (gs_app_get_state (app) == GS_APP_STATE_INSTALLED) {
		gs_page_remove_app (GS_PAGE (self), app, self->search_cancellable);
	} else {
		g_critical ("extras: app in unexpected state %u", gs_app_get_state (app));
	}
}

static void
gs_extras_page_button_install_all_cb (GtkWidget *button,
				      GsExtrasPage *self)
{
	for (GtkWidget *child = gtk_widget_get_first_child (self->list_box_results);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		GsApp *app;

		/* Might be a separator from list_header_func(). */
		if (!GS_IS_APP_ROW (child))
			continue;

		app = gs_app_row_get_app (GS_APP_ROW (child));

		if (gs_extras_page_can_install_app (app)) {
			gs_page_install_app (GS_PAGE (self), app, GS_SHELL_INTERACTION_FULL,
					     self->search_cancellable);
		}
	}
}

static void
gs_extras_page_app_notify_state_cb (GsApp *app,
				    GParamSpec *param,
				    GsExtrasPage *self)
{
	GtkWidget *child;
	guint n_can_install = 0;

	/* No need to insensitive the button, when it's not visible */
	if (!gtk_widget_get_visible (self->button_install_all))
		return;

	if (gs_app_get_state (app) == GS_APP_STATE_INSTALLING ||
	    gs_app_get_state (app) == GS_APP_STATE_REMOVING ||
	    gs_app_get_state (app) == GS_APP_STATE_DOWNLOADING) {
		gtk_widget_set_sensitive (self->button_install_all, FALSE);
		return;
	}

	for (child = gtk_widget_get_first_child (self->list_box_results);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		GsApp *existing_app;

		/* Might be a separator from list_header_func(). */
		if (!GS_IS_APP_ROW (child))
			continue;

		existing_app = gs_app_row_get_app (GS_APP_ROW (child));
		if (gs_extras_page_can_install_app (existing_app)) {
			n_can_install++;
			if (n_can_install > 1)
				break;
		}
	}

	gtk_widget_set_sensitive (self->button_install_all, n_can_install > 1);
}

static void
gs_extras_page_add_app (GsExtrasPage *self, GsApp *app, GsAppList *list, SearchData *search_data)
{
	GtkWidget *app_row, *child;
	guint n_can_install = 0;
	guint n_codecs = 0;

	/* Don't add same app twice */
	for (child = gtk_widget_get_first_child (self->list_box_results);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		GsApp *existing_app;

		/* Might be a separator from list_header_func(). */
		if (!GS_IS_APP_ROW (child))
			continue;

		existing_app = gs_app_row_get_app (GS_APP_ROW (child));
		if (app == existing_app) {
			g_signal_handlers_disconnect_by_func (existing_app, G_CALLBACK (gs_extras_page_app_notify_state_cb), self);
			gtk_list_box_remove (GTK_LIST_BOX (self->list_box_results), child);
		} else {
			if (gs_extras_page_can_install_app (existing_app))
				n_can_install++;
			if (gs_app_get_kind (existing_app) == AS_COMPONENT_KIND_CODEC)
				n_codecs++;
		}
	}

	if (gs_extras_page_can_install_app (app))
		n_can_install++;
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_CODEC)
		n_codecs++;

	app_row = gs_app_row_new (app);
	gs_app_row_set_colorful (GS_APP_ROW (app_row), TRUE);
	gs_app_row_set_show_buttons (GS_APP_ROW (app_row), TRUE);

	g_object_set_data_full (G_OBJECT (app_row), "missing-title", g_strdup (search_data->title), g_free);

	g_signal_connect (app_row, "button-clicked",
	                  G_CALLBACK (app_row_button_clicked_cb),
	                  self);
	g_signal_connect_object (app, "notify::state", G_CALLBACK (gs_extras_page_app_notify_state_cb), self, 0);

	gtk_list_box_append (GTK_LIST_BOX (self->list_box_results), app_row);
	gs_app_row_set_size_groups (GS_APP_ROW (app_row),
				    self->sizegroup_name,
				    self->sizegroup_button_label,
				    self->sizegroup_button_image);

	gtk_widget_set_sensitive (self->button_install_all, TRUE);
	/* let install in bulk only codecs */
	gtk_widget_set_visible (self->button_install_all, n_can_install > 1 && n_can_install == n_codecs);
}

static GsApp *
create_missing_app (SearchData *search_data)
{
	GsExtrasPage *self = search_data->self;
	GsApp *app;
	GString *summary_missing;
	g_autofree gchar *name = NULL;
	g_autofree gchar *url = NULL;

	app = gs_app_new ("missing-codec");

	/* TRANSLATORS: This string is used for codecs that weren't found */
	name = g_strdup_printf (_("%s not found"), search_data->title);
	gs_app_set_name (app, GS_APP_QUALITY_HIGHEST, name);

	/* TRANSLATORS: hyperlink title */
	url = g_strdup_printf ("<a href=\"%s\">%s</a>", search_data->url_not_found, _("on the website"));

	summary_missing = g_string_new ("");
	switch (self->mode) {
	case GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_FILES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No apps are available that provide the file %s."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get missing apps "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PROVIDE_FILES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No apps are available for %s support."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get missing apps "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_NAMES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("%s is not available."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get missing apps "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_MIME_TYPES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No apps are available for %s support."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get an app that can support this format "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No fonts are available for the %s script support."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get additional fonts "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_GSTREAMER_RESOURCES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No addon codecs are available for the %s format."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get a codec that can play this format "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PLASMA_RESOURCES:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No Plasma resources are available for %s support."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
                 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get additional Plasma resources "
					"might be found %s."), search_data->title, url);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PRINTER_DRIVERS:
		/* TRANSLATORS: this is when we know about an app or
		 * addon, but it can't be listed for some reason */
		g_string_append_printf (summary_missing, _("No printer drivers are available for %s."), search_data->title);
		g_string_append (summary_missing, "\n");
		/* TRANSLATORS: first %s is the codec name, and second %s is a
		 * hyperlink with the "on the website" text */
		g_string_append_printf (summary_missing, _("Information about %s, as well as options "
					"for how to get a driver that supports this printer "
					"might be found %s."), search_data->title, url);

		break;
	default:
		g_assert_not_reached ();
		break;
	}
	gs_app_set_summary_missing (app, g_string_free (summary_missing, FALSE));

	gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
	gs_app_set_state (app, GS_APP_STATE_UNAVAILABLE);
	gs_app_set_url_missing (app, search_data->url_not_found);

	return app;
}

static gchar *
build_no_results_label (GsExtrasPage *self)
{
	GsApp *app = NULL;
	guint num = 0;
	g_autofree gchar *codec_titles = NULL;
	g_autofree gchar *url = NULL;
	g_autoptr(GPtrArray) array = NULL;
	GtkWidget *child;

	array = g_ptr_array_new ();
	for (child = gtk_widget_get_first_child (self->list_box_results);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		/* Might be a separator from list_header_func(). */
		if (!GS_IS_APP_ROW (child))
			continue;

		app = gs_app_row_get_app (GS_APP_ROW (child));
		g_ptr_array_add (array,
		                 g_object_get_data (G_OBJECT (child), "missing-title"));
		num++;
	}
	g_ptr_array_add (array, NULL);

	url = g_strdup_printf ("<a href=\"%s\">%s</a>",
	                       gs_app_get_url_missing (app),
                               /* TRANSLATORS: hyperlink title */
                               _("the documentation"));

	codec_titles = build_comma_separated_list ((gchar **) array->pdata);
	if (self->caller_app_name) {
		/* TRANSLATORS: no codecs were found. The first %s will be replaced by actual codec name(s),
		   the second %s is the app name, which requested the codecs, the third %s is a link titled "the documentation" */
		return g_strdup_printf (ngettext ("Unable to find the %s requested by %s. Please see %s for more information.",
						  "Unable to find the %s requested by %s. Please see %s for more information.",
						  num),
					codec_titles,
					self->caller_app_name,
					url);
	}

	/* TRANSLATORS: no codecs were found. First %s will be replaced by actual codec name(s), second %s is a link titled "the documentation" */
	return g_strdup_printf (ngettext ("Unable to find the %s you were searching for. Please see %s for more information.",
	                                  "Unable to find the %s you were searching for. Please see %s for more information.",
	                                  num),
	                        codec_titles,
	                        url);
}

static void
show_search_results (GsExtrasPage *self)
{
	GtkWidget *first_child, *child;
	GsApp *app;
	guint n_children;
	guint n_missing;

	/* count the number of rows with missing codecs */
	n_children = n_missing = 0;
	first_child = gtk_widget_get_first_child (self->list_box_results);
	for (child = first_child;
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		/* Might be a separator from list_header_func(). */
		if (!GS_IS_APP_ROW (child))
			continue;

		app = gs_app_row_get_app (GS_APP_ROW (child));
		if (g_strcmp0 (gs_app_get_id (app), "missing-codec") == 0) {
			n_missing++;
		}
		n_children++;
	}

	if (n_children == 0 || n_children == n_missing) {
		g_autofree gchar *str = NULL;

		/* no results */
		g_debug ("extras: failed to find any results, %u", n_missing);
		str = build_no_results_label (self);
		gtk_label_set_label (GTK_LABEL (self->label_no_results), str);
		gs_extras_page_set_state (self, GS_EXTRAS_PAGE_STATE_NO_RESULTS);
	} else {
		/* show what we got */
		g_debug ("extras: got %u search results, showing", n_children);
		gs_extras_page_set_state (self, GS_EXTRAS_PAGE_STATE_READY);

		if (n_children == 1) {
			/* switch directly to details view */
			g_debug ("extras: found one result, showing in details view");
			g_assert (first_child != NULL);
			app = gs_app_row_get_app (GS_APP_ROW (first_child));
			gs_shell_show_app (self->shell, app);
			if (gs_app_is_installed (app))
				gs_extras_page_maybe_emit_installed_resources_done (self);
		}
	}
}

static void
search_files_cb (GObject *source_object,
                 GAsyncResult *res,
                 gpointer user_data)
{
	SearchData *search_data = (SearchData *) user_data;
	GsExtrasPage *self = search_data->self;
	g_autoptr(GsAppList) list = NULL;
	guint i;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		g_autofree gchar *str = NULL;
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_debug ("extras: search files cancelled");
			return;
		}
		g_warning ("failed to find any search results: %s", error->message);
		str = g_strdup_printf (_("Failed to find any search results: %s"), error->message);
		gtk_label_set_label (GTK_LABEL (self->label_failed), str);
		gs_extras_page_set_state (self, GS_EXTRAS_PAGE_STATE_FAILED);
		return;
	}

	/* add missing item */
	if (gs_app_list_length (list) == 0) {
		g_autoptr(GsApp) app = NULL;
		g_debug ("extras: no search result for %s, showing as missing",
			 search_data->title);
		app = create_missing_app (search_data);
		gs_app_list_add (list, app);
	}

	for (i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		g_debug ("%s\n\n", gs_app_to_string (app));
		gs_extras_page_add_app (self, app, list, search_data);
	}

	self->pending_search_cnt--;

	/* have all searches finished? */
	if (self->pending_search_cnt == 0)
		show_search_results (self);
}

static void
file_to_app_cb (GObject *source_object,
                GAsyncResult *res,
                gpointer user_data)
{
	SearchData *search_data = (SearchData *) user_data;
	GsExtrasPage *self = search_data->self;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) list = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_debug ("extras: search what provides cancelled");
			return;
		}
		if (g_error_matches (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_FAILED)) {
			g_debug ("extras: no search result for %s, showing as missing", search_data->title);
			app = create_missing_app (search_data);
		} else {
			g_autofree gchar *str = NULL;

			g_warning ("failed to find any search results: %s", error->message);
			str = g_strdup_printf (_("Failed to find any search results: %s"), error->message);
			gtk_label_set_label (GTK_LABEL (self->label_failed), str);
			gs_extras_page_set_state (self, GS_EXTRAS_PAGE_STATE_FAILED);
			return;
		}
	} else {
		app = g_object_ref (gs_app_list_index (list, 0));
	}

	g_debug ("%s\n\n", gs_app_to_string (app));
	gs_extras_page_add_app (self, app, list, search_data);

	self->pending_search_cnt--;

	/* have all searches finished? */
	if (self->pending_search_cnt == 0)
		show_search_results (self);
}

static void
get_search_what_provides_cb (GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
	SearchData *search_data = (SearchData *) user_data;
	GsExtrasPage *self = search_data->self;
	g_autoptr(GsAppList) list = NULL;
	guint i;
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader, res, &error);
	if (list == NULL) {
		g_autofree gchar *str = NULL;
		if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_debug ("extras: search what provides cancelled");
			return;
		}
		g_warning ("failed to find any search results: %s", error->message);
		str = g_strdup_printf (_("Failed to find any search results: %s"), error->message);
		gtk_label_set_label (GTK_LABEL (self->label_failed), str);
		gs_extras_page_set_state (self, GS_EXTRAS_PAGE_STATE_FAILED);
		return;
	}

	/* add missing item */
	if (gs_app_list_length (list) == 0) {
		g_autoptr(GsApp) app = NULL;
		g_debug ("extras: no search result for %s, showing as missing",
			 search_data->title);
		app = create_missing_app (search_data);
		gs_app_list_add (list, app);
	}

	for (i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		g_debug ("%s\n\n", gs_app_to_string (app));
		gs_extras_page_add_app (self, app, list, search_data);
	}

	self->pending_search_cnt--;

	/* have all searches finished? */
	if (self->pending_search_cnt == 0)
		show_search_results (self);
}

static void
gs_extras_page_load (GsExtrasPage *self, GPtrArray *array_search_data)
{
	GtkWidget *child;
	guint i;

	/* cancel any pending searches */
	g_cancellable_cancel (self->search_cancellable);
	g_clear_object (&self->search_cancellable);
	self->search_cancellable = g_cancellable_new ();

	gtk_widget_set_visible (self->button_install_all, FALSE);

	if (array_search_data != NULL) {
		if (self->array_search_data != NULL)
			g_ptr_array_unref (self->array_search_data);
		self->array_search_data = g_ptr_array_ref (array_search_data);
	}

	self->pending_search_cnt = 0;

	/* remove old entries */
	while ((child = gtk_widget_get_first_child (self->list_box_results)) != NULL) {
		if (GS_IS_APP_ROW (child)) {
			GsApp *app = gs_app_row_get_app (GS_APP_ROW (child));
			g_signal_handlers_disconnect_by_func (app, G_CALLBACK (gs_extras_page_app_notify_state_cb), self);
		}

		gtk_list_box_remove (GTK_LIST_BOX (self->list_box_results), child);
	}

	/* set state as loading */
	self->state = GS_EXTRAS_PAGE_STATE_LOADING;

	/* start new searches, separate one for each codec */
	for (i = 0; i < self->array_search_data->len; i++) {
		GsPluginRefineFlags refine_flags;
		SearchData *search_data;

		refine_flags = GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
		               GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING |
		               GS_PLUGIN_REFINE_FLAGS_ALLOW_PACKAGES;

		search_data = g_ptr_array_index (self->array_search_data, i);
		if (search_data->search_filename != NULL) {
			g_autoptr(GsPluginJob) plugin_job = NULL;
			g_autoptr(GsAppQuery) query = NULL;
			const gchar *provides_files[2] = { search_data->search_filename, NULL };

			query = gs_app_query_new ("provides-files", provides_files,
						  "refine-flags", refine_flags,
						  "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
						  "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
						  NULL);

			plugin_job = gs_plugin_job_list_apps_new (query,
								  GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

			g_debug ("searching filename: '%s'", search_data->search_filename);
			gs_plugin_loader_job_process_async (self->plugin_loader,
							    plugin_job,
							    self->search_cancellable,
							    search_files_cb,
							    search_data);
		} else if (search_data->package_filename != NULL) {
			g_autoptr (GFile) file = NULL;
			g_autoptr(GsPluginJob) plugin_job = NULL;
			file = g_file_new_for_path (search_data->package_filename);
			plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_FILE_TO_APP,
							 "file", file,
							 "refine-flags", refine_flags,
							 NULL);
			g_debug ("resolving filename to app: '%s'", search_data->package_filename);
			gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
							    self->search_cancellable,
							    file_to_app_cb,
							    search_data);
		} else {
			g_autoptr(GsPluginJob) plugin_job = NULL;
			g_autoptr(GsAppQuery) query = NULL;

			query = gs_app_query_new ("provides-tag", search_data->search,
						  "provides-type", search_data->search_provides_type,
						  "refine-flags", refine_flags,
						  "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
						  "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
						  NULL);

			plugin_job = gs_plugin_job_list_apps_new (query,
								  GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

			g_debug ("searching what provides: '%s'", search_data->search);
			gs_plugin_loader_job_process_async (self->plugin_loader,
							    plugin_job,
							    self->search_cancellable,
							    get_search_what_provides_cb,
							    search_data);
		}
		self->pending_search_cnt++;
	}

	/* the page title will have changed */
	g_object_notify (G_OBJECT (self), "title");
}

static void
gs_extras_page_reload (GsPage *page)
{
	GsExtrasPage *self = GS_EXTRAS_PAGE (page);
	if (self->array_search_data != NULL)
		gs_extras_page_load (self, NULL);
}

static void
gs_extras_page_search_package_files (GsExtrasPage *self, gchar **files)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; files[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup (files[i]);
		search_data->package_filename = g_strdup (files[i]);
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_DEFAULT);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_provide_files (GsExtrasPage *self, gchar **files)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; files[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup (files[i]);
		search_data->search_filename = g_strdup (files[i]);
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_DEFAULT);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_package_names (GsExtrasPage *self, gchar **package_names)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; package_names[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup (package_names[i]);
		search_data->search = g_strdup (package_names[i]);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_PACKAGE_NAME;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_DEFAULT);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_mime_types (GsExtrasPage *self, gchar **mime_types)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; mime_types[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup_printf (_("%s file format"), mime_types[i]);
		search_data->search = g_strdup (mime_types[i]);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_MIME_HANDLER;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_MIME);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static gchar *
font_tag_to_lang (const gchar *tag)
{
	if (g_str_has_prefix (tag, ":lang="))
		return g_strdup (tag + 6);

	return NULL;
}

static gchar *
gs_extras_page_font_tag_to_localised_name (GsExtrasPage *self, const gchar *tag)
{
	gchar *name;
	g_autofree gchar *lang = NULL;
	g_autofree gchar *language = NULL;

	/* use fontconfig to get the language code */
	lang = font_tag_to_lang (tag);
	if (lang == NULL) {
		g_warning ("Could not parse language tag '%s'", tag);
		return NULL;
	}

	/* convert to localisable name */
	language = gs_language_iso639_to_language (self->language, lang);
	if (language == NULL) {
		g_warning ("Could not match language code '%s' to an ISO639 language", lang);
		return NULL;
	}

	/* get translation, or return untranslated string */
	name = g_strdup (dgettext("iso_639", language));
	if (name == NULL)
		name = g_strdup (language);

	return name;
}

static void
gs_extras_page_search_fontconfig_resources (GsExtrasPage *self, gchar **resources)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; resources[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = gs_extras_page_font_tag_to_localised_name (self, resources[i]);
		search_data->search = g_strdup (resources[i]);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_FONT;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_FONT);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_gstreamer_resources (GsExtrasPage *self, gchar **resources)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; resources[i] != NULL; i++) {
		SearchData *search_data;
		g_auto(GStrv) parts = NULL;

		parts = g_strsplit (resources[i], "|", 2);

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup (parts[0]);
		search_data->search = g_strdup (parts[1]);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_GSTREAMER;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_CODEC);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_plasma_resources (GsExtrasPage *self, gchar **resources)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i;

	for (i = 0; resources[i] != NULL; i++) {
		SearchData *search_data;

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup (resources[i]);
		search_data->search = g_strdup (resources[i]);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_PLASMA;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_DEFAULT);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static void
gs_extras_page_search_printer_drivers (GsExtrasPage *self, gchar **device_ids)
{
	g_autoptr(GPtrArray) array_search_data = g_ptr_array_new_with_free_func ((GDestroyNotify) search_data_free);
	guint i, j;
	guint len;

	len = g_strv_length (device_ids);
	if (len > 1)
		/* hardcode for now as we only support one at a time */
		len = 1;

	/* make a list of provides tags */
	for (i = 0; i < len; i++) {
		SearchData *search_data;
		gchar *p;
		guint n_fields;
		g_autofree gchar *tag = NULL;
		g_autofree gchar *mfg = NULL;
		g_autofree gchar *mdl = NULL;
		g_auto(GStrv) fields = NULL;

		fields = g_strsplit (device_ids[i], ";", 0);
		n_fields = g_strv_length (fields);
		mfg = mdl = NULL;
		for (j = 0; j < n_fields && (!mfg || !mdl); j++) {
			if (g_str_has_prefix (fields[j], "MFG:"))
				mfg = g_strdup (fields[j] + 4);
			else if (g_str_has_prefix (fields[j], "MDL:"))
				mdl = g_strdup (fields[j] + 4);
		}

		if (!mfg || !mdl) {
			g_warning("invalid line '%s', missing field",
				    device_ids[i]);
			continue;
		}

		tag = g_strdup_printf ("%s;%s;", mfg, mdl);

		/* Replace spaces with underscores */
		for (p = tag; *p != '\0'; p++)
			if (*p == ' ')
				*p = '_';

		search_data = g_slice_new0 (SearchData);
		search_data->title = g_strdup_printf ("%s %s", mfg, mdl);
		search_data->search = g_ascii_strdown (tag, -1);
		search_data->search_provides_type = GS_APP_QUERY_PROVIDES_PS_DRIVER;
		search_data->url_not_found = gs_vendor_get_not_found_url (self->vendor, GS_VENDOR_URL_TYPE_HARDWARE);
		search_data->self = g_object_ref (self);
		g_ptr_array_add (array_search_data, search_data);
	}

	gs_extras_page_load (self, array_search_data);
}

static gchar *
gs_extras_page_get_app_name (const gchar *desktop_id)
{
	g_autoptr(GDesktopAppInfo) app_info = NULL;

	if (!desktop_id || !*desktop_id)
		return NULL;

	app_info = g_desktop_app_info_new (desktop_id);
	if (!app_info)
		return NULL;

	return g_strdup (g_app_info_get_display_name (G_APP_INFO (app_info)));
}

void
gs_extras_page_search (GsExtrasPage  *self,
                       const gchar   *mode_str,
                       gchar        **resources,
                       const gchar   *desktop_id,
                       const gchar   *ident)
{
	GsExtrasPageMode old_mode;

	old_mode = self->mode;
	self->mode = gs_extras_page_mode_from_string (mode_str);

	if (old_mode != self->mode)
		g_object_notify (G_OBJECT (self), "title");

	g_clear_pointer (&self->caller_app_name, g_free);
	self->caller_app_name = gs_extras_page_get_app_name (desktop_id);
	g_clear_pointer (&self->install_resources_ident, g_free);
	self->install_resources_ident = (ident && *ident) ? g_strdup (ident) : NULL;

	switch (self->mode) {
	case GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_FILES:
		gs_extras_page_search_package_files (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PROVIDE_FILES:
		gs_extras_page_search_provide_files (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_NAMES:
		gs_extras_page_search_package_names (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_MIME_TYPES:
		gs_extras_page_search_mime_types (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES:
		gs_extras_page_search_fontconfig_resources (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_GSTREAMER_RESOURCES:
		gs_extras_page_search_gstreamer_resources (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PLASMA_RESOURCES:
		gs_extras_page_search_plasma_resources (self, resources);
		break;
	case GS_EXTRAS_PAGE_MODE_INSTALL_PRINTER_DRIVERS:
		gs_extras_page_search_printer_drivers (self, resources);
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
gs_extras_page_switch_to (GsPage *page)
{
	GsExtrasPage *self = GS_EXTRAS_PAGE (page);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_EXTRAS) {
		g_warning ("Called switch_to(codecs) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}

	gs_extras_page_update_ui_state (self);
}

static void
row_activated_cb (GtkListBox *list_box,
                  GtkListBoxRow *row,
                  GsExtrasPage *self)
{
	GsApp *app;

	app = gs_app_row_get_app (GS_APP_ROW (row));

	if (gs_app_get_state (app) == GS_APP_STATE_UNAVAILABLE &&
	    gs_app_get_url_missing (app) != NULL) {
		gs_shell_show_uri (self->shell,
		                   gs_app_get_url_missing (app));
	} else {
		gs_shell_show_app (self->shell, app);
	}
}

static gchar *
get_app_sort_key (GsApp *app)
{
	GString *key = NULL;
	g_autofree gchar *sort_name = NULL;

	key = g_string_sized_new (64);

	/* sort missing apps as last */
	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_UNAVAILABLE:
		g_string_append (key, "9:");
		break;
	default:
		g_string_append (key, "1:");
		break;
	}

	/* finally, sort by short name */
	if (gs_app_get_name (app) != NULL) {
		sort_name = gs_utils_sort_key (gs_app_get_name (app));
		g_string_append (key, sort_name);
	}

	return g_string_free (key, FALSE);
}

static gint
list_sort_func (GtkListBoxRow *a,
                GtkListBoxRow *b,
                gpointer user_data)
{
	GsApp *a1 = gs_app_row_get_app (GS_APP_ROW (a));
	GsApp *a2 = gs_app_row_get_app (GS_APP_ROW (b));
	g_autofree gchar *key1 = get_app_sort_key (a1);
	g_autofree gchar *key2 = get_app_sort_key (a2);

	/* compare the keys according to the algorithm above */
	return g_strcmp0 (key1, key2);
}

static void
list_header_func (GtkListBoxRow *row,
                  GtkListBoxRow *before,
                  gpointer user_data)
{
	GtkWidget *header;

	/* first entry */
	header = gtk_list_box_row_get_header (row);
	if (before == NULL) {
		gtk_list_box_row_set_header (row, NULL);
		return;
	}

	/* already set */
	if (header != NULL)
		return;

	/* set new */
	header = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_list_box_row_set_header (row, header);
}

static gboolean
gs_extras_page_setup (GsPage *page,
                      GsShell *shell,
                      GsPluginLoader *plugin_loader,
                      GCancellable *cancellable,
                      GError **error)
{
	GsExtrasPage *self = GS_EXTRAS_PAGE (page);
	GtkWidget *box;

	g_return_val_if_fail (GS_IS_EXTRAS_PAGE (self), TRUE);

	self->shell = shell;

	self->plugin_loader = g_object_ref (plugin_loader);

	g_signal_connect (self->list_box_results, "row-activated",
			  G_CALLBACK (row_activated_cb), self);
	gtk_list_box_set_header_func (GTK_LIST_BOX (self->list_box_results),
				      list_header_func,
				      self, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_results),
				    list_sort_func,
				    self, NULL);

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	gs_page_set_header_end_widget (GS_PAGE (self), box);
	self->button_install_all = gtk_button_new_with_mnemonic (_("Install _All"));
	gtk_widget_set_visible (self->button_install_all, FALSE);
	gtk_box_prepend (GTK_BOX (box), self->button_install_all);
	g_signal_connect (self->button_install_all, "clicked",
			  G_CALLBACK (gs_extras_page_button_install_all_cb),
			  self);

	return TRUE;
}

static void
gs_extras_page_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	GsExtrasPage *self = GS_EXTRAS_PAGE (object);

	switch ((GsExtrasPageProperty) prop_id) {
	case PROP_VADJUSTMENT:
		g_value_set_object (value, gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow)));
		break;
	case PROP_TITLE:
		switch (self->state) {
		case GS_EXTRAS_PAGE_STATE_LOADING:
		case GS_EXTRAS_PAGE_STATE_READY:
			g_value_take_string (value, build_title (self));
			break;
		case GS_EXTRAS_PAGE_STATE_NO_RESULTS:
		case GS_EXTRAS_PAGE_STATE_FAILED:
			g_value_set_string (value, _("Unable to Find Requested Appstore"));
			break;
		default:
			g_assert_not_reached ();
			break;
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_extras_page_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	switch ((GsExtrasPageProperty) prop_id) {
	case PROP_VADJUSTMENT:
	case PROP_TITLE:
		/* Read-only */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_extras_page_dispose (GObject *object)
{
	GsExtrasPage *self = GS_EXTRAS_PAGE (object);

	g_cancellable_cancel (self->search_cancellable);
	g_clear_object (&self->search_cancellable);

	g_clear_object (&self->sizegroup_name);
	g_clear_object (&self->sizegroup_button_label);
	g_clear_object (&self->sizegroup_button_image);
	g_clear_object (&self->language);
	g_clear_object (&self->vendor);
	g_clear_object (&self->plugin_loader);

	g_clear_pointer (&self->array_search_data, g_ptr_array_unref);
	g_clear_pointer (&self->caller_app_name, g_free);
	g_clear_pointer (&self->install_resources_ident, g_free);

	G_OBJECT_CLASS (gs_extras_page_parent_class)->dispose (object);
}

static void
gs_extras_page_init (GsExtrasPage *self)
{
	g_autoptr(GError) error = NULL;

	gtk_widget_init_template (GTK_WIDGET (self));

	self->state = GS_EXTRAS_PAGE_STATE_LOADING;
	self->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_label = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_image = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->vendor = gs_vendor_new ();

	/* map ISO639 to language names */
	self->language = gs_language_new ();
	gs_language_populate (self->language, &error);
	if (error != NULL)
		g_error ("Failed to map ISO639 to language names: %s", error->message);
}

static void
gs_extras_page_class_init (GsExtrasPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_extras_page_get_property;
	object_class->set_property = gs_extras_page_set_property;
	object_class->dispose = gs_extras_page_dispose;

	page_class->switch_to = gs_extras_page_switch_to;
	page_class->reload = gs_extras_page_reload;
	page_class->setup = gs_extras_page_setup;

	g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
	g_object_class_override_property (object_class, PROP_TITLE, "title");

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-extras-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, label_failed);
	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, label_no_results);
	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, list_box_results);
	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, scrolledwindow);
	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, spinner);
	gtk_widget_class_bind_template_child (widget_class, GsExtrasPage, stack);
}

GsExtrasPage *
gs_extras_page_new (void)
{
	GsExtrasPage *self;
	self = g_object_new (GS_TYPE_EXTRAS_PAGE, NULL);
	return GS_EXTRAS_PAGE (self);
}
