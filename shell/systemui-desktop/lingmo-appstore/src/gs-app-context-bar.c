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
 * SECTION:gs-app-context-bar
 * @short_description: A bar containing context tiles describing an app
 *
 * #GsAppContextBar is a bar which contains ‘context tiles’ to describe some of
 * the key features of an app. Each tile describes one aspect of the app, such
 * as its download/installed size, hardware requirements, or content rating.
 * Tiles are intended to convey the most pertinent information about aspects of
 * the app, leaving further detail to be shown in a more detailed dialog.
 *
 * The widget has no special appearance if the app is unset, so callers will
 * typically want to hide the bar in that case.
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

#include "gs-age-rating-context-dialog.h"
#include "gs-app.h"
#include "gs-app-context-bar.h"
#include "gs-common.h"
#include "gs-hardware-support-context-dialog.h"
#include "gs-lozenge.h"
#include "gs-safety-context-dialog.h"
#include "gs-storage-context-dialog.h"

typedef struct
{
	GtkWidget	*tile;
	GtkWidget	*lozenge;
	GtkLabel	*title;
	GtkLabel	*description;
} GsAppContextTile;

typedef enum
{
	STORAGE_TILE,
	SAFETY_TILE,
	HARDWARE_SUPPORT_TILE,
	AGE_RATING_TILE,
} GsAppContextTileType;
#define N_TILE_TYPES (AGE_RATING_TILE + 1)

struct _GsAppContextBar
{
	GtkBox			 parent_instance;

	GsApp			*app;  /* (nullable) (owned) */
	gulong			 app_notify_handler;

	GsAppContextTile	tiles[N_TILE_TYPES];
};

G_DEFINE_TYPE (GsAppContextBar, gs_app_context_bar, GTK_TYPE_BOX)

typedef enum {
	PROP_APP = 1,
} GsAppContextBarProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

/* Certain tiles only make sense for apps which the user can run, and
 * not for (say) fonts.
 *
 * Update the visibility of the tile’s parent box to hide it if both tiles
 * are hidden. */
static gboolean
show_tile_for_non_applications (GsAppContextBar      *self,
                                GsAppContextTileType  tile_type)
{
	GtkWidget *sibling;
	GtkBox *parent_box;
	gboolean any_siblings_visible;
	AsComponentKind app_kind = gs_app_get_kind (self->app);
	gboolean is_application = (app_kind == AS_COMPONENT_KIND_DESKTOP_APP ||
				   app_kind == AS_COMPONENT_KIND_CONSOLE_APP ||
				   app_kind == AS_COMPONENT_KIND_WEB_APP);

	gtk_widget_set_visible (self->tiles[tile_type].tile, is_application);

	parent_box = GTK_BOX (gtk_widget_get_parent (self->tiles[tile_type].tile));
	g_assert (GTK_IS_BOX (parent_box));

	any_siblings_visible = FALSE;

	for (sibling = gtk_widget_get_first_child (GTK_WIDGET (parent_box));
	     sibling != NULL;
	     sibling = gtk_widget_get_next_sibling (sibling)) {
		g_assert (GTK_IS_BUTTON (sibling));
		any_siblings_visible |= gtk_widget_get_visible (sibling);
	}

	gtk_widget_set_visible (GTK_WIDGET (parent_box), any_siblings_visible);

	return is_application;
}

static void
update_storage_tile (GsAppContextBar *self)
{
	g_autofree gchar *lozenge_text = NULL;
	gboolean lozenge_text_is_markup = FALSE;
	const gchar *title;
	g_autofree gchar *description = NULL;
	guint64 size_bytes;
	GsSizeType size_type;

	g_assert (self->app != NULL);

	if (gs_app_is_installed (self->app)) {
		guint64 size_installed, size_user_data, size_cache_data;
		GsSizeType size_installed_type, size_user_data_type, size_cache_data_type;
		g_autofree gchar *size_user_data_str = NULL;
		g_autofree gchar *size_cache_data_str = NULL;

		size_installed_type = gs_app_get_size_installed (self->app, &size_installed);
		size_user_data_type = gs_app_get_size_user_data (self->app, &size_user_data);
		size_cache_data_type = gs_app_get_size_cache_data (self->app, &size_cache_data);

		/* Treat `0` sizes as `unknown`, to not show `0 bytes` in the text. */
		if (size_user_data == 0)
			size_user_data_type = GS_SIZE_TYPE_UNKNOWN;
		if (size_cache_data == 0)
			size_cache_data_type = GS_SIZE_TYPE_UNKNOWN;

		/* If any installed sizes are unknowable, ignore them. This
		 * means the stated installed size is a lower bound on the
		 * actual installed size.
		 * Don’t include dependencies in the stated installed size,
		 * because uninstalling the app won’t reclaim that space unless
		 * it’s the last app using those dependencies. */
		size_bytes = size_installed;
		size_type = size_installed_type;
		if (size_user_data_type == GS_SIZE_TYPE_VALID)
			size_bytes += size_user_data;
		if (size_cache_data_type == GS_SIZE_TYPE_VALID)
			size_bytes += size_cache_data;

		size_user_data_str = g_format_size (size_user_data);
		size_cache_data_str = g_format_size (size_cache_data);

		/* Translators: The disk usage of an app when installed.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Installed Size");

		if (size_user_data_type == GS_SIZE_TYPE_VALID && size_cache_data_type == GS_SIZE_TYPE_VALID)
			description = g_strdup_printf (_("Includes %s of data and %s of cache"),
						       size_user_data_str, size_cache_data_str);
		else if (size_user_data_type == GS_SIZE_TYPE_VALID)
			description = g_strdup_printf (_("Includes %s of data"),
						       size_user_data_str);
		else if (size_cache_data_type == GS_SIZE_TYPE_VALID)
			description = g_strdup_printf (_("Includes %s of cache"),
						       size_cache_data_str);
		else
			description = g_strdup (_("Cache and data usage unknown"));
	} else {
		guint64 app_download_size_bytes, dependencies_download_size_bytes;
		GsSizeType app_download_size_type, dependencies_download_size_type;

		app_download_size_type = gs_app_get_size_download (self->app, &app_download_size_bytes);
		dependencies_download_size_type = gs_app_get_size_download_dependencies (self->app, &dependencies_download_size_bytes);

		size_bytes = app_download_size_bytes;
		size_type = app_download_size_type;

		/* Translators: The download size of an app.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Download Size");

		if (dependencies_download_size_type == GS_SIZE_TYPE_VALID &&
		    dependencies_download_size_bytes == 0) {
			description = g_strdup (_("Needs no additional system downloads"));
		} else if (dependencies_download_size_type != GS_SIZE_TYPE_VALID) {
			description = g_strdup (_("Needs an unknown size of additional system downloads"));
		} else {
			g_autofree gchar *size = g_format_size (dependencies_download_size_bytes);
			/* Translators: The placeholder is for a size string,
			 * such as ‘150 MB’ or ‘1.5 GB’. */
			description = g_strdup_printf (_("Needs %s of additional system downloads"), size);
		}
	}

	if (size_type != GS_SIZE_TYPE_VALID) {
		/* Translators: This is displayed for the download size in an
		 * app’s context tile if the size is unknown. It should be short
		 * (at most a couple of characters wide). */
		lozenge_text = g_strdup (_("?"));

		g_free (description);
		/* Translators: Displayed if the download or installed size of
		 * an app could not be determined.
		 * This is displayed in a context tile, so the string should be short. */
		description = g_strdup (_("Size is unknown"));
	} else {
		lozenge_text = gs_utils_format_size (size_bytes, &lozenge_text_is_markup);
	}

	if (lozenge_text_is_markup)
		gs_lozenge_set_markup (GS_LOZENGE (self->tiles[STORAGE_TILE].lozenge), lozenge_text);
	else
		gs_lozenge_set_text (GS_LOZENGE (self->tiles[STORAGE_TILE].lozenge), lozenge_text);
	gtk_label_set_text (self->tiles[STORAGE_TILE].title, title);
	gtk_label_set_text (self->tiles[STORAGE_TILE].description, description);
}

typedef enum
{
	/* The code in this file relies on the fact that these enum values
	 * numerically increase as they get more unsafe. */
	SAFETY_SAFE,
	SAFETY_PRIVILEGED,
	SAFETY_PROBABLY_SAFE,
	SAFETY_POTENTIALLY_UNSAFE,
	SAFETY_UNSAFE
} SafetyRating;

static void
add_to_safety_rating_full (SafetyRating *chosen_rating,
                           GPtrArray    *descriptions,
                           SafetyRating  item_rating,
                           const gchar  *item_description,
                           gboolean      can_clear_descriptions)
{
	if (item_rating > *chosen_rating) {
		if (can_clear_descriptions)
			g_ptr_array_set_size (descriptions, 0);
		*chosen_rating = item_rating;
	}

	if (item_rating == *chosen_rating)
		g_ptr_array_add (descriptions, (gpointer) item_description);
}

static void
add_to_safety_rating (SafetyRating *chosen_rating,
                      GPtrArray    *descriptions,
                      SafetyRating  item_rating,
                      const gchar  *item_description)
{
	/* Clear the existing descriptions and replace with @item_description if
	 * this item increases the @chosen_rating. This means the final list of
	 * @descriptions will only be the items which caused @chosen_rating to
	 * be so high. */
	add_to_safety_rating_full (chosen_rating, descriptions, item_rating, item_description, TRUE);
}

static void
update_safety_tile (GsAppContextBar *self)
{
	const gchar *icon_name, *title, *css_class;
	/* keep @reviewd_by global for the function, because it's added as-is into the @descriptions array, not copied */
	g_autofree gchar *reviewed_by = NULL;
	g_autofree gchar *description = NULL;
	g_autoptr(GPtrArray) descriptions = g_ptr_array_new_with_free_func (NULL);
	g_autoptr(GsAppPermissions) permissions = NULL;
	GsAppPermissionsFlags perm_flags = GS_APP_PERMISSIONS_FLAGS_NONE;

	/* Treat everything as safe to begin with, and downgrade its safety
	 * based on app properties. */
	SafetyRating chosen_rating = SAFETY_SAFE;

	g_assert (self->app != NULL);

	permissions = gs_app_dup_permissions (self->app);
	if (permissions != NULL)
		perm_flags = gs_app_permissions_get_flags (permissions);

	if (perm_flags == GS_APP_PERMISSIONS_FLAGS_NONE &&
	    (permissions != NULL ||
	    !gs_app_has_quirk (self->app, GS_APP_QUIRK_PROVENANCE))) {
		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_SAFE,
				      /* Translators: This indicates an app requires no permissions to run.
				       * It’s used in a context tile, so should be short. */
				      _("No permissions"));
	}

	for (GsAppPermissionsFlags i = (1 << 0); i < GS_APP_PERMISSIONS_FLAGS_LAST; i <<= 1) {
		if (!(perm_flags & i))
			continue;

		switch (i) {
		case GS_APP_PERMISSIONS_FLAGS_NETWORK:
			add_to_safety_rating (&chosen_rating, descriptions,
					      /* This isn’t actually safe (network access can expand a local
					       * vulnerability into a remotely exploitable one), but it’s
					       * needed commonly enough that marking it as
					       * %SAFETY_POTENTIALLY_UNSAFE is too noisy. */
					      SAFETY_PROBABLY_SAFE,
					      /* Translators: This indicates an app uses the network.
					       * It’s used in a context tile, so should be short. */
					      _("Has network access"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_SYSTEM_BUS:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app uses D-Bus system services.
					       * It’s used in a context tile, so should be short. */
					      _("Uses system services"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_SESSION_BUS:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app uses D-Bus session services.
					       * It’s used in a context tile, so should be short. */
					      _("Uses session services"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_DEVICES:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can access arbitrary hardware devices.
					       * It’s used in a context tile, so should be short. */
					      _("Can access hardware devices"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_INPUT_DEVICES:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_PROBABLY_SAFE,
					      /* Translators: This indicates an app can access input devices.
					       * It’s used in a context tile, so should be short. */
					      _("Can access input devices"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_PULSEAUDIO_DEVICES:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_PROBABLY_SAFE,
					      /* Translators: This indicates an app can access input devices.
					       * It’s used in a context tile, so should be short. */
					      _("Can access microphones"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_SYSTEM_DEVICES:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can access system devices such as /dev/shm.
					       * It’s used in a context tile, so should be short. */
					      _("Can access system devices"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_SCREEN:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can access the screen/display contents.
					       * It’s used in a context tile, so should be short. */
					      _("Can access screen contents"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_HOME_FULL:
		case GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL:
			/* Don’t add twice. */
			if (i == GS_APP_PERMISSIONS_FLAGS_HOME_FULL && (perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL))
				break;

			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can read/write to the user’s home or the entire filesystem.
					       * It’s used in a context tile, so should be short. */
					      _("Can read/write all your data"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_HOME_READ:
		case GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ:
			/* Don’t add twice. */
			if (i == GS_APP_PERMISSIONS_FLAGS_HOME_READ && (perm_flags & GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ))
				break;

			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can read (but not write) from the user’s home or the entire filesystem.
					       * It’s used in a context tile, so should be short. */
					      _("Can read all your data"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can read/write to the user’s Downloads directory.
					       * It’s used in a context tile, so should be short. */
					      _("Can read/write your downloads"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can read (but not write) from the user’s Downloads directory.
					       * It’s used in a context tile, so should be short. */
					      _("Can read your downloads"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_OTHER:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can access data in the system unknown to the Appstore.
					       * It’s used in a context tile, so should be short. */
					      _("Can access some specific files"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_SETTINGS:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can access or change user settings.
					       * It’s used in a context tile, so should be short. */
					      _("Can access and change user settings"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_X11:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app uses the X11 windowing system.
					       * It’s used in a context tile, so should be short. */
					      _("Uses a legacy windowing system"));
			break;
		case GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX:
			add_to_safety_rating (&chosen_rating, descriptions,
					      SAFETY_POTENTIALLY_UNSAFE,
					      /* Translators: This indicates an app can escape its sandbox.
					       * It’s used in a context tile, so should be short. */
					      _("Can acquire arbitrary permissions"));
			break;
		default:
			break;
		}
	}

	if (gs_app_has_quirk (self->app, GS_APP_QUIRK_DEVELOPER_VERIFIED))
		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_SAFE,
				      /* Translators: This indicates an app was written and released by a developer who has been verified.
				       * It’s used in a context tile, so should be short. */
				      _("Appstore developer is verified"));

	/* Unknown permissions (`permissions == NULL`) typically come from non-sandboxed packaging
	 * systems like RPM or DEB. Telling the user the software has unknown
	 * permissions is unhelpful; it’s more relevant to say it’s not
	 * sandboxed but is (or is not) packaged by a trusted vendor. They will
	 * have (at least) done some basic checks to make sure the software is
	 * not overtly malware. That doesn’t protect the user from exploitable
	 * bugs in the software, but it does mean they’re not accidentally
	 * installing something which is actively malicious. */
	if (permissions == NULL &&
	    gs_app_has_quirk (self->app, GS_APP_QUIRK_PROVENANCE)) {
		/* It's a new key suggested at https://github.com/systemd/systemd/issues/27777 */
		g_autofree gchar *name = g_get_os_info ("VENDOR_NAME");
		if (name == NULL) {
			/* Translators: This indicates that an app has been packaged
			 * by the user’s distribution and is probably safe.
			 * It’s used in a context tile, so should be short. */
			reviewed_by = g_strdup (_("Reviewed by OS distributor"));
		} else {
			/* Translators: This indicates that an app has been packaged
			 * by the user’s distribution and is probably safe.
			 * It’s used in a context tile, so should be short.
			 * The '%s' is replaced by the distribution name. */
			reviewed_by = g_strdup_printf (_("Reviewed by %s"), name);
		}

		/* Show as 'probably safe' when the app is considered safe until now and it's provided by the distribution */
		if (chosen_rating == SAFETY_SAFE)
			chosen_rating = SAFETY_PRIVILEGED;

		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_PRIVILEGED,
				      reviewed_by);
	} else if (permissions == NULL) {
		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_POTENTIALLY_UNSAFE,
				      /* Translators: This indicates that an app has been packaged
				       * by someone other than the user’s distribution, so might not be safe.
				       * It’s used in a context tile, so should be short. */
				      _("Provided by a third party"));
	}

	if (gs_app_get_metadata_item (self->app, "LingmoAppstore::EolReason") != NULL || (
	    gs_app_get_runtime (self->app) != NULL &&
	    gs_app_get_metadata_item (gs_app_get_runtime (self->app), "LingmoAppstore::EolReason") != NULL))
		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_POTENTIALLY_UNSAFE,
				      /* Translators: This indicates an app or its runtime reached its end of life.
				       * It’s used in a context tile, so should be short. */
				      _("Appstore no longer supported"));

	/* Is the code FOSS and hence inspectable? This doesn’t distinguish
	 * between closed source and open-source-but-not-FOSS software, even
	 * though the code of the latter is technically publicly auditable. This
	 * is because I don’t want to get into the business of maintaining lists
	 * of ‘auditable’ source code licenses. */
	if (gs_app_get_license_is_free (self->app)) {
		add_to_safety_rating (&chosen_rating, descriptions,
				      SAFETY_SAFE,
				      /* Translators: This indicates an app’s source code is freely available, so can be audited for security.
				       * It’s used in a context tile, so should be short. */
				      _("Auditable code"));
	} else if (gs_app_get_license (self->app) == NULL) {
		add_to_safety_rating_full (&chosen_rating, descriptions,
					   SAFETY_PRIVILEGED,
					   /* Translators: This indicates an app does not specify which license it's developed under.
					    * It’s used in a context tile, so should be short. */
					   _("Unknown license"),
					   FALSE);
	} else if (g_ascii_strncasecmp (gs_app_get_license (self->app), "LicenseRef-proprietary", strlen ("LicenseRef-proprietary")) == 0) {
		add_to_safety_rating_full (&chosen_rating, descriptions,
					   SAFETY_PROBABLY_SAFE,
					   /* Translators: This indicates an app is not licensed under a free software license.
					    * It’s used in a context tile, so should be short. */
					   _("Proprietary code"),
					   FALSE);
	} else {
		add_to_safety_rating_full (&chosen_rating, descriptions,
					   SAFETY_PROBABLY_SAFE,
					   /* Translators: This indicates an app is not licensed under a free software license.
					    * It’s used in a context tile, so should be short. */
					   _("Special license"),
					   FALSE);
	}

	g_assert (descriptions->len > 0);

	g_ptr_array_add (descriptions, NULL);
	/* Translators: This string is used to join various other translated
	 * strings into an inline list of reasons why an app has been marked as
	 * ‘safe’, ‘potentially safe’ or ‘unsafe’. For example:
	 * “App comes from a trusted source; Auditable code; No permissions”
	 * If concatenating strings as a list using a separator like this is not
	 * possible in your language, please file an issue against lingmo-appstore:
	 * https://github.com/LingmoOS/lingmo-appstore/-/issues/new */
	description = g_strjoinv (_("; "), (gchar **) descriptions->pdata);

	/* Update the UI. */
	switch (chosen_rating) {
	case SAFETY_PRIVILEGED:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered privileged, aka provided by the distribution.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Privileged");
		css_class = "grey";
		break;
	case SAFETY_SAFE:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered safe to install and run.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Safe");
		css_class = "green";
		break;
	case SAFETY_PROBABLY_SAFE:
		icon_name = "safety-symbolic";
		/* Translators: The app is considered probably safe to install and run.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Probably Safe");
		css_class = "yellow";
		break;
	case SAFETY_POTENTIALLY_UNSAFE:
		icon_name = "dialog-question-symbolic";
		/* Translators: The app is considered potentially unsafe to install and run.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Potentially Unsafe");
		css_class = "orange";
		break;
	case SAFETY_UNSAFE:
		icon_name = "dialog-warning-symbolic";
		/* Translators: The app is considered unsafe to install and run.
		 * This is displayed in a context tile, so the string should be short. */
		title = _("Unsafe");
		css_class = "red";
		break;
	default:
		g_assert_not_reached ();
	}

	gs_lozenge_set_icon_name (GS_LOZENGE (self->tiles[SAFETY_TILE].lozenge), icon_name);
	gtk_label_set_text (self->tiles[SAFETY_TILE].title, title);
	gtk_label_set_text (self->tiles[SAFETY_TILE].description, description);

	gtk_widget_remove_css_class (self->tiles[SAFETY_TILE].lozenge, "green");
	gtk_widget_remove_css_class (self->tiles[SAFETY_TILE].lozenge, "grey");
	gtk_widget_remove_css_class (self->tiles[SAFETY_TILE].lozenge, "yellow");
	gtk_widget_remove_css_class (self->tiles[SAFETY_TILE].lozenge, "orange");
	gtk_widget_remove_css_class (self->tiles[SAFETY_TILE].lozenge, "red");

	gtk_widget_add_css_class (self->tiles[SAFETY_TILE].lozenge, css_class);
}

typedef struct {
	guint min;
	guint max;
} Range;

static void
update_hardware_support_tile (GsAppContextBar *self)
{
	g_autoptr(GPtrArray) relations = NULL;
	AsRelationKind control_relations[AS_CONTROL_KIND_LAST] = { AS_RELATION_KIND_UNKNOWN, };
	GdkDisplay *display;
	GdkMonitor *monitor = NULL;
	gboolean any_control_relations_set;
	const gchar *icon_name = NULL, *title = NULL, *description = NULL, *css_class = NULL;
	gboolean has_touchscreen = FALSE, has_keyboard = FALSE, has_mouse = FALSE;

	g_assert (self->app != NULL);

	/* Don’t show the hardware support tile for non-desktop apps. */
	if (!show_tile_for_non_applications (self, HARDWARE_SUPPORT_TILE))
		return;

	relations = gs_app_get_relations (self->app);

	/* Extract the %AS_RELATION_ITEM_KIND_CONTROL relations and summarise
	 * them. */
	display = gtk_widget_get_display (GTK_WIDGET (self));
	gs_hardware_support_context_dialog_get_control_support (display, relations,
								&any_control_relations_set,
								control_relations,
								&has_touchscreen,
								&has_keyboard,
								&has_mouse);

	/* Warn about screen size mismatches. Compare against the largest
	 * monitor associated with this widget’s #GdkDisplay, defaulting to
	 * the primary monitor.
	 *
	 * See https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-requires-recommends-display_length
	 * for the semantics of the display length relations.*/
	if (display != NULL)
		monitor = gs_hardware_support_context_dialog_get_largest_monitor (display);

	if (monitor != NULL) {
		AsRelationKind desktop_relation_kind, mobile_relation_kind, current_relation_kind;
		gboolean desktop_match, mobile_match, current_match;

		gs_hardware_support_context_dialog_get_display_support (monitor, relations,
									NULL,
									&desktop_match, &desktop_relation_kind,
									&mobile_match, &mobile_relation_kind,
									&current_match, &current_relation_kind);

		/* If the current screen size is not supported, try and
		 * summarise the restrictions into a single context tile. */
		if (!current_match &&
		    !mobile_match && mobile_relation_kind == AS_RELATION_KIND_REQUIRES) {
			icon_name = "phone-symbolic";
			title = _("Mobile Only");
			description = _("Only works on a small screen");
			css_class = "red";
		} else if (!current_match &&
			   !desktop_match && desktop_relation_kind == AS_RELATION_KIND_REQUIRES) {
			icon_name = "desktop-symbolic";
			title = _("Desktop Only");
			description = _("Only works on a large screen");
			css_class = "red";
		} else if (!current_match && current_relation_kind == AS_RELATION_KIND_REQUIRES) {
			icon_name = "desktop-symbolic";
			title = _("Screen Size Mismatch");
			description = _("Doesn’t support your current screen size");
			css_class = "red";
		}
	}

	/* Warn about missing touchscreen or keyboard support. There are some
	 * assumptions here that certain input devices are only available on
	 * certain platforms; they can change in future.
	 *
	 * As with the rest of the tile contents in this function, tile contents
	 * which are checked lower down in the function are only used if nothing
	 * more important has already been set earlier.
	 *
	 * The available information is being summarised to quite an extreme
	 * degree here, and it’s likely this code will have to evolve for
	 * corner cases in future. */
	if (icon_name == NULL &&
	    control_relations[AS_CONTROL_KIND_TOUCH] == AS_RELATION_KIND_REQUIRES &&
	    !has_touchscreen) {
		icon_name = "phone-symbolic";
		title = _("Mobile Only");
		description = _("Requires a touchscreen");
		css_class = "red";
	} else if (icon_name == NULL &&
		   control_relations[AS_CONTROL_KIND_KEYBOARD] == AS_RELATION_KIND_REQUIRES &&
		   !has_keyboard) {
		icon_name = "input-keyboard-symbolic";
		title = _("Desktop Only");
		description = _("Requires a keyboard");
		css_class = "red";
	} else if (icon_name == NULL &&
		   control_relations[AS_CONTROL_KIND_POINTING] == AS_RELATION_KIND_REQUIRES &&
		   !has_mouse) {
		icon_name = "input-mouse-symbolic";
		title = _("Desktop Only");
		description = _("Requires a mouse");
		css_class = "red";
	}

	/* Say if the app requires a gamepad. We can’t reliably detect whether
	 * the computer has a gamepad, as it might be unplugged unless the user
	 * is currently playing a game. So this might be shown even if the user
	 * has a gamepad available. */
	if (icon_name == NULL &&
	    control_relations[AS_CONTROL_KIND_GAMEPAD] == AS_RELATION_KIND_REQUIRES) {
		icon_name = "input-gaming-symbolic";
		title = _("Gamepad Needed");
		description = _("Requires a gamepad to play");
		css_class = "yellow";
	}

	/* Otherwise, is it adaptive? Note that %AS_RELATION_KIND_RECOMMENDS
	 * means more like ‘supports’ than ‘recommends’. */
#if AS_CHECK_VERSION(0, 15, 0)
	if (icon_name == NULL &&
	    (control_relations[AS_CONTROL_KIND_TOUCH] == AS_RELATION_KIND_RECOMMENDS ||
	     control_relations[AS_CONTROL_KIND_TOUCH] == AS_RELATION_KIND_SUPPORTS) &&
	    (control_relations[AS_CONTROL_KIND_KEYBOARD] == AS_RELATION_KIND_RECOMMENDS ||
	     control_relations[AS_CONTROL_KIND_KEYBOARD] == AS_RELATION_KIND_SUPPORTS) &&
	    (control_relations[AS_CONTROL_KIND_POINTING] == AS_RELATION_KIND_RECOMMENDS ||
	     control_relations[AS_CONTROL_KIND_POINTING] == AS_RELATION_KIND_SUPPORTS)) {
#else
	if (icon_name == NULL &&
	    control_relations[AS_CONTROL_KIND_TOUCH] == AS_RELATION_KIND_RECOMMENDS &&
	    control_relations[AS_CONTROL_KIND_KEYBOARD] == AS_RELATION_KIND_RECOMMENDS &&
	    control_relations[AS_CONTROL_KIND_POINTING] == AS_RELATION_KIND_RECOMMENDS) {
#endif
		icon_name = "adaptive-symbolic";
		/* Translators: This is used in a context tile to indicate that
		 * an app works on phones, tablets *and* desktops. It should be
		 * short and in title case. */
		title = _("Adaptive");
		description = _("Works on phones, tablets and desktops");
		css_class = "green";
	}

	/* Fallback. At the moment (June 2021) almost no apps have any metadata
	 * about hardware support, so this case will be hit most of the time.
	 *
	 * So in the absence of any other information, assume that all apps
	 * support desktop, and none support mobile. */
	if (icon_name == NULL) {
		if (!has_keyboard || !has_mouse) {
			icon_name = "desktop-symbolic";
			title = _("Desktop Only");
			description = _("Probably requires a keyboard or mouse");
			css_class = "yellow";
		} else {
			icon_name = "desktop-symbolic";
			title = _("Desktop Only");
			description = _("Works on desktops and laptops");
			css_class = "grey";
		}
	}

	/* Update the UI. The `adaptive-symbolic` icon needs a special size to
	 * be set, as it is wider than it is tall. Setting the size ensures it’s
	 * rendered at the right height. */
	gs_lozenge_set_icon_name (GS_LOZENGE (self->tiles[HARDWARE_SUPPORT_TILE].lozenge), icon_name);
	gs_lozenge_set_pixel_size (GS_LOZENGE (self->tiles[HARDWARE_SUPPORT_TILE].lozenge), g_str_equal (icon_name, "adaptive-symbolic") ? 56 : -1);

	gtk_label_set_text (self->tiles[HARDWARE_SUPPORT_TILE].title, title);
	gtk_label_set_text (self->tiles[HARDWARE_SUPPORT_TILE].description, description);

	gtk_widget_remove_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "green");
	gtk_widget_remove_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "grey");
	gtk_widget_remove_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "yellow");
	gtk_widget_remove_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "red");

	gtk_widget_add_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, css_class);

	if (g_str_equal (icon_name, "adaptive-symbolic"))
		gtk_widget_add_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "wide-image");
	else
		gtk_widget_remove_css_class (self->tiles[HARDWARE_SUPPORT_TILE].lozenge, "wide-image");
}

static void
build_age_rating_description_cb (const gchar          *attribute,
                                 AsContentRatingValue  value,
                                 gpointer              user_data)
{
	GPtrArray *descriptions = user_data;
	const gchar *description;

	/* (attribute == NULL) is used by the caller to indicate that no
	 * attributes apply. This callback will be called at most once like
	 * that. */
	if (attribute == NULL)
		/* Translators: This indicates that the content rating for an
		 * app says it can be used by all ages of people, as it contains
		 * no objectionable content. */
		description = _("Contains no age-inappropriate content");
	else
		description = as_content_rating_attribute_get_description (attribute, value);

	g_ptr_array_add (descriptions, (gpointer) description);
}

static gchar *
build_age_rating_description (AsContentRating *content_rating)
{
	g_autoptr(GPtrArray) descriptions = g_ptr_array_new_with_free_func (NULL);

	gs_age_rating_context_dialog_process_attributes (content_rating,
							 TRUE,
							 build_age_rating_description_cb,
							 descriptions);

	g_ptr_array_add (descriptions, NULL);
	/* Translators: This string is used to join various other translated
	 * strings into an inline list of reasons why an app has been given a
	 * certain content rating. For example:
	 * “References to alcoholic beverages; Moderated chat functionality between users”
	 * If concatenating strings as a list using a separator like this is not
	 * possible in your language, please file an issue against lingmo-appstore:
	 * https://github.com/LingmoOS/lingmo-appstore/-/issues/new */
	return g_strjoinv (_("; "), (gchar **) descriptions->pdata);
}

static void
update_age_rating_tile (GsAppContextBar *self)
{
	g_autoptr(AsContentRating) content_rating = NULL;
	gboolean is_unknown;
	g_autofree gchar *description = NULL;

	g_assert (self->app != NULL);

	/* Don’t show the age rating tile for non-desktop apps. */
	if (!show_tile_for_non_applications (self, AGE_RATING_TILE))
		return;

	content_rating = gs_app_dup_content_rating (self->app);
	gs_age_rating_context_dialog_update_lozenge (self->app,
						     GS_LOZENGE (self->tiles[AGE_RATING_TILE].lozenge),
						     &is_unknown);

	/* Description */
	if (content_rating == NULL || is_unknown) {
		description = g_strdup (_("No age rating information available"));
	} else {
		description = build_age_rating_description (content_rating);
	}

	gtk_label_set_text (self->tiles[AGE_RATING_TILE].description, description);

	/* Disable the button if no content rating information is available, as
	 * it would only show a dialogue full of rows saying ‘Unknown’ */
	gtk_widget_set_sensitive (self->tiles[AGE_RATING_TILE].tile, (content_rating != NULL));
}

static void
update_tiles (GsAppContextBar *self)
{
	if (self->app == NULL)
		return;

	update_storage_tile (self);
	update_safety_tile (self);
	update_hardware_support_tile (self);
	update_age_rating_tile (self);
}

static void
app_notify_cb (GObject    *obj,
               GParamSpec *pspec,
               gpointer    user_data)
{
	GsAppContextBar *self = GS_APP_CONTEXT_BAR (user_data);

	update_tiles (self);
}

static void
tile_clicked_cb (GtkWidget *widget,
                 gpointer   user_data)
{
	GsAppContextBar *self = GS_APP_CONTEXT_BAR (user_data);
	GtkWindow *dialog;
	GtkRoot *root = gtk_widget_get_root (widget);

	if (GTK_IS_WINDOW (root)) {
		if (widget == self->tiles[STORAGE_TILE].tile)
			dialog = GTK_WINDOW (gs_storage_context_dialog_new (self->app));
		else if (widget == self->tiles[SAFETY_TILE].tile)
			dialog = GTK_WINDOW (gs_safety_context_dialog_new (self->app));
		else if (widget == self->tiles[HARDWARE_SUPPORT_TILE].tile)
			dialog = GTK_WINDOW (gs_hardware_support_context_dialog_new (self->app));
		else if (widget == self->tiles[AGE_RATING_TILE].tile)
			dialog = GTK_WINDOW (gs_age_rating_context_dialog_new (self->app));
		else
			g_assert_not_reached ();

		gtk_window_set_transient_for (dialog, GTK_WINDOW (root));
		gtk_widget_set_visible (GTK_WIDGET (dialog), TRUE);
	}
}

static void
gs_app_context_bar_init (GsAppContextBar *self)
{
	g_type_ensure (GS_TYPE_LOZENGE);

	gtk_widget_init_template (GTK_WIDGET (self));
}

static void
gs_app_context_bar_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	GsAppContextBar *self = GS_APP_CONTEXT_BAR (object);

	switch ((GsAppContextBarProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, gs_app_context_bar_get_app (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_context_bar_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	GsAppContextBar *self = GS_APP_CONTEXT_BAR (object);

	switch ((GsAppContextBarProperty) prop_id) {
	case PROP_APP:
		gs_app_context_bar_set_app (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_context_bar_dispose (GObject *object)
{
	GsAppContextBar *self = GS_APP_CONTEXT_BAR (object);

	if (self->app_notify_handler != 0) {
		g_signal_handler_disconnect (self->app, self->app_notify_handler);
		self->app_notify_handler = 0;
	}
	g_clear_object (&self->app);

	G_OBJECT_CLASS (gs_app_context_bar_parent_class)->dispose (object);
}

static void
gs_app_context_bar_class_init (GsAppContextBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_app_context_bar_get_property;
	object_class->set_property = gs_app_context_bar_set_property;
	object_class->dispose = gs_app_context_bar_dispose;

	/**
	 * GsAppContextBar:app: (nullable)
	 *
	 * The app to display the context details for.
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

	gtk_widget_class_set_css_name (widget_class, "app-context-bar");
	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-app-context-bar.ui");

	gtk_widget_class_bind_template_child_full (widget_class, "storage_tile", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[STORAGE_TILE].tile));
	gtk_widget_class_bind_template_child_full (widget_class, "storage_tile_lozenge", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[STORAGE_TILE].lozenge));
	gtk_widget_class_bind_template_child_full (widget_class, "storage_tile_title", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[STORAGE_TILE].title));
	gtk_widget_class_bind_template_child_full (widget_class, "storage_tile_description", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[STORAGE_TILE].description));
	gtk_widget_class_bind_template_child_full (widget_class, "safety_tile", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[SAFETY_TILE].tile));
	gtk_widget_class_bind_template_child_full (widget_class, "safety_tile_lozenge", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[SAFETY_TILE].lozenge));
	gtk_widget_class_bind_template_child_full (widget_class, "safety_tile_title", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[SAFETY_TILE].title));
	gtk_widget_class_bind_template_child_full (widget_class, "safety_tile_description", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[SAFETY_TILE].description));
	gtk_widget_class_bind_template_child_full (widget_class, "hardware_support_tile", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[HARDWARE_SUPPORT_TILE].tile));
	gtk_widget_class_bind_template_child_full (widget_class, "hardware_support_tile_lozenge", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[HARDWARE_SUPPORT_TILE].lozenge));
	gtk_widget_class_bind_template_child_full (widget_class, "hardware_support_tile_title", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[HARDWARE_SUPPORT_TILE].title));
	gtk_widget_class_bind_template_child_full (widget_class, "hardware_support_tile_description", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[HARDWARE_SUPPORT_TILE].description));
	gtk_widget_class_bind_template_child_full (widget_class, "age_rating_tile", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[AGE_RATING_TILE].tile));
	gtk_widget_class_bind_template_child_full (widget_class, "age_rating_tile_lozenge", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[AGE_RATING_TILE].lozenge));
	gtk_widget_class_bind_template_child_full (widget_class, "age_rating_tile_title", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[AGE_RATING_TILE].title));
	gtk_widget_class_bind_template_child_full (widget_class, "age_rating_tile_description", FALSE, G_STRUCT_OFFSET (GsAppContextBar, tiles[AGE_RATING_TILE].description));
	gtk_widget_class_bind_template_callback (widget_class, tile_clicked_cb);
}

/**
 * gs_app_context_bar_new:
 * @app: (nullable): the app to display context tiles for, or %NULL
 *
 * Create a new #GsAppContextBar and set its initial app to @app.
 *
 * Returns: (transfer full): a new #GsAppContextBar
 * Since: 41
 */
GtkWidget *
gs_app_context_bar_new (GsApp *app)
{
	g_return_val_if_fail (app == NULL || GS_IS_APP (app), NULL);

	return g_object_new (GS_TYPE_APP_CONTEXT_BAR,
			     "app", app,
			     NULL);
}

/**
 * gs_app_context_bar_get_app:
 * @self: a #GsAppContextBar
 *
 * Gets the value of #GsAppContextBar:app.
 *
 * Returns: (nullable) (transfer none): app whose context tiles are being
 *     displayed, or %NULL if none is set
 * Since: 41
 */
GsApp *
gs_app_context_bar_get_app (GsAppContextBar *self)
{
	g_return_val_if_fail (GS_IS_APP_CONTEXT_BAR (self), NULL);

	return self->app;
}

/**
 * gs_app_context_bar_set_app:
 * @self: a #GsAppContextBar
 * @app: (nullable) (transfer none): the app to display context tiles for,
 *     or %NULL for none
 *
 * Set the value of #GsAppContextBar:app.
 *
 * Since: 41
 */
void
gs_app_context_bar_set_app (GsAppContextBar *self,
                            GsApp           *app)
{
	g_return_if_fail (GS_IS_APP_CONTEXT_BAR (self));
	g_return_if_fail (app == NULL || GS_IS_APP (app));

	if (app == self->app)
		return;

	if (self->app_notify_handler != 0) {
		g_signal_handler_disconnect (self->app, self->app_notify_handler);
		self->app_notify_handler = 0;
	}

	g_set_object (&self->app, app);

	if (self->app != NULL)
		self->app_notify_handler = g_signal_connect (self->app, "notify", G_CALLBACK (app_notify_cb), self);

	/* Update the tiles. */
	update_tiles (self);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
}
