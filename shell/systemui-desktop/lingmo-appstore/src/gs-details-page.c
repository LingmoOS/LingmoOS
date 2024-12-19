/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <locale.h>
#include <string.h>
#include <glib/gi18n.h>

#include "lib/gs-appstream.h"

#include "gs-common.h"
#include "gs-utils.h"

#include "gs-details-page.h"
#include "gs-app-addon-row.h"
#include "gs-app-context-bar.h"
#include "gs-app-reviews-dialog.h"
#include "gs-app-translation-dialog.h"
#include "gs-app-version-history-row.h"
#include "gs-app-version-history-dialog.h"
#include "gs-description-box.h"
#include "gs-license-tile.h"
#include "gs-origin-popover-row.h"
#include "gs-progress-button.h"
#include "gs-screenshot-carousel.h"
#include "gs-star-widget.h"
#include "gs-summary-tile.h"
#include "gs-review-histogram.h"
#include "gs-review-dialog.h"
#include "gs-review-row.h"

/* the number of reviews to show before clicking the 'More Reviews' button */
#define SHOW_NR_REVIEWS_INITIAL		4

/* How many other developer apps can be shown; should be divisible by 3 and 2,
   to catch full width and smaller width without bottom gap */
#define N_DEVELOPER_APPS 18

#define GS_DETAILS_PAGE_REFINE_FLAGS	GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_CONTENT_RATING | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_DEVELOPER_NAME | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_HISTORY | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_PERMISSIONS | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROJECT_GROUP | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_RELATED | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_RUNTIME | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_SCREENSHOTS | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_SETUP_ACTION | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE_DATA | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_URL | \
					GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION

static void gs_details_page_refresh_addons (GsDetailsPage *self);
static void gs_details_page_refresh_all (GsDetailsPage *self);
static void gs_details_page_refresh_progress (GsDetailsPage *self);
static void gs_details_page_refresh_buttons (GsDetailsPage *self);
static void gs_details_page_app_refine_cb (GObject *source, GAsyncResult *res, gpointer user_data);

typedef enum {
	GS_DETAILS_PAGE_STATE_LOADING,
	GS_DETAILS_PAGE_STATE_READY,
	GS_DETAILS_PAGE_STATE_FAILED
} GsDetailsPageState;

struct _GsDetailsPage
{
	GsPage			 parent_instance;

	GsPluginLoader		*plugin_loader;
	GCancellable		*cancellable;
	GCancellable		*app_cancellable;
	GsApp			*app;
	GsApp			*app_local_file;
	GsShell			*shell;
	gboolean		 show_all_reviews;
	GSettings		*settings;
	GsOdrsProvider		*odrs_provider;  /* (nullable) (owned), NULL if reviews are disabled */
	GAppInfoMonitor		*app_info_monitor; /* (owned) */
	gchar		       **packaging_format_preference; /* (owned) */
	GtkWidget		*app_reviews_dialog;
	GtkCssProvider		*origin_css_provider; /* (nullable) (owned) */
	GtkCssProvider		*developer_verified_image_css_provider; /* (nullable) (owned) */
	GtkCssProvider		*developer_verified_label_css_provider; /* (nullable) (owned) */
	gboolean		 origin_by_packaging_format; /* when TRUE, change the 'app' to the most preferred
								packaging format when the alternatives are found */
	gboolean		 is_narrow;
	gboolean		 title_visible;

	guint			 job_manager_watch_id;

	GtkWidget		*application_details_icon;
	GtkWidget		*application_details_summary;
	GtkWidget		*application_details_title;
	GtkWidget		*box_addons;
	GtkWidget		*box_details;
	GtkWidget		*box_details_description;
	GtkWidget		*box_details_header;
	GtkWidget		*box_details_header_not_icon;
	GtkWidget		*label_webapp_warning;
	GtkWidget		*star;
	GtkWidget		*label_review_count;
	GtkWidget		*screenshot_carousel;
	GtkWidget		*button_details_launch;
	GtkStack		*links_stack;
	GtkWidget		*label_no_metadata_info;
	AdwActionRow		*project_website_row;
	AdwActionRow		*donate_row;
	AdwActionRow		*translate_row;
	AdwActionRow		*report_an_issue_row;
	AdwActionRow		*help_row;
	AdwActionRow		*contact_row;
	GtkWidget		*button_install;
	GtkWidget		*button_update;
	GtkWidget		*button_remove;
	GsProgressButton	*button_cancel;
	GtkWidget		*infobar_details_eol;
	GtkWidget		*label_eol;
	GtkWidget		*infobar_details_problems_label;
	GtkWidget		*infobar_details_app_norepo;
	GtkWidget		*infobar_details_app_repo;
	GtkWidget		*infobar_details_package_baseos;
	GtkWidget		*label_package_baseos;
	GtkWidget		*infobar_details_repo;
	GtkWidget		*infobar_app_data;
	GtkWidget		*infobar_app_data_label;
	GtkWidget		*label_progress_percentage;
	GtkWidget		*label_progress_status;
	GsAppContextBar		*context_bar;
	GtkLabel		*developer_name_label;
	GtkWidget		*developer_verified_image;
	GtkWidget		*developer_verified_label;
	AdwStatusPage           *page_failed;
	GtkWidget		*list_box_addons;
	GtkWidget		*list_box_featured_review;
	GtkWidget		*list_box_reviews_summary;
	GtkWidget		*list_box_version_history;
	GtkWidget		*row_latest_version;
	GtkWidget		*version_history_button;
	GtkWidget		*box_reviews;
	GtkWidget		*box_reviews_internal;
	GtkWidget		*histogram;
	GtkWidget		*histogram_row;
	GtkWidget		*button_review;
	GtkWidget		*scrolledwindow_details;
	GtkWidget		*spinner_details;
	GtkWidget		*stack_details;
	GtkWidget		*box_with_source;
	GtkWidget		*origin_popover;
	GtkWidget		*origin_popover_list_box;
	GtkWidget		*origin_box;
	GtkWidget		*origin_packaging_image;
	GtkWidget		*origin_packaging_label;
	GtkWidget		*box_license;
	GsLicenseTile		*license_tile;
	AdwBanner               *translation_banner;
	GtkWidget		*developer_apps_heading;
	GtkWidget		*box_developer_apps;
	gchar			*last_developer_name;
};

G_DEFINE_TYPE (GsDetailsPage, gs_details_page, GS_TYPE_PAGE)

enum {
	SIGNAL_METAINFO_LOADED,
	SIGNAL_APP_CLICKED,
	SIGNAL_LAST
};

typedef enum {
	PROP_ODRS_PROVIDER = 1,
	PROP_IS_NARROW,
	/* Override properties: */
	PROP_TITLE,
} GsDetailsPageProperty;

static GParamSpec *obj_props[PROP_IS_NARROW + 1] = { NULL, };
static guint signals[SIGNAL_LAST] = { 0 };

static void
gs_details_page_cancel_cb (GCancellable *cancellable,
			   GsDetailsPage *self)
{
	if (self->app_reviews_dialog) {
		gtk_window_destroy (GTK_WINDOW (self->app_reviews_dialog));
		g_clear_object (&self->app_reviews_dialog);
	}
}

static GsDetailsPageState
gs_details_page_get_state (GsDetailsPage *self)
{
	const gchar *visible_child_name = gtk_stack_get_visible_child_name (GTK_STACK (self->stack_details));

	if (g_str_equal (visible_child_name, "spinner"))
		return GS_DETAILS_PAGE_STATE_LOADING;
	else if (g_str_equal (visible_child_name, "ready"))
		return GS_DETAILS_PAGE_STATE_READY;
	else if (g_str_equal (visible_child_name, "failed"))
		return GS_DETAILS_PAGE_STATE_FAILED;
	else
		g_assert_not_reached ();
}

static void
gs_details_page_set_state (GsDetailsPage *self,
                           GsDetailsPageState state)
{
	if (state == gs_details_page_get_state (self))
		return;

	/* spinner */
	switch (state) {
	case GS_DETAILS_PAGE_STATE_LOADING:
		gtk_spinner_start (GTK_SPINNER (self->spinner_details));
		break;
	case GS_DETAILS_PAGE_STATE_READY:
	case GS_DETAILS_PAGE_STATE_FAILED:
		gtk_spinner_stop (GTK_SPINNER (self->spinner_details));
		break;
	default:
		g_assert_not_reached ();
	}

	/* stack */
	switch (state) {
	case GS_DETAILS_PAGE_STATE_LOADING:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_details), "spinner");
		break;
	case GS_DETAILS_PAGE_STATE_READY:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_details), "ready");
		break;
	case GS_DETAILS_PAGE_STATE_FAILED:
		gtk_stack_set_visible_child_name (GTK_STACK (self->stack_details), "failed");
		break;
	default:
		g_assert_not_reached ();
	}

	/* the page title will have changed */
	g_object_notify (G_OBJECT (self), "title");
}

static gboolean
gs_details_page_app_has_pending_action (GsDetailsPage *self)
{
	GsJobManager *job_manager = gs_plugin_loader_get_job_manager (self->plugin_loader);
	g_autoptr(GPtrArray) pending_jobs_for_app = NULL;  /* (element-type GsPluginJob) */
	GsAppState app_state = gs_app_get_state (self->app);

	/* sanitize the pending state change by verifying we're in one of the
	 * expected states */
	if (app_state != GS_APP_STATE_AVAILABLE &&
	    app_state != GS_APP_STATE_UPDATABLE_LIVE &&
	    app_state != GS_APP_STATE_UPDATABLE &&
	    app_state != GS_APP_STATE_QUEUED_FOR_INSTALL)
		return FALSE;

	pending_jobs_for_app = gs_job_manager_get_pending_jobs_for_app (job_manager, self->app);

	return (gs_app_get_pending_action (self->app) != GS_PLUGIN_ACTION_UNKNOWN) ||
	       (gs_app_get_state (self->app) == GS_APP_STATE_QUEUED_FOR_INSTALL) ||
	       pending_jobs_for_app->len > 0;
}

static void
gs_details_page_update_origin_button (GsDetailsPage *self,
				      gboolean sensitive)
{
	const gchar *packaging_icon;
	const gchar *packaging_base_css_color;
	g_autofree gchar *css = NULL;
	g_autofree gchar *origin_ui = NULL;

	if (self->app == NULL ||
	    gs_shell_get_mode (self->shell) != GS_SHELL_MODE_DETAILS) {
		gtk_widget_set_visible (self->origin_box, FALSE);
		return;
	}

	origin_ui = gs_app_dup_origin_ui (self->app, FALSE);
	gtk_label_set_text (GTK_LABEL (self->origin_packaging_label), origin_ui != NULL ? origin_ui : "");

	gtk_widget_set_sensitive (self->origin_box, sensitive);
	gtk_widget_set_visible (self->origin_box, TRUE);

	packaging_icon = gs_app_get_metadata_item (self->app, "LingmoAppstore::PackagingIcon");
	if (packaging_icon == NULL)
		packaging_icon = "package-x-generic-symbolic";

	packaging_base_css_color = gs_app_get_metadata_item (self->app, "LingmoAppstore::PackagingBaseCssColor");

	gtk_image_set_from_icon_name (GTK_IMAGE (self->origin_packaging_image), packaging_icon);

	if (packaging_base_css_color != NULL)
		css = g_strdup_printf ("color: @%s;\n", packaging_base_css_color);

	gs_utils_widget_set_css (self->origin_packaging_image, &self->origin_css_provider, css);
	gs_utils_widget_set_css (self->developer_verified_image, &self->developer_verified_image_css_provider, css);
	gs_utils_widget_set_css (self->developer_verified_label, &self->developer_verified_label_css_provider, css);
}

static void
gs_details_page_switch_to (GsPage *page)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (page);
	GtkAdjustment *adj;

	if (gs_shell_get_mode (self->shell) != GS_SHELL_MODE_DETAILS) {
		g_warning ("Called switch_to(details) when in mode %s",
			   gs_shell_get_mode_string (self->shell));
		return;
	}

	/* Always refresh other developer apps */
	g_clear_pointer (&self->last_developer_name, g_free);

	/* hide the alternates for now until the query is complete */
	gtk_widget_set_visible (self->origin_box, FALSE);

	/* not set, perhaps file-to-app */
	if (self->app == NULL)
		return;

	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_details));
	gtk_adjustment_set_value (adj, gtk_adjustment_get_lower (adj));

	gs_grab_focus_when_mapped (self->scrolledwindow_details);
}

static void
gs_details_page_refresh_progress (GsDetailsPage *self)
{
	GsJobManager *job_manager = gs_plugin_loader_get_job_manager (self->plugin_loader);
	guint percentage;
	GsAppState state;

	/* cancel button */
	state = gs_app_get_state (self->app);
	switch (state) {
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (GTK_WIDGET (self->button_cancel), TRUE);
		/* If the app is installing, the user can only cancel it if
		 * 1) They haven't already, and
		 * 2) the plugin hasn't said that they can't, for example if a
		 *    package manager has already gone 'too far'
		 */
		gtk_widget_set_sensitive (GTK_WIDGET (self->button_cancel),
					  !g_cancellable_is_cancelled (self->app_cancellable) &&
					   gs_app_get_allow_cancel (self->app));
		break;
	default:
		gtk_widget_set_visible (GTK_WIDGET (self->button_cancel), FALSE);
		break;
	}
	if (gs_details_page_app_has_pending_action (self)) {
		gtk_widget_set_visible (GTK_WIDGET (self->button_cancel), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (self->button_cancel),
					  !g_cancellable_is_cancelled (self->app_cancellable) &&
					  gs_app_get_allow_cancel (self->app));
	}

	/* progress status label */
	switch (state) {
	case GS_APP_STATE_REMOVING:
		gtk_widget_set_visible (self->label_progress_status, TRUE);
		gtk_label_set_label (GTK_LABEL (self->label_progress_status),
				     _("Removing…"));
		break;
	case GS_APP_STATE_INSTALLING:
		gtk_widget_set_visible (self->label_progress_status, TRUE);
		gtk_label_set_label (GTK_LABEL (self->label_progress_status),
				     _("Installing"));
		break;
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (self->label_progress_status, TRUE);
		gtk_label_set_label (GTK_LABEL (self->label_progress_status),
				     _("Downloading"));
		break;
	case GS_APP_STATE_PENDING_INSTALL:
		gtk_widget_set_visible (self->label_progress_status, TRUE);
		if (gs_app_has_quirk (self->app, GS_APP_QUIRK_NEEDS_REBOOT))
			gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Requires restart to finish install"));
		else
			gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Pending install"));
		break;
	case GS_APP_STATE_PENDING_REMOVE:
		gtk_widget_set_visible (self->label_progress_status, TRUE);
		if (gs_app_has_quirk (self->app, GS_APP_QUIRK_NEEDS_REBOOT))
			gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Requires restart to finish remove"));
		else
			gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Pending remove"));
		break;

	default:
		gtk_widget_set_visible (self->label_progress_status, FALSE);
		break;
	}
	if (gs_details_page_app_has_pending_action (self)) {
		GsPluginAction action = gs_app_get_pending_action (self->app);
		gtk_widget_set_visible (self->label_progress_status, TRUE);

		if (action == GS_PLUGIN_ACTION_INSTALL) {
			gtk_label_set_label (GTK_LABEL (self->label_progress_status),
					     /* TRANSLATORS: This is a label on top of the app's progress
					      * bar to inform the user that the app should be installed soon */
					     _("Pending installation…"));
		} else if (gs_job_manager_app_has_pending_job_type (job_manager, self->app, GS_TYPE_PLUGIN_JOB_UPDATE_APPS) ||
			   action == GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD) {
			gtk_label_set_label (GTK_LABEL (self->label_progress_status),
					     /* TRANSLATORS: This is a label on top of the app's progress
					      * bar to inform the user that the app should be updated soon */
					     _("Pending update…"));
		} else {
			gtk_widget_set_visible (self->label_progress_status, FALSE);
		}
	}

	/* percentage bar */
	switch (state) {
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_DOWNLOADING:
		percentage = gs_app_get_progress (self->app);
		if (percentage == GS_APP_PROGRESS_UNKNOWN) {
			if (state == GS_APP_STATE_DOWNLOADING) {
				/* Translators: This string is shown when downloading an app before install. */
				gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Downloading…"));
			} else if (state == GS_APP_STATE_INSTALLING) {
				/* Translators: This string is shown when preparing to download and install an app. */
				gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Preparing…"));
			} else {
				/* Translators: This string is shown when uninstalling an app. */
				gtk_label_set_label (GTK_LABEL (self->label_progress_status), _("Uninstalling…"));
			}

			gtk_widget_set_visible (self->label_progress_status, TRUE);
			gtk_widget_set_visible (self->label_progress_percentage, FALSE);
			gs_progress_button_set_progress (self->button_cancel, percentage);
			gs_progress_button_set_show_progress (self->button_cancel, TRUE);
			break;
		} else if (percentage <= 100) {
			g_autofree gchar *str = g_strdup_printf ("%u%%", percentage);
			gtk_label_set_label (GTK_LABEL (self->label_progress_percentage), str);
			gtk_widget_set_visible (self->label_progress_percentage, TRUE);
			gs_progress_button_set_progress (self->button_cancel, percentage);
			gs_progress_button_set_show_progress (self->button_cancel, TRUE);
			break;
		}
		/* FALLTHROUGH */
	default:
		gtk_widget_set_visible (self->label_progress_percentage, FALSE);
		gs_progress_button_set_show_progress (self->button_cancel, FALSE);
		gs_progress_button_set_progress (self->button_cancel, 0);
		break;
	}
	if (gs_details_page_app_has_pending_action (self)) {
		gs_progress_button_set_progress (self->button_cancel, 0);
		gs_progress_button_set_show_progress (self->button_cancel, TRUE);
	}
}

static gboolean
gs_details_page_refresh_progress_idle (gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	gs_details_page_refresh_progress (self);
	g_object_unref (self);
	return G_SOURCE_REMOVE;
}

static void
gs_details_page_progress_changed_cb (GsApp *app,
                                     GParamSpec *pspec,
                                     GsDetailsPage *self)
{
	g_idle_add (gs_details_page_refresh_progress_idle, g_object_ref (self));
}

static gboolean
gs_details_page_allow_cancel_changed_idle (gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	gtk_widget_set_sensitive (GTK_WIDGET (self->button_cancel),
				  gs_app_get_allow_cancel (self->app));
	g_object_unref (self);
	return G_SOURCE_REMOVE;
}

static void
gs_details_page_allow_cancel_changed_cb (GsApp *app,
                                                    GParamSpec *pspec,
                                                    GsDetailsPage *self)
{
	g_idle_add (gs_details_page_allow_cancel_changed_idle,
		    g_object_ref (self));
}

static gboolean
gs_details_page_refresh_idle (gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);

	if (gs_shell_get_mode (self->shell) == GS_SHELL_MODE_DETAILS) {
		/* update widgets */
		gs_details_page_refresh_all (self);
	}

	g_object_unref (self);
	return G_SOURCE_REMOVE;
}

static void
gs_details_page_notify_state_changed_cb (GsApp *app,
                                         GParamSpec *pspec,
                                         GsDetailsPage *self)
{
	g_idle_add (gs_details_page_refresh_idle, g_object_ref (self));
}

static void
gs_details_page_refresh_app_data_info (GsDetailsPage *self)
{
	g_autofree gchar *dir = NULL;
	gboolean visible = TRUE;
	AsBundleKind bundle_kind;

	if (self->app == NULL || gs_app_is_installed (self->app)) {
		gtk_widget_set_visible (self->infobar_app_data, FALSE);
		return;
	}

	dir = gs_utils_get_app_data_dir (self->app);
	if (dir == NULL) {
		gtk_widget_set_visible (self->infobar_app_data, FALSE);
		return;
	}

	bundle_kind = gs_app_get_bundle_kind (self->app);

	/* Multiple remotes can provide the app, thus check whether
	   any alternative is a flatpak and is installed. */
	for (GtkWidget *child = gtk_widget_get_first_child (self->origin_popover_list_box);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		GsApp *alternative_app;

		g_assert (GS_IS_ORIGIN_POPOVER_ROW (child));

		alternative_app = gs_origin_popover_row_get_app (GS_ORIGIN_POPOVER_ROW (child));
		if (gs_app_get_bundle_kind (alternative_app) == bundle_kind &&
		    gs_app_is_installed (alternative_app)) {
			visible = FALSE;
			break;
		}
	}

	if (visible) {
		g_autofree gchar *tmp = NULL;
		/* Translators: the "%s" is replaced with an app name */
		tmp = g_strdup_printf (_("%s is not installed, but it still has data present."), gs_app_get_name (self->app));
		gtk_label_set_label (GTK_LABEL (self->infobar_app_data_label), tmp);
	}

	gtk_widget_set_visible (self->infobar_app_data, visible);
}

static void
gs_details_page_app_data_clear_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	if (gs_utils_remove_app_data_dir (self->app, self->plugin_loader))
		gs_details_page_refresh_app_data_info (self);
}

static void
job_manager_jobs_changed_cb (GsJobManager *job_manager,
                             GsPluginJob  *job,
                             gpointer      user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);

	/* The set of pending jobs for self->app has changed, so update the UI. */
	gs_details_page_refresh_progress (self);
	gs_details_page_refresh_buttons (self);
}

static void
gs_details_page_link_row_activated_cb (AdwActionRow *row, GsDetailsPage *self)
{
	gs_shell_show_uri (self->shell, adw_action_row_get_subtitle (row));
}

static void
gs_details_page_license_tile_get_involved_activated_cb (GsLicenseTile *license_tile,
							GsDetailsPage *self)
{
	g_autofree gchar *license_url = NULL;
	const gchar *uri = NULL;

	if (gs_app_get_license_is_free (self->app)) {
#if AS_CHECK_VERSION(0, 15, 3)
		uri = gs_app_get_url (self->app, AS_URL_KIND_CONTRIBUTE);
#endif
		if (uri == NULL)
			uri = gs_app_get_url (self->app, AS_URL_KIND_HOMEPAGE);
	} else {
		if (gs_app_get_license (self->app) == NULL) {
			uri = "https://github.com/LingmoOS/lingmo-appstore/-/wikis/software-metadata#license";
		} else {
			license_url = as_get_license_url (gs_app_get_license (self->app));

			if (license_url != NULL && *license_url != '\0') {
				uri = license_url;
			} else {
				/* Page to explain the differences between FOSS and proprietary
				 * software. This is a page on the lingmo-appstore wiki for now,
				 * so that we can update the content independently of the release
				 * cycle. Likely, we will link to a more authoritative source
				 * to explain the differences.
				 * Ultimately, we could ship a user manual page to explain the
				 * differences (so that it’s available offline), but that’s too
				 * much work for right now. */
				uri = "https://github.com/LingmoOS/lingmo-appstore/-/wikis/Appstore-licensing";
			}
		}
	}

	gs_shell_show_uri (self->shell, uri);
}

static void
gs_details_page_translation_banner_clicked_cb (GsDetailsPage *self)
{
	GtkWindow *window;

	window = GTK_WINDOW (gs_app_translation_dialog_new (self->app));
	gs_shell_modal_dialog_present (self->shell, window);
}

static void
gs_details_page_set_description (GsDetailsPage *self, const gchar *tmp)
{
	gs_description_box_set_text (GS_DESCRIPTION_BOX (self->box_details_description), tmp);
	gs_description_box_set_collapsed (GS_DESCRIPTION_BOX (self->box_details_description), TRUE);
	gtk_widget_set_visible (self->label_webapp_warning, gs_app_get_kind (self->app) == AS_COMPONENT_KIND_WEB_APP);
}

static gboolean
app_origin_equal (GsApp *a,
                  GsApp *b)
{
	g_autofree gchar *a_origin_ui = NULL, *b_origin_ui = NULL;
	GFile *a_local_file, *b_local_file;

	if (a == b)
		return TRUE;

	a_origin_ui = gs_app_dup_origin_ui (a, TRUE);
	b_origin_ui = gs_app_dup_origin_ui (b, TRUE);

	a_local_file = gs_app_get_local_file (a);
	b_local_file = gs_app_get_local_file (b);

	/* Compare all the fields used in GsOriginPopoverRow. */
	if (g_strcmp0 (a_origin_ui, b_origin_ui) != 0)
		return FALSE;

	if (!((a_local_file == NULL && b_local_file == NULL) ||
	      (a_local_file != NULL && b_local_file != NULL &&
	       g_file_equal (a_local_file, b_local_file))))
		return FALSE;

	if (g_strcmp0 (gs_app_get_origin_hostname (a),
		       gs_app_get_origin_hostname (b)) != 0)
		return FALSE;

	if (gs_app_get_bundle_kind (a) != gs_app_get_bundle_kind (b))
		return FALSE;

	if (gs_app_get_scope (a) != gs_app_get_scope (b))
		return FALSE;

	if (g_strcmp0 (gs_app_get_branch (a), gs_app_get_branch (b)) != 0)
		return FALSE;

	if (g_strcmp0 (gs_app_get_version (a), gs_app_get_version (b)) != 0)
		return FALSE;

	return TRUE;
}

static gint
gs_details_page_get_app_packaging_format_preference_index (GsDetailsPage *self,
							   GsApp *app)
{
	const gchar *packaging_format;
	guint packaging_format_len;

	/* Index 0 means unspecified packaging format in the preference array */
	if (self->packaging_format_preference == NULL)
		return 0;

	packaging_format = gs_app_get_packaging_format_raw (app);
	if (packaging_format == NULL)
		return 0;

	packaging_format_len = strlen (packaging_format);

	/* The preference can be defined either as the packaging format
	   on its own, like "rpm", or with an origin name, like "flatpak:flathub".
	   The packaging format can be empty too, then is prefered the origin,
	   like: ":system" prefers any "system" origin.*/
	for (guint i = 0; self->packaging_format_preference[i]; i++) {
		const gchar *preference = self->packaging_format_preference[i];
		if (preference[0] == ':') {
			const gchar *origin = gs_app_get_origin (app);
			if (origin != NULL &&
			    g_ascii_strcasecmp (origin, preference + 1) == 0)
				return (gint) i + 1;
		} else if (g_ascii_strncasecmp (preference, packaging_format, packaging_format_len) == 0) {
			if (preference[packaging_format_len] == '\0')
				return (gint) i + 1;
			if (preference[packaging_format_len] == ':') {
				const gchar *origin = gs_app_get_origin (app);
				if (origin != NULL &&
				    g_ascii_strcasecmp (origin, preference + packaging_format_len + 1) == 0)
					return (gint) i + 1;
			}
		}
	}

	return 0;
}

static gint
sort_by_packaging_format_preference (GsApp *app1,
				     GsApp *app2,
				     gpointer user_data)
{
	GsDetailsPage *self = user_data;
	gint index1, index2;

	index1 = gs_details_page_get_app_packaging_format_preference_index (self, app1);
	index2 = gs_details_page_get_app_packaging_format_preference_index (self, app2);

	if (index1 == index2) {
		g_autofree gchar *a1_origin = gs_app_dup_origin_ui (app1, TRUE);
		g_autofree gchar *a2_origin = gs_app_dup_origin_ui (app2, TRUE);

		return gs_utils_sort_strcmp (a1_origin, a2_origin);
	}

	/* Index 0 means unspecified packaging format in the preference array,
	   thus move these at the end. */
	if (index1 == 0 || index2 == 0)
		return index1 == 0 ? 1 : -1;

	return index1 - index2;
}

static void
gs_details_page_refresh_screenshots (GsDetailsPage *self)
{
	if (self->app != NULL) {
		gboolean is_online = gs_plugin_loader_get_network_available (self->plugin_loader);
		gboolean has_screenshots;

		gs_screenshot_carousel_load_screenshots (GS_SCREENSHOT_CAROUSEL (self->screenshot_carousel), self->app, is_online, NULL);
		has_screenshots = gs_screenshot_carousel_get_has_screenshots (GS_SCREENSHOT_CAROUSEL (self->screenshot_carousel));
		gtk_widget_set_visible (self->screenshot_carousel, has_screenshots);
	} else {
		gtk_widget_set_visible (self->screenshot_carousel, FALSE);
	}
}

static void _set_app (GsDetailsPage *self, GsApp *app);

static void
gs_details_page_get_alternates_cb (GObject *source_object,
                                   GAsyncResult *res,
                                   gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source_object);
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	gboolean instance_changed = FALSE;
	gboolean origin_by_packaging_format = self->origin_by_packaging_format;
	GtkWidget *first_row = NULL;
	GtkWidget *select_row = NULL;
	GtkWidget *origin_row_by_packaging_format = NULL;
	gint origin_row_by_packaging_format_index = 0;
	guint n_rows = 0;

	self->origin_by_packaging_format = FALSE;
	gs_widget_remove_all (self->origin_popover_list_box, (GsRemoveFunc) gtk_list_box_remove);

	/* Did we switch away from the page in the meantime? */
	if (!gs_page_is_active (GS_PAGE (self))) {
		gtk_widget_set_visible (self->origin_box, FALSE);
		return;
	}

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &error);
	if (list == NULL) {
		if (!g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) &&
		    !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning ("failed to get alternates: %s", error->message);
		gtk_widget_set_visible (self->origin_box, FALSE);
		return;
	}

	/* deduplicate the list; duplicates can get in the list if
	 * get_alternates() returns the old/new version of a renamed app, which
	 * happens to come from the same origin; see
	 * https://github.com/LingmoOS/lingmo-appstore/-/issues/1192
	 *
	 * This nested loop is OK as the origin list is normally only 2 or 3
	 * items long. */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *i_app = gs_app_list_index (list, i);
		gboolean did_remove = FALSE;

		for (guint j = i + 1; j < gs_app_list_length (list);) {
			GsApp *j_app = gs_app_list_index (list, j);

			if (app_origin_equal (i_app, j_app)) {
				gs_app_list_remove (list, j_app);
				did_remove = TRUE;
			} else {
				j++;
			}
		}

		/* Needed to catch cases when the same pointer is in the array multiple times,
		   interleaving with another pointer. The removal can skip the first occurrence
		   due to the g_ptr_array_remove() removing the first instance in the array,
		   which shifts the array content. */
		if (did_remove)
			i--;
	}

	/* add the local file to the list so that we can carry it over when
	 * switching between alternates */
	if (self->app_local_file != NULL) {
		if (gs_app_get_state (self->app_local_file) != GS_APP_STATE_INSTALLED &&
		    gs_app_get_local_file (self->app_local_file) != NULL) {
			gboolean already_in_list = FALSE;
			/* The app itself can be returned as an alternative too, thus check for it */
			for (guint i = 0; i < gs_app_list_length (list); i++) {
				GsApp *i_app = gs_app_list_index (list, i);
				if (app_origin_equal (i_app, self->app_local_file)) {
					already_in_list = TRUE;
					break;
				}
			}
			if (!already_in_list) {
				GtkWidget *row = gs_origin_popover_row_new (self->app_local_file);
				gtk_widget_set_visible (row, TRUE);
				gtk_list_box_append (GTK_LIST_BOX (self->origin_popover_list_box), row);
				first_row = row;
				select_row = row;
				n_rows++;
			}
		}

		/* Do not allow change of the app by the packaging format when it's a local file */
		origin_by_packaging_format = FALSE;
	}

	/* Do not allow change of the app by the packaging format when it's installed */
	origin_by_packaging_format = origin_by_packaging_format &&
		self->app != NULL &&
		gs_app_get_state (self->app) != GS_APP_STATE_INSTALLED &&
		gs_app_get_state (self->app) != GS_APP_STATE_UPDATABLE &&
		gs_app_get_state (self->app) != GS_APP_STATE_UPDATABLE_LIVE;

	/* Sort the alternates by the user's packaging preferences and by origin name */
	gs_app_list_sort (list, sort_by_packaging_format_preference, self);

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		GtkWidget *row = gs_origin_popover_row_new (app);
		gtk_widget_set_visible (row, TRUE);
		n_rows++;
		if (first_row == NULL)
			first_row = row;
		if (app == self->app || (
		    (gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_UNKNOWN ||
		    gs_app_get_bundle_kind (app) == gs_app_get_bundle_kind (self->app)) &&
		    (gs_app_get_scope (app) == AS_COMPONENT_SCOPE_UNKNOWN ||
		    gs_app_get_scope (app) == gs_app_get_scope (self->app)) &&
		    g_strcmp0 (gs_app_get_origin (app), gs_app_get_origin (self->app)) == 0 &&
		    g_strcmp0 (gs_app_get_branch (app), gs_app_get_branch (self->app)) == 0 &&
		    g_strcmp0 (gs_app_get_version (app), gs_app_get_version (self->app)) == 0 &&
		    (self->app_local_file == NULL || self->app != self->app_local_file))) {
			/* This can happen on reload of the page */
			if (app != self->app) {
				_set_app (self, app);
				instance_changed = TRUE;
			}
			select_row = row;
		}
		gtk_list_box_append (GTK_LIST_BOX (self->origin_popover_list_box), row);

		if (origin_by_packaging_format) {
			gint index = gs_details_page_get_app_packaging_format_preference_index (self, app);
			if (index > 0 && (index < origin_row_by_packaging_format_index || origin_row_by_packaging_format_index == 0)) {
				origin_row_by_packaging_format_index = index;
				origin_row_by_packaging_format = row;
			}
		}
	}

	if (origin_row_by_packaging_format) {
		GsOriginPopoverRow *row = GS_ORIGIN_POPOVER_ROW (origin_row_by_packaging_format);
		GsApp *app = gs_origin_popover_row_get_app (row);
		select_row = origin_row_by_packaging_format;
		if (app != self->app) {
			_set_app (self, app);
			instance_changed = TRUE;
		}
	}

	if (select_row == NULL && first_row != NULL) {
		GsOriginPopoverRow *row = GS_ORIGIN_POPOVER_ROW (first_row);
		GsApp *app = gs_origin_popover_row_get_app (row);
		select_row = first_row;
		if (app != self->app) {
			_set_app (self, app);
			instance_changed = TRUE;
		}
	}

	/* Do not show the "selected" check when there's only one app in the list */
	if (select_row && n_rows > 1)
		gs_origin_popover_row_set_selected (GS_ORIGIN_POPOVER_ROW (select_row), TRUE);
	else if (select_row)
		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (select_row), FALSE);

	if (select_row != NULL)
		gs_details_page_update_origin_button (self, TRUE);
	else
		gtk_widget_set_visible (self->origin_box, FALSE);

	if (instance_changed) {
		g_autoptr(GsPluginJob) plugin_job = NULL;

		/* Make sure the changed instance contains the reviews and such */
		plugin_job = gs_plugin_job_refine_new_for_app (self->app,
							       GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING |
							       GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS |
							       GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS |
							       GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE);
		gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
						    self->cancellable,
						    gs_details_page_app_refine_cb,
						    self);

		/* To refresh also developer apps, to not have shown the same instance
		   of the app in the flowbox, because it won't change the Details page
		   when it is clicked. */
		g_clear_pointer (&self->last_developer_name, g_free);

		gs_details_page_refresh_screenshots (self);
		gs_details_page_refresh_all (self);
	} else {
		gs_details_page_refresh_app_data_info (self);
	}
}

static gboolean
gs_details_page_can_launch_app (GsDetailsPage *self)
{
	const gchar *desktop_id;
	GDesktopAppInfo *desktop_appinfo;
	g_autoptr(GAppInfo) appinfo = NULL;

	if (!self->app)
		return FALSE;

	switch (gs_app_get_state (self->app)) {
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_UPDATABLE_LIVE:
		break;
	default:
		return FALSE;
	}

	if (gs_app_has_quirk (self->app, GS_APP_QUIRK_NOT_LAUNCHABLE) ||
	    gs_app_has_quirk (self->app, GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE))
		return FALSE;

	/* don't show the launch button if the app doesn't have a desktop ID */
	if (gs_app_get_id (self->app) == NULL)
		return FALSE;

	desktop_id = gs_app_get_launchable (self->app, AS_LAUNCHABLE_KIND_DESKTOP_ID);
	if (!desktop_id)
		desktop_id = gs_app_get_id (self->app);
	if (!desktop_id)
		return FALSE;

	desktop_appinfo = gs_utils_get_desktop_app_info (desktop_id);
	if (!desktop_appinfo)
		return FALSE;

	appinfo = G_APP_INFO (desktop_appinfo);

	return g_app_info_should_show (appinfo);
}

static void
gs_details_page_refresh_buttons (GsDetailsPage *self)
{
	GsAppState state;
	GtkWidget *buttons_in_order[] = {
		self->button_details_launch,
		self->button_install,
		self->button_update,
		self->button_remove,
	};
	GtkWidget *highlighted_button = NULL;
	gboolean remove_is_destructive = TRUE;

	state = gs_app_get_state (self->app);

	/* install button */
	switch (state) {
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
		gtk_widget_set_visible (self->button_install, TRUE);
		/* TRANSLATORS: button text in the header when an app
		 * can be installed */
		gtk_button_set_label (GTK_BUTTON (self->button_install), _("_Install"));
		break;
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
		gtk_widget_set_visible (self->button_install, FALSE);
		break;
	case GS_APP_STATE_UNKNOWN:
	case GS_APP_STATE_INSTALLED:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_UPDATABLE:
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		gtk_widget_set_visible (self->button_install, FALSE);
		break;
	case GS_APP_STATE_PENDING_INSTALL:
	case GS_APP_STATE_PENDING_REMOVE:
		if (gs_app_has_quirk (self->app, GS_APP_QUIRK_NEEDS_REBOOT)) {
			gtk_widget_set_visible (self->button_install, TRUE);
			gtk_button_set_label (GTK_BUTTON (self->button_install), _("_Restart"));
		} else {
			gtk_widget_set_visible (self->button_install, FALSE);
		}
		break;
	case GS_APP_STATE_UPDATABLE_LIVE:
		if (gs_app_get_kind (self->app) == AS_COMPONENT_KIND_FIRMWARE) {
			gtk_widget_set_visible (self->button_install, TRUE);
			/* TRANSLATORS: button text in the header when firmware
			 * can be live-installed */
			gtk_button_set_label (GTK_BUTTON (self->button_install), _("_Install"));
		} else {
			gtk_widget_set_visible (self->button_install, FALSE);
		}
		break;
	case GS_APP_STATE_UNAVAILABLE:
		if (gs_app_get_url_missing (self->app) != NULL) {
			gtk_widget_set_visible (self->button_install, FALSE);
		} else {
			gtk_widget_set_visible (self->button_install, TRUE);
			/* TRANSLATORS: this is a button that allows the apps to
			 * be installed.
			 * The ellipsis indicates that further steps are required,
			 * e.g. enabling software repositories or the like */
			gtk_button_set_label (GTK_BUTTON (self->button_install), _("_Install…"));
		}
		break;
	default:
		g_warning ("App unexpectedly in state %s",
			   gs_app_state_to_string (state));
		g_assert_not_reached ();
	}

	/* update button */
	switch (state) {
	case GS_APP_STATE_UPDATABLE_LIVE:
		if (gs_app_get_kind (self->app) == AS_COMPONENT_KIND_FIRMWARE) {
			gtk_widget_set_visible (self->button_update, FALSE);
		} else {
			gtk_widget_set_visible (self->button_update, TRUE);
		}
		break;
	default:
		gtk_widget_set_visible (self->button_update, FALSE);
		break;
	}

	/* launch button */
	gtk_widget_set_visible (self->button_details_launch, gs_details_page_can_launch_app (self));

	/* remove button */
	if (gs_app_has_quirk (self->app, GS_APP_QUIRK_COMPULSORY) ||
	    gs_app_get_kind (self->app) == AS_COMPONENT_KIND_FIRMWARE) {
		gtk_widget_set_visible (self->button_remove, FALSE);
	} else {
		switch (state) {
		case GS_APP_STATE_INSTALLED:
		case GS_APP_STATE_UPDATABLE:
		case GS_APP_STATE_UPDATABLE_LIVE:
			gtk_widget_set_visible (self->button_remove, TRUE);
			gtk_widget_set_sensitive (self->button_remove, TRUE);
			break;
		case GS_APP_STATE_AVAILABLE_LOCAL:
		case GS_APP_STATE_AVAILABLE:
		case GS_APP_STATE_INSTALLING:
		case GS_APP_STATE_REMOVING:
		case GS_APP_STATE_DOWNLOADING:
		case GS_APP_STATE_UNAVAILABLE:
		case GS_APP_STATE_UNKNOWN:
		case GS_APP_STATE_QUEUED_FOR_INSTALL:
		case GS_APP_STATE_PENDING_INSTALL:
		case GS_APP_STATE_PENDING_REMOVE:
			gtk_widget_set_visible (self->button_remove, FALSE);
			break;
		default:
			g_warning ("App unexpectedly in state %s",
				   gs_app_state_to_string (state));
			g_assert_not_reached ();
		}
	}

	if (gs_details_page_app_has_pending_action (self)) {
		gtk_widget_set_visible (self->button_install, FALSE);
		gtk_widget_set_visible (self->button_update, FALSE);
		gtk_widget_set_visible (self->button_details_launch, FALSE);
		gtk_widget_set_visible (self->button_remove, FALSE);
	}

	if (!gtk_widget_get_visible (self->button_details_launch) &&
	    !gtk_widget_get_visible (self->button_install) &&
	    !gtk_widget_get_visible (self->button_update)) {
		remove_is_destructive = FALSE;
		gtk_button_set_label (GTK_BUTTON (self->button_remove), _("_Uninstall…"));
	} else {
		gtk_button_set_icon_name (GTK_BUTTON (self->button_remove), "user-trash-symbolic");
	}

	/* Update the styles so that the first visible button gets
	 * `suggested-action` or `destructive-action` and the rest are
	 * unstyled. This draws the user’s attention to the most likely
	 * action to perform. */
	for (gsize i = 0; i < G_N_ELEMENTS (buttons_in_order); i++) {
		if (highlighted_button != NULL) {
			gtk_widget_remove_css_class (buttons_in_order[i], "suggested-action");
			gtk_widget_remove_css_class (buttons_in_order[i], "destructive-action");
		} else if (gtk_widget_get_visible (buttons_in_order[i])) {
			highlighted_button = buttons_in_order[i];

			if (buttons_in_order[i] == self->button_remove) {
				if (remove_is_destructive)
					gtk_widget_add_css_class (buttons_in_order[i], "destructive-action");
				else
					gtk_widget_remove_css_class (buttons_in_order[i], "destructive-action");
			} else
					gtk_widget_add_css_class (buttons_in_order[i], "suggested-action");
		}
	}
}

static gboolean
update_action_row_from_link (AdwActionRow *row,
                             GsApp        *app,
                             AsUrlKind     url_kind)
{
	const gchar *url = gs_app_get_url (app, url_kind);

#if ADW_CHECK_VERSION(1,2,0)
	adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);

	if (url != NULL)
		adw_action_row_set_subtitle (row, url);
#else
	if (url != NULL) {
		g_autofree gchar *escaped_url = g_markup_escape_text (url, -1);
		adw_action_row_set_subtitle (row, escaped_url);
	}
#endif

	gtk_widget_set_visible (GTK_WIDGET (row), url != NULL);

	return (url != NULL);
}

static void
app_activated_cb (GsDetailsPage *self, GsAppTile *tile)
{
	GsApp *app;

	app = gs_app_tile_get_app (tile);

	if (!app)
		return;

	g_signal_emit (self, signals[SIGNAL_APP_CLICKED], 0, app);
}

/* Consider app IDs with and without the ".desktop" suffix being the same app */
static gboolean
gs_details_page_app_id_equal (GsApp *app1,
			      GsApp *app2)
{
	const gchar *id1, *id2;

	id1 = gs_app_get_id (app1);
	id2 = gs_app_get_id (app2);
	if (g_strcmp0 (id1, id2) == 0)
		return TRUE;

	if (id1 == NULL || id2 == NULL)
		return FALSE;

	if (g_str_has_suffix (id1, ".desktop")) {
		return !g_str_has_suffix (id2, ".desktop") &&
			strlen (id1) == strlen (id2) + 8 /* strlen (".desktop") */ &&
			g_str_has_prefix (id1, id2);
	}

	return g_str_has_suffix (id2, ".desktop") &&
		!g_str_has_suffix (id1, ".desktop") &&
		strlen (id2) == strlen (id1) + 8 /* strlen (".desktop") */ &&
		g_str_has_prefix (id2, id1);
}

static void
gs_details_page_search_developer_apps_cb (GObject *source_object,
					  GAsyncResult *result,
					  gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) local_error = NULL;
	guint n_added = 0;

	list = gs_plugin_loader_job_process_finish (GS_PLUGIN_LOADER (source_object), result, &local_error);
	if (list == NULL) {
		if (g_error_matches (local_error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED) ||
		    g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_debug ("search cancelled");
			return;
		}
		g_warning ("failed to get other apps: %s", local_error->message);
		return;
	}

	if (!self->app || !gs_page_is_active (GS_PAGE (self)))
		return;

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		if (app != self->app && !gs_details_page_app_id_equal (app, self->app)) {
			GtkWidget *tile = gs_summary_tile_new (app);
			gtk_flow_box_insert (GTK_FLOW_BOX (self->box_developer_apps), tile, -1);

			n_added++;
			if (n_added == N_DEVELOPER_APPS)
				break;
		}
	}

	gtk_widget_set_visible (self->box_developer_apps, n_added > 0);
}

static void
gs_details_page_refresh_all (GsDetailsPage *self)
{
	g_autoptr(GIcon) icon = NULL;
	const gchar *tmp;
	g_autoptr(GPtrArray) version_history = NULL;
	gboolean link_rows_visible;

	/* change widgets */
	tmp = gs_app_get_name (self->app);
	if (tmp != NULL && tmp[0] != '\0') {
		g_autofree gchar *title = NULL;
		gtk_label_set_label (GTK_LABEL (self->application_details_title), tmp);
		gtk_widget_set_visible (self->application_details_title, TRUE);
		/* Translators: %s is the user-visible app name */
		title = g_strdup_printf (_("%s will appear in US English"), tmp);
		adw_banner_set_title (self->translation_banner, title);
	} else {
		gtk_widget_set_visible (self->application_details_title, FALSE);
	}
	tmp = gs_app_get_summary (self->app);
	if (tmp != NULL && tmp[0] != '\0') {
		if (gs_app_is_application (self->app))
			adw_banner_set_title (self->translation_banner, _("This app will appear in US English"));
		else
			adw_banner_set_title (self->translation_banner, _("This software will appear in US English"));
		gtk_label_set_label (GTK_LABEL (self->application_details_summary), tmp);
		gtk_widget_set_visible (self->application_details_summary, TRUE);
	} else {
		gtk_widget_set_visible (self->application_details_summary, FALSE);
	}

	tmp = gs_app_get_metadata_item (self->app, "LingmoAppstore::problems");
	if (tmp == NULL || *tmp == '\0') {
		/* Show runtime problems on the apps which use them, unless they have their own problems */
		GsApp *runtime = gs_app_get_runtime (self->app);
		if (runtime != NULL)
			tmp = gs_app_get_metadata_item (runtime, "LingmoAppstore::problems");
	}
	gtk_label_set_text (GTK_LABEL (self->infobar_details_problems_label), (tmp != NULL && *tmp != '\0') ? tmp : "");
	gtk_widget_set_visible (self->infobar_details_problems_label, tmp != NULL && *tmp != '\0');

	tmp = gs_app_get_metadata_item (self->app, "LingmoAppstore::EolReason");
	if (tmp == NULL || *tmp == '\0') {
		/* Show runtime EOL on the apps which use them, unless they have their own EOL */
		GsApp *runtime = gs_app_get_runtime (self->app);
		if (runtime != NULL)
			tmp = gs_app_get_metadata_item (runtime, "LingmoAppstore::EolReason");
	}
	/* ignore the provided EOL reason, which might not be localized */
	gtk_widget_set_visible (self->infobar_details_eol, tmp != NULL && *tmp != '\0');

	/* refresh buttons */
	gs_details_page_refresh_buttons (self);

	/* Set up the translation infobar. Assume that translations can be
	 * contributed to if an app is FOSS and it has provided a link for
	 * contributing translations. */
	if (gs_app_translation_dialog_app_has_url (self->app) && gs_app_get_license_is_free (self->app)) {
		adw_banner_set_button_label (self->translation_banner,
					     _("Help _Translate"));
	} else {
 		adw_banner_set_button_label (self->translation_banner, NULL);
	}

	adw_banner_set_revealed (self->translation_banner,
				 gs_app_get_has_translations (self->app) &&
				 !gs_app_has_kudo (self->app, GS_APP_KUDO_MY_LANGUAGE));

	/* set the description */
	tmp = gs_app_get_description (self->app);
	gs_details_page_set_description (self, tmp);

	/* set the icon; fall back to 96px and 64px if 128px isn’t available,
	 * which sometimes happens at 2× scale factor (hi-DPI) */
	{
		const struct {
			guint icon_size;
			const gchar *fallback_icon_name;  /* (nullable) */
		} icon_fallbacks[] = {
			{ 128, NULL },
			{ 96, NULL },
			{ 64, NULL },
			{ 128, "application-x-executable" },
		};

		for (gsize i = 0; i < G_N_ELEMENTS (icon_fallbacks) && icon == NULL; i++) {
			icon = gs_app_get_icon_for_size (self->app,
							 icon_fallbacks[i].icon_size,
							 gtk_widget_get_scale_factor (self->application_details_icon),
							 icon_fallbacks[i].fallback_icon_name);
		}
	}

	gtk_image_set_from_gicon (GTK_IMAGE (self->application_details_icon), icon);

	/* Set various external links. If none are visible, show a fallback
	 * message instead. */
	link_rows_visible = FALSE;
	link_rows_visible = update_action_row_from_link (self->project_website_row, self->app, AS_URL_KIND_HOMEPAGE) || link_rows_visible;
	link_rows_visible = update_action_row_from_link (self->donate_row, self->app, AS_URL_KIND_DONATION) || link_rows_visible;
	link_rows_visible = update_action_row_from_link (self->translate_row, self->app, AS_URL_KIND_TRANSLATE) || link_rows_visible;
	link_rows_visible = update_action_row_from_link (self->report_an_issue_row, self->app, AS_URL_KIND_BUGTRACKER) || link_rows_visible;
	link_rows_visible = update_action_row_from_link (self->help_row, self->app, AS_URL_KIND_HELP) || link_rows_visible;
	link_rows_visible = update_action_row_from_link (self->contact_row, self->app, AS_URL_KIND_CONTACT) || link_rows_visible;

	gtk_stack_set_visible_child_name (self->links_stack, link_rows_visible ? "links" : "empty");

	tmp = gs_app_get_developer_name (self->app);
	if (tmp != NULL) {
		gtk_label_set_label (GTK_LABEL (self->developer_name_label), tmp);

		if (g_strcmp0 (tmp, self->last_developer_name) != 0) {
			g_autoptr(GsAppQuery) query = NULL;
			g_autoptr(GsPluginJob) plugin_job = NULL;
			g_autofree gchar *heading = NULL;
			const gchar *names[2] = { NULL, NULL };

			/* Hide the section, it will be shown only if any other app had been found */
			gtk_widget_set_visible (self->box_developer_apps, FALSE);

			g_clear_pointer (&self->last_developer_name, g_free);
			self->last_developer_name = g_strdup (tmp);

			/* Translators: the '%s' is replaced with a developer name or a project group */
			heading = g_strdup_printf (_("Other Apps by %s"), self->last_developer_name);
			gtk_label_set_label (GTK_LABEL (self->developer_apps_heading), heading);
			gs_widget_remove_all (self->box_developer_apps, (GsRemoveFunc) gtk_flow_box_remove);

			names[0] = self->last_developer_name;
			query = gs_app_query_new ("developers", names,
						  "max-results", N_DEVELOPER_APPS * 3, /* Ask for more, some can be skipped */
						  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
						  "dedupe-flags", GS_APP_LIST_FILTER_FLAG_KEY_ID,
						  "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
						  "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
						  NULL);

			plugin_job = gs_plugin_job_list_apps_new (query,
								  GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

			g_debug ("searching other apps for: '%s'", names[0]);
			gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
							    self->cancellable,
							    gs_details_page_search_developer_apps_cb,
							    self);
		}
	} else if (tmp == NULL) {
		g_clear_pointer (&self->last_developer_name, g_free);
		gs_widget_remove_all (self->box_developer_apps, (GsRemoveFunc) gtk_flow_box_remove);
		gtk_widget_set_visible (self->box_developer_apps, FALSE);
	}

	gtk_widget_set_visible (GTK_WIDGET (self->developer_name_label), tmp != NULL);
	gtk_widget_set_visible (self->developer_verified_image, gs_app_has_quirk (self->app, GS_APP_QUIRK_DEVELOPER_VERIFIED));

	if (gs_app_has_quirk (self->app, GS_APP_QUIRK_DEVELOPER_VERIFIED)) {
		g_autofree gchar *tooltip = NULL;

		if (tmp != NULL)
			/* Translators: the first %s is replaced with the developer name, the second %s is replaced with the app id */
			tooltip = g_strdup_printf (_("Developer %s has proven the ownership of %s"), tmp, gs_app_get_id (self->app));
		else
			/* Translators: the %s is replaced with the app id */
			tooltip = g_strdup_printf (_("Developer has proven the ownership of %s"), gs_app_get_id (self->app));

		gtk_widget_set_tooltip_text (self->developer_verified_image, tooltip);
	}

	/* set version history */
	version_history = gs_app_get_version_history (self->app);
	if (version_history == NULL || version_history->len == 0) {
		const gchar *version = gs_app_get_version_ui (self->app);
		if (version == NULL || *version == '\0')
			gtk_widget_set_visible (self->list_box_version_history, FALSE);
		else {
			gs_app_version_history_row_set_info (GS_APP_VERSION_HISTORY_ROW (self->row_latest_version),
							     version, gs_app_get_release_date (self->app), NULL);
			gtk_widget_set_visible (self->list_box_version_history, TRUE);
		}
	} else {
		AsRelease *latest_version = g_ptr_array_index (version_history, 0);
		const gchar *version = gs_app_get_version_ui (self->app);
		if (version == NULL || *version == '\0') {
			gs_app_version_history_row_set_info (GS_APP_VERSION_HISTORY_ROW (self->row_latest_version),
						     as_release_get_version (latest_version),
						     as_release_get_timestamp (latest_version),
						     as_release_get_description (latest_version));
		} else {
			gboolean same_version = g_strcmp0 (version, as_release_get_version (latest_version)) == 0;
			/* Inherit the description from the release history, when the versions match */
			gs_app_version_history_row_set_info (GS_APP_VERSION_HISTORY_ROW (self->row_latest_version),
							     version, gs_app_get_release_date (self->app),
							     same_version ? as_release_get_description (latest_version) : NULL);
		}
		gtk_widget_set_visible (self->list_box_version_history, TRUE);
	}

	gtk_widget_set_visible (self->version_history_button, version_history != NULL && version_history->len > 1);

	/* are we trying to replace something in the baseos */
	gtk_widget_set_visible (self->infobar_details_package_baseos,
				gs_app_has_quirk (self->app, GS_APP_QUIRK_COMPULSORY) &&
				gs_app_get_state (self->app) == GS_APP_STATE_AVAILABLE_LOCAL);

	switch (gs_app_get_kind (self->app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		/* installing an app with a repo file */
		gtk_widget_set_visible (GTK_WIDGET (self->context_bar), TRUE);
		gtk_widget_set_visible (GTK_WIDGET (self->license_tile), TRUE);
		gtk_widget_set_visible (self->infobar_details_app_repo,
					gs_app_has_quirk (self->app,
							  GS_APP_QUIRK_HAS_SOURCE) &&
					gs_app_get_state (self->app) == GS_APP_STATE_AVAILABLE_LOCAL);
		gtk_widget_set_visible (self->infobar_details_repo, FALSE);
		break;
	case AS_COMPONENT_KIND_GENERIC:
		/* installing a repo-release package */
		gtk_widget_set_visible (GTK_WIDGET (self->context_bar), TRUE);
		gtk_widget_set_visible (GTK_WIDGET (self->license_tile), TRUE);
		gtk_widget_set_visible (self->infobar_details_app_repo, FALSE);
		gtk_widget_set_visible (self->infobar_details_repo,
					gs_app_has_quirk (self->app,
							  GS_APP_QUIRK_HAS_SOURCE) &&
					gs_app_get_state (self->app) == GS_APP_STATE_AVAILABLE_LOCAL);
		break;
	case AS_COMPONENT_KIND_WEB_APP:
		gtk_widget_set_visible (GTK_WIDGET (self->context_bar), TRUE);
		gtk_widget_set_visible (GTK_WIDGET (self->license_tile), TRUE);
		gtk_widget_set_visible (self->infobar_details_app_repo, FALSE);
		gtk_widget_set_visible (self->infobar_details_repo, FALSE);
		break;
	default:
		gtk_widget_set_visible (GTK_WIDGET (self->context_bar), FALSE);
		gtk_widget_set_visible (GTK_WIDGET (self->license_tile), FALSE);
		gtk_widget_set_visible (self->infobar_details_app_repo, FALSE);
		gtk_widget_set_visible (self->infobar_details_repo, FALSE);
		break;
	}

	/* installing a app without a repo file */
	switch (gs_app_get_kind (self->app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		if (gs_app_get_kind (self->app) == AS_COMPONENT_KIND_FIRMWARE) {
			gtk_widget_set_visible (self->infobar_details_app_norepo, FALSE);
		} else {
			gtk_widget_set_visible (self->infobar_details_app_norepo,
						!gs_app_has_quirk (self->app,
							  GS_APP_QUIRK_HAS_SOURCE) &&
						gs_app_get_state (self->app) == GS_APP_STATE_AVAILABLE_LOCAL);
		}
		break;
	default:
		gtk_widget_set_visible (self->infobar_details_app_norepo, FALSE);
		break;
	}

	/* update progress */
	gs_details_page_refresh_progress (self);

	gs_details_page_refresh_addons (self);
	gs_details_page_refresh_app_data_info (self);
}

static gint
list_sort_func (GtkListBoxRow *a,
		GtkListBoxRow *b,
		gpointer user_data)
{
	GsApp *a1 = gs_app_addon_row_get_addon (GS_APP_ADDON_ROW (a));
	GsApp *a2 = gs_app_addon_row_get_addon (GS_APP_ADDON_ROW (b));

	return gs_utils_sort_strcmp (gs_app_get_name (a1),
				     gs_app_get_name (a2));
}

static void
addons_list_row_activated_cb (GtkListBox *list_box,
			      GtkListBoxRow *row,
			      GsDetailsPage *self)
{
	g_return_if_fail (GS_IS_APP_ADDON_ROW (row));

	gs_app_addon_row_activate (GS_APP_ADDON_ROW (row));
}

static void
version_history_list_row_activated_cb (GtkListBox *list_box,
				       GtkListBoxRow *row,
				       GsDetailsPage *self)
{
	GtkWidget *dialog;

	/* Only the row with the arrow is clickable */
	if (GS_IS_APP_VERSION_HISTORY_ROW (row))
		return;

	dialog = gs_app_version_history_dialog_new (GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (list_box), GTK_TYPE_WINDOW)),
						    self->app);
	gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (dialog));
}

static void gs_details_page_refresh_reviews (GsDetailsPage *self);

static void
app_reviews_dialog_destroy_cb (GsDetailsPage *self)
{
	self->app_reviews_dialog = NULL;
}

static void
featured_review_list_row_activated_cb (GtkListBox *list_box,
				       GtkListBoxRow *row,
				       GsDetailsPage *self)
{
	/* Only the row with the arrow is clickable */
	if (GS_IS_REVIEW_ROW (row))
		return;

	g_assert (GS_IS_ODRS_PROVIDER (self->odrs_provider));

	if (self->app_reviews_dialog == NULL) {
		GtkWindow *parent;

		parent = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (list_box), GTK_TYPE_WINDOW));

		self->app_reviews_dialog =
			gs_app_reviews_dialog_new (parent, self->app,
						   self->odrs_provider, self->plugin_loader);
		g_object_bind_property (self, "odrs-provider",
					self->app_reviews_dialog, "odrs-provider", 0);
		g_signal_connect_swapped (self->app_reviews_dialog, "reviews-updated",
					  G_CALLBACK (gs_details_page_refresh_reviews), self);
		g_signal_connect_swapped (self->app_reviews_dialog, "destroy",
					  G_CALLBACK (app_reviews_dialog_destroy_cb), self);
	}

	gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (self->app_reviews_dialog));
}

static void gs_details_page_addon_install_cb (GsAppAddonRow *row, gpointer user_data);
static void gs_details_page_addon_remove_cb (GsAppAddonRow *row, gpointer user_data);

static void
gs_details_page_refresh_addons (GsDetailsPage *self)
{
	g_autoptr(GsAppList) addons = NULL;
	gboolean sensitive;
	guint i, rows = 0;

	gs_widget_remove_all (self->list_box_addons, (GsRemoveFunc) gtk_list_box_remove);

	/* Make addons installable only if the app itself is installed */
	sensitive = gs_app_get_state (self->app) == GS_APP_STATE_INSTALLED ||
		    gs_app_get_state (self->app) == GS_APP_STATE_UPDATABLE ||
		    gs_app_get_state (self->app) == GS_APP_STATE_UPDATABLE_LIVE;

	addons = gs_app_dup_addons (self->app);
	for (i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
		GsApp *addon;
		GtkWidget *row;

		addon = gs_app_list_index (addons, i);
		if (gs_app_get_state (addon) == GS_APP_STATE_UNKNOWN ||
		    gs_app_get_state (addon) == GS_APP_STATE_UNAVAILABLE)
			continue;

		if (gs_app_has_quirk (addon, GS_APP_QUIRK_HIDE_EVERYWHERE))
			continue;

		row = gs_app_addon_row_new (addon);

		gtk_widget_set_sensitive (row, sensitive);

		g_signal_connect (row, "install-button-clicked",
				  G_CALLBACK (gs_details_page_addon_install_cb),
				  self);
		g_signal_connect (row, "remove-button-clicked",
				  G_CALLBACK (gs_details_page_addon_remove_cb),
				  self);

		gtk_list_box_append (GTK_LIST_BOX (self->list_box_addons), row);

		rows++;
	}

	gtk_widget_set_visible (self->box_addons, rows > 0);
}

static AsReview *
get_featured_review (GPtrArray *reviews)
{
	AsReview *featured;
	g_autoptr(GDateTime) now_utc = NULL;
	g_autoptr(GDateTime) min_date = NULL;
	gint featured_priority;

	g_assert (reviews->len > 0);

	now_utc = g_date_time_new_now_utc ();
	min_date = g_date_time_add_months (now_utc, -6);

	featured = g_ptr_array_index (reviews, 0);
	featured_priority = as_review_get_priority (featured);

	for (gsize i = 1; i < reviews->len; i++) {
		AsReview *new = g_ptr_array_index (reviews, i);
		gint new_priority = as_review_get_priority (new);

		/* Skip reviews older than 6 months for the featured pick */
		if (g_date_time_compare (as_review_get_date (new), min_date) < 0)
			continue;

		if (featured_priority > new_priority ||
		    (featured_priority == new_priority &&
		     g_date_time_compare (as_review_get_date (featured), as_review_get_date (new)) > 0)) {
			featured = new;
			featured_priority = new_priority;
		}
	}

	return featured;
}

static void
gs_details_page_refresh_reviews (GsDetailsPage *self)
{
	GArray *review_ratings = NULL;
	GPtrArray *reviews;
	gboolean show_review_button = TRUE;
	gboolean show_reviews = FALSE;
	guint n_reviews = 0;
	guint i;
	GtkWidget *child;

	/* nothing to show */
	if (self->app == NULL)
		return;

	/* show or hide the entire reviews section */
	switch (gs_app_get_kind (self->app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
	case AS_COMPONENT_KIND_FONT:
	case AS_COMPONENT_KIND_INPUT_METHOD:
	case AS_COMPONENT_KIND_WEB_APP:
		/* don't show a missing rating on a local file */
		if (gs_app_get_state (self->app) != GS_APP_STATE_AVAILABLE_LOCAL &&
		    self->odrs_provider != NULL)
			show_reviews = TRUE;
		break;
	default:
		break;
	}

	/* some apps are unreviewable */
	if (gs_app_has_quirk (self->app, GS_APP_QUIRK_NOT_REVIEWABLE))
		show_reviews = FALSE;

	/* set the star rating */
	if (show_reviews) {
		gtk_widget_set_sensitive (self->star, gs_app_get_rating (self->app) >= 0);
		gs_star_widget_set_rating (GS_STAR_WIDGET (self->star),
					   gs_app_get_rating (self->app));

		review_ratings = gs_app_get_review_ratings (self->app);
		if (review_ratings != NULL) {
			gs_review_histogram_set_ratings (GS_REVIEW_HISTOGRAM (self->histogram),
							 gs_app_get_rating (self->app),
						         review_ratings);
		}
		if (review_ratings != NULL) {
			for (i = 0; i < review_ratings->len; i++)
				n_reviews += (guint) g_array_index (review_ratings, guint32, i);
		} else if (gs_app_get_reviews (self->app) != NULL) {
			n_reviews = gs_app_get_reviews (self->app)->len;
		}
	}

	/* enable appropriate widgets */
	gtk_widget_set_visible (self->star, show_reviews);
	gtk_widget_set_visible (self->histogram_row, review_ratings != NULL && review_ratings->len > 0);
	gtk_widget_set_visible (self->label_review_count, n_reviews > 0);

	/* update the review label next to the star widget */
	if (n_reviews > 0) {
		g_autofree gchar *text = NULL;
		gtk_widget_set_visible (self->label_review_count, TRUE);
		text = g_strdup_printf ("(%u)", n_reviews);
		gtk_label_set_text (GTK_LABEL (self->label_review_count), text);
	}

	/* no point continuing */
	if (!show_reviews) {
		gtk_widget_set_visible (self->box_reviews, FALSE);
		return;
	}

	/* add all the reviews */
	while ((child = gtk_widget_get_first_child (self->list_box_featured_review)) != NULL) {
		if (GS_IS_REVIEW_ROW (child))
			gtk_list_box_remove (GTK_LIST_BOX (self->list_box_featured_review), child);
		else
			break;
	}

	reviews = gs_app_get_reviews (self->app);
	if (reviews->len > 0) {
		AsReview *review = get_featured_review (reviews);
		GtkWidget *row = gs_review_row_new (review);

		gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), FALSE);
		gtk_list_box_prepend (GTK_LIST_BOX (self->list_box_featured_review), row);

		gs_review_row_set_network_available (GS_REVIEW_ROW (row),
						     gs_plugin_loader_get_network_available (self->plugin_loader));
	}

	/* show the button only if the user never reviewed */
	gtk_widget_set_visible (self->button_review, show_review_button);
	if (!gs_app_is_installed (self->app)) {
		gtk_widget_set_visible (self->button_review, FALSE);
		gtk_widget_set_sensitive (self->button_review, FALSE);
		gtk_widget_set_sensitive (self->star, FALSE);
	} else if (gs_plugin_loader_get_network_available (self->plugin_loader)) {
		gtk_widget_set_sensitive (self->button_review, TRUE);
		gtk_widget_set_sensitive (self->star, TRUE);
		gtk_widget_set_tooltip_text (self->button_review, NULL);
	} else {
		gtk_widget_set_sensitive (self->button_review, FALSE);
		gtk_widget_set_sensitive (self->star, FALSE);
		gtk_widget_set_tooltip_text (self->button_review,
					     /* TRANSLATORS: we need a remote server to process */
					     _("You need internet access to write a review"));
	}

	gtk_widget_set_visible (self->list_box_featured_review, reviews->len > 0);

	/* Update the overall container. */
	gtk_widget_set_visible (self->list_box_reviews_summary,
				show_reviews &&
				(gtk_widget_get_visible (self->histogram_row) ||
				 gtk_widget_get_visible (self->button_review)));
	gtk_widget_set_visible (self->box_reviews,
				reviews->len > 0 ||
				gtk_widget_get_visible (self->list_box_reviews_summary));
}

static void
gs_details_page_app_refine_cb (GObject *source,
				GAsyncResult *res,
				gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	g_autoptr(GError) error = NULL;
	if (!gs_plugin_loader_job_action_finish (plugin_loader, res, &error)) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
			g_warning ("failed to refine %s: %s",
				   gs_app_get_id (self->app),
				   error->message);
		}
		return;
	}
	gs_details_page_refresh_reviews (self);
	gs_details_page_refresh_addons (self);
}

static void
_set_app (GsDetailsPage *self, GsApp *app)
{
	GsJobManager *job_manager = gs_plugin_loader_get_job_manager (self->plugin_loader);

	if (self->app == app)
		return;

	/* do not show all the reviews by default */
	self->show_all_reviews = FALSE;

	/* disconnect the old handlers */
	if (self->app != NULL) {
		g_signal_handlers_disconnect_by_func (self->app, gs_details_page_notify_state_changed_cb, self);
		g_signal_handlers_disconnect_by_func (self->app, gs_details_page_progress_changed_cb, self);
		g_signal_handlers_disconnect_by_func (self->app, gs_details_page_allow_cancel_changed_cb,
						      self);
		gs_job_manager_remove_watch (job_manager, self->job_manager_watch_id);
		self->job_manager_watch_id = 0;
	}

	/* save app */
	g_set_object (&self->app, app);

	gs_app_context_bar_set_app (self->context_bar, app);
	gs_license_tile_set_app (self->license_tile, app);

	/* title/app name will have changed */
	g_object_notify (G_OBJECT (self), "title");

	if (self->app == NULL) {
		g_set_object (&self->app_cancellable, NULL);
		return;
	}

	g_set_object (&self->app_cancellable, gs_app_get_cancellable (app));

	self->job_manager_watch_id = gs_job_manager_add_watch (job_manager,
							       app,
							       G_TYPE_INVALID,
							       job_manager_jobs_changed_cb,
							       job_manager_jobs_changed_cb,
							       self,
							       NULL);

	g_signal_connect_object (self->app, "notify::state",
				 G_CALLBACK (gs_details_page_notify_state_changed_cb),
				 self, 0);
	g_signal_connect_object (self->app, "notify::size",
				 G_CALLBACK (gs_details_page_notify_state_changed_cb),
				 self, 0);
	g_signal_connect_object (self->app, "notify::quirk",
				 G_CALLBACK (gs_details_page_notify_state_changed_cb),
				 self, 0);
	g_signal_connect_object (self->app, "notify::progress",
				 G_CALLBACK (gs_details_page_progress_changed_cb),
				 self, 0);
	g_signal_connect_object (self->app, "notify::allow-cancel",
				 G_CALLBACK (gs_details_page_allow_cancel_changed_cb),
				 self, 0);
	g_signal_connect_object (self->app, "notify::pending-action",
				 G_CALLBACK (gs_details_page_notify_state_changed_cb),
				 self, 0);

	if (gs_app_is_application (self->app)) {
		gtk_label_set_text (GTK_LABEL (self->label_eol), _("This app is no longer receiving updates, including security fixes"));
		gtk_label_set_text (GTK_LABEL (self->label_package_baseos), _("This app is already provided by your distribution and should not be replaced."));
		gtk_label_set_text (GTK_LABEL (self->label_no_metadata_info), _("This app doesn’t provide any links to a website, code repository or issue tracker."));
	} else {
		gtk_label_set_text (GTK_LABEL (self->label_eol), _("This software is no longer receiving updates, including security fixes"));
		gtk_label_set_text (GTK_LABEL (self->label_package_baseos), _("This software is already provided by your distribution and should not be replaced."));
		gtk_label_set_text (GTK_LABEL (self->label_no_metadata_info), _("This software doesn’t provide any links to a website, code repository or issue tracker."));
	}
}

static gboolean
gs_details_page_filter_origin (GsApp *app,
			       gpointer user_data)
{
	/* Keep only local apps or those, which have an origin set */
	return gs_app_get_state (app) == GS_APP_STATE_AVAILABLE_LOCAL ||
	       gs_app_get_local_file (app) != NULL ||
	       gs_app_get_origin (app) != NULL;
}

/* show the UI and do operations that should not block page load */
static void
gs_details_page_load_stage2 (GsDetailsPage *self,
			     gboolean continue_loading)
{
	g_autofree gchar *tmp = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GsPluginJob) plugin_job1 = NULL;
	g_autoptr(GsPluginJob) plugin_job2 = NULL;

	/* print what we've got */
	tmp = gs_app_to_string (self->app);
	g_debug ("%s", tmp);

	/* update UI */
	gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_READY);
	gs_details_page_refresh_screenshots (self);
	gs_details_page_refresh_reviews (self);
	gs_details_page_refresh_all (self);
	gs_details_page_update_origin_button (self, FALSE);

	if (!continue_loading)
		return;

	/* if these tasks fail (e.g. because we have no networking) then it's
	 * of no huge importance if we don't get the required data */
	plugin_job1 = gs_plugin_job_refine_new_for_app (self->app,
							GS_PLUGIN_REFINE_FLAGS_REQUIRE_RATING |
							GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEW_RATINGS |
							GS_PLUGIN_REFINE_FLAGS_REQUIRE_REVIEWS |
							GS_PLUGIN_REFINE_FLAGS_REQUIRE_SIZE);

	query = gs_app_query_new ("alternate-of", self->app,
				  "refine-flags", GS_DETAILS_PAGE_REFINE_FLAGS,
				  "dedupe-flags", GS_APP_LIST_FILTER_FLAG_NONE,
				  "filter-func", gs_details_page_filter_origin,
				  "sort-func", gs_utils_app_sort_priority,
				  "license-type", gs_page_get_query_license_type (GS_PAGE (self)),
				  "developer-verified-type", gs_page_get_query_developer_verified_type (GS_PAGE (self)),
				  NULL);
	plugin_job2 = gs_plugin_job_list_apps_new (query,
						   GS_PLUGIN_LIST_APPS_FLAGS_INTERACTIVE);

	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job1,
					    self->cancellable,
					    gs_details_page_app_refine_cb,
					    self);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job2,
					    self->cancellable,
					    gs_details_page_get_alternates_cb,
					    self);
}

static void
gs_details_page_load_stage1_cb (GObject *source,
				GAsyncResult *res,
				gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	g_autoptr(GError) error = NULL;

	if (!gs_plugin_loader_job_action_finish (plugin_loader, res, &error)) {
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
		    !g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_CANCELLED)) {
			g_warning ("failed to refine %s: %s",
				   gs_app_get_id (self->app),
				   error->message);
		} else {
			return;
		}
	}
	if (gs_app_get_kind (self->app) == AS_COMPONENT_KIND_UNKNOWN ||
	    gs_app_get_state (self->app) == GS_APP_STATE_UNKNOWN) {
		g_autofree gchar *str = NULL;
		const gchar *id = gs_app_get_id (self->app);
		str = g_strdup_printf (_("Unable to find “%s”"), id == NULL ? gs_app_get_source_default (self->app) : id);
		adw_status_page_set_title (ADW_STATUS_PAGE (self->page_failed), str);
		gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_FAILED);
		return;
	}

	/* Hide the app if it’s not suitable for the user, but only if it’s not
	 * already installed — a parent could have decided that a particular
	 * app *is* actually suitable for their child, despite its age rating.
	 *
	 * Make it look like the app doesn’t exist, to not tantalise the
	 * child. */
	if (!gs_app_is_installed (self->app) &&
	    gs_app_has_quirk (self->app, GS_APP_QUIRK_PARENTAL_FILTER)) {
		g_autofree gchar *str = NULL;
		const gchar *id = gs_app_get_id (self->app);
		str = g_strdup_printf (_("Unable to find “%s”"), id == NULL ? gs_app_get_source_default (self->app) : id);
		adw_status_page_set_title (ADW_STATUS_PAGE (self->page_failed), str);
		gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_FAILED);
		return;
	}

	/* do 2nd stage refine */
	gs_details_page_load_stage2 (self, TRUE);
}

static void
gs_details_page_file_to_app_cb (GObject *source,
                                GAsyncResult *res,
                                gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) error = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &error);
	if (list == NULL) {
		g_warning ("failed to convert file to GsApp: %s", error->message);
		/* go back to the overview */
		gs_shell_set_mode (self->shell, GS_SHELL_MODE_OVERVIEW);
	} else {
		GsApp *app = gs_app_list_index (list, 0);
		g_set_object (&self->app_local_file, app);
		_set_app (self, app);
		gs_details_page_load_stage2 (self, TRUE);
	}
}

static void
gs_details_page_url_to_app_cb (GObject *source,
                               GAsyncResult *res,
                               gpointer user_data)
{
	GsPluginLoader *plugin_loader = GS_PLUGIN_LOADER (source);
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) error = NULL;

	list = gs_plugin_loader_job_process_finish (plugin_loader,
						    res,
						    &error);
	if (list == NULL) {
		g_warning ("failed to convert URL to GsApp: %s", error->message);
		/* go back to the overview */
		gs_shell_set_mode (self->shell, GS_SHELL_MODE_OVERVIEW);
	} else {
		GsApp *app = gs_app_list_index (list, 0);
		g_set_object (&self->app_local_file, app);
		_set_app (self, app);
		gs_details_page_load_stage2 (self, TRUE);
	}
}

void
gs_details_page_set_local_file (GsDetailsPage *self, GFile *file)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_LOADING);
	g_clear_object (&self->app_local_file);
	_set_app (self, NULL);
	self->origin_by_packaging_format = FALSE;
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_FILE_TO_APP,
					 "file", file,
					 "refine-flags", GS_DETAILS_PAGE_REFINE_FLAGS,
					 NULL);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    gs_details_page_file_to_app_cb,
					    self);
}

void
gs_details_page_set_url (GsDetailsPage *self, const gchar *url)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_LOADING);
	g_clear_object (&self->app_local_file);
	_set_app (self, NULL);
	self->origin_by_packaging_format = FALSE;
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_URL_TO_APP,
					 "search", url,
					 "refine-flags", GS_DETAILS_PAGE_REFINE_FLAGS |
							 GS_PLUGIN_REFINE_FLAGS_ALLOW_PACKAGES,
					 NULL);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    gs_details_page_url_to_app_cb,
					    self);
}

/* refines a GsApp */
static void
gs_details_page_load_stage1 (GsDetailsPage *self)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GCancellable) cancellable = g_cancellable_new ();

	/* update UI */
	gs_page_switch_to (GS_PAGE (self));
	gs_page_scroll_up (GS_PAGE (self));
	gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_LOADING);

	g_cancellable_cancel (self->cancellable);
	g_set_object (&self->cancellable, cancellable);
	g_cancellable_connect (self->cancellable, G_CALLBACK (gs_details_page_cancel_cb), self, NULL);

	/* get extra details about the app */
	plugin_job = gs_plugin_job_refine_new_for_app (self->app, GS_DETAILS_PAGE_REFINE_FLAGS);
	gs_plugin_loader_job_process_async (self->plugin_loader, plugin_job,
					    self->cancellable,
					    gs_details_page_load_stage1_cb,
					    self);

	/* update UI with loading page */
	gs_details_page_refresh_all (self);
}

static void
gs_details_page_reload (GsPage *page)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (page);
	if (self->app != NULL && gs_shell_get_mode (self->shell) == GS_SHELL_MODE_DETAILS) {
		GsAppState state = gs_app_get_state (self->app);
		/* Do not reload the page when the app is "doing something" */
		if (state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_REMOVING ||
		    state == GS_APP_STATE_DOWNLOADING ||
		    state == GS_APP_STATE_PURCHASING)
			return;
		gs_details_page_load_stage1 (self);
	}
}

static void
origin_popover_row_activated_cb (GtkListBox *list_box,
                                 GtkListBoxRow *row,
                                 gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);
	GsApp *app;

	gtk_popover_popdown (GTK_POPOVER (self->origin_popover));

	app = gs_origin_popover_row_get_app (GS_ORIGIN_POPOVER_ROW (row));
	if (app != self->app) {
		_set_app (self, app);
		gs_details_page_load_stage1 (self);
	}
}

static void
gs_details_page_read_packaging_format_preference (GsDetailsPage *self)
{
	g_auto(GStrv) preference = NULL;

	g_clear_pointer (&self->packaging_format_preference, g_strfreev);

	preference = g_settings_get_strv (self->settings, "packaging-format-preference");
	/* Ignore empty arrays or arrays with a single empty string item */
	if (preference == NULL || preference[0] == NULL ||
	    (preference[0][0] == '\0' && preference[1] == NULL))
		return;

	self->packaging_format_preference = g_steal_pointer (&preference);
}

static void
settings_changed_cb (GsDetailsPage *self, const gchar *key, gpointer data)
{
	if (g_strcmp0 (key, "packaging-format-preference") == 0) {
		gs_details_page_read_packaging_format_preference (self);
		return;
	}

	if (self->app == NULL)
		return;
	if (g_strcmp0 (key, "show-nonfree-ui") == 0) {
		gs_details_page_refresh_all (self);
	}
}

static void
gs_details_page_app_info_changed_cb (GAppInfoMonitor *monitor,
				     gpointer user_data)
{
	GsDetailsPage *self = user_data;

	g_return_if_fail (GS_IS_DETAILS_PAGE (self));

	if (!self->app || !gs_page_is_active (GS_PAGE (self)))
		return;

	gs_details_page_refresh_buttons (self);
}

/* this is being called from GsShell */
void
gs_details_page_set_app (GsDetailsPage *self, GsApp *app)
{
	g_return_if_fail (GS_IS_DETAILS_PAGE (self));
	g_return_if_fail (GS_IS_APP (app));

	/* clear old state */
	g_clear_object (&self->app_local_file);

	/* save GsApp */
	_set_app (self, app);
	self->origin_by_packaging_format = TRUE;
	gs_details_page_load_stage1 (self);
}

GsApp *
gs_details_page_get_app (GsDetailsPage *self)
{
	return self->app;
}

static void
gs_details_page_remove_app (GsDetailsPage *self)
{
	g_set_object (&self->app_cancellable, gs_app_get_cancellable (self->app));
	gs_page_remove_app (GS_PAGE (self), self->app, self->app_cancellable);
}

static void
gs_details_page_app_remove_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	gs_details_page_remove_app (self);
}

static void
gs_details_page_app_cancel_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	g_cancellable_cancel (self->app_cancellable);
	gtk_widget_set_sensitive (widget, FALSE);

	/* reset the pending-action from the app if needed */
	gs_app_set_pending_action (self->app, GS_PLUGIN_ACTION_UNKNOWN);

	/* FIXME: We should be able to revert the QUEUED_FOR_INSTALL without
	 * having to pretend to remove the app */
	if (gs_app_get_state (self->app) == GS_APP_STATE_QUEUED_FOR_INSTALL)
		gs_details_page_remove_app (self);
}

static void
gs_details_page_app_install_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	switch (gs_app_get_state (self->app)) {
	case GS_APP_STATE_PENDING_INSTALL:
	case GS_APP_STATE_PENDING_REMOVE:
		g_return_if_fail (gs_app_has_quirk (self->app, GS_APP_QUIRK_NEEDS_REBOOT));
		gs_utils_invoke_reboot_async (NULL, NULL, NULL);
		return;
	default:
		break;
	}

	g_set_object (&self->app_cancellable, gs_app_get_cancellable (self->app));

	if (gs_app_get_state (self->app) == GS_APP_STATE_UPDATABLE_LIVE) {
		gs_page_update_app (GS_PAGE (self), self->app, self->app_cancellable);
		return;
	}

	gs_page_install_app (GS_PAGE (self), self->app, GS_SHELL_INTERACTION_FULL,
			     self->app_cancellable);
}

static void
gs_details_page_app_update_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	g_set_object (&self->app_cancellable, gs_app_get_cancellable (self->app));
	gs_page_update_app (GS_PAGE (self), self->app, self->app_cancellable);
}

static void
gs_details_page_addon_install_cb (GsAppAddonRow *row,
				  gpointer user_data)
{
	GsApp *addon;
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);

	addon = gs_app_addon_row_get_addon (row);
	gs_page_install_app (GS_PAGE (self), addon, GS_SHELL_INTERACTION_FULL, NULL);
}

static void
gs_details_page_addon_remove_cb (GsAppAddonRow *row, gpointer user_data)
{
	GsApp *addon;
	GsDetailsPage *self = GS_DETAILS_PAGE (user_data);

	addon = gs_app_addon_row_get_addon (row);
	gs_page_remove_app (GS_PAGE (self), addon, NULL);
}

static void
gs_details_page_app_launch_button_cb (GtkWidget *widget, GsDetailsPage *self)
{
	g_autoptr(GCancellable) cancellable = g_cancellable_new ();

	/* hide the notification */
	g_application_withdraw_notification (g_application_get_default (),
					     "installed");

	g_set_object (&self->cancellable, cancellable);
	g_cancellable_connect (cancellable, G_CALLBACK (gs_details_page_cancel_cb), self, NULL);
	gs_page_launch_app (GS_PAGE (self), self->app, self->cancellable);
}

static void
gs_details_page_review_send_cb (GtkDialog *dialog,
				GsDetailsPage *self)
{
	g_autofree gchar *text = NULL;
	g_autoptr(GDateTime) now = NULL;
	g_autoptr(AsReview) review = NULL;
	GsReviewDialog *rdialog = GS_REVIEW_DIALOG (dialog);
	g_autoptr(GError) local_error = NULL;

	review = as_review_new ();
	as_review_set_summary (review, gs_review_dialog_get_summary (rdialog));
	text = gs_review_dialog_get_text (rdialog);
	as_review_set_description (review, text);
	as_review_set_rating (review, gs_review_dialog_get_rating (rdialog));
	as_review_set_version (review, gs_app_get_version (self->app));
	now = g_date_time_new_now_local ();
	as_review_set_date (review, now);

	/* call into the plugins to set the new value */
	/* FIXME: Make this async */
	g_assert (self->odrs_provider != NULL);

	gs_odrs_provider_submit_review (self->odrs_provider, self->app, review,
					self->cancellable, &local_error);

	if (local_error != NULL) {
		g_autofree gchar *tmp = NULL;
		g_debug ("failed to submit review on '%s': %s",
			   gs_app_get_id (self->app), local_error->message);
		tmp = g_strdup_printf (_("Failed to submit review for “%s”: %s"), gs_app_get_name (self->app), local_error->message);
		gs_review_dialog_set_error_text (rdialog, tmp);
		return;
	}

	gs_details_page_refresh_reviews (self);

	/* unmap the dialog */
	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
gs_details_page_write_review (GsDetailsPage *self)
{
	GtkWidget *dialog;
	dialog = gs_review_dialog_new ();
	g_signal_connect (dialog, "send",
			  G_CALLBACK (gs_details_page_review_send_cb), self);
	gs_shell_modal_dialog_present (self->shell, GTK_WINDOW (dialog));
}

static void
gs_details_page_app_installed (GsPage *page, GsApp *app)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (page);
	g_autoptr(GsAppList) addons = NULL;
	guint i;

	/* if the app is just an addon, no need for a full refresh */
	addons = gs_app_dup_addons (self->app);
	for (i = 0; addons != NULL && i < gs_app_list_length (addons); i++) {
		GsApp *addon;
		addon = gs_app_list_index (addons, i);
		if (addon == app)
			return;
	}

	gs_details_page_reload (page);
}

static void
gs_details_page_app_removed (GsPage *page, GsApp *app)
{
	gs_details_page_app_installed (page, app);
}

static void
gs_details_page_network_available_notify_cb (GsPluginLoader *plugin_loader,
                                             GParamSpec *pspec,
                                             GsDetailsPage *self)
{
	gs_details_page_refresh_reviews (self);
}

static void
gs_details_page_star_pressed_cb (GtkGestureClick *click,
                                 gint             n_press,
                                 gdouble          x,
                                 gdouble          y,
                                 GsDetailsPage   *self)
{
	gs_details_page_write_review (self);
}

static void
gs_details_page_shell_allocation_width_cb (GObject *shell,
					   GParamSpec *pspec,
					   GsDetailsPage *self)
{
	gint allocation_width = 0;
	GtkOrientation orientation;

	g_object_get (shell, "allocation-width", &allocation_width, NULL);

	if (allocation_width > 0 && allocation_width < 500)
		orientation = GTK_ORIENTATION_VERTICAL;
	else
		orientation = GTK_ORIENTATION_HORIZONTAL;

	if (orientation != gtk_orientable_get_orientation (GTK_ORIENTABLE (self->box_details_header_not_icon)))
		gtk_orientable_set_orientation (GTK_ORIENTABLE (self->box_details_header_not_icon), orientation);
}

static gboolean
gs_details_page_setup (GsPage *page,
                       GsShell *shell,
                       GsPluginLoader *plugin_loader,
                       GCancellable *cancellable,
                       GError **error)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (page);

	g_return_val_if_fail (GS_IS_DETAILS_PAGE (self), FALSE);

	self->shell = shell;

	self->plugin_loader = g_object_ref (plugin_loader);
	self->cancellable = g_cancellable_new ();
	g_cancellable_connect (cancellable, G_CALLBACK (gs_details_page_cancel_cb), self, NULL);

	g_signal_connect_object (self->shell, "notify::allocation-width",
				 G_CALLBACK (gs_details_page_shell_allocation_width_cb),
				 self, 0);

	/* hide some UI when offline */
	g_signal_connect_object (self->plugin_loader, "notify::network-available",
				 G_CALLBACK (gs_details_page_network_available_notify_cb),
				 self, 0);
	return TRUE;
}

static gboolean
gs_details_page_should_show_title (GsDetailsPage *self)
{
	/* only when not scrolled at the very top */
	return ((gint) gs_details_page_get_vscroll_position (self)) >= 1;
}

static void
gs_details_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (object);

	switch ((GsDetailsPageProperty) prop_id) {
	case PROP_TITLE:
		switch (gs_details_page_get_state (self)) {
		case GS_DETAILS_PAGE_STATE_LOADING:
			/* 'Loading' is shown in the page already, no need to repeat it in the title */
			g_value_set_string (value, NULL);
			break;
		case GS_DETAILS_PAGE_STATE_READY:
			self->title_visible = gs_details_page_should_show_title (self);
			if (self->title_visible)
				g_value_set_string (value, gs_app_get_name (self->app));
			else
				g_value_set_string (value, NULL);
			break;
		case GS_DETAILS_PAGE_STATE_FAILED:
			g_value_set_string (value, NULL);
			break;
		default:
			g_assert_not_reached ();
		}
		break;
	case PROP_ODRS_PROVIDER:
		g_value_set_object (value, gs_details_page_get_odrs_provider (self));
		break;
	case PROP_IS_NARROW:
		g_value_set_boolean (value, gs_details_page_get_is_narrow (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_details_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (object);

	switch ((GsDetailsPageProperty) prop_id) {
	case PROP_TITLE:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_ODRS_PROVIDER:
		gs_details_page_set_odrs_provider (self, g_value_get_object (value));
		break;
	case PROP_IS_NARROW:
		gs_details_page_set_is_narrow (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_details_page_dispose (GObject *object)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (object);

	_set_app (self, NULL);

	g_clear_pointer (&self->packaging_format_preference, g_strfreev);
	g_clear_object (&self->origin_css_provider);
	g_clear_object (&self->developer_verified_image_css_provider);
	g_clear_object (&self->developer_verified_label_css_provider);
	g_clear_object (&self->app_local_file);
	g_clear_object (&self->app_reviews_dialog);
	g_clear_object (&self->plugin_loader);
	g_clear_object (&self->cancellable);
	g_clear_object (&self->app_cancellable);
	g_clear_object (&self->odrs_provider);
	g_clear_object (&self->app_info_monitor);
	g_clear_pointer (&self->last_developer_name, g_free);

	G_OBJECT_CLASS (gs_details_page_parent_class)->dispose (object);
}

static void
gs_details_page_class_init (GsDetailsPageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPageClass *page_class = GS_PAGE_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_details_page_get_property;
	object_class->set_property = gs_details_page_set_property;
	object_class->dispose = gs_details_page_dispose;

	page_class->app_installed = gs_details_page_app_installed;
	page_class->app_removed = gs_details_page_app_removed;
	page_class->switch_to = gs_details_page_switch_to;
	page_class->reload = gs_details_page_reload;
	page_class->setup = gs_details_page_setup;

	/**
	 * GsDetailsPage:odrs-provider: (nullable)
	 *
	 * An ODRS provider to give access to ratings and reviews information
	 * for the app being displayed.
	 *
	 * If this is %NULL, ratings and reviews will be disabled.
	 *
	 * Since: 41
	 */
	obj_props[PROP_ODRS_PROVIDER] =
		g_param_spec_object ("odrs-provider", NULL, NULL,
				     GS_TYPE_ODRS_PROVIDER,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsDetailsPage:is-narrow:
	 *
	 * Whether the page is in narrow mode.
	 *
	 * In narrow mode, the page will take up less horizontal space, doing so
	 * by e.g. turning horizontal boxes into vertical ones. This is needed
	 * to keep the UI useable on small form-factors like smartphones.
	 *
	 * Since: 41
	 */
	obj_props[PROP_IS_NARROW] =
		g_param_spec_boolean ("is-narrow", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	g_object_class_override_property (object_class, PROP_TITLE, "title");

	/**
	 * GsDetailsPage::metainfo-loaded:
	 * @app: a #GsApp
	 *
	 * Emitted after a custom metainfo @app is loaded in the page, but before
	 * it's fully shown.
	 *
	 * Since: 42
	 */
	signals[SIGNAL_METAINFO_LOADED] =
		g_signal_new ("metainfo-loaded",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, GS_TYPE_APP);

	/**
	 * GsDetailsPage::app-clicked:
	 * @app: the #GsApp which was clicked on
	 *
	 * Emitted when one of the app tiles is clicked. Typically the caller
	 * should display the details of the given app in the callback.
	 *
	 * Since: 43
	 */
	signals[SIGNAL_APP_CLICKED] =
		g_signal_new ("app-clicked",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1, GS_TYPE_APP);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-details-page.ui");

	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, application_details_icon);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, application_details_summary);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, application_details_title);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_addons);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_details);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_details_description);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_details_header);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_details_header_not_icon);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_webapp_warning);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, star);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_review_count);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, screenshot_carousel);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_details_launch);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, links_stack);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_no_metadata_info);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, project_website_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, donate_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, translate_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, report_an_issue_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, help_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, contact_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_install);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_update);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_remove);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_cancel);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_eol);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_eol);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_problems_label);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_app_norepo);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_app_repo);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_package_baseos);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_package_baseos);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_details_repo);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_app_data);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, infobar_app_data_label);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, context_bar);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_progress_percentage);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, label_progress_status);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, developer_name_label);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, developer_verified_image);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, developer_verified_label);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, page_failed);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, list_box_addons);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, list_box_featured_review);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, list_box_reviews_summary);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, list_box_version_history);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, row_latest_version);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, version_history_button);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_reviews);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_reviews_internal);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, histogram);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, histogram_row);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, button_review);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, scrolledwindow_details);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, spinner_details);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, stack_details);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_with_source);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, origin_popover);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, origin_popover_list_box);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, origin_box);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, origin_packaging_image);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, origin_packaging_label);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_license);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, license_tile);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, translation_banner);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, developer_apps_heading);
	gtk_widget_class_bind_template_child (widget_class, GsDetailsPage, box_developer_apps);

	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_link_row_activated_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_license_tile_get_involved_activated_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_translation_banner_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_star_pressed_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_install_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_update_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_remove_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_cancel_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_launch_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, gs_details_page_app_data_clear_button_cb);
	gtk_widget_class_bind_template_callback (widget_class, origin_popover_row_activated_cb);
	gtk_widget_class_bind_template_callback (widget_class, app_activated_cb);
}

static gboolean
narrow_to_orientation (GBinding *binding, const GValue *from_value, GValue *to_value, gpointer user_data)
{
	if (g_value_get_boolean (from_value))
		g_value_set_enum (to_value, GTK_ORIENTATION_VERTICAL);
	else
		g_value_set_enum (to_value, GTK_ORIENTATION_HORIZONTAL);

	return TRUE;
}

static gboolean
narrow_to_spacing (GBinding *binding, const GValue *from_value, GValue *to_value, gpointer user_data)
{
	if (g_value_get_boolean (from_value))
		g_value_set_int (to_value, 12);
	else
		g_value_set_int (to_value, 24);

	return TRUE;
}

static gboolean
narrow_to_halign (GBinding *binding, const GValue *from_value, GValue *to_value, gpointer user_data)
{
	if (g_value_get_boolean (from_value))
		g_value_set_enum (to_value, GTK_ALIGN_START);
	else
		g_value_set_enum (to_value, GTK_ALIGN_END);

	return TRUE;
}

static void
scrolledwindow_details_value_changed_cb (GtkAdjustment *adjustment,
					 GsDetailsPage *self)
{
	gboolean title_visible = gs_details_page_should_show_title (self);
	if ((!title_visible) != (!self->title_visible))
		g_object_notify (G_OBJECT (self), "title");
}

static void
gs_details_page_init (GsDetailsPage *self)
{
	GtkAdjustment *adjustment;

	g_type_ensure (GS_TYPE_APP_CONTEXT_BAR);
	g_type_ensure (GS_TYPE_APP_VERSION_HISTORY_ROW);
	g_type_ensure (GS_TYPE_DESCRIPTION_BOX);
	g_type_ensure (GS_TYPE_LICENSE_TILE);
	g_type_ensure (GS_TYPE_PROGRESS_BUTTON);
	g_type_ensure (GS_TYPE_REVIEW_HISTOGRAM);
	g_type_ensure (GS_TYPE_SCREENSHOT_CAROUSEL);
	g_type_ensure (GS_TYPE_STAR_WIDGET);

	gtk_widget_init_template (GTK_WIDGET (self));

	self->settings = g_settings_new ("org.lingmo.appstore");
	g_signal_connect_swapped (self->settings, "changed",
				  G_CALLBACK (settings_changed_cb),
				  self);
	self->app_info_monitor = g_app_info_monitor_get ();
	g_signal_connect_object (self->app_info_monitor, "changed",
				 G_CALLBACK (gs_details_page_app_info_changed_cb), self, 0);

	gtk_list_box_set_sort_func (GTK_LIST_BOX (self->list_box_addons),
				    list_sort_func,
				    self, NULL);

	g_signal_connect (self->list_box_addons, "row-activated",
			  G_CALLBACK (addons_list_row_activated_cb), self);

	g_signal_connect (self->list_box_version_history, "row-activated",
			  G_CALLBACK (version_history_list_row_activated_cb), self);

	g_signal_connect_swapped (self->list_box_reviews_summary, "row-activated",
				  G_CALLBACK (gs_details_page_write_review), self);

	g_signal_connect (self->list_box_featured_review, "row-activated",
			  G_CALLBACK (featured_review_list_row_activated_cb), self);

	gs_details_page_read_packaging_format_preference (self);

	g_object_bind_property_full (self, "is-narrow", self->box_details_header, "spacing", G_BINDING_SYNC_CREATE,
				     narrow_to_spacing, NULL, NULL, NULL);
	g_object_bind_property_full (self, "is-narrow", self->box_with_source, "halign", G_BINDING_SYNC_CREATE,
				     narrow_to_halign, NULL, NULL, NULL);
	g_object_bind_property_full (self, "is-narrow", self->box_license, "orientation", G_BINDING_SYNC_CREATE,
				     narrow_to_orientation, NULL, NULL, NULL);
	g_object_bind_property_full (self, "is-narrow", self->context_bar, "orientation", G_BINDING_SYNC_CREATE,
				     narrow_to_orientation, NULL, NULL, NULL);
	g_object_bind_property_full (self, "is-narrow", self->box_reviews_internal, "orientation", G_BINDING_SYNC_CREATE,
				     narrow_to_orientation, NULL, NULL, NULL);

	adjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_details));
	g_signal_connect_object (adjustment, "value-changed",
				 G_CALLBACK (scrolledwindow_details_value_changed_cb), self, 0);
}

GsDetailsPage *
gs_details_page_new (void)
{
	return GS_DETAILS_PAGE (g_object_new (GS_TYPE_DETAILS_PAGE, NULL));
}

/**
 * gs_details_page_get_odrs_provider:
 * @self: a #GsDetailsPage
 *
 * Get the value of #GsDetailsPage:odrs-provider.
 *
 * Returns: (nullable) (transfer none): a #GsOdrsProvider, or %NULL if unset
 * Since: 41
 */
GsOdrsProvider *
gs_details_page_get_odrs_provider (GsDetailsPage *self)
{
	g_return_val_if_fail (GS_IS_DETAILS_PAGE (self), NULL);

	return self->odrs_provider;
}

/**
 * gs_details_page_set_odrs_provider:
 * @self: a #GsDetailsPage
 * @odrs_provider: (nullable) (transfer none): new #GsOdrsProvider or %NULL
 *
 * Set the value of #GsDetailsPage:odrs-provider.
 *
 * Since: 41
 */
void
gs_details_page_set_odrs_provider (GsDetailsPage  *self,
                                   GsOdrsProvider *odrs_provider)
{
	g_return_if_fail (GS_IS_DETAILS_PAGE (self));
	g_return_if_fail (odrs_provider == NULL || GS_IS_ODRS_PROVIDER (odrs_provider));

	if (g_set_object (&self->odrs_provider, odrs_provider)) {
		gs_details_page_refresh_reviews (self);
		g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_ODRS_PROVIDER]);
	}
}

/**
 * gs_details_page_get_is_narrow:
 * @self: a #GsDetailsPage
 *
 * Get the value of #GsDetailsPage:is-narrow.
 *
 * Returns: %TRUE if the page is in narrow mode, %FALSE otherwise
 *
 * Since: 41
 */
gboolean
gs_details_page_get_is_narrow (GsDetailsPage *self)
{
	g_return_val_if_fail (GS_IS_DETAILS_PAGE (self), FALSE);

	return self->is_narrow;
}

/**
 * gs_details_page_set_is_narrow:
 * @self: a #GsDetailsPage
 * @is_narrow: %TRUE to set the page in narrow mode, %FALSE otherwise
 *
 * Set the value of #GsDetailsPage:is-narrow.
 *
 * Since: 41
 */
void
gs_details_page_set_is_narrow (GsDetailsPage *self, gboolean is_narrow)
{
	g_return_if_fail (GS_IS_DETAILS_PAGE (self));

	is_narrow = !!is_narrow;

	if (self->is_narrow == is_narrow)
		return;

	self->is_narrow = is_narrow;
	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_IS_NARROW]);
}

static void
gs_details_page_metainfo_ready_cb (GObject *source_object,
				   GAsyncResult *result,
				   gpointer user_data)
{
	GsDetailsPage *self = GS_DETAILS_PAGE (source_object);
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;

	app = g_task_propagate_pointer (G_TASK (result), &error);
	if (error) {
		adw_status_page_set_title (ADW_STATUS_PAGE (self->page_failed), error->message);
		gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_FAILED);
		return;
	}

	g_set_object (&self->app_local_file, app);
	_set_app (self, app);
	gs_details_page_load_stage2 (self, FALSE);

	g_signal_emit (self, signals[SIGNAL_METAINFO_LOADED], 0, app);
}

static void
gs_details_page_metainfo_thread (GTask *task,
				 gpointer source_object,
				 gpointer task_data,
				 GCancellable *cancellable)
{
	g_autofree gchar *path = NULL;
	g_autofree gchar *icon_path = NULL;
	g_autoptr(XbBuilder) builder = NULL;
	g_autoptr(XbBuilderSource) builder_source = NULL;
	g_autoptr(XbSilo) silo = NULL;
	g_autoptr(GPtrArray) nodes = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GFile) tmp_file = NULL;
	GFile *file = task_data;
	XbNode *component;

	path = g_file_get_path (file);
	if (path && strstr (path, ",icon=")) {
		gchar *pos = strstr (path, ",icon=");

		*pos = '\0';

		tmp_file = g_file_new_for_path (path);
		file = tmp_file;

		pos += 6;
		if (*pos)
			icon_path = g_strdup (pos);
	}
	g_clear_pointer (&path, g_free);

	builder_source = xb_builder_source_new ();
	if (!xb_builder_source_load_file (builder_source, file, XB_BUILDER_SOURCE_FLAG_NONE, cancellable, &error)) {
		g_task_return_error (task, g_steal_pointer (&error));
		return;
	}

	builder = xb_builder_new ();

	gs_appstream_add_current_locales (builder);

	xb_builder_import_source (builder, builder_source);

	silo = xb_builder_compile (builder, XB_BUILDER_COMPILE_FLAG_IGNORE_INVALID | XB_BUILDER_COMPILE_FLAG_SINGLE_LANG, cancellable, &error);
	if (silo == NULL) {
		g_task_return_error (task, g_steal_pointer (&error));
		return;
	}

	nodes = xb_silo_query (silo, "component", 0, NULL);
	if (nodes == NULL)
		nodes = xb_silo_query (silo, "application", 0, NULL);
	if (nodes == NULL) {
		g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_FAILED, "%s",
			"Passed-in file doesn't have a 'component' (nor 'app') top-level element");
		return;
	}

	if (nodes->len != 1) {
		g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_FAILED,
			"Only one top-level element expected, received %u instead", nodes->len);
		return;
	}

	component = g_ptr_array_index (nodes, 0);

	app = gs_appstream_create_app (NULL, silo, component, NULL, AS_COMPONENT_SCOPE_UNKNOWN, &error);
	if (app == NULL) {
		g_task_return_error (task, g_steal_pointer (&error));
		return;
	}

	if (!gs_appstream_refine_app (NULL, app, silo, component, GS_DETAILS_PAGE_REFINE_FLAGS, NULL, NULL, AS_COMPONENT_SCOPE_UNKNOWN, &error)) {
		g_task_return_error (task, g_steal_pointer (&error));
		return;
	}

	path = g_file_get_path (file);
	gs_app_set_origin (app, path);

	if (icon_path) {
		g_autoptr(GFile) icon_file = g_file_new_for_path (icon_path);
		g_autoptr(GIcon) icon = g_file_icon_new (icon_file);
		gs_icon_set_width (icon, (guint) -1);
		gs_app_add_icon (app, G_ICON (icon));
	} else {
		g_autoptr(SoupSession) soup_session = NULL;
		guint maximum_icon_size;

		/* Currently a 160px icon is needed for #GsFeatureTile, at most.
		 * The '2' is to pretend the hiDPI/GDK's scale factor is 2, to
		 * allow larger icons. The 'icons' plugin uses proper scale factor.
		 */
		maximum_icon_size = 160 * 2;

		soup_session = gs_build_soup_session ();
		gs_app_ensure_icons_downloaded (app, soup_session, maximum_icon_size, cancellable);
	}

	gs_app_set_state (app, GS_APP_STATE_UNKNOWN);

	g_task_return_pointer (task, g_steal_pointer (&app), g_object_unref);
}

/**
 * gs_details_page_set_metainfo:
 * @self: a #GsDetailsPage
 * @file: path to a metainfo file to display
 *
 * Load and show the given metainfo @file on the details page.
 *
 * The file must be a single metainfo file, not an appstream file
 * containing multiple components. It will be shown as if it came
 * from a configured repository. This function is intended to be
 * used by application developers wanting to test how their metainfo
 * will appear to users.
 *
 * Since: 42
 */
void
gs_details_page_set_metainfo (GsDetailsPage *self,
			      GFile *file)
{
	g_autoptr(GTask) task = NULL;

	g_return_if_fail (GS_IS_DETAILS_PAGE (self));
	g_return_if_fail (G_IS_FILE (file));
	gs_details_page_set_state (self, GS_DETAILS_PAGE_STATE_LOADING);
	g_clear_object (&self->app_local_file);
	_set_app (self, NULL);
	self->origin_by_packaging_format = FALSE;
	task = g_task_new (self, self->cancellable, gs_details_page_metainfo_ready_cb, NULL);
	g_task_set_source_tag (task, gs_details_page_set_metainfo);
	g_task_set_task_data (task, g_object_ref (file), g_object_unref);
	g_task_run_in_thread (task, gs_details_page_metainfo_thread);
}

gdouble
gs_details_page_get_vscroll_position (GsDetailsPage *self)
{
	GtkAdjustment *adj;

	g_return_val_if_fail (GS_IS_DETAILS_PAGE (self), -1);

	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_details));
	return gtk_adjustment_get_value (adj);
}

void
gs_details_page_set_vscroll_position (GsDetailsPage *self,
				      gdouble value)
{
	GtkAdjustment *adj;

	g_return_if_fail (GS_IS_DETAILS_PAGE (self));

	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->scrolledwindow_details));
	if (value >= 0.0)
		gtk_adjustment_set_value (adj, value);
}

