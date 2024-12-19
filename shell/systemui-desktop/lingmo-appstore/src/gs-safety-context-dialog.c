/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-safety-context-dialog
 * @short_description: A dialog showing safety information about an app
 *
 * #GsSafetyContextDialog is a dialog which shows detailed information about
 * how safe or trustworthy an app is. This information is derived from the
 * permissions the app requires to run, its runtime, origin, and various other
 * sources.
 *
 * It is designed to show a more detailed view of the information which the
 * app’s safety tile in #GsAppContextBar is derived from.
 *
 * The widget has no special appearance if the app is unset, so callers will
 * typically want to hide the dialog in that case.
 *
 * Since: 41
 */

#include "config.h"

#include <adwaita.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "gs-app.h"
#include "gs-common.h"
#include "gs-context-dialog-row.h"
#include "gs-lozenge.h"
#include "gs-safety-context-dialog.h"

struct _GsSafetyContextDialog
{
	GsInfoWindow		 parent_instance;

	GsApp			*app;  /* (nullable) (owned) */
	gulong			 app_notify_handler_permissions;
	gulong			 app_notify_handler_name;
	gulong			 app_notify_handler_quirk;
	gulong			 app_notify_handler_license;
	gulong			 app_notify_handler_related;

	GtkWidget		*lozenge;
	GtkLabel		*title;
	GtkListBox		*permissions_list;

	GtkLabel		*license_label;
	GBinding		*license_label_binding;  /* (owned) (nullable) */
	GtkLabel		*source_label;
	GBinding		*source_label_binding;  /* (owned) (nullable) */
	GtkWidget		*packagename_row;
	GtkLabel		*packagename_title;
	GtkLabel		*packagename_value;
	GtkLabel		*sdk_label;
	GtkImage		*sdk_eol_image;
	GtkWidget		*sdk_row;
};

G_DEFINE_TYPE (GsSafetyContextDialog, gs_safety_context_dialog, GS_TYPE_INFO_WINDOW)

typedef enum {
	PROP_APP = 1,
} GsSafetyContextDialogProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

/* @icon_name_without_permission, @title_without_permission and
 * @description_without_permission are all nullable. If they are NULL, no row
 * is added if @has_permission is false. */
static void
add_permission_row (GtkListBox                   *list_box,
                    GsContextDialogRowImportance *chosen_rating,
                    gboolean                      has_permission,
                    GsContextDialogRowImportance  item_rating,
                    const gchar                  *icon_name_with_permission,
                    const gchar                  *title_with_permission,
                    const gchar                  *description_with_permission,
                    const gchar                  *icon_name_without_permission,
                    const gchar                  *title_without_permission,
                    const gchar                  *description_without_permission)
{
	GtkListBoxRow *row;

	if (has_permission && item_rating > *chosen_rating)
		*chosen_rating = item_rating;

	if (!has_permission && title_without_permission == NULL)
		return;

	row = gs_context_dialog_row_new (has_permission ? icon_name_with_permission : icon_name_without_permission,
					 has_permission ? item_rating : GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT,
					 has_permission ? title_with_permission : title_without_permission,
					 has_permission ? description_with_permission : description_without_permission);
	gtk_list_box_append (list_box, GTK_WIDGET (row));
}

static void
update_permissions_list (GsSafetyContextDialog *self)
{
	const gchar *icon_name, *css_class;
	g_autofree gchar *title = NULL;
	g_autoptr(GsAppPermissions) permissions = NULL;
	GsAppPermissionsFlags perm_flags = GS_APP_PERMISSIONS_FLAGS_NONE;
	GsContextDialogRowImportance chosen_rating;
	GsContextDialogRowImportance license_rating;

	/* Treat everything as safe to begin with, and downgrade its safety
	 * based on app properties. */
	chosen_rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT;

	gs_widget_remove_all (GTK_WIDGET (self->permissions_list), (GsRemoveFunc) gtk_list_box_remove);

	/* UI state is undefined if app is not set. */
	if (self->app == NULL)
		return;

	permissions = gs_app_dup_permissions (self->app);
	if (permissions != NULL)
		perm_flags = gs_app_permissions_get_flags (permissions);

	/* Handle unknown permissions. This means the app isn’t
	 * sandboxed, so we can only really base decisions on whether it was
	 * packaged by an organisation we trust or not.
	 *
	 * FIXME: See the comment for GS_APP_PERMISSIONS_FLAGS_UNKNOWN in
	 * gs-app-context-bar.c. */
	if (permissions == NULL) {
		chosen_rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL;

		if (gs_app_has_quirk (self->app, GS_APP_QUIRK_PROVENANCE)) {
			/* It's a new key suggested at https://github.com/systemd/systemd/issues/27777 */
			g_autofree gchar *name = g_get_os_info ("VENDOR_NAME");
			g_autofree gchar *reviewed_by = NULL;
			if (name == NULL) {
				reviewed_by = g_strdup (_("Reviewed by OS distributor"));
			} else {
				/* Translators: The '%s' is replaced by the distribution name. */
				reviewed_by = g_strdup_printf (_("Reviewed by %s"), name);
			}
			add_permission_row (self->permissions_list, &chosen_rating,
					    TRUE,
					    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL,
					    "channel-secure-symbolic",
					    reviewed_by,
					    _("App isn’t sandboxed but the distribution has checked that it is not malicious"),
					    NULL, NULL, NULL);
		} else {
			add_permission_row (self->permissions_list, &chosen_rating,
					    TRUE,
					    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
					    "channel-insecure-symbolic",
					    _("Provided by a third party"),
					    _("Check that you trust the vendor, as the app isn’t sandboxed"),
					    NULL, NULL, NULL);
		}
	} else {
		const GPtrArray *filesystem_read, *filesystem_full;

		filesystem_read = gs_app_permissions_get_filesystem_read (permissions);
		filesystem_full = gs_app_permissions_get_filesystem_full (permissions);

		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags == GS_APP_PERMISSIONS_FLAGS_NONE) &&
				    filesystem_read == NULL && filesystem_full == NULL,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT,
				    "folder-documents-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("No Permissions"),
				    _("App is fully sandboxed"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_NETWORK) != 0,
				    /* This isn’t actually unimportant (network access can expand a local
				     * vulnerability into a remotely exploitable one), but it’s
				     * needed commonly enough that marking it as
				     * %GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING is too noisy. */
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION,
				    "network-wireless-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Network Access"),
				    _("Can access the internet"),
				    "network-wireless-disabled-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("No Network Access"),
				    _("Cannot access the internet"));
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_SYSTEM_BUS) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "emblem-system-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Uses System Services"),
				    _("Can request data from system services"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_SESSION_BUS) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "emblem-system-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Uses Session Services"),
				    _("Can request data from session services"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_DEVICES) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "camera-photo-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("User Device Access"),
				    _("Can access devices such as webcams or gaming controllers"),
				    "camera-disabled-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("No User Device Access"),
				    _("Cannot access devices such as webcams or gaming controllers"));
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_INPUT_DEVICES) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION,
				    "input-keyboard-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Input Device Access"),
				    _("Can access input devices"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_PULSEAUDIO_DEVICES) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION,
				    "audio-input-microphone-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Microphone Access"),
				    _("Can listen using microphones without asking permission"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_SYSTEM_DEVICES) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "computer-chip-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("System Device Access"),
				    _("Can access system devices which require elevated permissions"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_SCREEN) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "screen-privacy-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Screen Contents Access"),
				    _("Can access the contents of the screen or other windows"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_X11) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "desktop-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Legacy Windowing System"),
				    _("Uses a legacy windowing system"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "dialog-warning-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Arbitrary Permissions"),
				    _("Can acquire arbitrary permissions"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_SETTINGS) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "preferences-system-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("User Settings"),
				    _("Can access and change user settings"),
				    NULL, NULL, NULL);

		/* File system permissions are a bit more complex, since there are
		 * varying scopes of what’s readable/writable, and a difference between
		 * read-only and writable access. */
		add_permission_row (self->permissions_list, &chosen_rating,
				    (perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL) != 0,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "folder-documents-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Full File System Read/Write Access"),
				    _("Can read and write all data on the file system"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    ((perm_flags & GS_APP_PERMISSIONS_FLAGS_HOME_FULL) != 0 &&
				     !(perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL)),
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "user-home-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Home Folder Read/Write Access"),
				    _("Can read and write all data in your home directory"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    ((perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ) != 0 &&
				     !(perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL)),
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "folder-documents-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Full File System Read Access"),
				    _("Can read all data on the file system"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    ((perm_flags & GS_APP_PERMISSIONS_FLAGS_HOME_READ) != 0 &&
				     !(perm_flags & (GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL |
						     GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ))),
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "user-home-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Home Folder Read Access"),
				    _("Can read all data in your home directory"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    ((perm_flags & GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL) != 0 &&
				     !(perm_flags & (GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL |
						     GS_APP_PERMISSIONS_FLAGS_HOME_FULL))),
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "folder-download-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Download Folder Read/Write Access"),
				    _("Can read and write all data in your downloads directory"),
				    NULL, NULL, NULL);
		add_permission_row (self->permissions_list, &chosen_rating,
				    ((perm_flags & GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ) != 0 &&
				     !(perm_flags & (GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL |
						     GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ |
						     GS_APP_PERMISSIONS_FLAGS_HOME_FULL |
						     GS_APP_PERMISSIONS_FLAGS_HOME_READ))),
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
				    "folder-download-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Download Folder Read Access"),
				    _("Can read all data in your downloads directory"),
				    NULL, NULL, NULL);

		for (guint i = 0; filesystem_full != NULL && i < filesystem_full->len; i++) {
			const gchar *fs_title = g_ptr_array_index (filesystem_full, i);
			add_permission_row (self->permissions_list, &chosen_rating,
					    TRUE,
					    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
					    "folder-documents-symbolic",
					    fs_title,
					    _("Can read and write all data in the directory"),
					    NULL, NULL, NULL);
		}

		for (guint i = 0; filesystem_read != NULL && i < filesystem_read->len; i++) {
			const gchar *fs_title = g_ptr_array_index (filesystem_read, i);
			add_permission_row (self->permissions_list, &chosen_rating,
					    TRUE,
					    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
					    "folder-documents-symbolic",
					    fs_title,
					    _("Can read all data in the directory"),
					    NULL, NULL, NULL);
		}

		add_permission_row (self->permissions_list, &chosen_rating,
				    !(perm_flags & (GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL |
						    GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ |
						    GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_OTHER |
						    GS_APP_PERMISSIONS_FLAGS_HOME_FULL |
						    GS_APP_PERMISSIONS_FLAGS_HOME_READ |
						    GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL |
						    GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ)) &&
				    filesystem_read == NULL && filesystem_full == NULL,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT,
				    "folder-documents-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("No File System Access"),
				    _("Cannot access the file system at all"),
				    NULL, NULL, NULL);
	}

	add_permission_row (self->permissions_list, &chosen_rating,
			    gs_app_has_quirk (self->app, GS_APP_QUIRK_DEVELOPER_VERIFIED),
			    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT,
			    "app-installed-symbolic",
			    /* Translators: This indicates an app was written and released by a developer who has been verified.
			     * It’s used in a context tile, so should be short. */
			    _("App developer is verified"),
			    _("The developer of this app has been verified to be who they say they are"),
			    NULL, NULL, NULL);

	add_permission_row (self->permissions_list, &chosen_rating,
			    gs_app_get_metadata_item (self->app, "LingmoAppstore::EolReason") != NULL || (
			    gs_app_get_runtime (self->app) != NULL &&
			    gs_app_get_metadata_item (gs_app_get_runtime (self->app), "LingmoAppstore::EolReason") != NULL),
			    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
			    "dialog-warning-symbolic",
			    /* Translators: This indicates an app uses an outdated SDK.
			     * It’s used in a context tile, so should be short. */
			    _("Insecure Dependencies"),
			    _("Appstore or its dependencies are no longer supported and may be insecure"),
			    NULL, NULL, NULL);

	license_rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION;

	if (gs_app_get_license (self->app) == NULL) {
		add_permission_row (self->permissions_list, &chosen_rating,
				    TRUE,
				    GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL,
				    "dialog-warning-symbolic",
				    /* Translators: This indicates an app does not specify which license it's developed under. */
				    _("Unknown License"),
				    gs_app_is_application (self->app) ?
				    _("This app does not specify what license it is developed under, and may be proprietary") :
				    _("This software does not specify what license it is developed under, and may be proprietary"),
				    NULL, NULL, NULL);
	/* Is the code FOSS and hence inspectable? This doesn’t distinguish
	 * between closed source and open-source-but-not-FOSS software, even
	 * though the code of the latter is technically publicly auditable. This
	 * is because I don’t want to get into the business of maintaining lists
	 * of ‘auditable’ source code licenses. */
	} else if (g_ascii_strncasecmp (gs_app_get_license (self->app), "LicenseRef-proprietary", strlen ("LicenseRef-proprietary")) == 0) {
		add_permission_row (self->permissions_list, &chosen_rating,
				    TRUE,
				    license_rating,
				    "dialog-warning-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Proprietary Code"),
				    _("The source code is not public, so it cannot be independently audited and might be unsafe"),
				    NULL, NULL, NULL);
	} else {
		g_autofree gchar *description = NULL;

		if (!gs_app_get_license_is_free (self->app)) {
			if (gs_app_is_application (self->app)) {
				/* Translators: The placeholder here is the name of a software license. */
				description = g_strdup_printf (_("This app is developed under the special license “%s”"), gs_app_get_license (self->app));
			} else {
				/* Translators: The placeholder here is the name of a software license. */
				description = g_strdup_printf (_("This software is developed under the special license “%s”"), gs_app_get_license (self->app));
			}
		}

		add_permission_row (self->permissions_list, &chosen_rating,
				    !gs_app_get_license_is_free (self->app),
				    license_rating,
				    "dialog-warning-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Special License"),
				    description,
				    "app-installed-symbolic",
				    /* Translators: This refers to permissions (for example, from flatpak) which an app requests from the user. */
				    _("Auditable Code"),
				    _("The source code is public and can be independently audited, which makes the app more likely to be safe"));
	}

	/* Update the UI. */
	switch (chosen_rating) {
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered privileged, aka provided by the distribution.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s is privileged"), gs_app_get_name (self->app));
		css_class = "grey";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered safe to install and run.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s is safe"), gs_app_get_name (self->app));
		css_class = "green";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered probably safe to install and run.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s is probably safe"), gs_app_get_name (self->app));
		css_class = "yellow";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING:
		icon_name = "dialog-question-symbolic";
		/* Translators: The app is considered potentially unsafe to install and run.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s is potentially unsafe"), gs_app_get_name (self->app));
		css_class = "orange";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT:
		icon_name = "dialog-warning-symbolic";
		/* Translators: The app is considered unsafe to install and run.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s is unsafe"), gs_app_get_name (self->app));
		css_class = "red";
		break;
	default:
		g_assert_not_reached ();
	}

	gs_lozenge_set_icon_name (GS_LOZENGE (self->lozenge), icon_name);
	gtk_label_set_text (self->title, title);

	gtk_widget_remove_css_class (self->lozenge, "green");
	gtk_widget_remove_css_class (self->lozenge, "yellow");
	gtk_widget_remove_css_class (self->lozenge, "orange");
	gtk_widget_remove_css_class (self->lozenge, "red");

	gtk_widget_add_css_class (self->lozenge, css_class);
}

static void
app_notify_cb (GObject    *obj,
               GParamSpec *pspec,
               gpointer    user_data)
{
	GsSafetyContextDialog *self = GS_SAFETY_CONTEXT_DIALOG (user_data);

	update_permissions_list (self);
}

static void
update_sdk (GsSafetyContextDialog *self)
{
	GsApp *runtime;

	/* UI state is undefined if app is not set. */
	if (self->app == NULL)
		return;

	runtime = gs_app_get_runtime (self->app);

	if (runtime != NULL) {
		g_autofree gchar *label = NULL;
		const gchar *version = gs_app_get_version_ui (runtime);
		gboolean is_eol = gs_app_get_metadata_item (runtime, "LingmoAppstore::EolReason") != NULL;

		if (version != NULL) {
			/* Translators: The first placeholder is an app runtime
			 * name, the second is its version number. */
			label = g_strdup_printf (_("%s (%s)"),
						 gs_app_get_name (runtime),
						 version);
		} else {
			label = g_strdup (gs_app_get_name (runtime));
		}

		gtk_label_set_label (self->sdk_label, label);

		if (is_eol) {
			gtk_widget_add_css_class (GTK_WIDGET (self->sdk_label), "eol-red");
			gtk_widget_remove_css_class (GTK_WIDGET (self->sdk_label), "dim-label");
		} else {
			gtk_widget_add_css_class (GTK_WIDGET (self->sdk_label), "dim-label");
			gtk_widget_remove_css_class (GTK_WIDGET (self->sdk_label), "eol-red");
		}

		gtk_widget_set_visible (GTK_WIDGET (self->sdk_eol_image), is_eol);
	}

	/* Only show the row if a runtime was found. */
	gtk_widget_set_visible (self->sdk_row, (runtime != NULL));
}

static void
app_notify_related_cb (GObject    *obj,
                       GParamSpec *pspec,
                       gpointer    user_data)
{
	GsSafetyContextDialog *self = GS_SAFETY_CONTEXT_DIALOG (user_data);

	update_sdk (self);
}

static gboolean
sanitize_license_text_cb (GBinding *binding,
			  const GValue *from_value,
			  GValue *to_value,
			  gpointer user_data)
{
	const gchar *license = g_value_get_string (from_value);

	if (license == NULL)
		/* Translators: This is used for "License    Unknown" */
		g_value_set_string (to_value, C_("Unknown license", "Unknown"));
	else if (g_ascii_strncasecmp (license, "LicenseRef-proprietary", strlen ("LicenseRef-proprietary")) == 0)
		/* Translators: This is used for "License    Proprietary" */
		g_value_set_string (to_value, _("Proprietary"));
	else
		g_value_set_string (to_value, license);

	return TRUE;
}

static void
gs_safety_context_dialog_init (GsSafetyContextDialog *self)
{
	g_type_ensure (GS_TYPE_LOZENGE);

	gtk_widget_init_template (GTK_WIDGET (self));
}

static void
gs_safety_context_dialog_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
	GsSafetyContextDialog *self = GS_SAFETY_CONTEXT_DIALOG (object);

	switch ((GsSafetyContextDialogProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, gs_safety_context_dialog_get_app (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_safety_context_dialog_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
	GsSafetyContextDialog *self = GS_SAFETY_CONTEXT_DIALOG (object);

	switch ((GsSafetyContextDialogProperty) prop_id) {
	case PROP_APP:
		gs_safety_context_dialog_set_app (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_safety_context_dialog_dispose (GObject *object)
{
	GsSafetyContextDialog *self = GS_SAFETY_CONTEXT_DIALOG (object);

	gs_safety_context_dialog_set_app (self, NULL);

	G_OBJECT_CLASS (gs_safety_context_dialog_parent_class)->dispose (object);
}

static void
gs_safety_context_dialog_class_init (GsSafetyContextDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_safety_context_dialog_get_property;
	object_class->set_property = gs_safety_context_dialog_set_property;
	object_class->dispose = gs_safety_context_dialog_dispose;

	/**
	 * GsSafetyContextDialog:app: (nullable)
	 *
	 * The app to display the safety context details for.
	 *
	 * This may be %NULL; if so, the content of the widget will be
	 * undefined.
	 *
	 * Since: 41
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", NULL, NULL,
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-safety-context-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, lozenge);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, title);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, permissions_list);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, license_label);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, source_label);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, packagename_row);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, packagename_title);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, packagename_value);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, sdk_label);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, sdk_eol_image);
	gtk_widget_class_bind_template_child (widget_class, GsSafetyContextDialog, sdk_row);
}

/**
 * gs_safety_context_dialog_new:
 * @app: (nullable): the app to display safety context information for, or %NULL
 *
 * Create a new #GsSafetyContextDialog and set its initial app to @app.
 *
 * Returns: (transfer full): a new #GsSafetyContextDialog
 * Since: 41
 */
GsSafetyContextDialog *
gs_safety_context_dialog_new (GsApp *app)
{
	g_return_val_if_fail (app == NULL || GS_IS_APP (app), NULL);

	return g_object_new (GS_TYPE_SAFETY_CONTEXT_DIALOG,
			     "app", app,
			     NULL);
}

/**
 * gs_safety_context_dialog_get_app:
 * @self: a #GsSafetyContextDialog
 *
 * Gets the value of #GsSafetyContextDialog:app.
 *
 * Returns: (nullable) (transfer none): app whose safety context information is
 *     being displayed, or %NULL if none is set
 * Since: 41
 */
GsApp *
gs_safety_context_dialog_get_app (GsSafetyContextDialog *self)
{
	g_return_val_if_fail (GS_IS_SAFETY_CONTEXT_DIALOG (self), NULL);

	return self->app;
}

/**
 * gs_safety_context_dialog_set_app:
 * @self: a #GsSafetyContextDialog
 * @app: (nullable) (transfer none): the app to display safety context
 *     information for, or %NULL for none
 *
 * Set the value of #GsSafetyContextDialog:app.
 *
 * Since: 41
 */
void
gs_safety_context_dialog_set_app (GsSafetyContextDialog *self,
                                  GsApp                 *app)
{
	g_return_if_fail (GS_IS_SAFETY_CONTEXT_DIALOG (self));
	g_return_if_fail (app == NULL || GS_IS_APP (app));

	if (app == self->app)
		return;

	g_clear_signal_handler (&self->app_notify_handler_permissions, self->app);
	g_clear_signal_handler (&self->app_notify_handler_name, self->app);
	g_clear_signal_handler (&self->app_notify_handler_quirk, self->app);
	g_clear_signal_handler (&self->app_notify_handler_license, self->app);
	g_clear_signal_handler (&self->app_notify_handler_related, self->app);

	g_clear_object (&self->license_label_binding);
	g_clear_object (&self->source_label_binding);

	g_set_object (&self->app, app);

	if (self->app != NULL) {
		const gchar *packagename_value;

		self->app_notify_handler_permissions = g_signal_connect (self->app, "notify::permissions", G_CALLBACK (app_notify_cb), self);
		self->app_notify_handler_name = g_signal_connect (self->app, "notify::name", G_CALLBACK (app_notify_cb), self);
		self->app_notify_handler_quirk = g_signal_connect (self->app, "notify::quirk", G_CALLBACK (app_notify_cb), self);
		self->app_notify_handler_license = g_signal_connect (self->app, "notify::license", G_CALLBACK (app_notify_cb), self);

		self->app_notify_handler_related = g_signal_connect (self->app, "notify::related", G_CALLBACK (app_notify_related_cb), self);

		self->license_label_binding = g_object_bind_property_full (self->app, "license", self->license_label, "label", G_BINDING_SYNC_CREATE,
									   sanitize_license_text_cb, NULL, NULL, NULL);
		self->source_label_binding = g_object_bind_property (self->app, "origin-ui", self->source_label, "label", G_BINDING_SYNC_CREATE);

		packagename_value = gs_app_get_metadata_item (app, "LingmoAppstore::packagename-value");
		if (packagename_value != NULL && *packagename_value != '\0') {
			const gchar *packagename_title = gs_app_get_metadata_item (app, "LingmoAppstore::packagename-title");
			if (packagename_title == NULL || *packagename_title == '\0') {
				/* Translators: This is a heading for a row showing the package name of an app (such as ‘lingmo-appstore-46.0-1’). */
				packagename_title = _("Package");
			}
			gtk_label_set_label (self->packagename_title, packagename_title);
			gtk_label_set_label (self->packagename_value, packagename_value);
		}

		gtk_widget_set_visible (self->packagename_row, packagename_value != NULL && *packagename_value != '\0');
	} else {
		gtk_widget_set_visible (self->packagename_row, FALSE);
	}

	/* Update the UI. */
	update_permissions_list (self);
	update_sdk (self);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
}
