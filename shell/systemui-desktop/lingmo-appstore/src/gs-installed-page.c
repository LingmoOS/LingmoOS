/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

#include "gs-shell.h"
#include "gs-installed-page.h"
#include "gs-common.h"
#include "gs-app-row.h"
#include "gs-utils.h"

struct _GsInstalledPage
{
	GsPage			 parent_instance;

	GsPluginLoader		*plugin_loader;
	GCancellable		*cancellable;
	GtkSizeGroup		*sizegroup_name;
	GtkSizeGroup		*sizegroup_button_label;
	GtkSizeGroup		*sizegroup_button_image;
	gboolean		 cache_valid;
	gboolean		 waiting;
	GsShell			*shell;
	GSettings		*settings;
	guint			 pending_apps_counter;
	gboolean		 is_narrow;

	GtkWidget		*group_install_in_progress;
	GtkWidget		*group_install_apps;
	GtkWidget		*group_install_system_apps;
	GtkWidget		*group_install_addons;
	GtkWidget		*group_install_web_apps;

	GtkWidget		*list_box_install_in_progress;
	GtkWidget		*list_box_install_apps;
	GtkWidget		*list_box_install_system_apps;
	GtkWidget		*list_box_install_addons;
	GtkWidget		*list_box_install_web_apps;
	GtkWidget		*scrolledwindow_install;
	GtkWidget		*spinner_install;
	GtkWidget		*stack_install;
};

G_DEFINE_TYPE (GsInstalledPage, gs_installed_page, GS_TYPE_PAGE)

typedef enum {
	PROP_IS_NARROW = 1,
	/* Overrides: */
	PROP_VADJUSTMENT,
	PROP_TITLE,
} GsInstalledPageProperty;

static GParamSpec *obj_props[PROP_IS_NARROW + 1] = { NULL, };

static void gs_installed_page_pending_apps_refined_cb (GObject *source,
						       GAsyncResult *res,
						       gpointer user_data);
static GsPluginRefineFlags gs_installed_page_get_refine_flags (GsInstalledPage *self);
static void gs_installed_page_notify_state_changed_cb (GsApp *app,
						       GParamSpec *pspec,
						       GsInstalledPage *self);

typedef enum {
	GS_UPDATE_LIST_SECTION_INSTALLING_AND_REMOVING,
	GS_UPDATE_LIST_SECTION_REMOVABLE_APPS,
	GS_UPDATE_LIST_SECTION_SYSTEM_APPS,
	GS_UPDATE_LIST_SECTION_ADDONS,
	GS_UPDATE_LIST_SECTION_WEB_APPS,
	GS_UPDATE_LIST_SECTION_LAST
} GsInstalledPageSection;

/* This must mostly mirror gs_installed_page_get_app_sort_key() otherwise apps
 * will end up sorted into a section they don’t belong in. */
static GsInstalledPageSection
gs_installed_page_get_app_section (GsApp *app)
{
	GsAppState state = gs_app_get_state (app);
	AsComponentKind kind = gs_app_get_kind (app);

	if (state == GS_APP_STATE_INSTALLING ||
	    state == GS_APP_STATE_QUEUED_FOR_INSTALL ||
	    state == GS_APP_STATE_REMOVING ||
	    state == GS_APP_STATE_DOWNLOADING)
		return GS_UPDATE_LIST_SECTION_INSTALLING_AND_REMOVING;

	if (kind == AS_COMPONENT_KIND_DESKTOP_APP) {
		if (gs_app_has_quirk (app, GS_APP_QUIRK_COMPULSORY))
			return GS_UPDATE_LIST_SECTION_SYSTEM_APPS;
		return GS_UPDATE_LIST_SECTION_REMOVABLE_APPS;
	}

	if (kind == AS_COMPONENT_KIND_WEB_APP)
		return GS_UPDATE_LIST_SECTION_WEB_APPS;

	return GS_UPDATE_LIST_SECTION_ADDONS;
}

static void
update_groups (GsInstalledPage *self)
{
	gtk_widget_set_visible (self->group_install_in_progress,
				gtk_widget_get_first_child (self->list_box_install_in_progress) != NULL);
	gtk_widget_set_visible (self->group_install_apps,
				gtk_widget_get_first_child (self->list_box_install_apps) != NULL);
	gtk_widget_set_visible (self->group_install_system_apps,
				gtk_widget_get_first_child (self->list_box_install_system_apps) != NULL);
	gtk_widget_set_visible (self->group_install_addons,
				gtk_widget_get_first_child (self->list_box_install_addons) != NULL);
	gtk_widget_set_visible (self->group_install_web_apps,
				gtk_widget_get_first_child (self->list_box_install_web_apps) != NULL);
}

static GsInstalledPageSection
gs_installed_page_get_row_section (GsInstalledPage *self,
				   GsAppRow *app_row)
{
	GtkWidget *parent;

	g_return_val_if_fail (GS_IS_INSTALLED_PAGE (self), GS_UPDATE_LIST_SECTION_LAST);
	g_return_val_if_fail (GS_IS_APP_ROW (app_row), GS_UPDATE_LIST_SECTION_LAST);

	parent = gtk_widget_get_parent (GTK_WIDGET (app_row));
	if (parent == self->list_box_install_in_progress)
		return GS_UPDATE_LIST_SECTION_INSTALLING_AND_REMOVING;
	if (parent == self->list_box_install_apps)
		return GS_UPDATE_LIST_SECTION_REMOVABLE_APPS;
	if (parent == self->list_box_install_system_apps)
		return GS_UPDATE_LIST_SECTION_SYSTEM_APPS;
	if (parent == self->list_box_install_addons)
		return GS_UPDATE_LIST_SECTION_ADDONS;
	if (parent == self->list_box_install_web_apps)
		return GS_UPDATE_LIST_SECTION_WEB_APPS;

	g_warn_if_reached ();

	return GS_UPDATE_LIST_SECTION_LAST;
}

static void
gs_installed_page_invalidate (GsInstalledPage *self)
{
	self->cache_valid = FALSE;
}

static void
gs_installed_page_app_row_activated_cb (GtkListBox *list_box,
                                        GtkListBoxRow *row,
                                        GsInstalledPage *self)
{
	GsApp *app;
	app = gs_app_row_get_app (GS_APP_ROW (row));
	gs_shell_show_app (self->shell, app);
}

static void
row_unrevealed (GObject *row, GParamSpec *pspec, gpointer data)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (gtk_widget_get_ancestor (GTK_WIDGET (row),
									    GS_TYPE_INSTALLED_PAGE));
	GtkWidget *list;

	list = gtk_widget_get_parent (GTK_WIDGET (row));
	if (list == NULL)
		return;
	gtk_list_box_remove (GTK_LIST_BOX (list), GTK_WIDGET (row));
	update_groups (self);
}

static void
gs_installed_page_unreveal_row (GsAppRow *app_row)
{
	GsApp *app = gs_app_row_get_app (app_row);
	if (app != NULL) {
		g_signal_handlers_disconnect_matched (app, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
						      G_CALLBACK (gs_installed_page_notify_state_changed_cb), NULL);
	}

	g_signal_connect (app_row, "unrevealed",
			  G_CALLBACK (row_unrevealed), NULL);
	gs_app_row_unreveal (app_row);
}

static GsAppRow *  /* (transfer none) */
gs_installed_page_find_app_row (GsInstalledPage *self,
				GsApp *app)
{
	GtkWidget *lists[] = {
		self->list_box_install_in_progress,
		self->list_box_install_apps,
		self->list_box_install_system_apps,
		self->list_box_install_addons,
		self->list_box_install_web_apps,
		NULL
	};

	for (gsize i = 0; lists[i]; i++) {
		for (GtkWidget *child = gtk_widget_get_first_child (lists[i]);
		     child != NULL;
		     child = gtk_widget_get_next_sibling (child)) {
			GsAppRow *app_row = GS_APP_ROW (child);
			if (gs_app_row_get_app (app_row) == app) {
				return app_row;
			}
		}
	}

	return NULL;
}


static void
gs_installed_page_app_removed (GsPage *page, GsApp *app)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (page);
	GsAppRow *app_row = gs_installed_page_find_app_row (self, app);
	if (app_row != NULL)
		gs_installed_page_unreveal_row (app_row);
}

static void
gs_installed_page_app_remove_cb (GsAppRow *app_row,
                                 GsInstalledPage *self)
{
	GsApp *app;

	app = gs_app_row_get_app (app_row);
	gs_page_remove_app (GS_PAGE (self), app, self->cancellable);
}

static void
gs_installed_page_maybe_move_app_row (GsInstalledPage *self,
				      GsAppRow *app_row)
{
	GsInstalledPageSection current_section, expected_section;

	current_section = gs_installed_page_get_row_section (self, app_row);
	g_return_if_fail (current_section != GS_UPDATE_LIST_SECTION_LAST);

	expected_section = gs_installed_page_get_app_section (gs_app_row_get_app (app_row));
	if (expected_section != current_section) {
		GtkWidget *widget = GTK_WIDGET (app_row);

		g_object_ref (app_row);
		gtk_list_box_remove (GTK_LIST_BOX (gtk_widget_get_parent (widget)), widget);
		switch (expected_section) {
		case GS_UPDATE_LIST_SECTION_INSTALLING_AND_REMOVING:
			widget = self->list_box_install_in_progress;
			break;
		case GS_UPDATE_LIST_SECTION_REMOVABLE_APPS:
			widget = self->list_box_install_apps;
			break;
		case GS_UPDATE_LIST_SECTION_SYSTEM_APPS:
			widget = self->list_box_install_system_apps;
			break;
		case GS_UPDATE_LIST_SECTION_ADDONS:
			widget = self->list_box_install_addons;
			break;
		case GS_UPDATE_LIST_SECTION_WEB_APPS:
			widget = self->list_box_install_web_apps;
			break;
		default:
			g_warn_if_reached ();
			widget = NULL;
			break;
		}

		if (widget != NULL)
			gtk_list_box_append (GTK_LIST_BOX (widget), GTK_WIDGET (app_row));

		g_object_unref (app_row);
		update_groups (self);
	}
}

static void
gs_installed_page_notify_state_changed_cb (GsApp *app,
                                           GParamSpec *pspec,
                                           GsInstalledPage *self)
{
	GsAppState state = gs_app_get_state (app);
	GsAppRow *app_row = gs_installed_page_find_app_row (self, app);

	g_assert (app_row != NULL);

	gtk_list_box_row_changed (GTK_LIST_BOX_ROW (app_row));

	/* Filter which apps can be shown in the installed page */
	if (state != GS_APP_STATE_INSTALLING &&
	    state != GS_APP_STATE_INSTALLED &&
	    state != GS_APP_STATE_REMOVING &&
	    state != GS_APP_STATE_DOWNLOADING &&
	    state != GS_APP_STATE_UPDATABLE &&
	    state != GS_APP_STATE_UPDATABLE_LIVE)
		gs_installed_page_unreveal_row (app_row);
	else
		gs_installed_page_maybe_move_app_row (self, app_row);
}

static gboolean
should_show_installed_size (GsInstalledPage *self)
{
	return g_settings_get_boolean (self->settings,
				       "installed-page-show-size");
}

static gboolean
gs_installed_page_is_actual_app (GsApp *app)
{
	if (gs_app_get_description (app) != NULL)
		return TRUE;

	/* special snowflake */
	if (g_strcmp0 (gs_app_get_id (app), "google-chrome.desktop") == 0)
		return TRUE;

	/* web apps sometimes don't have descriptions */
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_WEB_APP)
		return TRUE;

	g_debug ("%s is not an actual app", gs_app_get_unique_id (app));
	return FALSE;
}

static void
gs_installed_page_add_app (GsInstalledPage *self, GsAppList *list, GsApp *app)
{
	GtkWidget *app_row;

	/* only show if is an actual app */
	if (!gs_installed_page_is_actual_app (app))
		return;

	app_row = g_object_new (GS_TYPE_APP_ROW,
				"app", app,
				"show-buttons", TRUE,
				"show-source", gs_utils_list_has_component_fuzzy (list, app),
				"show-installed-size", !gs_app_has_quirk (app, GS_APP_QUIRK_COMPULSORY) && should_show_installed_size (self),
				NULL);

	g_signal_connect (app_row, "button-clicked",
			  G_CALLBACK (gs_installed_page_app_remove_cb), self);
	g_signal_connect_object (app, "notify::state",
				 G_CALLBACK (gs_installed_page_notify_state_changed_cb),
				 self, 0);

	switch (gs_installed_page_get_app_section (app)) {
	case GS_UPDATE_LIST_SECTION_INSTALLING_AND_REMOVING:
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install_in_progress), app_row);
		break;
	case GS_UPDATE_LIST_SECTION_REMOVABLE_APPS:
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install_apps), app_row);
		break;
	case GS_UPDATE_LIST_SECTION_SYSTEM_APPS:
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install_system_apps), app_row);
		break;
	case GS_UPDATE_LIST_SECTION_ADDONS:
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install_addons), app_row);
		break;
	case GS_UPDATE_LIST_SECTION_WEB_APPS:
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_install_web_apps), app_row);
		break;
	default:
		g_assert_not_reached ();
	}

	update_groups (self);

	gs_app_row_set_size_groups (GS_APP_ROW (app_row),
				    self->sizegroup_name,
				    self->sizegroup_button_label,
				    self->sizegroup_button_image);

	gs_app_row_set_show_description (GS_APP_ROW (app_row), FALSE);
	gs_app_row_set_show_source (GS_APP_ROW (app_row), FALSE);
	g_object_bind_property (self, "is-narrow", app_row, "is-narrow", G_BINDING_SYNC_CREATE);
}

static void
gs_installed_page_get_installed_cb (GObject *source_object,
                                    GAsyncResult *res,
                                    gpointer user_data)
{
	guint i;
	GsApp *app;
	GsInstalledPage *self = GS_INSTALLED_PAGE (user_data);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsAppList) pending = gs_plugin_loader_get_pending (plugin_loader);
	g_autoptr(GsPluginJob) plugin_job = NULL;

	gtk_spinner_stop (GTK_SPINNER (self->spinner_install));
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_install), "view");

	self->waiting = FALSE;
	self->cache_valid = TRUE;

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &error);
	if (list == NULL) {
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get installed apps: %s", error->message);
		goto out;
	}
	for (i = 0; i < gs_app_list_length (list); i++) {
		app = gs_app_list_index (list, i);
		gs_installed_page_add_app (self, list, app);
	}
out:
	if (gs_app_list_length (pending) > 0) {
		plugin_job = gs_plugin_job_refine_new (pending,
						       gs_installed_page_get_refine_flags (self));
		gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
						    self->cancellable,
						    gs_installed_page_pending_apps_refined_cb,
						    self);

	}
}

static void
gs_installed_page_remove_all_cb (GtkWidget *container,
				 GtkWidget *child)
{
	if (GS_IS_APP_ROW (child)) {
		GsApp *app = gs_app_row_get_app (GS_APP_ROW (child));
		if (app != NULL) {
			g_signal_handlers_disconnect_matched (app, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
							      G_CALLBACK (gs_installed_page_notify_state_changed_cb), NULL);
		}
	} else {
		g_warn_if_reached ();
	}

	gtk_list_box_remove (GTK_LIST_BOX (container), child);
}

static gboolean
filter_app_kinds_cb (GsApp    *app,
                     gpointer  user_data)
{
	/* Remove invalid apps. */
	if (!gs_plugin_loader_app_is_valid (app, GS_PLUGIN_REFINE_FLAGS_NONE))
		return FALSE;

	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_OPERATING_SYSTEM:
	case AS_COMPONENT_KIND_CODEC:
	case AS_COMPONENT_KIND_FONT:
		g_debug ("app invalid as %s: %s",
			 as_component_kind_to_string (gs_app_get_kind (app)),
			 gs_app_get_unique_id (app));
		return FALSE;
	default:
		return TRUE;
	}
}

static GsPluginRefineFlags
gs_installed_page_get_refine_flags (GsInstalledPage *self)
{
	GsPluginRefineFlags flags;

	flags = GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES |
		GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING;

	if (should_show_installed_size (self))
		flags |= GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE;

	return flags;
}

static void
gs_installed_page_load (GsInstalledPage *self)
{
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	if (self->waiting)
		return;
	self->waiting = TRUE;

	/* remove old entries */
	gs_widget_remove_all (self->list_box_install_in_progress, gs_installed_page_remove_all_cb);
	gs_widget_remove_all (self->list_box_install_apps, gs_installed_page_remove_all_cb);
	gs_widget_remove_all (self->list_box_install_system_apps, gs_installed_page_remove_all_cb);
	gs_widget_remove_all (self->list_box_install_addons, gs_installed_page_remove_all_cb);
	gs_widget_remove_all (self->list_box_install_web_apps, gs_installed_page_remove_all_cb);
	update_groups (self);

	/* get installed apps */
	query = gs_app_query_new ("is-installed", GS_APP_QUERY_TRISTATE_TRUE,
				  "refine-flags", gs_installed_page_get_refine_flags (self),
				  "filter-func", filter_app_kinds_cb,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);
	gs_plugin_loader_job_process_async (self->plugin_loader,
					    plugin_job,
					    self->cancellable,
					    gs_installed_page_get_installed_cb,
					    self);
	gtk_spinner_start (GTK_SPINNER (self->spinner_install));
	gtk_stack_set_visible_child_name (GTK_STACK (self->stack_install), "spinner");
}

static void
gs_installed_page_reload (GsPage *page)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (page);
	gs_installed_page_invalidate (self);
	gs_installed_page_load (self);
}

static void
gs_installed_page_switch_to (GsPage *page)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (page);

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_INSTALLED) {
		g_warning ("Called switch_to(installed) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}

	if (gs_shell_get_mode (self->shell) == GS_SHELL_MODE_INSTALLED) {
		gs_grab_focus_when_mapped (self->scrolledwindow_install);
	}

	/* no need to refresh */
	if (self->cache_valid)
		return;

	gs_installed_page_load (self);
}

/**
 * gs_installed_page_get_app_sort_key:
 *
 * Get a sort key to achive this:
 *
 * 1. state:installing apps
 * 2. state: apps queued for installing
 * 3. state:removing apps
 * 4. kind:normal apps
 * 5. kind:system apps
 *
 * Within each of these groups, they are sorted by the install date and then
 * by name.
 **/
static gchar *
gs_installed_page_get_app_sort_key (GsApp *app)
{
	GString *key;
	g_autofree gchar *sort_name = NULL;

	key = g_string_sized_new (64);

	/* sort installed, removing, other */
	switch (gs_app_get_state (app)) {
	case GS_APP_STATE_DOWNLOADING:
		g_string_append (key, "1:");
		break;
	case GS_APP_STATE_INSTALLING:
		g_string_append (key, "2:");
		break;
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		g_string_append (key, "3:");
		break;
	case GS_APP_STATE_REMOVING:
		g_string_append (key, "4:");
		break;
	default:
		g_string_append (key, "5:");
		break;
	}

	/* sort apps by kind */
	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		g_string_append (key, "2:");
		break;
	case AS_COMPONENT_KIND_WEB_APP:
		g_string_append (key, "3:");
		break;
	case AS_COMPONENT_KIND_RUNTIME:
		g_string_append (key, "4:");
		break;
	case AS_COMPONENT_KIND_ADDON:
		g_string_append (key, "5:");
		break;
	case AS_COMPONENT_KIND_CODEC:
		g_string_append (key, "6:");
		break;
	case AS_COMPONENT_KIND_FONT:
		g_string_append (key, "6:");
		break;
	case AS_COMPONENT_KIND_INPUT_METHOD:
		g_string_append (key, "7:");
		break;
	default:
		if (gs_app_get_special_kind (app) == GS_APP_SPECIAL_KIND_OS_UPDATE)
			g_string_append (key, "1:");
		else
			g_string_append (key, "8:");
		break;
	}

	/* sort normal, compulsory */
	if (!gs_app_has_quirk (app, GS_APP_QUIRK_COMPULSORY))
		g_string_append (key, "1:");
	else
		g_string_append (key, "2:");

	/* finally, sort by short name */
	if (gs_app_get_name (app) != NULL) {
		sort_name = gs_utils_sort_key (gs_app_get_name (app));
		g_string_append (key, sort_name);
	}

	return g_string_free (key, FALSE);
}

static gint
gs_installed_page_sort_func (GtkListBoxRow *a,
                             GtkListBoxRow *b,
                             gpointer user_data)
{
	GsApp *a1, *a2;
	g_autofree gchar *key1 = NULL;
	g_autofree gchar *key2 = NULL;

	a1 = gs_app_row_get_app (GS_APP_ROW (a));
	a2 = gs_app_row_get_app (GS_APP_ROW (b));
	key1 = gs_installed_page_get_app_sort_key (a1);
	key2 = gs_installed_page_get_app_sort_key (a2);

	/* compare the keys according to the algorithm above */
	return g_strcmp0 (key1, key2);
}

static gboolean
gs_installed_page_has_app (GsInstalledPage *self,
                           GsApp *app)
{
	GtkWidget *lists[] = {
		self->list_box_install_in_progress,
		self->list_box_install_apps,
		self->list_box_install_system_apps,
		self->list_box_install_addons,
		self->list_box_install_web_apps,
		NULL
	};

	for (gsize i = 0; lists[i]; i++) {
		for (GtkWidget *child = gtk_widget_get_first_child (lists[i]);
		     child != NULL;
		     child = gtk_widget_get_next_sibling (child)) {
			GsAppRow *app_row = GS_APP_ROW (child);
			if (gs_app_row_get_app (app_row) == app)
				return TRUE;
		}
	}

	return FALSE;
}

static void
gs_installed_page_add_pending_apps (GsInstalledPage *self,
				    GsAppList *list,
				    gboolean should_install)
{
	guint pending_apps_count = 0;

	for (guint i = 0; i < gs_app_list_length (list); ++i) {
		GsApp *app = gs_app_list_index (list, i);
		if (gs_app_is_installed (app)) {
			continue;
		}

		/* never show OS upgrades, we handle the scheduling and
		 * cancellation in GsUpgradeBanner */
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_OPERATING_SYSTEM)
			continue;

		if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE)
			gs_app_set_state (app, GS_APP_STATE_QUEUED_FOR_INSTALL);

		if (should_install &&
		    gs_app_get_state (app) == GS_APP_STATE_QUEUED_FOR_INSTALL &&
		    gs_plugin_loader_get_network_available (self->plugin_loader) &&
		    !gs_plugin_loader_get_network_metered (self->plugin_loader))
			gs_page_install_app (GS_PAGE (self), app,
					     GS_SHELL_INTERACTION_FULL,
					     gs_app_get_cancellable (app));

		++pending_apps_count;
		if (!gs_installed_page_has_app (self, app))
			gs_installed_page_add_app (self, list, app);
	}

	/* update the number of on-going operations */
	if (pending_apps_count != self->pending_apps_counter) {
		self->pending_apps_counter = pending_apps_count;
		g_object_notify (G_OBJECT (self), "counter");
	}
}

static void
gs_installed_page_pending_apps_refined_cb (GObject *source,
					   GAsyncResult *res,
					   gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	GsInstalledPage *self = GS_INSTALLED_PAGE (user_data);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = gs_plugin_loader_job_process_finish (plugin_loader,
									 res,
									 &error);
	if (list == NULL) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED))
			g_warning ("failed to refine pending apps: %s", error->message);
		return;
	}

	/* we add the pending apps and install them because this is called after we
	 * populate the page, and there may be pending apps coming from the saved list
	 * (i.e. after loading the saved pending apps from the disk) */
	gs_installed_page_add_pending_apps (self, list, TRUE);
}

static void
gs_installed_page_pending_apps_changed_cb (GsPluginLoader *plugin_loader,
                                           GsInstalledPage *self)
{
	g_autoptr(GsAppList) pending = gs_plugin_loader_get_pending (plugin_loader);
	/* we don't call install every time the pending apps list changes because
	 * it may be queued in the plugin loader */
	gs_installed_page_add_pending_apps (self, pending, FALSE);
}

static gboolean
gs_installed_page_setup (GsPage *page,
                         GsShell *shell,
                         GsPluginLoader *plugin_loader,
                         GCancellable *cancellable,
                         GError **error)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (page);

	g_return_val_if_fail (GS_IS_INSTALLED_PAGE (self), TRUE);

	self->shell = shell;
	self->plugin_loader = g_object_ref (plugin_loader);
	g_signal_connect (self->plugin_loader, "pending-apps-changed",
			  G_CALLBACK (gs_installed_page_pending_apps_changed_cb),
			  self);

	self->cancellable = g_object_ref (cancellable);

	/* setup installed */
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_install_in_progress),
				    gs_installed_page_sort_func,
				    self, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_install_apps),
				    gs_installed_page_sort_func,
				    self, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_install_system_apps),
				    gs_installed_page_sort_func,
				    self, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_install_addons),
				    gs_installed_page_sort_func,
				    self, NULL);
	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_install_web_apps),
				    gs_installed_page_sort_func,
				    self, NULL);
	return TRUE;
}

static void
gs_installed_page_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (object);

	switch ((GsInstalledPageProperty) prop_id) {
	case PROP_IS_NARROW:
		g_value_set_boolean (value, gs_installed_page_get_is_narrow (self));
		break;
	case PROP_VADJUSTMENT:
		g_value_set_object (value, gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_install)));
		break;
	case PROP_TITLE:
		/* Translators: This is in the context of a list of apps which are installed on the system. */
		g_value_set_string (value, C_("List of installed apps", "Installed"));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_installed_page_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (object);

	switch ((GsInstalledPageProperty) prop_id) {
	case PROP_IS_NARROW:
		gs_installed_page_set_is_narrow (self, g_value_get_boolean (value));
		break;
	case PROP_VADJUSTMENT:
	case PROP_TITLE:
		/* Read only. */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_installed_page_dispose (GObject *object)
{
	GsInstalledPage *self = GS_INSTALLED_PAGE (object);

	g_clear_object (&self->sizegroup_name);
	g_clear_object (&self->sizegroup_button_label);
	g_clear_object (&self->sizegroup_button_image);

	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->settings);

	G_OBJECT_CLASS (gs_installed_page_parent_class)->dispose (object);
}

static void
gs_installed_page_class_init (GsInstalledPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_installed_page_get_property;
	object_class->set_property = gs_installed_page_set_property;
	object_class->dispose = gs_installed_page_dispose;

	page_class->app_removed = gs_installed_page_app_removed;
	page_class->switch_to = gs_installed_page_switch_to;
	page_class->reload = gs_installed_page_reload;
	page_class->setup = gs_installed_page_setup;

	/**
	 * GsInstalledPage:is-narrow:
	 *
	 * Whether the page is in narrow mode.
	 *
	 * In narrow mode, the page will take up less horizontal space, doing so
	 * by e.g. using icons rather than labels in buttons. This is needed to
	 * keep the UI useable on small form-factors like smartphones.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IS_NARROW] =
		g_param_spec_boolean ("is-narrow", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
	g_object_class_override_property (object_class, PROP_TITLE, "title");

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-installed-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, group_install_in_progress);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, group_install_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, group_install_system_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, group_install_addons);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, group_install_web_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, list_box_install_in_progress);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, list_box_install_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, list_box_install_system_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, list_box_install_addons);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, list_box_install_web_apps);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, scrolledwindow_install);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, spinner_install);
	gtk_widget_class_bind_template_child (widget_class, GsInstalledPage, stack_install);

	gtk_widget_class_bind_template_callback (widget_class, gs_installed_page_app_row_activated_cb);
}

static void
gs_installed_page_init (GsInstalledPage *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));

	self->sizegroup_name = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_label = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	self->sizegroup_button_image = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	self->settings = g_settings_new ("org.lingmo.appstore");
}

/**
 * gs_installed_page_get_is_narrow:
 * @self: a #GsInstalledPage
 *
 * Get the value of #GsInstalledPage:is-narrow.
 *
 * Returns: %TRUE if the page is in narrow mode, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_installed_page_get_is_narrow (GsInstalledPage *self)
{
	g_return_val_if_fail (GS_IS_INSTALLED_PAGE (self), FALSE);

	return self->is_narrow;
}

/**
 * gs_installed_page_set_is_narrow:
 * @self: a #GsInstalledPage
 * @is_narrow: %TRUE to set the page in narrow mode, %FALSE otherwise
 *
 * Set the value of #GsInstalledPage:is-narrow.
 *
 * Since: 41
 */
void
gs_installed_page_set_is_narrow (GsInstalledPage *self, gboolean is_narrow)
{
	g_return_if_fail (GS_IS_INSTALLED_PAGE (self));

	is_narrow = !!is_narrow;

	if (self->is_narrow == is_narrow)
		return;

	self->is_narrow = is_narrow;
	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_IS_NARROW]);
}

GsInstalledPage *
gs_installed_page_new (void)
{
	GsInstalledPage *self;
	self = g_object_new (GS_TYPE_INSTALLED_PAGE, NULL);
	return GS_INSTALLED_PAGE (self);
}
