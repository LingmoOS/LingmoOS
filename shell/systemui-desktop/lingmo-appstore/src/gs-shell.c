/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2020 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <adwaita.h>
#include <malloc.h>
#include <string.h>
#include <glib/gi18n.h>

#ifdef HAVE_MOGWAI
#include <libmogwai-schedule-client/scheduler.h>
#endif

#include "gs-common.h"
#include "gs-shell.h"
#include "gs-basic-auth-dialog.h"
#include "gs-details-page.h"
#include "gs-installed-page.h"
#include "gs-moderate-page.h"
#include "gs-loading-page.h"
#include "gs-search-page.h"
#include "gs-overview-page.h"
#include "gs-updates-page.h"
#include "gs-updates-paused-banner.h"
#include "gs-category-page.h"
#include "gs-extras-page.h"
#include "gs-repos-dialog.h"
#include "gs-prefs-dialog.h"
#include "gs-toast.h"
#include "gs-update-dialog.h"
#include "gs-update-monitor.h"
#include "gs-utils.h"

#define NARROW_WIDTH_THRESHOLD 800

static const gchar *page_name[] = {
	"unknown",
	"overview",
	"installed",
	"search",
	"updates",
	"details",
	"category",
	"extras",
	"moderate",
	"loading",
};

typedef struct {
	GsShellMode	 mode;
	GtkWidget	*focus;
	GsCategory	*category;
	gchar		*search;
	GsApp		*app;
	gdouble		 vscroll_position;
} BackEntry;

struct _GsShell
{
	AdwApplicationWindow	 parent_object;

	GSettings		*settings;
	GCancellable		*cancellable;
	GsPluginLoader		*plugin_loader;
	GtkWidget		*header_start_widget;
	GtkWidget		*header_end_widget;
	GtkWidget		*sub_header_end_widget;
	GQueue			*back_entry_stack;
	GPtrArray		*modal_dialogs;
	AdwLeaflet		*main_leaflet;
	AdwLeaflet		*details_leaflet;
	AdwViewStack		*stack_loading;
	AdwViewStack		*stack_main;
	AdwViewStack		*stack_sub;
	GsPage			*page;

	GBinding		*sub_page_header_title_binding;

#ifdef HAVE_MOGWAI
	MwscScheduler		*scheduler;
	gboolean		 scheduler_held;
	gulong			 scheduler_invalidated_handler;
#endif  /* HAVE_MOGWAI */

	GtkWidget		*main_header;
	GtkWidget		*details_header;
	GtkWidget		*updates_paused_banner;
	GtkWidget		*search_button;
	GtkWidget		*entry_search;
	GtkWidget		*search_bar;
	GtkWidget		*button_back;
	GtkWidget		*button_back2;
	GtkWidget		*toast_overlay;
	GtkWidget		*primary_menu;
	GtkWidget		*sub_header;
	GtkWidget		*sub_page_header_title;

	gboolean		 activate_after_setup;
	gboolean		 is_narrow;
	gint			 allocation_width;
	guint			 allocation_changed_cb_id;

	GsPage			*pages[GS_SHELL_MODE_LAST];
};

G_DEFINE_TYPE (GsShell, gs_shell, ADW_TYPE_APPLICATION_WINDOW)

typedef enum {
	PROP_IS_NARROW = 1,
	PROP_ALLOCATION_WIDTH,
} GsShellProperty;

enum {
	SIGNAL_LOADED,
	SIGNAL_LAST
};

static GParamSpec *obj_props[PROP_ALLOCATION_WIDTH + 1] = { NULL, };

static guint signals [SIGNAL_LAST] = { 0 };

static void
modal_dialog_unmapped_cb (GtkWidget *dialog,
                          GsShell *shell)
{
	g_debug ("modal dialog %p unmapped", dialog);
	g_ptr_array_remove (shell->modal_dialogs, dialog);
}

void
gs_shell_modal_dialog_present (GsShell *shell, GtkWindow *window)
{
	GtkWindow *parent;

	/* show new modal on top of old modal */
	if (shell->modal_dialogs->len > 0) {
		parent = g_ptr_array_index (shell->modal_dialogs,
					    shell->modal_dialogs->len - 1);
		g_debug ("using old modal %p as parent", parent);
	} else {
		parent = GTK_WINDOW (shell);
		g_debug ("using main window");
	}
	gtk_window_set_transient_for (window, parent);

	/* add to stack, transfer ownership to here */
	g_ptr_array_add (shell->modal_dialogs, window);
	g_signal_connect (GTK_WIDGET (window), "unmap",
	                  G_CALLBACK (modal_dialog_unmapped_cb), shell);

	/* present the new one */
	gtk_window_set_modal (window, TRUE);
	gtk_window_present (window);
}

void
gs_shell_activate (GsShell *shell)
{
	/* Waiting for plugin loader to setup first */
	if (shell->plugin_loader == NULL) {
		shell->activate_after_setup = TRUE;
		return;
	}

	gtk_widget_set_visible (GTK_WIDGET (shell), TRUE);
	gtk_window_present (GTK_WINDOW (shell));
}

static void
gs_shell_set_header_start_widget (GsShell *shell, GtkWidget *widget)
{
	GtkWidget *old_widget;

	old_widget = shell->header_start_widget;

	if (shell->header_start_widget == widget)
		return;

	if (widget != NULL) {
		g_object_ref (widget);
		adw_header_bar_pack_start (ADW_HEADER_BAR (shell->main_header), widget);
	}

	shell->header_start_widget = widget;

	if (old_widget != NULL) {
		adw_header_bar_remove (ADW_HEADER_BAR (shell->main_header), old_widget);
		g_object_unref (old_widget);
	}
}

static void
gs_shell_set_header_end_widget (GsShell *shell, GtkWidget *widget)
{
	GtkWidget *old_widget;

	old_widget = shell->header_end_widget;

	if (shell->header_end_widget == widget)
		return;

	if (widget != NULL) {
		g_object_ref (widget);
		adw_header_bar_pack_end (ADW_HEADER_BAR (shell->main_header), widget);
	}

	shell->header_end_widget = widget;

	if (old_widget != NULL) {
		adw_header_bar_remove (ADW_HEADER_BAR (shell->main_header), old_widget);
		g_object_unref (old_widget);
	}
}

static void
gs_shell_set_sub_header_end_widget (GsShell *shell, GtkWidget *widget)
{
	GtkWidget *old_widget;

	old_widget = shell->sub_header_end_widget;

	if (shell->sub_header_end_widget == widget)
		return;

	if (widget != NULL) {
		g_object_ref (widget);
		adw_header_bar_pack_end (ADW_HEADER_BAR (shell->sub_header), widget);
	}

	shell->sub_header_end_widget = widget;

	if (old_widget != NULL) {
		adw_header_bar_remove (ADW_HEADER_BAR (shell->sub_header), old_widget);
		g_object_unref (old_widget);
	}
}

static void
gs_shell_refresh_auto_updates_ui (GsShell *shell)
{
	gboolean automatic_updates_enabled;
	GsUpdatesPausedBannerFlags flags = GS_UPDATES_PAUSED_BANNER_FLAGS_NONE;

	automatic_updates_enabled = g_settings_get_boolean (shell->settings, "download-updates");
	if (!automatic_updates_enabled || gs_shell_get_mode (shell) == GS_SHELL_MODE_LOADING)
		return;

#ifdef HAVE_MOGWAI
	if (shell->scheduler == NULL || !mwsc_scheduler_get_allow_downloads (shell->scheduler))
		flags |= GS_UPDATES_PAUSED_BANNER_FLAGS_NO_LARGE_DOWNLOADS;
#else
	if (gs_plugin_loader_get_network_metered (shell->plugin_loader))
		flags |= GS_UPDATES_PAUSED_BANNER_FLAGS_METERED;
#endif
	if (gs_plugin_loader_get_power_saver (shell->plugin_loader))
		flags |= GS_UPDATES_PAUSED_BANNER_FLAGS_POWER_SAVER;
	if (gs_plugin_loader_get_game_mode (shell->plugin_loader))
		flags |= GS_UPDATES_PAUSED_BANNER_FLAGS_GAME_MODE;

	gs_updates_paused_banner_set_flags (GS_UPDATES_PAUSED_BANNER (shell->updates_paused_banner),
					    flags);
}

static void
gs_shell_download_updates_changed_cb (GSettings   *settings,
				      const gchar *key,
				      gpointer     user_data)
{
	GsShell *shell = user_data;

	gs_shell_refresh_auto_updates_ui (shell);
}

static void
gs_shell_network_metered_notify_cb (GsPluginLoader *plugin_loader,
				    GParamSpec     *pspec,
				    gpointer        user_data)
{
#ifndef HAVE_MOGWAI
	GsShell *shell = user_data;

	/* @automatic_updates_paused only depends on network-metered if we’re
	 * compiled without Mogwai. */
	gs_shell_refresh_auto_updates_ui (shell);
#endif
}

#ifdef HAVE_MOGWAI
static void
scheduler_invalidated_cb (GsShell *shell)
{
	/* The scheduler shouldn’t normally be invalidated, since we Hold() it
	 * until we’re done with it. However, if the scheduler is stopped by
	 * systemd (`systemctl stop mogwai-scheduled`) this signal will be
	 * emitted. It may also be invalidated while our main window is hidden,
	 * as we release our Hold() then. */
	g_signal_handler_disconnect (shell->scheduler,
				     shell->scheduler_invalidated_handler);
	shell->scheduler_invalidated_handler = 0;
	shell->scheduler_held = FALSE;

	g_clear_object (&shell->scheduler);
}

static void
scheduler_allow_downloads_changed_cb (GsShell *shell)
{
	gs_shell_refresh_auto_updates_ui (shell);
}

static void
scheduler_hold_cb (GObject *source_object,
		   GAsyncResult *result,
		   gpointer data)
{
	g_autoptr(GError) error_local = NULL;
	MwscScheduler *scheduler = (MwscScheduler *) source_object;
	g_autoptr(GsShell) shell = data;  /* reference added when starting the async operation */

	if (mwsc_scheduler_hold_finish (scheduler, result, &error_local)) {
		shell->scheduler_held = TRUE;
	} else if (!g_error_matches (error_local, G_DBUS_ERROR, G_DBUS_ERROR_FAILED)) {
		g_warning ("Couldn't hold the Mogwai Scheduler daemon: %s",
			   error_local->message);
	}

	g_clear_error (&error_local);

	shell->scheduler_invalidated_handler =
		g_signal_connect_swapped (scheduler, "invalidated",
					  (GCallback) scheduler_invalidated_cb,
					  shell);

	g_signal_connect_object (scheduler, "notify::allow-downloads",
				 (GCallback) scheduler_allow_downloads_changed_cb,
				 shell,
				 G_CONNECT_SWAPPED);

	g_assert (shell->scheduler == NULL);
	shell->scheduler = scheduler;

	/* Update the UI accordingly. */
	gs_shell_refresh_auto_updates_ui (shell);
}

static void
scheduler_release_cb (GObject *source_object,
		      GAsyncResult *result,
		      gpointer data)
{
	MwscScheduler *scheduler = (MwscScheduler *) source_object;
	g_autoptr(GsShell) shell = data;  /* reference added when starting the async operation */
	g_autoptr(GError) error_local = NULL;

	if (!mwsc_scheduler_release_finish (scheduler, result, &error_local))
		g_warning ("Couldn't release the Mogwai Scheduler daemon: %s",
			   error_local->message);

	shell->scheduler_held = FALSE;
	g_clear_object (&shell->scheduler);
}

static void
scheduler_ready_cb (GObject *source_object,
		    GAsyncResult *result,
		    gpointer data)
{
	MwscScheduler *scheduler;
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GsShell) shell = data;  /* reference added when starting the async operation */

	scheduler = mwsc_scheduler_new_finish (result, &error_local);

	if (scheduler == NULL) {
		g_warning ("%s: Error getting Mogwai Scheduler: %s", G_STRFUNC,
			   error_local->message);
		return;
	}

	mwsc_scheduler_hold_async (scheduler,
				   "monitoring allow-downloads property",
				   NULL,
				   scheduler_hold_cb,
				   g_object_ref (shell));
}
#endif  /* HAVE_MOGWAI */

static void
gs_shell_basic_auth_start_cb (GsPluginLoader *plugin_loader,
                              const gchar *remote,
                              const gchar *realm,
                              GsBasicAuthCallback callback,
                              gpointer callback_data,
                              GsShell *shell)
{
	GtkWidget *dialog;

	dialog = gs_basic_auth_dialog_new (GTK_WINDOW (shell), remote, realm, callback, callback_data);
	gs_shell_modal_dialog_present (shell, GTK_WINDOW (dialog));
}

static gboolean
gs_shell_ask_untrusted_cb (GsPluginLoader *plugin_loader,
			   const gchar *title,
			   const gchar *msg,
			   const gchar *details,
			   const gchar *accept_label,
			   GsShell *shell)
{
	return gs_utils_ask_user_accepts (GTK_WINDOW (shell), title, msg, details, accept_label);
}

static void
free_back_entry (BackEntry *entry)
{
	if (entry->focus != NULL)
		g_object_remove_weak_pointer (G_OBJECT (entry->focus),
		                              (gpointer *) &entry->focus);
	g_clear_object (&entry->category);
	g_clear_object (&entry->app);
	g_free (entry->search);
	g_free (entry);
}

static void
gs_shell_clean_back_entry_stack (GsShell *shell)
{
	BackEntry *entry;

	while ((entry = g_queue_pop_head (shell->back_entry_stack)) != NULL) {
		free_back_entry (entry);
	}
}

static gboolean
gs_shell_get_mode_is_main (GsShellMode mode)
{
	switch (mode) {
	case GS_SHELL_MODE_OVERVIEW:
	case GS_SHELL_MODE_INSTALLED:
	case GS_SHELL_MODE_SEARCH:
	case GS_SHELL_MODE_UPDATES:
	case GS_SHELL_MODE_LOADING:
		return TRUE;
	case GS_SHELL_MODE_DETAILS:
	case GS_SHELL_MODE_CATEGORY:
	case GS_SHELL_MODE_EXTRAS:
	case GS_SHELL_MODE_MODERATE:
		return FALSE;
	default:
		return TRUE;
	}
}

static void search_bar_search_mode_enabled_changed_cb (GtkSearchBar *search_bar,
                                                       GParamSpec   *pspec,
                                                       GsShell      *shell);
static void gs_overview_page_button_cb (GtkWidget *widget, GsShell *shell);

static void
update_header_widgets (GsShell *shell)
{
	GsShellMode mode = gs_shell_get_mode (shell);

	/* only show the search button in overview and search pages */
	g_signal_handlers_block_by_func (shell->search_bar, search_bar_search_mode_enabled_changed_cb, shell);

	/* hide unless we're going to search */
	gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (shell->search_bar),
					mode == GS_SHELL_MODE_SEARCH);

	g_signal_handlers_unblock_by_func (shell->search_bar, search_bar_search_mode_enabled_changed_cb, shell);
}

static void
stack_notify_visible_child_cb (GObject    *object,
                               GParamSpec *pspec,
                               gpointer    user_data)
{
	GsShell *shell = GS_SHELL (user_data);
	GsPage *page;
	GtkWidget *widget;
	GsShellMode mode = gs_shell_get_mode (shell);
	gsize i;

	update_header_widgets (shell);

	/* do action for mode */
	page = shell->pages[mode];

	if (mode == GS_SHELL_MODE_OVERVIEW ||
	    mode == GS_SHELL_MODE_INSTALLED ||
	    mode == GS_SHELL_MODE_UPDATES)
		gs_shell_clean_back_entry_stack (shell);

	if (shell->page != NULL)
		gs_page_switch_from (shell->page);
	g_set_object (&shell->page, page);
	gs_page_switch_to (page);

	/* update header bar widgets */
	switch (mode) {
	case GS_SHELL_MODE_OVERVIEW:
	case GS_SHELL_MODE_INSTALLED:
	case GS_SHELL_MODE_SEARCH:
		gtk_widget_set_visible (shell->search_button, TRUE);
		break;
	case GS_SHELL_MODE_UPDATES:
		gtk_widget_set_visible (shell->search_button, FALSE);
		break;
	default:
		/* We don't care about changing the visibility of the search
		 * button in modes appearing in sub-pages.  */
		break;
	}

	widget = gs_page_get_header_start_widget (page);
	switch (mode) {
	case GS_SHELL_MODE_OVERVIEW:
	case GS_SHELL_MODE_INSTALLED:
	case GS_SHELL_MODE_UPDATES:
	case GS_SHELL_MODE_SEARCH:
		gs_shell_set_header_start_widget (shell, widget);
		break;
	default:
		g_assert (widget == NULL);
		break;
	}

	widget = gs_page_get_header_end_widget (page);
	switch (mode) {
	case GS_SHELL_MODE_OVERVIEW:
	case GS_SHELL_MODE_INSTALLED:
	case GS_SHELL_MODE_UPDATES:
	case GS_SHELL_MODE_SEARCH:
		gs_shell_set_header_end_widget (shell, widget);
		break;
	case GS_SHELL_MODE_EXTRAS:
		gs_shell_set_sub_header_end_widget (shell, widget);
		break;
	default:
		g_assert (widget == NULL);
		break;
	}

	g_clear_object (&shell->sub_page_header_title_binding);
	shell->sub_page_header_title_binding = g_object_bind_property (adw_view_stack_get_visible_child (shell->stack_sub), "title",
								       shell->sub_page_header_title, "label",
								       G_BINDING_SYNC_CREATE);

	/* refresh the updates bar when moving out of the loading mode, but only
	 * if the Mogwai scheduler state is already known, to avoid spuriously
	 * showing the updates bar */
#ifdef HAVE_MOGWAI
	if (shell->scheduler != NULL)
#else
	if (TRUE)
#endif
		gs_shell_refresh_auto_updates_ui (shell);

	/* destroy any existing modals */
	if (shell->modal_dialogs != NULL) {
		/* block signal emission of 'unmapped' since that will
		 * call g_ptr_array_remove_index. The unmapped signal may
		 * be emitted whilst running unref handlers for
		 * g_ptr_array_set_size */
		for (i = 0; i < shell->modal_dialogs->len; ++i) {
			GtkWidget *dialog = g_ptr_array_index (shell->modal_dialogs, i);
			g_signal_handlers_disconnect_by_func (dialog,
							      modal_dialog_unmapped_cb,
							      shell);
			gtk_window_destroy (GTK_WINDOW (dialog));
		}
		g_ptr_array_set_size (shell->modal_dialogs, 0);
	}
}

void
gs_shell_change_mode (GsShell *shell,
		      GsShellMode mode,
		      gpointer data,
		      gboolean scroll_up)
{
	GsApp *app;
	GsPage *page;
	gboolean mode_is_main = gs_shell_get_mode_is_main (mode);

	if (gs_shell_get_mode (shell) == mode &&
	    (mode != GS_SHELL_MODE_DETAILS ||
	     data == gs_details_page_get_app (GS_DETAILS_PAGE (shell->pages[mode])))) {
		return;
	}

	/* switch page */
	if (mode == GS_SHELL_MODE_LOADING) {
		adw_view_stack_set_visible_child_name (shell->stack_loading, "loading");
		return;
	}

	adw_view_stack_set_visible_child_name (shell->stack_loading, "main");
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (mode == GS_SHELL_MODE_DETAILS) {
		adw_leaflet_set_visible_child_name (shell->details_leaflet, "details");
	} else {
		adw_leaflet_set_visible_child_name (shell->details_leaflet, "main");
		/* We only change the main leaflet when not reaching the details
		 * page to preserve the navigation history in the UI's state.
		 * First change the page, then the leaflet, to avoid load of
		 * the previously shown page, which will be changed shortly after. */
		adw_view_stack_set_visible_child_name (mode_is_main ? shell->stack_main : shell->stack_sub, page_name[mode]);
		adw_leaflet_set_visible_child_name (shell->main_leaflet, mode_is_main ? "main" : "sub");
	}
G_GNUC_END_IGNORE_DEPRECATIONS

	/* do any mode-specific actions */
	page = shell->pages[mode];

	if (mode == GS_SHELL_MODE_SEARCH) {
		gs_search_page_set_text (GS_SEARCH_PAGE (page), data);
		gtk_editable_set_text (GTK_EDITABLE (shell->entry_search), data);
		gtk_editable_set_position (GTK_EDITABLE (shell->entry_search), -1);
	} else if (mode == GS_SHELL_MODE_DETAILS) {
		app = GS_APP (data);
		if (gs_app_get_metadata_item (app, "LingmoAppstore::show-metainfo") != NULL) {
			gs_details_page_set_metainfo (GS_DETAILS_PAGE (page),
						      gs_app_get_local_file (app));
		} else if (gs_app_get_local_file (app) != NULL) {
			gs_details_page_set_local_file (GS_DETAILS_PAGE (page),
			                                gs_app_get_local_file (app));
		} else if (gs_app_get_metadata_item (app, "LingmoAppstore::from-url") != NULL) {
			gs_details_page_set_url (GS_DETAILS_PAGE (page),
			                         gs_app_get_metadata_item (app, "LingmoAppstore::from-url"));
		} else {
			gs_details_page_set_app (GS_DETAILS_PAGE (page), data);
		}
	} else if (mode == GS_SHELL_MODE_CATEGORY) {
		gs_category_page_set_category (GS_CATEGORY_PAGE (page),
		                               GS_CATEGORY (data));
	}

	if (scroll_up)
		gs_page_scroll_up (page);
}

static void
gs_overview_page_button_cb (GtkWidget *widget, GsShell *shell)
{
	GsShellMode mode;
	mode = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
						   "lingmo-appstore::overview-mode"));
	gs_shell_change_mode (shell, mode, NULL, TRUE);
}

static void
save_back_entry (GsShell *shell)
{
	BackEntry *entry;

	entry = g_new0 (BackEntry, 1);
	entry->mode = gs_shell_get_mode (shell);

	entry->focus = gtk_window_get_focus (GTK_WINDOW (shell));
	if (entry->focus != NULL)
		g_object_add_weak_pointer (G_OBJECT (entry->focus),
					   (gpointer *) &entry->focus);

	switch (entry->mode) {
	case GS_SHELL_MODE_CATEGORY:
		entry->category = gs_category_page_get_category (GS_CATEGORY_PAGE (shell->pages[GS_SHELL_MODE_CATEGORY]));
		g_object_ref (entry->category);
		g_debug ("pushing back entry for %s with %s",
			 page_name[entry->mode],
			 gs_category_get_id (entry->category));
		break;
	case GS_SHELL_MODE_SEARCH:
		entry->search = g_strdup (gs_search_page_get_text (GS_SEARCH_PAGE (shell->pages[GS_SHELL_MODE_SEARCH])));
		g_debug ("pushing back entry for %s with %s",
			 page_name[entry->mode], entry->search);
		break;
	case GS_SHELL_MODE_DETAILS:
		entry->app = g_object_ref (gs_details_page_get_app (GS_DETAILS_PAGE (shell->pages[GS_SHELL_MODE_DETAILS])));
		entry->vscroll_position = gs_details_page_get_vscroll_position (GS_DETAILS_PAGE (shell->pages[GS_SHELL_MODE_DETAILS]));
		break;
	default:
		g_debug ("pushing back entry for %s", page_name[entry->mode]);
		break;
	}

	g_queue_push_head (shell->back_entry_stack, entry);
}

static void
gs_shell_plugin_events_sources_cb (GsShell *shell)
{
	gs_shell_show_sources (shell);
}

static void
gs_shell_plugin_events_no_space_cb (GsShell *shell)
{
	g_autoptr(GError) error = NULL;
	if (!g_spawn_command_line_async ("baobab", &error))
		g_warning ("failed to exec baobab: %s", error->message);
}

static void
gs_shell_plugin_events_network_settings_cb (GsShell *shell)
{
	g_autoptr(GError) error = NULL;
	if (!g_spawn_command_line_async ("lingmo-control-center network", &error))
		g_warning ("failed to exec lingmo-control-center: %s", error->message);
}

static void
gs_shell_plugin_events_details_text_cb (GsShell *shell,
					AdwToast *toast)
{
	const gchar *details_message;
	const gchar *details_text;

	details_message = gs_toast_get_details_message (toast);
	details_text = gs_toast_get_details_text (toast);

	if (details_message == NULL || *details_message == '\0')
		details_message = adw_toast_get_title (toast);

	gs_utils_show_error_dialog_simple (GTK_WINDOW (shell),
					   details_message,
					   details_text);
}

static void
gs_shell_plugin_events_details_uri_cb (GsShell *shell,
				       AdwToast *toast)
{
	/* in this case the details text contains a URI to be opened */
	const gchar *uri = gs_toast_get_details_text (toast);
	g_autoptr(GError) error = NULL;

	g_return_if_fail (uri != NULL);

	if (!g_app_info_launch_default_for_uri (uri, NULL, &error)) {
		g_warning ("failed to launch URI %s: %s",
			   uri, error->message);
	}
}

static void
gs_shell_plugin_events_restart_required_cb (GsShell *shell)
{
	g_autoptr(GError) error = NULL;
	if (!g_spawn_command_line_async (LIBEXECDIR "/lingmo-appstore-restarter", &error))
		g_warning ("failed to restart: %s", error->message);
}

static void gs_shell_rescan_events (GsShell *shell);

static void
gs_shell_plugin_event_dismissed_cb (GsShell *shell)
{
	guint i;
	g_autoptr(GPtrArray) events = NULL;

	/* mark any events currently showing as invalid */
	events = gs_plugin_loader_get_events (shell->plugin_loader);
	for (i = 0; i < events->len; i++) {
		GsPluginEvent *event = g_ptr_array_index (events, i);
		if (gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_VISIBLE)) {
			gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INVALID);
			gs_plugin_event_remove_flag (event, GS_PLUGIN_EVENT_FLAG_VISIBLE);
		}
	}

	/* show the next event */
	gs_shell_rescan_events (shell);
}

static void
gs_shell_go_back (GsShell *shell)
{
	BackEntry *entry;

	/* nothing to do */
	if (g_queue_is_empty (shell->back_entry_stack)) {
		g_debug ("no back stack, showing overview");
		gs_shell_change_mode (shell, GS_SHELL_MODE_OVERVIEW, NULL, FALSE);
		return;
	}

	entry = g_queue_pop_head (shell->back_entry_stack);

	switch (entry->mode) {
	case GS_SHELL_MODE_UNKNOWN:
	case GS_SHELL_MODE_LOADING:
		/* happens when using --search, --details, --install, etc. options */
		g_debug ("popping back entry for %s", page_name[entry->mode]);
		gs_shell_change_mode (shell, GS_SHELL_MODE_OVERVIEW, NULL, FALSE);
		break;
	case GS_SHELL_MODE_CATEGORY:
		g_debug ("popping back entry for %s with %s",
			 page_name[entry->mode],
			 gs_category_get_id (entry->category));
		gs_shell_change_mode (shell, entry->mode, entry->category, FALSE);
		break;
	case GS_SHELL_MODE_SEARCH:
		g_debug ("popping back entry for %s with %s",
			 page_name[entry->mode], entry->search);

		/* set the mode directly */
		gs_shell_change_mode (shell, entry->mode,
				      (gpointer) entry->search, FALSE);
		break;
	case GS_SHELL_MODE_DETAILS:
		g_debug ("popping back entry for %s with app %s and vscroll position %f",
			 page_name[entry->mode],
			 gs_app_get_unique_id (entry->app),
			 entry->vscroll_position);
		gs_shell_change_mode (shell, entry->mode, entry->app, FALSE);
		gs_details_page_set_vscroll_position (GS_DETAILS_PAGE (shell->pages[GS_SHELL_MODE_DETAILS]), entry->vscroll_position);
		break;
	default:
		g_debug ("popping back entry for %s", page_name[entry->mode]);
		gs_shell_change_mode (shell, entry->mode, NULL, FALSE);
		break;
	}

	if (entry->focus != NULL)
		gtk_widget_grab_focus (entry->focus);

	free_back_entry (entry);
}

static void
gs_shell_details_back_button_cb (GtkWidget *widget, GsShell *shell)
{
	gs_shell_go_back (shell);
}

static void
gs_shell_back_button_cb (GtkWidget *widget, GsShell *shell)
{
	gs_shell_go_back (shell);
}

static void
gs_shell_reload_cb (GsPluginLoader *plugin_loader, GsShell *shell)
{
	for (gsize i = 0; i < G_N_ELEMENTS (shell->pages); i++) {
		GsPage *page = shell->pages[i];
		if (page != NULL)
			gs_page_reload (page);
	}
}

static void
gs_shell_details_page_metainfo_loaded_cb (GtkWidget *details_page,
					  GsApp *app,
					  GsShell *self)
{
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_SHELL (self));

	/* If the user has manually loaded some metainfo to
	 * preview, override the featured carousel with it too,
	 * so they can see how it looks in the carousel. */
	gs_overview_page_override_featured (GS_OVERVIEW_PAGE (self->pages[GS_SHELL_MODE_OVERVIEW]), app);
}

static gboolean
change_mode_idle (gpointer user_data)
{
	GsShell *shell = user_data;

	gs_page_reload (GS_PAGE (shell->pages[GS_SHELL_MODE_UPDATES]));
	gs_page_reload (GS_PAGE (shell->pages[GS_SHELL_MODE_INSTALLED]));

	/* Switch only when still on the loading page, otherwise the page
	   could be changed from the command line or such, which would mean
	   hiding the chosen page. */
	if (gs_shell_get_mode (shell) == GS_SHELL_MODE_LOADING)
		gs_shell_change_mode (shell, GS_SHELL_MODE_OVERVIEW, NULL, TRUE);

	return G_SOURCE_REMOVE;
}

static void
overview_page_refresh_done (GsOverviewPage *overview_page, gpointer data)
{
	GsShell *shell = data;

	g_signal_handlers_disconnect_by_func (overview_page, overview_page_refresh_done, data);

	/* now that we're finished with the loading page, connect the reload signal handler */
	g_signal_connect (shell->plugin_loader, "reload",
	                  G_CALLBACK (gs_shell_reload_cb), shell);

	/* schedule to change the mode in an idle callback, since it can take a
	 * while and this callback handler is typically called at the end of a
	 * long main context iteration already */
	g_idle_add (change_mode_idle, shell);
}

static void
initial_refresh_done (GsLoadingPage *loading_page, gpointer data)
{
	GsShell *shell = data;
	gboolean been_overview;

	g_signal_handlers_disconnect_by_func (loading_page, initial_refresh_done, data);

	been_overview = gs_shell_get_mode (shell) == GS_SHELL_MODE_OVERVIEW;

	g_signal_emit (shell, signals[SIGNAL_LOADED], 0);

	/* if the "loaded" signal handler didn't change the mode, kick off async
	 * overview page refresh, and switch to the page once done */
	if (gs_shell_get_mode (shell) == GS_SHELL_MODE_LOADING || been_overview) {
		g_signal_connect (shell->pages[GS_SHELL_MODE_OVERVIEW], "refreshed",
		                  G_CALLBACK (overview_page_refresh_done), shell);
		gs_page_reload (GS_PAGE (shell->pages[GS_SHELL_MODE_OVERVIEW]));
		return;
	}

	/* now that we're finished with the loading page, connect the reload signal handler */
	g_signal_connect (shell->plugin_loader, "reload",
	                  G_CALLBACK (gs_shell_reload_cb), shell);
}

static gboolean
window_keypress_handler (GtkEventControllerKey *key_controller,
                         guint                  keyval,
                         guint                  keycode,
                         GdkModifierType        state,
                         GsShell               *shell)
{
	/* handle ctrl+f shortcut */
	if ((state & GDK_CONTROL_MASK) > 0 && keyval == GDK_KEY_f) {
		if (!gtk_search_bar_get_search_mode (GTK_SEARCH_BAR (shell->search_bar))) {
			GsShellMode mode = gs_shell_get_mode (shell);

			gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (shell->search_bar), TRUE);
			gtk_widget_grab_focus (shell->entry_search);

			/* If the mode doesn't have a search button,
			 * switch to the search page right away,
			 * otherwise we would show the search bar
			 * without a button to toggle it. */
			switch (mode) {
			case GS_SHELL_MODE_OVERVIEW:
			case GS_SHELL_MODE_INSTALLED:
			case GS_SHELL_MODE_SEARCH:
				break;
			default:
				gs_shell_show_search (shell, "");
				break;
			}
		} else {
			gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (shell->search_bar), FALSE);
		}
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static void
search_changed_handler (GObject *entry, GsShell *shell)
{
	g_autofree gchar *text = NULL;

	text = g_strdup (gtk_editable_get_text (GTK_EDITABLE (entry)));
	if (strlen (text) >= 2) {
		if (gs_shell_get_mode (shell) != GS_SHELL_MODE_SEARCH) {
			save_back_entry (shell);
			gs_shell_change_mode (shell, GS_SHELL_MODE_SEARCH,
					      (gpointer) text, TRUE);
		} else {
			gs_search_page_set_text (GS_SEARCH_PAGE (shell->pages[GS_SHELL_MODE_SEARCH]), text);
			gs_page_switch_to (shell->pages[GS_SHELL_MODE_SEARCH]);
		}
	}
}

static void
search_bar_search_mode_enabled_changed_cb (GtkSearchBar *search_bar,
					   GParamSpec *pspec,
					   GsShell *shell)
{
	/* go back when exiting the search view */
	if (gs_shell_get_mode (shell) == GS_SHELL_MODE_SEARCH &&
	    !gtk_search_bar_get_search_mode (search_bar))
		gs_shell_go_back (shell);
}

static void
go_back (GsShell *shell)
{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (adw_leaflet_get_adjacent_child (shell->details_leaflet,
					    ADW_NAVIGATION_DIRECTION_BACK)) {
		gtk_widget_activate (shell->button_back2);
	} else {
		gtk_widget_activate (shell->button_back);
	}
G_GNUC_END_IGNORE_DEPRECATIONS
}

static gboolean
window_key_pressed_cb (GtkEventControllerKey *key_controller,
                       guint                  keyval,
                       guint                  keycode,
                       GdkModifierType        state,
                       GsShell               *shell)
{
	gboolean is_rtl = gtk_widget_get_direction (shell->button_back) == GTK_TEXT_DIR_RTL;
	gboolean is_alt = (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK)) == GDK_ALT_MASK;

	if ((!is_rtl && is_alt && keyval == GDK_KEY_Left) ||
	    (is_rtl && is_alt && keyval == GDK_KEY_Right) ||
	    keyval == GDK_KEY_Back) {
		go_back (shell);
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static void
window_button_pressed_cb (GtkGestureClick *click_gesture,
                          gint             n_press,
                          gdouble          x,
                          gdouble          y,
                          GsShell         *shell)
{
	go_back (shell);

	gtk_gesture_set_state (GTK_GESTURE (click_gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}

static gboolean
main_window_closed_cb (GtkWidget *dialog, gpointer user_data)
{
	GsShell *shell = user_data;

	/* hide any notifications */
	g_application_withdraw_notification (g_application_get_default (),
					     "installed");
	g_application_withdraw_notification (g_application_get_default (),
					     "install-resources");

	/* release our hold on the download scheduler */
#ifdef HAVE_MOGWAI
	if (shell->scheduler != NULL) {
		if (shell->scheduler_invalidated_handler > 0)
			g_signal_handler_disconnect (shell->scheduler,
						     shell->scheduler_invalidated_handler);
		shell->scheduler_invalidated_handler = 0;

		if (shell->scheduler_held)
			mwsc_scheduler_release_async (shell->scheduler,
						      NULL,
						      scheduler_release_cb,
						      g_object_ref (shell));
		else
			g_clear_object (&shell->scheduler);
	}
#endif  /* HAVE_MOGWAI */

	gs_shell_clean_back_entry_stack (shell);
	gtk_widget_set_visible (dialog, FALSE);

#ifdef __GLIBC__
	/* Free unused memory with GNU extension of malloc.h */
	malloc_trim (0);
#endif

	return TRUE;
}

static void
gs_shell_main_window_mapped_cb (GtkWidget *widget, GsShell *shell)
{
	gs_plugin_loader_set_scale (shell->plugin_loader,
				    (guint) gtk_widget_get_scale_factor (widget));

	/* Set up the updates bar. Do this here rather than in gs_shell_setup()
	 * since we only want to hold the scheduler open while the lingmo-appstore
	 * main window is visible, and not while we’re running in the background. */
#ifdef HAVE_MOGWAI
	if (shell->scheduler == NULL)
		mwsc_scheduler_new_async (shell->cancellable,
					  (GAsyncReadyCallback) scheduler_ready_cb,
					  g_object_ref (shell));
#else
	gs_shell_refresh_auto_updates_ui (shell);
#endif  /* HAVE_MOGWAI */
}

static void
gs_shell_main_window_realized_cb (GtkWidget *widget, GsShell *shell)
{
	GdkRectangle geometry;
	GdkSurface *surface;
	GdkDisplay *display;
	GdkMonitor *monitor;

	display = gtk_widget_get_display (GTK_WIDGET (shell));
	surface = gtk_native_get_surface (GTK_NATIVE (shell));
	monitor = gdk_display_get_monitor_at_surface (display, surface);

	/* adapt the window for low and medium resolution screens */
	if (monitor != NULL) {
		gdk_monitor_get_geometry (monitor, &geometry);
		if (geometry.width < 800 || geometry.height < 600) {
		} else if (geometry.width < 1366 || geometry.height < 768) {
			gtk_window_set_default_size (GTK_WINDOW (shell), 1050, 600);
		}
	}
}

static gboolean
gs_shell_has_disk_examination_app (void)
{
	g_autofree gchar *baobab = g_find_program_in_path ("baobab");
	return (baobab != NULL);
}

static void
gs_shell_show_event_app_notify (GsShell *shell,
				const gchar *title,
				GsToastButton button,
				const gchar *details_text,
				const gchar *details_message)
{
	g_autoptr(AdwToast) toast = NULL;

	toast = gs_toast_new (title, button, details_message, details_text);

	if (details_text != NULL)
		button = GS_TOAST_BUTTON_NONE;

	g_signal_connect_object (toast, "dismissed",
				 G_CALLBACK (gs_shell_plugin_event_dismissed_cb), shell, G_CONNECT_SWAPPED);

	switch (button) {
	case GS_TOAST_BUTTON_SOURCES:
		g_signal_connect_object (toast, "button-clicked",
					 G_CALLBACK (gs_shell_plugin_events_sources_cb), shell, G_CONNECT_SWAPPED);
		break;
	case GS_TOAST_BUTTON_NO_SPACE:
		if (gs_shell_has_disk_examination_app ()) {
			g_signal_connect_object (toast, "button-clicked",
						 G_CALLBACK (gs_shell_plugin_events_no_space_cb), shell, G_CONNECT_SWAPPED);
		}
		break;
	case GS_TOAST_BUTTON_NETWORK_SETTINGS:
		g_signal_connect_object (toast, "button-clicked",
					 G_CALLBACK (gs_shell_plugin_events_network_settings_cb), shell, G_CONNECT_SWAPPED);
		break;
	case GS_TOAST_BUTTON_RESTART_REQUIRED:
		g_signal_connect_object (toast, "button-clicked",
					 G_CALLBACK (gs_shell_plugin_events_restart_required_cb), shell, G_CONNECT_SWAPPED);
		break;
	case GS_TOAST_BUTTON_DETAILS_URI:
		g_signal_connect_object (toast, "button-clicked",
					 G_CALLBACK (gs_shell_plugin_events_details_uri_cb), shell, G_CONNECT_SWAPPED);
		break;
	default:
		g_warn_if_reached ();
	/* fall-through */
	case GS_TOAST_BUTTON_NONE:
	case GS_TOAST_BUTTON_LAST:
		if (details_text != NULL) {
			g_signal_connect_object (toast, "button-clicked",
						 G_CALLBACK (gs_shell_plugin_events_details_text_cb), shell, G_CONNECT_SWAPPED);
		}
		break;
	}

	adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (shell->toast_overlay), g_steal_pointer (&toast));
}

void
gs_shell_show_notification (GsShell *shell, const gchar *title)
{
	gs_shell_show_event_app_notify (shell, title, GS_TOAST_BUTTON_NONE, NULL, NULL);
}

static gchar *
gs_shell_get_title_from_origin (GsApp *app)
{
	/* get a title, falling back */
	if (gs_app_get_origin_hostname (app) != NULL) {
		/* TRANSLATORS: this is part of the in-app notification,
		 * where the %s is the truncated hostname, e.g.
		 * 'alt.fedoraproject.org' */
		return g_strdup_printf (_("“%s”"), gs_app_get_origin_hostname (app));
	}
	if (gs_app_get_origin (app) != NULL) {
		/* TRANSLATORS: this is part of the in-app notification,
		 * where the %s is the origin id, e.g. 'fedora' */
		return g_strdup_printf (_("“%s”"), gs_app_get_origin (app));
	}
	return g_strdup_printf ("“%s”", gs_app_get_id (app));
}

/* return a name for the app, using quotes if the name is more than one word */
static gchar *
gs_shell_get_title_from_app (GsApp *app)
{
	const gchar *tmp = gs_app_get_name (app);
	if (tmp != NULL) {
		if (g_strstr_len (tmp, -1, " ") != NULL) {
			/* TRANSLATORS: this is part of the in-app notification,
			 * where the %s is a multi-word localised app name
			 * e.g. 'Getting things LINGMO!" */
			return g_strdup_printf (_("“%s”"), tmp);
		}
		return g_strdup (tmp);
	}
	return g_strdup_printf (_("“%s”"), gs_app_get_id (app));
}

/* returns whether the `out_details_text` is a URI */
static gboolean
gs_shell_handle_events_more_info (GsShell *self,
				  GsApp *origin,
				  const gchar *suggested_details_text,
				  const gchar *suggested_details_message,
				  const gchar **out_details_text,
				  const gchar **out_details_message)
{
	const gchar *uri;

	/* Prefer detailed error message against origin's help URL */
	if (suggested_details_text != NULL && *suggested_details_text != '\0') {
		if (out_details_message != NULL)
			*out_details_message = suggested_details_message;
		*out_details_text = suggested_details_text;
		return FALSE;
	}

	if (origin == NULL)
		return FALSE;

	uri = gs_app_get_url (origin, AS_URL_KIND_HELP);
	if (uri != NULL) {
		*out_details_text = uri;
		return TRUE;
	}

	return FALSE;
}

static gboolean
gs_shell_show_event_refresh (GsShell *shell, GsPluginEvent *event)
{
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *str_origin = NULL;
	GsPluginJob *job = gs_plugin_event_get_job (event);

	/* ignore any errors from background downloads */
	if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
		return TRUE;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_DOWNLOAD_FAILED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to download updates");
		if (origin != NULL) {
			str_origin = gs_shell_get_title_from_origin (origin);
			if (gs_app_get_bundle_kind (origin) == AS_BUNDLE_KIND_CABINET) {
				/* TRANSLATORS: failure text for the in-app notification */
				toast_text = _("Unable to download firmware updates");
			} else {
				if (!gs_app_has_management_plugin (origin, NULL))
					button = GS_TOAST_BUTTON_SOURCES;
			}
		}
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_NETWORK)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: internet access required");
		button = GS_TOAST_BUTTON_NETWORK_SETTINGS;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: not enough disk space");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: authentication required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: invalid authentication");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: permission required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		if (GS_IS_PLUGIN_JOB_UPDATE_APPS (job) &&
		    !(gs_plugin_job_update_apps_get_flags (GS_PLUGIN_JOB_UPDATE_APPS (job)) & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD)) {
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Unable to download updates");
		} else {
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Unable to get list of updates");
		}
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_install (GsShell *shell, GsPluginEvent *event)
{
	GsApp *app = gs_plugin_event_get_app (event);
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *tmp_toast_text = NULL;
	g_autofree gchar *str_app = NULL;

	str_app = gs_shell_get_title_from_app (app);

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_DOWNLOAD_FAILED)) {
		toast_text = _("Unable to install: download failed");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
		if (origin != NULL) {
			toast_text = _("Unable to install: missing runtime");
		} else {
			tmp_toast_text = g_strdup_printf (_("Unable to install %s"), str_app);
			toast_text = tmp_toast_text;
		}
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_NETWORK)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text  = _("Unable to install: internet access required");
		button = GS_TOAST_BUTTON_NETWORK_SETTINGS;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_INVALID_FORMAT)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to install: invalid app format");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		toast_text = _("Unable to install: not enough disk space");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED)) {
		toast_text = _("Unable to install: authentication required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID)) {
		toast_text = _("Unable to install: invalid authentication");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		toast_text = _("Unable to install: permission required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AC_POWER_REQUIRED)) {
		toast_text = _("Unable to install: device must be plugged in");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW)) {
		toast_text = _("Unable to install: low battery");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* TRANSLATORS: failure text for the in-app notification,
		 * where the %s is the app name (e.g. "GIMP") */
		tmp_toast_text = g_strdup_printf (_("Unable to install %s"), str_app);
		toast_text = tmp_toast_text;
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_update (GsShell *shell, GsPluginEvent *event)
{
	GsApp *app = gs_plugin_event_get_app (event);
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *tmp_toast_text = NULL;
	g_autofree gchar *str_app = NULL;

	/* ignore any errors from background downloads */
	if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
		return TRUE;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_DOWNLOAD_FAILED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: download failed");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_NETWORK)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: internet access required");
		button = GS_TOAST_BUTTON_NETWORK_SETTINGS;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: not enough disk space");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: authentication required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: invalid authentication");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: permission required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AC_POWER_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: device must be plugged in");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to update: low battery");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		if (app != NULL) {
			str_app = gs_shell_get_title_from_app (app);
			/* TRANSLATORS: failure text for the in-app notification,
			 * where the %s is the app name (e.g. "GIMP") */
			tmp_toast_text = g_strdup_printf (_("Unable to update %s"), str_app);
			toast_text = tmp_toast_text;
		} else {
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Unable to install updates");
		}
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_upgrade (GsShell *shell, GsPluginEvent *event)
{
	GsApp *app = gs_plugin_event_get_app (event);
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *str_app = NULL;
	g_autofree gchar *str_origin = NULL;

	str_app = g_strdup_printf ("%s %s", gs_app_get_name (app), gs_app_get_version (app));

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_DOWNLOAD_FAILED)) {
		if (origin != NULL) {
			str_origin = gs_shell_get_title_from_origin (origin);
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Unable to upgrade");
		} else {
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Unable to upgrade: download failed");
		}
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_NETWORK)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: internet access required");
		button = GS_TOAST_BUTTON_NETWORK_SETTINGS;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: not enough disk space");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: authentication required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: invalid authentication");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: permission required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AC_POWER_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: device must be plugged in");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade: low battery");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to upgrade");
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_remove (GsShell *shell, GsPluginEvent *event)
{
	GsApp *app = gs_plugin_event_get_app (event);
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *tmp_toast_text = NULL;
	g_autofree gchar *str_app = NULL;

	str_app = gs_shell_get_title_from_app (app);

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to uninstall: authentication required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AUTH_INVALID)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to uninstall: authentication invalid");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to uninstall: permission required");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AC_POWER_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to uninstall: device must be plugged in");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to uninstall: low battery");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* non-interactive generic */
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
			return TRUE;
		/* TRANSLATORS: failure text for the in-app notification,
		 * where the %s is the app name (e.g. "GIMP") */
		tmp_toast_text = g_strdup_printf (_("Unable to uninstall %s"), str_app);
		toast_text = tmp_toast_text;
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_launch (GsShell *shell, GsPluginEvent *event)
{
	GsApp *app = gs_plugin_event_get_app (event);
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *details_text = NULL;
	const gchar *details_message = NULL;
	const gchar *suggested_details_text = NULL;
	g_autofree gchar *tmp_toast_text = NULL;
	g_autofree gchar *str_app = NULL;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
		if (app != NULL)
			str_app = gs_shell_get_title_from_app (app);
		if (str_app != NULL) {
			/* TRANSLATORS: failure text for the in-app notification, where the '%s' is
			   replaced with the app name (e.g. "GIMP") */
			tmp_toast_text = g_strdup_printf (_("Unable to launch %s"),
							  str_app);
			toast_text = tmp_toast_text;
		} else {
			/* TRANSLATORS: failure text for the in-app notification */
			toast_text = _("Sorry, something went wrong");
		}
		if (str_app == NULL || origin == NULL) {
			/* non-interactive generic */
			if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
				return FALSE;
		}
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Not enough disk space for operation");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* non-interactive generic */
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
			return TRUE;
		/* TRANSLATORS: we failed to get a proper error code */
		toast_text = _("Sorry, something went wrong");
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, &details_message))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, details_message);
	return TRUE;
}

static gboolean
gs_shell_show_event_file_to_app (GsShell *shell, GsPluginEvent *event)
{
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *suggested_details_text = NULL;
	const gchar *details_text = NULL;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to install: file type not supported");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to install: authentication failed");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Not enough disk space for operation");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* non-interactive generic */
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
			return TRUE;
		/* TRANSLATORS: we failed to get a proper error code */
		toast_text = _("Sorry, something went wrong");
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, NULL, suggested_details_text, NULL, &details_text, NULL))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, NULL);
	return TRUE;
}

static gboolean
gs_shell_show_event_url_to_app (GsShell *shell, GsPluginEvent *event)
{
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *suggested_details_text = NULL;
	const gchar *details_text = NULL;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to install");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SECURITY)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Unable to install: authentication failed");
		suggested_details_text = error->message;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Not enough disk space for operation");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* non-interactive generic */
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
			return TRUE;
		/* TRANSLATORS: we failed to get a proper error code */
		toast_text = _("Sorry, something went wrong");
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, NULL, suggested_details_text, NULL, &details_text, NULL))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, NULL);
	return TRUE;
}

static gboolean
gs_shell_show_event_fallback (GsShell *shell, GsPluginEvent *event)
{
	GsApp *origin = gs_plugin_event_get_origin (event);
	GsToastButton button = GS_TOAST_BUTTON_NONE;
	const GError *error = gs_plugin_event_get_error (event);
	const gchar *toast_text = NULL;
	const gchar *suggested_details_text = NULL;
	const gchar *details_text = NULL;
	g_autofree gchar *tmp_toast_text = NULL;
	g_autofree gchar *str_origin = NULL;

	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_DOWNLOAD_FAILED)) {
		if (origin != NULL) {
			str_origin = gs_shell_get_title_from_origin (origin);
			/* TRANSLATORS: failure text for the in-app notification,
			 * the %s is the origin, e.g. "Fedora" or
			 * "Fedora Project [fedoraproject.org]" */
			tmp_toast_text = g_strdup_printf (_("Unable to contact %s"),
							  str_origin);
			toast_text = tmp_toast_text;
			suggested_details_text = error->message;
		}
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NO_SPACE)) {
		/* TRANSLATORS: failure text for the in-app notification */
		toast_text = _("Not enough disk space for operation");
		button = GS_TOAST_BUTTON_NO_SPACE;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_RESTART_REQUIRED)) {
		/* TRANSLATORS: failure text for the in-app notification, where the 'Appstore' means this app, aka 'LINGMO Appstore'. */
		toast_text = _("Restart Appstore to use new plugins");
		button = GS_TOAST_BUTTON_RESTART_REQUIRED;
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_AC_POWER_REQUIRED)) {
		/* TRANSLATORS: need to be connected to the AC power */
		toast_text = _("Device needs to be plugged in");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW)) {
		/* TRANSLATORS: not enough juice to do this safely */
		toast_text = _("Battery level is too low");
	} else if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		   g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Do nothing. */
	} else {
		/* non-interactive generic */
		if (!gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE))
			return TRUE;
		/* TRANSLATORS: we failed to get a proper error code */
		toast_text = _("Sorry, something went wrong");
		suggested_details_text = error->message;
	}

	if (toast_text == NULL)
		return FALSE;

	if (gs_shell_handle_events_more_info (shell, origin, suggested_details_text, NULL, &details_text, NULL))
		button = GS_TOAST_BUTTON_DETAILS_URI;

	/* show in-app notification */
	gs_shell_show_event_app_notify (shell, toast_text, button, details_text, NULL);
	return TRUE;
}

static gboolean
gs_shell_show_event (GsShell *shell, GsPluginEvent *event)
{
	const GError *error;
	GsPluginAction action;
	GsPluginJob *job;

	/* get error */
	error = gs_plugin_event_get_error (event);
	if (error == NULL)
		return FALSE;

	/* name and shame the plugin */
	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_TIMED_OUT)) {
		gs_shell_show_event_app_notify (shell, error->message,
						GS_TOAST_BUTTON_NONE, NULL, NULL);
		return TRUE;
	}

	job = gs_plugin_event_get_job (event);
	if (GS_IS_PLUGIN_JOB_REFRESH_METADATA (job))
		return gs_shell_show_event_refresh (shell, event);
	else if (GS_IS_PLUGIN_JOB_UPDATE_APPS (job) &&
		 !(gs_plugin_job_update_apps_get_flags (GS_PLUGIN_JOB_UPDATE_APPS (job)) & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD))
		return gs_shell_show_event_refresh (shell, event);
	else if (GS_IS_PLUGIN_JOB_UPDATE_APPS (job) &&
		 !(gs_plugin_job_update_apps_get_flags (GS_PLUGIN_JOB_UPDATE_APPS (job)) & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY))
		return gs_shell_show_event_update (shell, event);

	/* split up the events by action */
	action = gs_plugin_event_get_action (event);
	switch (action) {
	case GS_PLUGIN_ACTION_INSTALL:
	case GS_PLUGIN_ACTION_INSTALL_REPO:
	case GS_PLUGIN_ACTION_ENABLE_REPO:
		return gs_shell_show_event_install (shell, event);
	case GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD:
		return gs_shell_show_event_upgrade (shell, event);
	case GS_PLUGIN_ACTION_REMOVE:
	case GS_PLUGIN_ACTION_REMOVE_REPO:
	case GS_PLUGIN_ACTION_DISABLE_REPO:
		return gs_shell_show_event_remove (shell, event);
	case GS_PLUGIN_ACTION_LAUNCH:
		return gs_shell_show_event_launch (shell, event);
	case GS_PLUGIN_ACTION_FILE_TO_APP:
		return gs_shell_show_event_file_to_app (shell, event);
	case GS_PLUGIN_ACTION_URL_TO_APP:
		return gs_shell_show_event_url_to_app (shell, event);
	default:
		break;
	}

	/* capture some warnings every time */
	return gs_shell_show_event_fallback (shell, event);
}

static void
gs_shell_rescan_events (GsShell *shell)
{
	g_autoptr(GsPluginEvent) event = NULL;

	/* find the first active event and show it */
	event = gs_plugin_loader_get_event_default (shell->plugin_loader);
	if (event != NULL) {
		GsPluginAction action = gs_plugin_event_get_action (event);
		const GError *error = gs_plugin_event_get_error (event);
		if (error != NULL &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			if (error->domain == GS_PLUGIN_ERROR) {
				g_debug ("%sinteractive action '%s' failed with error '%s': %s",
					 gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE) ? "" : "non-",
					 gs_plugin_action_to_string (action),
					 gs_plugin_error_to_string (error->code),
					 error->message);
			} else {
				g_debug ("%sinteractive action '%s' failed with error '%s::%d': %s",
					 gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE) ? "" : "non-",
					 gs_plugin_action_to_string (action),
					 g_quark_to_string (error->domain),
					 error->code,
					 error->message);
			}
		}
		if (!gs_shell_show_event (shell, event)) {
			if (error != NULL &&
			    !g_error_matches (error,
					      GS_PLUGIN_ERROR,
					      GS_PLUGIN_ERROR_CANCELLED) &&
			    !g_error_matches (error,
					      G_IO_ERROR,
					      G_IO_ERROR_CANCELLED)) {
				g_autofree gchar *msg = NULL;
				g_autofree gchar *error_ident = NULL;
				if (error->domain == GS_PLUGIN_ERROR) {
					error_ident = g_strdup (gs_plugin_error_to_string (error->code));
				} else {
					error_ident = g_strdup_printf ("%s::%d",
									g_quark_to_string (error->domain),
									error->code);
				}
				msg = g_strdup_printf ("not handling %sinteractive error '%s' for action '%s': %s",
						       gs_plugin_event_has_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE) ? "" : "non-",
						       error_ident,
						       gs_plugin_action_to_string (action),
						       error->message);
				if (g_strcmp0 (BUILD_TYPE, "debug") == 0)
					g_warning ("%s", msg);
				else
					g_debug ("%s", msg);
			}
			gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INVALID);
			return;
		}
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_VISIBLE);
		return;
	}
}

static void
gs_shell_events_notify_cb (GsPluginLoader *plugin_loader,
			   GParamSpec *pspec,
			   GsShell *shell)
{
	gs_shell_rescan_events (shell);
}

static void
gs_shell_setup_pages (GsShell *shell)
{
	for (gsize i = 0; i < G_N_ELEMENTS (shell->pages); i++) {
		g_autoptr(GError) error = NULL;
		GsPage *page = shell->pages[i];
		if (page != NULL &&
		    !gs_page_setup (page, shell,
				    shell->plugin_loader,
				    shell->cancellable,
				    &error)) {
			g_warning ("Failed to setup panel: %s", error->message);
		}
	}
}

static void
gs_shell_add_about_menu_item (GsShell *shell)
{
	g_autoptr(GMenuItem) menu_item = NULL;

	/* TRANSLATORS: this is the menu item that opens the about window */
	menu_item = g_menu_item_new (_("About Appstore"), "app.about");
	g_menu_append_item (G_MENU (shell->primary_menu), menu_item);
}

static void
updates_page_notify_counter_cb (GObject    *obj,
                                GParamSpec *pspec,
                                gpointer    user_data)
{
	GsPage *page = GS_PAGE (obj);
	GsShell *shell = GS_SHELL (user_data);
	AdwViewStackPage *stack_page;
	gboolean needs_attention;

	/* Update the needs-attention child property of the page in the
	 * AdwViewStack. There’s no need to account for whether it’s the currently
	 * visible page, as the CSS rules do that for us. This can’t be a simple
	 * property binding, though, as it’s a binding between an object
	 * property and a child property. */
	needs_attention = (gs_page_get_counter (page) > 0);

	stack_page = adw_view_stack_get_page (shell->stack_main, GTK_WIDGET (page));
	adw_view_stack_page_set_needs_attention (stack_page, needs_attention);
}

static void
category_page_app_clicked_cb (GsCategoryPage *page,
                              GsApp          *app,
                              gpointer        user_data)
{
	GsShell *shell = GS_SHELL (user_data);

	gs_shell_show_app (shell, app);
}

static void
details_page_app_clicked_cb (GsDetailsPage *page,
			     GsApp         *app,
			     gpointer       user_data)
{
	GsShell *shell = GS_SHELL (user_data);

	gs_shell_show_app (shell, app);
}

void
gs_shell_setup (GsShell *shell, GsPluginLoader *plugin_loader, GCancellable *cancellable)
{
	GsOdrsProvider *odrs_provider;

	g_return_if_fail (GS_IS_SHELL (shell));

	shell->plugin_loader = g_object_ref (plugin_loader);
	g_signal_connect_object (shell->plugin_loader, "notify::events",
				 G_CALLBACK (gs_shell_events_notify_cb),
				 shell, 0);
	g_signal_connect_object (shell->plugin_loader, "notify::network-metered",
				 G_CALLBACK (gs_shell_network_metered_notify_cb),
				 shell, 0);
	g_signal_connect_object (shell->plugin_loader, "basic-auth-start",
				 G_CALLBACK (gs_shell_basic_auth_start_cb),
				 shell, 0);
	g_signal_connect_object (shell->plugin_loader, "ask-untrusted",
				 G_CALLBACK (gs_shell_ask_untrusted_cb),
				 shell, 0);

	g_object_bind_property (shell->plugin_loader, "allow-updates",
				shell->pages[GS_SHELL_MODE_UPDATES], "visible",
				G_BINDING_SYNC_CREATE);

	shell->cancellable = g_object_ref (cancellable);

	shell->settings = g_settings_new ("org.lingmo.appstore");

	/* set up pages */
	gs_shell_setup_pages (shell);

	/* set up the metered data info bar and mogwai */
	g_signal_connect (shell->settings, "changed::download-updates",
			  (GCallback) gs_shell_download_updates_changed_cb, shell);

	odrs_provider = gs_plugin_loader_get_odrs_provider (shell->plugin_loader);
	gs_details_page_set_odrs_provider (GS_DETAILS_PAGE (shell->pages[GS_SHELL_MODE_DETAILS]), odrs_provider);
	gs_moderate_page_set_odrs_provider (GS_MODERATE_PAGE (shell->pages[GS_SHELL_MODE_MODERATE]), odrs_provider);

	/* coldplug */
	gs_shell_rescan_events (shell);

	/* primary menu */
	gs_shell_add_about_menu_item (shell);

	if (g_settings_get_boolean (shell->settings, "download-updates")) {
		/* show loading page, which triggers the initial refresh */
		gs_shell_change_mode (shell, GS_SHELL_MODE_LOADING, NULL, TRUE);
	} else {
		g_debug ("Skipped refresh of the repositories due to 'download-updates' disabled");
		initial_refresh_done (GS_LOADING_PAGE (shell->pages[GS_SHELL_MODE_LOADING]), shell);

		if (g_settings_get_boolean (shell->settings, "first-run"))
			g_settings_set_boolean (shell->settings, "first-run", FALSE);
	}

	if (shell->activate_after_setup) {
		shell->activate_after_setup = FALSE;
		gs_shell_activate (shell);
	}
}

void
gs_shell_reset_state (GsShell *shell)
{
	/* reset to overview, unless we're in the loading state which advances
	 * to overview on its own */
	if (gs_shell_get_mode (shell) != GS_SHELL_MODE_LOADING)
		gs_shell_change_mode (shell, GS_SHELL_MODE_OVERVIEW, NULL, TRUE);

	gs_shell_clean_back_entry_stack (shell);
}

void
gs_shell_set_mode (GsShell *shell, GsShellMode mode)
{
	gs_shell_change_mode (shell, mode, NULL, TRUE);
}

GsShellMode
gs_shell_get_mode (GsShell *shell)
{
	const gchar *name;

	if (g_strcmp0 (adw_view_stack_get_visible_child_name (shell->stack_loading), "loading") == 0)
		return GS_SHELL_MODE_LOADING;

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	if (g_strcmp0 (adw_leaflet_get_visible_child_name (shell->details_leaflet), "details") == 0)
		return GS_SHELL_MODE_DETAILS;

	if (g_strcmp0 (adw_leaflet_get_visible_child_name (shell->main_leaflet), "main") == 0)
		name = adw_view_stack_get_visible_child_name (shell->stack_main);
	else
		name = adw_view_stack_get_visible_child_name (shell->stack_sub);
G_GNUC_END_IGNORE_DEPRECATIONS

	for (gsize i = 0; i < G_N_ELEMENTS (page_name); i++)
		if (g_strcmp0 (page_name[i], name) == 0)
			return i;

	g_assert_not_reached ();
}

const gchar *
gs_shell_get_mode_string (GsShell *shell)
{
	GsShellMode mode = gs_shell_get_mode (shell);
	return page_name[mode];
}

void
gs_shell_install (GsShell *shell, GsApp *app, GsShellInteraction interaction)
{
	save_back_entry (shell);
	gs_shell_change_mode (shell, GS_SHELL_MODE_DETAILS,
			      (gpointer) app, TRUE);
	gs_page_install_app (shell->pages[GS_SHELL_MODE_DETAILS], app, interaction, shell->cancellable);
}

void
gs_shell_uninstall (GsShell *shell, GsApp *app)
{
	save_back_entry (shell);
	gs_shell_change_mode (shell, GS_SHELL_MODE_DETAILS, (gpointer) app, TRUE);
	gs_page_remove_app (shell->pages[GS_SHELL_MODE_DETAILS], app, shell->cancellable);
}

void
gs_shell_show_installed_updates (GsShell *shell)
{
	GtkWidget *dialog;

	dialog = gs_update_dialog_new (shell->plugin_loader);

	gs_shell_modal_dialog_present (shell, GTK_WINDOW (dialog));
}

void
gs_shell_show_sources (GsShell *shell)
{
	GtkWidget *dialog;

	dialog = gs_repos_dialog_new (GTK_WINDOW (shell), shell->plugin_loader);
	gs_shell_modal_dialog_present (shell, GTK_WINDOW (dialog));
}

void
gs_shell_show_prefs (GsShell *shell)
{
	GtkWidget *dialog;

	dialog = gs_prefs_dialog_new (GTK_WINDOW (shell), shell->plugin_loader);
	gs_shell_modal_dialog_present (shell, GTK_WINDOW (dialog));
}

void
gs_shell_show_app (GsShell *shell, GsApp *app)
{
	save_back_entry (shell);
	gs_shell_change_mode (shell, GS_SHELL_MODE_DETAILS, app, TRUE);
	gs_shell_activate (shell);
}

void
gs_shell_show_category (GsShell *shell, GsCategory *category)
{
	save_back_entry (shell);
	gs_shell_change_mode (shell, GS_SHELL_MODE_CATEGORY, category, TRUE);
}

void gs_shell_show_extras_search (GsShell *shell, const gchar *mode, gchar **resources, const gchar *desktop_id, const gchar *ident)
{
	save_back_entry (shell);
	gs_extras_page_search (GS_EXTRAS_PAGE (shell->pages[GS_SHELL_MODE_EXTRAS]), mode, resources, desktop_id, ident);
	gs_shell_change_mode (shell, GS_SHELL_MODE_EXTRAS, NULL, TRUE);
	gs_shell_activate (shell);
}

void
gs_shell_show_search (GsShell *shell, const gchar *search)
{
	save_back_entry (shell);
	gs_shell_change_mode (shell, GS_SHELL_MODE_SEARCH,
			      (gpointer) search, TRUE);
}

void
gs_shell_show_local_file (GsShell *shell, GFile *file)
{
	g_autoptr(GsApp) app = gs_app_new (NULL);
	save_back_entry (shell);
	gs_app_set_local_file (app, file);
	gs_shell_change_mode (shell, GS_SHELL_MODE_DETAILS,
			      (gpointer) app, TRUE);
	gs_shell_activate (shell);
}

/**
 * gs_shell_show_metainfo:
 * @shell: a #GsShell
 * @file: path to a metainfo file to display
 *
 * Open a metainfo file and display it on the details page as if it were
 * published in a repository configured on the system.
 *
 * This is intended for app developers to be able to test their metainfo files
 * locally.
 *
 * Since: 42
 */
void
gs_shell_show_metainfo (GsShell *shell, GFile *file)
{
	g_autoptr(GsApp) app = gs_app_new (NULL);

	g_return_if_fail (GS_IS_SHELL (shell));
	g_return_if_fail (G_IS_FILE (file));
	save_back_entry (shell);
	gs_app_set_metadata (app, "LingmoAppstore::show-metainfo", "1");
	gs_app_set_local_file (app, file);
	gs_shell_change_mode (shell, GS_SHELL_MODE_DETAILS,
			      (gpointer) app, TRUE);
	gs_shell_activate (shell);
}

void
gs_shell_show_search_result (GsShell *shell, const gchar *id, const gchar *search)
{
	save_back_entry (shell);
	gs_search_page_set_appid_to_show (GS_SEARCH_PAGE (shell->pages[GS_SHELL_MODE_SEARCH]), id);
	gs_shell_change_mode (shell, GS_SHELL_MODE_SEARCH,
			      (gpointer) search, TRUE);
}

void
gs_shell_show_uri (GsShell *shell, const gchar *url)
{
	gs_show_uri (GTK_WINDOW (shell), url);
}

/**
 * gs_shell_get_is_narrow:
 * @shell: a #GsShell
 *
 * Get the value of #GsShell:is-narrow.
 *
 * Returns: %TRUE if the window is in narrow mode, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_shell_get_is_narrow (GsShell *shell)
{
	g_return_val_if_fail (GS_IS_SHELL (shell), FALSE);

	return shell->is_narrow;
}

static gint
gs_shell_get_allocation_width (GsShell *self)
{
	return self->allocation_width;
}

static void
gs_shell_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsShell *shell = GS_SHELL (object);

	switch ((GsShellProperty) prop_id) {
	case PROP_IS_NARROW:
		g_value_set_boolean (value, gs_shell_get_is_narrow (shell));
		break;
	case PROP_ALLOCATION_WIDTH:
		g_value_set_int (value, gs_shell_get_allocation_width (shell));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_shell_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	switch ((GsShellProperty) prop_id) {
	case PROP_IS_NARROW:
	case PROP_ALLOCATION_WIDTH:
		/* Read only. */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_shell_dispose (GObject *object)
{
	GsShell *shell = GS_SHELL (object);

	g_clear_object (&shell->sub_page_header_title_binding);

	if (shell->back_entry_stack != NULL) {
		g_queue_free_full (shell->back_entry_stack, (GDestroyNotify) free_back_entry);
		shell->back_entry_stack = NULL;
	}
	g_clear_object (&shell->cancellable);
	g_clear_object (&shell->plugin_loader);
	g_clear_object (&shell->header_start_widget);
	g_clear_object (&shell->header_end_widget);
	g_clear_object (&shell->sub_header_end_widget);
	g_clear_object (&shell->page);
	g_clear_pointer (&shell->modal_dialogs, g_ptr_array_unref);
	g_clear_object (&shell->settings);

#ifdef HAVE_MOGWAI
	if (shell->scheduler != NULL) {
		if (shell->scheduler_invalidated_handler > 0)
			g_signal_handler_disconnect (shell->scheduler,
						     shell->scheduler_invalidated_handler);

		if (shell->scheduler_held)
			mwsc_scheduler_release_async (shell->scheduler,
						      NULL,
						      scheduler_release_cb,
						      g_object_ref (shell));
		else
			g_clear_object (&shell->scheduler);
	}
#endif  /* HAVE_MOGWAI */

	G_OBJECT_CLASS (gs_shell_parent_class)->dispose (object);
}

static gboolean
allocation_changed_cb (gpointer user_data)
{
	GsShell *shell = GS_SHELL (user_data);
	gint width;
	gboolean is_narrow;

	width = gtk_widget_get_width (GTK_WIDGET (shell));
	is_narrow = width <= NARROW_WIDTH_THRESHOLD;

	if (shell->is_narrow != is_narrow) {
		shell->is_narrow = is_narrow;
		g_object_notify_by_pspec (G_OBJECT (shell), obj_props[PROP_IS_NARROW]);
	}

	if (shell->allocation_width != width) {
		shell->allocation_width = width;
		g_object_notify_by_pspec (G_OBJECT (shell), obj_props[PROP_ALLOCATION_WIDTH]);
	}

	shell->allocation_changed_cb_id = 0;

	if (is_narrow)
		gtk_widget_add_css_class (GTK_WIDGET (shell), "narrow");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (shell), "narrow");

	return G_SOURCE_REMOVE;
}

static void
gs_shell_size_allocate (GtkWidget *widget,
                        gint       width,
                        gint       height,
                        gint       baseline)
{
	GsShell *shell = GS_SHELL (widget);

	GTK_WIDGET_CLASS (gs_shell_parent_class)->size_allocate (widget,
								 width,
								 height,
								 baseline);

	/* Delay updating is-narrow so children can adapt to it, which isn't
	 * possible during the widget's allocation phase as it would break their
	 * size request.
	 */
	if (shell->allocation_changed_cb_id == 0)
		shell->allocation_changed_cb_id = g_idle_add (allocation_changed_cb, shell);
}

static void
gs_shell_class_init (GsShellClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_shell_get_property;
	object_class->set_property = gs_shell_set_property;
	object_class->dispose = gs_shell_dispose;

	widget_class->size_allocate = gs_shell_size_allocate;

	/**
	 * GsShell:is-narrow:
	 *
	 * Whether the window is in narrow mode.
	 *
	 * Pages can track this property to adapt to the available width.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IS_NARROW] =
		g_param_spec_boolean ("is-narrow", NULL, NULL,
				      FALSE,
				      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsShell:allocation-width:
	 *
	 * The last allocation width for the window.
	 *
	 * The pages can track this property, possibly in combination with the #GsShell:is-narrow,
	 * to adapt its content to the available width.
	 *
	 * Since: 43
	 */
	obj_props[PROP_ALLOCATION_WIDTH] =
		g_param_spec_int ("allocation-width", NULL, NULL,
				  G_MININT, G_MAXINT, 0,
				  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	signals [SIGNAL_LOADED] =
		g_signal_new ("loaded",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-shell.ui");

	gtk_widget_class_bind_template_child (widget_class, GsShell, main_header);
	gtk_widget_class_bind_template_child (widget_class, GsShell, main_leaflet);
	gtk_widget_class_bind_template_child (widget_class, GsShell, details_header);
	gtk_widget_class_bind_template_child (widget_class, GsShell, details_leaflet);
	gtk_widget_class_bind_template_child (widget_class, GsShell, stack_loading);
	gtk_widget_class_bind_template_child (widget_class, GsShell, stack_main);
	gtk_widget_class_bind_template_child (widget_class, GsShell, stack_sub);
	gtk_widget_class_bind_template_child (widget_class, GsShell, updates_paused_banner);
	gtk_widget_class_bind_template_child (widget_class, GsShell, search_button);
	gtk_widget_class_bind_template_child (widget_class, GsShell, entry_search);
	gtk_widget_class_bind_template_child (widget_class, GsShell, search_bar);
	gtk_widget_class_bind_template_child (widget_class, GsShell, button_back);
	gtk_widget_class_bind_template_child (widget_class, GsShell, button_back2);
	gtk_widget_class_bind_template_child (widget_class, GsShell, toast_overlay);
	gtk_widget_class_bind_template_child (widget_class, GsShell, primary_menu);
	gtk_widget_class_bind_template_child (widget_class, GsShell, sub_header);
	gtk_widget_class_bind_template_child (widget_class, GsShell, sub_page_header_title);

	gtk_widget_class_bind_template_child_full (widget_class, "overview_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_OVERVIEW]));
	gtk_widget_class_bind_template_child_full (widget_class, "updates_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_UPDATES]));
	gtk_widget_class_bind_template_child_full (widget_class, "installed_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_INSTALLED]));
	gtk_widget_class_bind_template_child_full (widget_class, "moderate_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_MODERATE]));
	gtk_widget_class_bind_template_child_full (widget_class, "loading_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_LOADING]));
	gtk_widget_class_bind_template_child_full (widget_class, "search_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_SEARCH]));
	gtk_widget_class_bind_template_child_full (widget_class, "details_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_DETAILS]));
	gtk_widget_class_bind_template_child_full (widget_class, "category_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_CATEGORY]));
	gtk_widget_class_bind_template_child_full (widget_class, "extras_page", FALSE, G_STRUCT_OFFSET (GsShell, pages[GS_SHELL_MODE_EXTRAS]));

	gtk_widget_class_bind_template_callback (widget_class, gs_shell_main_window_mapped_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_shell_main_window_realized_cb);
	gtk_widget_class_bind_template_callback (widget_class, main_window_closed_cb);
	gtk_widget_class_bind_template_callback (widget_class, window_key_pressed_cb);
	gtk_widget_class_bind_template_callback (widget_class, window_keypress_handler);
	gtk_widget_class_bind_template_callback (widget_class, window_button_pressed_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_shell_details_back_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_shell_back_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_overview_page_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, updates_page_notify_counter_cb);
	gtk_widget_class_bind_template_callback (widget_class, category_page_app_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, search_bar_search_mode_enabled_changed_cb);
	gtk_widget_class_bind_template_callback (widget_class, search_changed_handler);
	gtk_widget_class_bind_template_callback (widget_class, stack_notify_visible_child_cb);
	gtk_widget_class_bind_template_callback (widget_class, initial_refresh_done);
	gtk_widget_class_bind_template_callback (widget_class, gs_shell_details_page_metainfo_loaded_cb);
	gtk_widget_class_bind_template_callback (widget_class, details_page_app_clicked_cb);

	gtk_widget_class_add_binding_action (widget_class, GDK_KEY_q, GDK_CONTROL_MASK, "window.close", NULL);
}

static void
gs_shell_init (GsShell *shell)
{
	g_type_ensure (GS_TYPE_CATEGORY_PAGE);
	g_type_ensure (GS_TYPE_DETAILS_PAGE);
	g_type_ensure (GS_TYPE_EXTRAS_PAGE);
	g_type_ensure (GS_TYPE_INSTALLED_PAGE);
	g_type_ensure (GS_TYPE_LOADING_PAGE);
	g_type_ensure (GS_TYPE_MODERATE_PAGE);
	g_type_ensure (GS_TYPE_OVERVIEW_PAGE);
	g_type_ensure (GS_TYPE_SEARCH_PAGE);
	g_type_ensure (GS_TYPE_UPDATES_PAGE);
	g_type_ensure (GS_TYPE_UPDATES_PAUSED_BANNER);

	gtk_widget_init_template (GTK_WIDGET (shell));

	gtk_search_bar_connect_entry (GTK_SEARCH_BAR (shell->search_bar), GTK_EDITABLE (shell->entry_search));

	shell->back_entry_stack = g_queue_new ();
	shell->modal_dialogs = g_ptr_array_new ();
}

GsShell *
gs_shell_new (void)
{
	return GS_SHELL (g_object_new (GS_TYPE_SHELL, NULL));
}

GsAppQueryLicenseType
gs_shell_get_query_license_type (GsShell *self)
{
	g_return_val_if_fail (GS_IS_SHELL (self), GS_APP_QUERY_LICENSE_ANY);

	if (g_settings_get_boolean (self->settings, "show-only-free-apps"))
		return GS_APP_QUERY_LICENSE_FOSS;
	return GS_APP_QUERY_LICENSE_ANY;
}

GsAppQueryDeveloperVerifiedType
gs_shell_get_query_developer_verified_type (GsShell *self)
{
	g_return_val_if_fail (GS_IS_SHELL (self), GS_APP_QUERY_DEVELOPER_VERIFIED_ANY);

	if (g_settings_get_boolean (self->settings, "show-only-verified-apps"))
		return GS_APP_QUERY_DEVELOPER_VERIFIED_ONLY;
	return GS_APP_QUERY_DEVELOPER_VERIFIED_ANY;
}
