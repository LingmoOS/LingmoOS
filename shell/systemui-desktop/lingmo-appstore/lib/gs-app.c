/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-app
 * @title: GsApp
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: An application that is either installed or that can be installed
 *
 * For GsApps of kind %AS_COMPONENT_KIND_DESKTOP_APP, this object represents a 1:1 mapping
 * to a .desktop file. The design is such so you can't have different GsApp's for different
 * versions or architectures of a package. For other AppStream component types, GsApp maps
 * their properties and %AS_COMPONENT_KIND_GENERIC is used if their type is a generic software
 * component. For LINGMO Appstore specific app-like entries, which don't correspond to desktop
 * files or distinct software components, but e.g. represent a system update and its individual
 * components, use the separate #GsAppSpecialKind enum and %gs_app_set_special_kind while setting
 * the AppStream component-kind to generic.
 *
 * The #GsPluginLoader de-duplicates the GsApp instances that are produced by
 * plugins to ensure that there is a single instance of GsApp for each id, making
 * the id the primary key for this object. This ensures that actions triggered on
 * a #GsApp in different parts of lingmo-appstore can be observed by connecting to
 * signals on the #GsApp.
 *
 * Information about other #GsApp objects can be stored in this object, for
 * instance in the gs_app_add_related() method or gs_app_get_history().
 */

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gs-app-collation.h"
#include "gs-app-private.h"
#include "gs-desktop-data.h"
#include "gs-enums.h"
#include "gs-icon.h"
#include "gs-key-colors.h"
#include "gs-os-release.h"
#include "gs-plugin.h"
#include "gs-plugin-private.h"
#include "gs-remote-icon.h"
#include "gs-utils.h"

typedef struct
{
	GMutex			 mutex;
	gchar			*id;
	gchar			*unique_id;
	gboolean		 unique_id_valid;
	gchar			*branch;
	gchar			*name;
	gchar			*renamed_from;
	GsAppQuality		 name_quality;
	GPtrArray		*icons;  /* (nullable) (owned) (element-type AsIcon), sorted by pixel size, smallest first */
	GPtrArray		*sources;
	GPtrArray		*source_ids;
	gchar			*project_group;
	gchar			*developer_name;
	gchar			*agreement;
	gchar			*version;
	gchar			*version_ui;
	gchar			*summary;
	GsAppQuality		 summary_quality;
	gchar			*summary_missing;
	gchar			*description;
	GsAppQuality		 description_quality;
	GPtrArray		*screenshots;
	GPtrArray		*categories;
	GArray			*key_colors;  /* (nullable) (element-type GdkRGBA) */
	gboolean		 user_key_colors;
	GHashTable		*urls;  /* (element-type AsUrlKind utf8) (owned) (nullable) */
	GHashTable		*launchables;
	gchar			*url_missing;
	gchar			*license;
	GsAppQuality		 license_quality;
	gchar			**menu_path;
	gchar			*origin;
	gchar			*origin_ui;
	gchar			*origin_appstream;
	gchar			*origin_hostname;
	gchar			*update_version;
	gchar			*update_version_ui;
	gchar			*update_details_markup;
	gboolean		 update_details_set;
	AsUrgencyKind		 update_urgency;
	GsAppPermissions        *update_permissions;
	GWeakRef		 management_plugin_weak;  /* (element-type GsPlugin) */
	guint			 match_value;
	guint			 priority;
	gint			 rating;
	GArray			*review_ratings;
	GPtrArray		*reviews; /* of AsReview */
	GPtrArray		*provided; /* of AsProvided */

	GsSizeType		 size_installed_type;
	guint64			 size_installed;
	GsSizeType		 size_download_type;
	guint64			 size_download;
	GsSizeType		 size_user_data_type;
	guint64			 size_user_data;
	GsSizeType		 size_cache_data_type;
	guint64			 size_cache_data;

	AsComponentKind		 kind;
	GsAppSpecialKind	 special_kind;
	GsAppState		 state;
	GsAppState		 state_recover;
	AsComponentScope	 scope;
	AsBundleKind		 bundle_kind;
	guint			 progress;  /* integer 0–100 (inclusive), or %GS_APP_PROGRESS_UNKNOWN */
	gboolean		 allow_cancel;
	GHashTable		*metadata;
	GsAppList		*addons;
	GsAppList		*related;
	GsAppList		*history;
	guint64			 install_date;
	guint64			 release_date;
	guint64			 kudos;
	gboolean		 to_be_installed;
	GsAppQuirk		 quirk;
	gboolean		 license_is_free;
	GsApp			*runtime;
	GFile			*local_file;
	AsContentRating		*content_rating;
	AsScreenshot		*action_screenshot;  /* (nullable) (owned) */
	GCancellable		*cancellable;
	GsPluginAction		 pending_action;
	GsAppPermissions        *permissions;
	gboolean		 is_update_downloaded;
	GPtrArray		*version_history; /* (element-type AsRelease) (nullable) (owned) */
	GPtrArray		*relations;  /* (nullable) (element-type AsRelation) (owned) */
	gboolean		 has_translations;
	GsAppIconsState		 icons_state;
} GsAppPrivate;

typedef enum {
	PROP_ID = 1,
	PROP_NAME,
	PROP_VERSION,
	PROP_SUMMARY,
	PROP_DESCRIPTION,
	PROP_RATING,
	PROP_KIND,
	PROP_SPECIAL_KIND,
	PROP_STATE,
	PROP_PROGRESS,
	PROP_CAN_CANCEL_INSTALLATION,
	PROP_INSTALL_DATE,
	PROP_RELEASE_DATE,
	PROP_QUIRK,
	PROP_PENDING_ACTION,
	PROP_KEY_COLORS,
	PROP_IS_UPDATE_DOWNLOADED,
	PROP_URLS,
	PROP_URL_MISSING,
	PROP_CONTENT_RATING,
	PROP_LICENSE,
	PROP_SIZE_CACHE_DATA_TYPE,
	PROP_SIZE_CACHE_DATA,
	PROP_SIZE_DOWNLOAD_TYPE,
	PROP_SIZE_DOWNLOAD,
	PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE,
	PROP_SIZE_DOWNLOAD_DEPENDENCIES,
	PROP_SIZE_INSTALLED_TYPE,
	PROP_SIZE_INSTALLED,
	PROP_SIZE_INSTALLED_DEPENDENCIES_TYPE,
	PROP_SIZE_INSTALLED_DEPENDENCIES,
	PROP_SIZE_USER_DATA_TYPE,
	PROP_SIZE_USER_DATA,
	PROP_PERMISSIONS,
	PROP_RELATIONS,
	PROP_ORIGIN_UI,
	PROP_HAS_TRANSLATIONS,
	PROP_ICONS_STATE,
} GsAppProperty;

static GParamSpec *obj_props[PROP_ICONS_STATE + 1] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE (GsApp, gs_app, G_TYPE_OBJECT)

static gboolean
_g_set_str (gchar **str_ptr, const gchar *new_str)
{
	if (*str_ptr == new_str || g_strcmp0 (*str_ptr, new_str) == 0)
		return FALSE;
	g_free (*str_ptr);
	*str_ptr = g_strdup (new_str);
	return TRUE;
}

static gboolean
_g_set_strv (gchar ***strv_ptr, gchar **new_strv)
{
	if (*strv_ptr == new_strv)
		return FALSE;
	g_strfreev (*strv_ptr);
	*strv_ptr = g_strdupv (new_strv);
	return TRUE;
}

static gboolean
_g_set_ptr_array (GPtrArray **array_ptr, GPtrArray *new_array)
{
	if (*array_ptr == new_array)
		return FALSE;
	if (new_array != NULL)
		g_ptr_array_ref (new_array);
	if (*array_ptr != NULL)
		g_ptr_array_unref (*array_ptr);
	*array_ptr = new_array;
	return TRUE;
}

static gboolean
_g_set_array (GArray **array_ptr, GArray *new_array)
{
	if (*array_ptr == new_array)
		return FALSE;
	if (new_array != NULL)
		g_array_ref (new_array);
	if (*array_ptr != NULL)
		g_array_unref (*array_ptr);
	*array_ptr = new_array;
	return TRUE;
}

/**
 * gs_app_state_to_string:
 * @state: the #GsAppState.
 *
 * Converts the enumerated value to an text representation.
 *
 * Returns: string version of @state, or %NULL for unknown
 **/
const gchar *
gs_app_state_to_string (GsAppState state)
{
	if (state == GS_APP_STATE_UNKNOWN)
		return "unknown";
	if (state == GS_APP_STATE_INSTALLED)
		return "installed";
	if (state == GS_APP_STATE_AVAILABLE)
		return "available";
	if (state == GS_APP_STATE_PURCHASABLE)
		return "purchasable";
	if (state == GS_APP_STATE_PURCHASING)
		return "purchasing";
	if (state == GS_APP_STATE_AVAILABLE_LOCAL)
		return "local";
	if (state == GS_APP_STATE_QUEUED_FOR_INSTALL)
		return "queued";
	if (state == GS_APP_STATE_INSTALLING)
		return "installing";
	if (state == GS_APP_STATE_REMOVING)
		return "removing";
	if (state == GS_APP_STATE_UPDATABLE)
		return "updatable";
	if (state == GS_APP_STATE_UPDATABLE_LIVE)
		return "updatable-live";
	if (state == GS_APP_STATE_UNAVAILABLE)
		return "unavailable";
	if (state == GS_APP_STATE_PENDING_INSTALL)
		return "pending-install";
	if (state == GS_APP_STATE_PENDING_REMOVE)
		return "pending-remove";
	if (state == GS_APP_STATE_DOWNLOADING)
		return "downloading";
	return NULL;
}

static void
gs_app_kv_lpad (GString *str, const gchar *key, const gchar *value)
{
	gs_utils_append_key_value (str, 20, key, value);
}

static void
gs_app_kv_size (GString     *str,
                const gchar *key,
                GsSizeType   size_type,
                guint64      value)
{
	g_autofree gchar *tmp = NULL;

	switch (size_type) {
	case GS_SIZE_TYPE_UNKNOWN:
		gs_app_kv_lpad (str, key, "unknown");
		break;
	case GS_SIZE_TYPE_UNKNOWABLE:
		gs_app_kv_lpad (str, key, "unknowable");
		break;
	case GS_SIZE_TYPE_VALID:
		tmp = g_format_size (value);
		gs_app_kv_lpad (str, key, tmp);
		break;
	default:
		g_assert_not_reached ();
	}
}

G_GNUC_PRINTF (3, 4)
static void
gs_app_kv_printf (GString *str, const gchar *key, const gchar *fmt, ...)
{
	va_list args;
	g_autofree gchar *tmp = NULL;
	va_start (args, fmt);
	tmp = g_strdup_vprintf (fmt, args);
	va_end (args);
	gs_app_kv_lpad (str, key, tmp);
}

static const gchar *
_as_component_quirk_flag_to_string (GsAppQuirk quirk)
{
	switch (quirk) {
	case GS_APP_QUIRK_PROVENANCE:
		return "provenance";
	case GS_APP_QUIRK_COMPULSORY:
		return "compulsory";
	case GS_APP_QUIRK_HAS_SOURCE:
		return "has-source";
	case GS_APP_QUIRK_IS_WILDCARD:
		return "is-wildcard";
	case GS_APP_QUIRK_NEEDS_REBOOT:
		return "needs-reboot";
	case GS_APP_QUIRK_NOT_REVIEWABLE:
		return "not-reviewable";
	case GS_APP_QUIRK_NOT_LAUNCHABLE:
		return "not-launchable";
	case GS_APP_QUIRK_NEEDS_USER_ACTION:
		return "needs-user-action";
	case GS_APP_QUIRK_IS_PROXY:
		return "is-proxy";
	case GS_APP_QUIRK_UNUSABLE_DURING_UPDATE:
		return "unusable-during-update";
	case GS_APP_QUIRK_DEVELOPER_VERIFIED:
		return "developer-verified";
	case GS_APP_QUIRK_PARENTAL_FILTER:
		return "parental-filter";
	case GS_APP_QUIRK_NEW_PERMISSIONS:
		return "new-permissions";
	case GS_APP_QUIRK_PARENTAL_NOT_LAUNCHABLE:
		return "parental-not-launchable";
	case GS_APP_QUIRK_HIDE_FROM_SEARCH:
		return "hide-from-search";
	case GS_APP_QUIRK_HIDE_EVERYWHERE:
		return "hide-everywhere";
	case GS_APP_QUIRK_DO_NOT_AUTO_UPDATE:
		return "do-not-auto-update";
	default:
		return NULL;
	}
}

/* mutex must be held */
static const gchar *
gs_app_get_unique_id_unlocked (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	/* invalid */
	if (priv->id == NULL)
		return NULL;

	/* hmm, do what we can */
	if (priv->unique_id == NULL || !priv->unique_id_valid) {
		g_free (priv->unique_id);
		priv->unique_id = gs_utils_build_unique_id (priv->scope,
							    priv->bundle_kind,
							    priv->origin,
							    priv->id,
							    priv->branch);
		priv->unique_id_valid = TRUE;
	}
	return priv->unique_id;
}

/**
 * gs_app_compare_priority:
 * @app1: a #GsApp
 * @app2: a #GsApp
 *
 * Compares two applications using their priority.
 *
 * Use `gs_plugin_add_rule(plugin,GS_PLUGIN_RULE_BETTER_THAN,"plugin-name")`
 * to set the application priority values.
 *
 * Returns: a negative value if @app1 is less than @app2, a positive value if
 *          @app1 is greater than @app2, and zero if @app1 is equal to @app2
 **/
gint
gs_app_compare_priority (GsApp *app1, GsApp *app2)
{
	GsAppPrivate *priv1 = gs_app_get_instance_private (app1);
	GsAppPrivate *priv2 = gs_app_get_instance_private (app2);
	guint prio1, prio2;

	g_return_val_if_fail (GS_IS_APP (app1), 0);
	g_return_val_if_fail (GS_IS_APP (app2), 0);

	/* prefer prio */
	prio1 = gs_app_get_priority (app1);
	prio2 = gs_app_get_priority (app2);
	if (prio1 > prio2)
		return -1;
	if (prio1 < prio2)
		return 1;

	/* fall back to bundle kind */
	if (priv1->bundle_kind < priv2->bundle_kind)
		return -1;
	if (priv1->bundle_kind > priv2->bundle_kind)
		return 1;
	return 0;
}

/**
 * gs_app_quirk_to_string:
 * @quirk: a #GsAppQuirk
 *
 * Returns the quirk bitfield as a string.
 *
 * Returns: (transfer full): a string
 **/
static gchar *
gs_app_quirk_to_string (GsAppQuirk quirk)
{
	GString *str = g_string_new ("");
	guint64 i;

	/* nothing set */
	if (quirk == GS_APP_QUIRK_NONE) {
		g_string_append (str, "none");
		return g_string_free (str, FALSE);
	}

	/* get flags */
	for (i = 1; i < GS_APP_QUIRK_LAST; i *= 2) {
		if ((quirk & i) == 0)
			continue;
		g_string_append_printf (str, "%s,",
					_as_component_quirk_flag_to_string (i));
	}

	/* nothing recognised */
	if (str->len == 0) {
		g_string_append (str, "unknown");
		return g_string_free (str, FALSE);
	}

	/* remove trailing comma */
	g_string_truncate (str, str->len - 1);
	return g_string_free (str, FALSE);
}

static gchar *
gs_app_kudos_to_string (guint64 kudos)
{
	g_autoptr(GPtrArray) array = g_ptr_array_new ();
	if ((kudos & GS_APP_KUDO_MY_LANGUAGE) > 0)
		g_ptr_array_add (array, (gpointer) "my-language");
	if ((kudos & GS_APP_KUDO_RECENT_RELEASE) > 0)
		g_ptr_array_add (array, (gpointer) "recent-release");
	if ((kudos & GS_APP_KUDO_FEATURED_RECOMMENDED) > 0)
		g_ptr_array_add (array, (gpointer) "featured-recommended");
	if ((kudos & GS_APP_KUDO_HAS_KEYWORDS) > 0)
		g_ptr_array_add (array, (gpointer) "has-keywords");
	if ((kudos & GS_APP_KUDO_HAS_SCREENSHOTS) > 0)
		g_ptr_array_add (array, (gpointer) "has-screenshots");
	if ((kudos & GS_APP_KUDO_HI_DPI_ICON) > 0)
		g_ptr_array_add (array, (gpointer) "hi-dpi-icon");
	if ((kudos & GS_APP_KUDO_SANDBOXED) > 0)
		g_ptr_array_add (array, (gpointer) "sandboxed");
	if ((kudos & GS_APP_KUDO_SANDBOXED_SECURE) > 0)
		g_ptr_array_add (array, (gpointer) "sandboxed-secure");
	g_ptr_array_add (array, NULL);
	return g_strjoinv ("|", (gchar **) array->pdata);
}

/**
 * gs_app_to_string:
 * @app: a #GsApp
 *
 * Converts the application to a string.
 * This is not designed to serialize the object but to produce a string suitable
 * for debugging.
 *
 * Returns: A multi-line string
 *
 * Since: 3.22
 **/
gchar *
gs_app_to_string (GsApp *app)
{
	GString *str;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	str = g_string_new ("GsApp:");
	gs_app_to_string_append (app, str);
	if (str->len > 0)
		g_string_truncate (str, str->len - 1);
	return g_string_free (str, FALSE);
}

/**
 * gs_app_to_string_append:
 * @app: a #GsApp
 * @str: a #GString
 *
 * Appends the application to an existing string.
 *
 * Since: 3.26
 **/
void
gs_app_to_string_append (GsApp *app, GString *str)
{
	GsAppClass *klass;
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	AsImage *im;
	GList *keys;
	const gchar *tmp;
	guint i;
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GsPlugin) management_plugin = NULL;
	GsSizeType size_download_dependencies_type, size_installed_dependencies_type;
	guint64 size_download_dependencies_bytes, size_installed_dependencies_bytes;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (str != NULL);

	klass = GS_APP_GET_CLASS (app);

	locker = g_mutex_locker_new (&priv->mutex);

	g_string_append_printf (str, " [%p]\n", app);
	gs_app_kv_lpad (str, "kind", as_component_kind_to_string (priv->kind));
	gs_app_kv_lpad (str, "state", gs_app_state_to_string (priv->state));
	if (priv->quirk > 0) {
		g_autofree gchar *qstr = gs_app_quirk_to_string (priv->quirk);
		gs_app_kv_lpad (str, "quirk", qstr);
	}
	if (priv->progress == GS_APP_PROGRESS_UNKNOWN)
		gs_app_kv_printf (str, "progress", "unknown");
	else
		gs_app_kv_printf (str, "progress", "%u%%", priv->progress);
	if (priv->id != NULL)
		gs_app_kv_lpad (str, "id", priv->id);
	if (priv->unique_id != NULL)
		gs_app_kv_lpad (str, "unique-id", priv->unique_id);
	if (priv->scope != AS_COMPONENT_SCOPE_UNKNOWN)
		gs_app_kv_lpad (str, "scope", as_component_scope_to_string (priv->scope));
	if (priv->bundle_kind != AS_BUNDLE_KIND_UNKNOWN) {
		gs_app_kv_lpad (str, "bundle-kind",
				as_bundle_kind_to_string (priv->bundle_kind));
	}
	if (priv->kudos > 0) {
		g_autofree gchar *kudo_str = NULL;
		kudo_str = gs_app_kudos_to_string (priv->kudos);
		gs_app_kv_lpad (str, "kudos", kudo_str);
	}
	gs_app_kv_printf (str, "kudo-percentage", "%u",
			  gs_app_get_kudos_percentage (app));
	if (priv->name != NULL)
		gs_app_kv_lpad (str, "name", priv->name);
	if (priv->action_screenshot != NULL)
		gs_app_kv_printf (str, "action-screenshot", "%p", priv->action_screenshot);
	for (i = 0; priv->icons != NULL && i < priv->icons->len; i++) {
		GIcon *icon = g_ptr_array_index (priv->icons, i);
		g_autofree gchar *icon_str = g_icon_to_string (icon);
		gs_app_kv_lpad (str, "icon", icon_str);
	}
	if (priv->match_value != 0)
		gs_app_kv_printf (str, "match-value", "%05x", priv->match_value);
	if (gs_app_get_priority (app) != 0)
		gs_app_kv_printf (str, "priority", "%u", gs_app_get_priority (app));
	if (priv->version != NULL)
		gs_app_kv_lpad (str, "version", priv->version);
	if (priv->version_ui != NULL)
		gs_app_kv_lpad (str, "version-ui", priv->version_ui);
	if (priv->update_version != NULL)
		gs_app_kv_lpad (str, "update-version", priv->update_version);
	if (priv->update_version_ui != NULL)
		gs_app_kv_lpad (str, "update-version-ui", priv->update_version_ui);
	if (priv->update_details_markup != NULL)
		gs_app_kv_lpad (str, "update-details-markup", priv->update_details_markup);
	if (priv->update_urgency != AS_URGENCY_KIND_UNKNOWN) {
		gs_app_kv_printf (str, "update-urgency", "%u",
				  priv->update_urgency);
	}
	if (priv->summary != NULL)
		gs_app_kv_lpad (str, "summary", priv->summary);
	if (priv->description != NULL)
		gs_app_kv_lpad (str, "description", priv->description);
	for (i = 0; i < priv->screenshots->len; i++) {
		AsScreenshot *ss = g_ptr_array_index (priv->screenshots, i);
		g_autofree gchar *key = NULL;
		tmp = as_screenshot_get_caption (ss);
#if AS_CHECK_VERSION(1, 0, 0)
		im = as_screenshot_get_image (ss, 0, 0, 1);
#else
		im = as_screenshot_get_image (ss, 0, 0);
#endif
		if (im == NULL)
			continue;
		key = g_strdup_printf ("screenshot-%02u", i);
		gs_app_kv_printf (str, key, "%s [%s]",
				  as_image_get_url (im),
				  tmp != NULL ? tmp : "<none>");
	}
	for (i = 0; i < priv->sources->len; i++) {
		g_autofree gchar *key = NULL;
		tmp = g_ptr_array_index (priv->sources, i);
		key = g_strdup_printf ("source-%02u", i);
		gs_app_kv_lpad (str, key, tmp);
	}
	for (i = 0; i < priv->source_ids->len; i++) {
		g_autofree gchar *key = NULL;
		tmp = g_ptr_array_index (priv->source_ids, i);
		key = g_strdup_printf ("source-id-%02u", i);
		gs_app_kv_lpad (str, key, tmp);
	}
	if (priv->local_file != NULL) {
		g_autofree gchar *fn = g_file_get_path (priv->local_file);
		gs_app_kv_lpad (str, "local-filename", fn);
	}
	if (priv->content_rating != NULL) {
		guint age = as_content_rating_get_minimum_age (priv->content_rating);
		if (age != G_MAXUINT) {
			g_autofree gchar *value = g_strdup_printf ("%u", age);
			gs_app_kv_lpad (str, "content-age", value);
		}
		gs_app_kv_lpad (str, "content-rating",
				as_content_rating_get_kind (priv->content_rating));
	}
	if (priv->urls != NULL) {
		tmp = g_hash_table_lookup (priv->urls, GINT_TO_POINTER (AS_URL_KIND_HOMEPAGE));
		if (tmp != NULL)
			gs_app_kv_lpad (str, "url{homepage}", tmp);
	}
	keys = g_hash_table_get_keys (priv->launchables);
	for (GList *l = keys; l != NULL; l = l->next) {
		g_autofree gchar *key = NULL;
		key = g_strdup_printf ("launchable{%s}", (const gchar *) l->data);
		tmp = g_hash_table_lookup (priv->launchables, l->data);
		gs_app_kv_lpad (str, key, tmp);
	}
	g_list_free (keys);
	if (priv->license != NULL) {
		gs_app_kv_lpad (str, "license", priv->license);
		gs_app_kv_lpad (str, "license-is-free",
				gs_app_get_license_is_free (app) ? "yes" : "no");
	}
	management_plugin = g_weak_ref_get (&priv->management_plugin_weak);
	if (management_plugin != NULL)
		gs_app_kv_lpad (str, "management-plugin", gs_plugin_get_name (management_plugin));
	if (priv->summary_missing != NULL)
		gs_app_kv_lpad (str, "summary-missing", priv->summary_missing);
	if (priv->menu_path != NULL &&
	    priv->menu_path[0] != NULL &&
	    priv->menu_path[0][0] != '\0') {
		g_autofree gchar *path = g_strjoinv (" → ", priv->menu_path);
		gs_app_kv_lpad (str, "menu-path", path);
	}
	if (priv->branch != NULL)
		gs_app_kv_lpad (str, "branch", priv->branch);
	if (priv->origin != NULL && priv->origin[0] != '\0')
		gs_app_kv_lpad (str, "origin", priv->origin);
	if (priv->origin_ui != NULL && priv->origin_ui[0] != '\0')
		gs_app_kv_lpad (str, "origin-ui", priv->origin_ui);
	if (priv->origin_appstream != NULL && priv->origin_appstream[0] != '\0')
		gs_app_kv_lpad (str, "origin-appstream", priv->origin_appstream);
	if (priv->origin_hostname != NULL && priv->origin_hostname[0] != '\0')
		gs_app_kv_lpad (str, "origin-hostname", priv->origin_hostname);
	if (priv->rating != -1)
		gs_app_kv_printf (str, "rating", "%i", priv->rating);
	if (priv->review_ratings != NULL) {
		for (i = 0; i < priv->review_ratings->len; i++) {
			guint32 rat = g_array_index (priv->review_ratings, guint32, i);
			gs_app_kv_printf (str, "review-rating", "[%u:%u]",
					  i, rat);
		}
	}
	if (priv->reviews != NULL)
		gs_app_kv_printf (str, "reviews", "%u", priv->reviews->len);
	if (priv->provided != NULL) {
		guint total = 0;
		for (i = 0; i < priv->provided->len; i++)
			total += as_provided_get_items (AS_PROVIDED (g_ptr_array_index (priv->provided, i)))->len;
		gs_app_kv_printf (str, "provided", "%u", total);
	}
	if (priv->install_date != 0) {
		gs_app_kv_printf (str, "install-date", "%"
				  G_GUINT64_FORMAT "",
				  priv->install_date);
	}
	if (priv->release_date != 0) {
		gs_app_kv_printf (str, "release-date", "%"
				  G_GUINT64_FORMAT "",
				  priv->release_date);
	}

	gs_app_kv_size (str, "size-installed", priv->size_installed_type, priv->size_installed);
	size_installed_dependencies_type = gs_app_get_size_installed_dependencies (app, &size_installed_dependencies_bytes);
	gs_app_kv_size (str, "size-installed-dependencies", size_installed_dependencies_type, size_installed_dependencies_bytes);
	gs_app_kv_size (str, "size-download", priv->size_download_type, priv->size_download);
	size_download_dependencies_type = gs_app_get_size_download_dependencies (app, &size_download_dependencies_bytes);
	gs_app_kv_size (str, "size-download-dependencies", size_download_dependencies_type, size_download_dependencies_bytes);
	gs_app_kv_size (str, "size-cache-data", priv->size_cache_data_type, priv->size_cache_data);
	gs_app_kv_size (str, "size-user-data", priv->size_user_data_type, priv->size_user_data);

	for (i = 0; i < gs_app_list_length (priv->related); i++) {
		GsApp *app_tmp = gs_app_list_index (priv->related, i);
		const gchar *id = gs_app_get_unique_id (app_tmp);
		if (id == NULL)
			id = gs_app_get_source_default (app_tmp);
		/* For example PackageKit can create apps without id */
		if (id != NULL)
			gs_app_kv_lpad (str, "related", id);
	}
	for (i = 0; i < gs_app_list_length (priv->history); i++) {
		GsApp *app_tmp = gs_app_list_index (priv->history, i);
		const gchar *id = gs_app_get_unique_id (app_tmp);
		if (id == NULL)
			id = gs_app_get_source_default (app_tmp);
		/* For example PackageKit can create apps without id */
		if (id != NULL)
			gs_app_kv_lpad (str, "history", id);
	}
	for (i = 0; i < priv->categories->len; i++) {
		tmp = g_ptr_array_index (priv->categories, i);
		gs_app_kv_lpad (str, "category", tmp);
	}
	if (priv->user_key_colors)
		gs_app_kv_lpad (str, "user-key-colors", "yes");
	for (i = 0; priv->key_colors != NULL && i < priv->key_colors->len; i++) {
		GdkRGBA *color = &g_array_index (priv->key_colors, GdkRGBA, i);
		g_autofree gchar *key = NULL;
		key = g_strdup_printf ("key-color-%02u", i);
		gs_app_kv_printf (str, key, "%.0f,%.0f,%.0f",
				  color->red * 255.f,
				  color->green * 255.f,
				  color->blue * 255.f);
	}
	keys = g_hash_table_get_keys (priv->metadata);
	for (GList *l = keys; l != NULL; l = l->next) {
		GVariant *val;
		const GVariantType *val_type;
		g_autofree gchar *key = NULL;
		g_autofree gchar *val_str = NULL;

		key = g_strdup_printf ("{%s}", (const gchar *) l->data);
		val = g_hash_table_lookup (priv->metadata, l->data);
		val_type = g_variant_get_type (val);
		if (g_variant_type_equal (val_type, G_VARIANT_TYPE_STRING)) {
			val_str = g_variant_dup_string (val, NULL);
		} else if (g_variant_type_equal (val_type, G_VARIANT_TYPE_BOOLEAN)) {
			val_str = g_strdup (g_variant_get_boolean (val) ? "True" : "False");
		} else if (g_variant_type_equal (val_type, G_VARIANT_TYPE_UINT32)) {
			val_str = g_strdup_printf ("%" G_GUINT32_FORMAT,
						   g_variant_get_uint32 (val));
		} else {
			val_str = g_strdup_printf ("unknown type of %s",
						   g_variant_get_type_string (val));
		}
		gs_app_kv_lpad (str, key, val_str);
	}
	g_list_free (keys);

	for (i = 0; priv->relations != NULL && i < priv->relations->len; i++) {
		AsRelation *relation = g_ptr_array_index (priv->relations, i);
		gs_app_kv_printf (str, "relation", "%s, %s",
				  as_relation_kind_to_string (as_relation_get_kind (relation)),
				  as_relation_item_kind_to_string (as_relation_get_item_kind (relation)));
	}

	/* add subclassed info */
	if (klass->to_string != NULL)
		klass->to_string (app, str);

	/* print runtime data too */
	if (priv->runtime != NULL) {
		g_string_append (str, "\n\tRuntime:\n\t");
		gs_app_to_string_append (priv->runtime, str);
	}
	g_string_append_printf (str, "\n");
}

typedef struct {
	GsApp *app;
	GParamSpec *pspec;
} AppNotifyData;

static gboolean
notify_idle_cb (gpointer data)
{
	AppNotifyData *notify_data = data;

	g_object_notify_by_pspec (G_OBJECT (notify_data->app), notify_data->pspec);

	g_object_unref (notify_data->app);
	g_free (notify_data);

	return G_SOURCE_REMOVE;
}

static void
gs_app_queue_notify (GsApp *app, GParamSpec *pspec)
{
	AppNotifyData *notify_data;

	notify_data = g_new (AppNotifyData, 1);
	notify_data->app = g_object_ref (app);
	notify_data->pspec = pspec;

	g_idle_add (notify_idle_cb, notify_data);
}

/**
 * gs_app_get_id:
 * @app: a #GsApp
 *
 * Gets the application ID.
 *
 * Returns: The whole ID, e.g. "gimp.desktop"
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_id (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->id;
}

/**
 * gs_app_set_id:
 * @app: a #GsApp
 * @id: a application ID, e.g. "gimp.desktop"
 *
 * Sets the application ID.
 */
void
gs_app_set_id (GsApp *app, const gchar *id)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (_g_set_str (&priv->id, id))
		priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_scope:
 * @app: a #GsApp
 *
 * Gets the scope of the application.
 *
 * Returns: the #AsComponentScope, e.g. %AS_COMPONENT_SCOPE_USER
 *
 * Since: 40
 **/
AsComponentScope
gs_app_get_scope (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), AS_COMPONENT_SCOPE_UNKNOWN);
	return priv->scope;
}

/**
 * gs_app_set_scope:
 * @app: a #GsApp
 * @scope: a #AsComponentScope, e.g. %AS_COMPONENT_SCOPE_SYSTEM
 *
 * This sets the scope of the application.
 *
 * Since: 40
 **/
void
gs_app_set_scope (GsApp *app, AsComponentScope scope)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	/* same */
	if (scope == priv->scope)
		return;

	priv->scope = scope;

	/* no longer valid */
	priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_bundle_kind:
 * @app: a #GsApp
 *
 * Gets the bundle kind of the application.
 *
 * Returns: the #AsComponentScope, e.g. %AS_BUNDLE_KIND_FLATPAK
 *
 * Since: 40
 **/
AsBundleKind
gs_app_get_bundle_kind (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), AS_BUNDLE_KIND_UNKNOWN);
	return priv->bundle_kind;
}

/**
 * gs_app_set_bundle_kind:
 * @app: a #GsApp
 * @bundle_kind: a #AsComponentScope, e.g. AS_BUNDLE_KIND_FLATPAK
 *
 * This sets the bundle kind of the application.
 *
 * Since: 40
 **/
void
gs_app_set_bundle_kind (GsApp *app, AsBundleKind bundle_kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	/* same */
	if (bundle_kind == priv->bundle_kind)
		return;

	priv->bundle_kind = bundle_kind;

	/* no longer valid */
	priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_special_kind:
 * @app: a #GsApp
 *
 * Gets the special occupation of the application.
 *
 * Returns: the #GsAppSpecialKind, e.g. %GS_APP_SPECIAL_KIND_OS_UPDATE
 *
 * Since: 40
 **/
GsAppSpecialKind
gs_app_get_special_kind (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), GS_APP_SPECIAL_KIND_NONE);
	return priv->special_kind;
}

/**
 * gs_app_set_special_kind:
 * @app: a #GsApp
 * @kind: a #GsAppSpecialKind, e.g. %GS_APP_SPECIAL_KIND_OS_UPDATE
 *
 * This sets the special occupation of the application (making
 * the #AsComponentKind of this application %AS_COMPONENT_KIND_GENERIC
 * per definition).
 *
 * Since: 40
 **/
void
gs_app_set_special_kind	(GsApp *app, GsAppSpecialKind kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));

	if (priv->special_kind == kind)
		return;
	gs_app_set_kind (app, AS_COMPONENT_KIND_GENERIC);
	priv->special_kind = kind;
	gs_app_queue_notify (app, obj_props[PROP_SPECIAL_KIND]);
}

/**
 * gs_app_get_state:
 * @app: a #GsApp
 *
 * Gets the state of the application.
 *
 * Returns: the #GsAppState, e.g. %GS_APP_STATE_INSTALLED
 *
 * Since: 40
 **/
GsAppState
gs_app_get_state (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), GS_APP_STATE_UNKNOWN);
	return priv->state;
}

/**
 * gs_app_get_progress:
 * @app: a #GsApp
 *
 * Gets the percentage completion.
 *
 * Returns: the percentage completion (0–100 inclusive), or %GS_APP_PROGRESS_UNKNOWN for unknown
 *
 * Since: 3.22
 **/
guint
gs_app_get_progress (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), GS_APP_PROGRESS_UNKNOWN);
	return priv->progress;
}

/**
 * gs_app_get_allow_cancel:
 * @app: a #GsApp
 *
 * Gets whether the app's installation or upgrade can be cancelled.
 *
 * Returns: TRUE if cancellation is possible, FALSE otherwise.
 *
 * Since: 3.26
 **/
gboolean
gs_app_get_allow_cancel (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return priv->allow_cancel;
}

/**
 * gs_app_set_state_recover:
 * @app: a #GsApp
 *
 * Sets the application state to the last status value that was not
 * transient.
 *
 * Since: 3.22
 **/
void
gs_app_set_state_recover (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	if (priv->state_recover == GS_APP_STATE_UNKNOWN)
		return;
	if (priv->state_recover == priv->state)
		return;

	g_debug ("recovering state on %s from %s to %s",
		 priv->id,
		 gs_app_state_to_string (priv->state),
		 gs_app_state_to_string (priv->state_recover));

	/* make sure progress gets reset when recovering state, to prevent
	 * confusing initial states when going through more than one attempt */
	gs_app_set_progress (app, GS_APP_PROGRESS_UNKNOWN);

	priv->state = priv->state_recover;
	gs_app_queue_notify (app, obj_props[PROP_STATE]);
}

/* mutex must be held */
static gboolean
gs_app_set_state_internal (GsApp *app, GsAppState state)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	gboolean state_change_ok = FALSE;

	/* same */
	if (priv->state == state)
		return FALSE;

	/* check the state change is allowed */
	switch (priv->state) {
	case GS_APP_STATE_UNKNOWN:
		/* unknown has to go into one of the stable states */
		if (state == GS_APP_STATE_INSTALLED ||
		    state == GS_APP_STATE_QUEUED_FOR_INSTALL ||
		    state == GS_APP_STATE_AVAILABLE ||
		    state == GS_APP_STATE_AVAILABLE_LOCAL ||
		    state == GS_APP_STATE_UPDATABLE ||
		    state == GS_APP_STATE_UPDATABLE_LIVE ||
		    state == GS_APP_STATE_UNAVAILABLE ||
		    state == GS_APP_STATE_PENDING_INSTALL ||
		    state == GS_APP_STATE_PENDING_REMOVE)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_INSTALLED:
		/* installed has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_REMOVING ||
		    state == GS_APP_STATE_UNAVAILABLE ||
		    state == GS_APP_STATE_UPDATABLE ||
		    state == GS_APP_STATE_UPDATABLE_LIVE)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_DOWNLOADING ||
		    state == GS_APP_STATE_AVAILABLE)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_AVAILABLE:
		/* available has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_QUEUED_FOR_INSTALL ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_DOWNLOADING)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_DOWNLOADING:
		/* downloading is similar to the installing state, to which it can go too */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_UPDATABLE ||
		    state == GS_APP_STATE_UPDATABLE_LIVE ||
		    state == GS_APP_STATE_AVAILABLE ||
		    state == GS_APP_STATE_PENDING_INSTALL)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_INSTALLING:
		/* installing has to go into an stable state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_INSTALLED ||
		    state == GS_APP_STATE_UPDATABLE ||
		    state == GS_APP_STATE_UPDATABLE_LIVE ||
		    state == GS_APP_STATE_AVAILABLE ||
		    state == GS_APP_STATE_PENDING_INSTALL)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_REMOVING:
		/* removing has to go into an stable state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_UNAVAILABLE ||
		    state == GS_APP_STATE_AVAILABLE ||
		    state == GS_APP_STATE_INSTALLED ||
		    state == GS_APP_STATE_PENDING_REMOVE)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_UPDATABLE:
		/* updatable has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_REMOVING ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_DOWNLOADING ||
		    state == GS_APP_STATE_PENDING_INSTALL)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_UPDATABLE_LIVE:
		/* updatable-live has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_REMOVING ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_DOWNLOADING)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_UNAVAILABLE:
		/* updatable has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_AVAILABLE)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_AVAILABLE_LOCAL:
		/* local has to go into an action state */
		if (state == GS_APP_STATE_UNKNOWN ||
		    state == GS_APP_STATE_QUEUED_FOR_INSTALL ||
		    state == GS_APP_STATE_INSTALLING ||
		    state == GS_APP_STATE_DOWNLOADING)
			state_change_ok = TRUE;
		break;
	case GS_APP_STATE_PENDING_INSTALL:
	case GS_APP_STATE_PENDING_REMOVE:
		state_change_ok = TRUE;
		break;
	default:
		g_warning ("state %s unhandled",
			   gs_app_state_to_string (priv->state));
		g_assert_not_reached ();
	}

	/* this state change was unexpected */
	if (!state_change_ok) {
		g_warning ("State change on %s (%s) from %s to %s is not OK",
			   gs_app_get_unique_id_unlocked (app),
			   priv->name,
			   gs_app_state_to_string (priv->state),
			   gs_app_state_to_string (state));
	}

	priv->state = state;

	if (state == GS_APP_STATE_UNKNOWN ||
	    state == GS_APP_STATE_AVAILABLE_LOCAL ||
	    state == GS_APP_STATE_AVAILABLE)
		priv->install_date = 0;

	/* save this to simplify error handling in the plugins */
	switch (state) {
	case GS_APP_STATE_DOWNLOADING:
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_REMOVING:
	case GS_APP_STATE_QUEUED_FOR_INSTALL:
		/* transient, so ignore */
		break;
	default:
		if (priv->state_recover != state)
			priv->state_recover = state;
		break;
	}

	return TRUE;
}

/**
 * gs_app_set_progress:
 * @app: a #GsApp
 * @percentage: a percentage progress (0–100 inclusive), or %GS_APP_PROGRESS_UNKNOWN
 *
 * This sets the progress completion of the application. Use
 * %GS_APP_PROGRESS_UNKNOWN if the progress is unknown or has a wide confidence
 * interval.
 *
 * If called more than once with the same value then subsequent calls
 * will be ignored.
 *
 * Since: 3.22
 **/
void
gs_app_set_progress (GsApp *app, guint percentage)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->progress == percentage)
		return;
	if (percentage != GS_APP_PROGRESS_UNKNOWN && percentage > 100) {
		g_warning ("cannot set %u%% for %s, setting instead: 100%%",
			   percentage, gs_app_get_unique_id_unlocked (app));
		percentage = 100;
	}
	priv->progress = percentage;
	gs_app_queue_notify (app, obj_props[PROP_PROGRESS]);
}

/**
 * gs_app_set_allow_cancel:
 * @app: a #GsApp
 * @allow_cancel: if the installation or upgrade can be cancelled or not
 *
 * This sets a flag indicating whether the operation can be cancelled or not.
 * This is used by the UI to set the "Cancel" button insensitive as
 * appropriate.
 *
 * Since: 3.26
 **/
void
gs_app_set_allow_cancel (GsApp *app, gboolean allow_cancel)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->allow_cancel == allow_cancel)
		return;
	priv->allow_cancel = allow_cancel;
	gs_app_queue_notify (app, obj_props[PROP_CAN_CANCEL_INSTALLATION]);
}

static void
gs_app_set_pending_action_internal (GsApp *app,
				    GsPluginAction action)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	if (priv->pending_action == action)
		return;

	priv->pending_action = action;
	gs_app_queue_notify (app, obj_props[PROP_PENDING_ACTION]);
}

/**
 * gs_app_set_state:
 * @app: a #GsApp
 * @state: a #GsAppState, e.g. GS_APP_STATE_UPDATABLE_LIVE
 *
 * This sets the state of the application.
 * The following state diagram explains the typical states.
 * All applications start in state %GS_APP_STATE_UNKNOWN,
 * but the frontend is not supposed to see GsApps with this state.
 *
 * Plugins are responsible for changing the state to one of the other
 * states before the GsApp is passed to the frontend.
 *
 * |[
 * UPDATABLE --> INSTALLING --> INSTALLED
 * UPDATABLE --> REMOVING   --> AVAILABLE
 * INSTALLED --> REMOVING   --> AVAILABLE
 * AVAILABLE --> INSTALLING --> INSTALLED
 * AVAILABLE <--> QUEUED --> INSTALLING --> INSTALLED
 * UNKNOWN   --> UNAVAILABLE
 * ]|
 *
 * Since: 3.22
 **/
void
gs_app_set_state (GsApp *app, GsAppState state)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (gs_app_set_state_internal (app, state)) {
		/* since the state changed, and the pending-action refers to
		 * actions that usually change the state, we assign it to the
		 * appropriate action here */
		GsPluginAction action = GS_PLUGIN_ACTION_UNKNOWN;
		if (priv->state == GS_APP_STATE_QUEUED_FOR_INSTALL) {
			if (priv->kind == AS_COMPONENT_KIND_REPOSITORY)
				action = GS_PLUGIN_ACTION_INSTALL_REPO;
			else
				action = GS_PLUGIN_ACTION_INSTALL;
		}
		gs_app_set_pending_action_internal (app, action);

		gs_app_queue_notify (app, obj_props[PROP_STATE]);
	}
}

/**
 * gs_app_get_kind:
 * @app: a #GsApp
 *
 * Gets the kind of the application.
 *
 * Returns: the #AsComponentKind, e.g. %AS_COMPONENT_KIND_UNKNOWN
 *
 * Since: 40
 **/
AsComponentKind
gs_app_get_kind (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), AS_COMPONENT_KIND_UNKNOWN);
	return priv->kind;
}

/**
 * gs_app_set_kind:
 * @app: a #GsApp
 * @kind: a #AsComponentKind, e.g. #AS_COMPONENT_KIND_DESKTOP_APP
 *
 * This sets the kind of the application.
 * The following state diagram explains the typical states.
 * All applications start with kind %AS_COMPONENT_KIND_UNKNOWN.
 *
 * |[
 * PACKAGE --> NORMAL
 * PACKAGE --> SYSTEM
 * NORMAL  --> SYSTEM
 * ]|
 *
 * Since: 40
 **/
void
gs_app_set_kind (GsApp *app, AsComponentKind kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	gboolean state_change_ok = FALSE;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* same */
	if (priv->kind == kind)
		return;

	/* trying to change */
	if (priv->kind != AS_COMPONENT_KIND_UNKNOWN &&
	    kind == AS_COMPONENT_KIND_UNKNOWN) {
		g_warning ("automatically prevented from changing "
			   "kind on %s from %s to %s!",
			   gs_app_get_unique_id_unlocked (app),
			   as_component_kind_to_string (priv->kind),
			   as_component_kind_to_string (kind));
		return;
	}

	/* check the state change is allowed */
	switch (priv->kind) {
	case AS_COMPONENT_KIND_UNKNOWN:
	case AS_COMPONENT_KIND_GENERIC:
		/* all others derive from generic */
		state_change_ok = TRUE;
		break;
	case AS_COMPONENT_KIND_DESKTOP_APP:
		/* desktop has to be reset to override */
		if (kind == AS_COMPONENT_KIND_UNKNOWN)
			state_change_ok = TRUE;
		break;
	default:
		/* this can never change state */
		break;
	}

	/* this state change was unexpected */
	if (!state_change_ok) {
		g_warning ("Kind change on %s from %s to %s is not OK",
			   priv->id,
			   as_component_kind_to_string (priv->kind),
			   as_component_kind_to_string (kind));
		return;
	}

	priv->kind = kind;
	gs_app_queue_notify (app, obj_props[PROP_KIND]);

	/* no longer valid */
	priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_unique_id:
 * @app: a #GsApp
 *
 * Gets the unique application ID used for de-duplication.
 *
 * The format is "&lt;scope&gt;/&lt;kind&gt;/&lt;origin&gt;/&lt;id&gt;/&lt;branch&gt;". Any unset fields will
 * appear as "*". This string can be used with libappstream's functions for
 * handling data IDs, e.g.
 * https://www.freedesktop.org/software/appstream/docs/api/appstream-as-utils.html#as-utils-data-id-valid
 *
 * Returns: The unique ID, e.g. `system/flatpak/flathub/org.lingmo.Notes/stable`, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_unique_id (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return gs_app_get_unique_id_unlocked (app);
}

/**
 * gs_app_set_unique_id:
 * @app: a #GsApp
 * @unique_id: a unique application ID, e.g. `user/fedora/\*\/gimp.desktop/\*`
 *
 * Sets the unique application ID used for de-duplication. See
 * gs_app_get_unique_id() for information about the format. Normally you should
 * not have to use this function since the unique ID can be constructed from
 * other fields, but it can be useful for unit tests.
 */
void
gs_app_set_unique_id (GsApp *app, const gchar *unique_id)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* check for sanity */
	if (!as_utils_data_id_valid (unique_id))
		g_warning ("unique_id %s not valid", unique_id);

	g_free (priv->unique_id);
	priv->unique_id = g_strdup (unique_id);
	priv->unique_id_valid = TRUE;
}

/**
 * gs_app_get_name:
 * @app: a #GsApp
 *
 * Gets the application name.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_name (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->name;
}

/**
 * gs_app_set_name:
 * @app: a #GsApp
 * @quality: A #GsAppQuality, e.g. %GS_APP_QUALITY_LOWEST
 * @name: The short localized name, e.g. "Calculator"
 *
 * Sets the application name.
 *
 * Since: 3.22
 **/
void
gs_app_set_name (GsApp *app, GsAppQuality quality, const gchar *name)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* only save this if the data is sufficiently high quality */
	if (quality < priv->name_quality)
		return;
	priv->name_quality = quality;
	if (_g_set_str (&priv->name, name))
		gs_app_queue_notify (app, obj_props[PROP_NAME]);
}

/**
 * gs_app_get_renamed_from:
 * @app: a #GsApp
 *
 * Gets the old human-readable name of an application that's being renamed, the
 * same name that was returned by gs_app_get_name() before the rename.
 *
 * Returns: (nullable): a string, or %NULL for unset
 *
 * Since: 40
 **/
const gchar *
gs_app_get_renamed_from (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->renamed_from;
}

/**
 * gs_app_set_renamed_from:
 * @app: a #GsApp
 * @renamed_from: (nullable): The old name, e.g. "Iagno"
 *
 * Sets the old name of an application that's being renamed
 *
 * Since: 40
 **/
void
gs_app_set_renamed_from (GsApp *app, const gchar *renamed_from)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_str (&priv->renamed_from, renamed_from);
}

/**
 * gs_app_get_branch:
 * @app: a #GsApp
 *
 * Gets the application branch.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_branch (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->branch;
}

/**
 * gs_app_set_branch:
 * @app: a #GsApp
 * @branch: The branch, e.g. "master"
 *
 * Sets the application branch.
 *
 * Since: 3.22
 **/
void
gs_app_set_branch (GsApp *app, const gchar *branch)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (_g_set_str (&priv->branch, branch))
		priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_source_default:
 * @app: a #GsApp
 *
 * Gets the default source.
 *
 * Returns: a string, or %NULL
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_source_default (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	if (priv->sources->len == 0)
		return NULL;
	return g_ptr_array_index (priv->sources, 0);
}

/**
 * gs_app_add_source:
 * @app: a #GsApp
 * @source: a source name
 *
 * Adds a source name for the application.
 *
 * Since: 3.22
 **/
void
gs_app_add_source (GsApp *app, const gchar *source)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	const gchar *tmp;
	guint i;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (source != NULL);

	locker = g_mutex_locker_new (&priv->mutex);

	/* check source doesn't already exist */
	for (i = 0; i < priv->sources->len; i++) {
		tmp = g_ptr_array_index (priv->sources, i);
		if (g_strcmp0 (tmp, source) == 0)
			return;
	}
	g_ptr_array_add (priv->sources, g_strdup (source));
}

/**
 * gs_app_get_sources:
 * @app: a #GsApp
 *
 * Gets the list of sources for the application.
 *
 * Returns: (element-type utf8) (transfer none): a list
 *
 * Since: 3.22
 **/
GPtrArray *
gs_app_get_sources (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->sources;
}

/**
 * gs_app_set_sources:
 * @app: a #GsApp
 * @sources: The non-localized short names, e.g. ["lingmo-calculator"]
 *
 * This name is used for the update page if the application is collected into
 * the 'OS Updates' group.
 * It is typically the package names, although this should not be relied upon.
 *
 * Since: 3.22
 **/
void
gs_app_set_sources (GsApp *app, GPtrArray *sources)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_ptr_array (&priv->sources, sources);
}

/**
 * gs_app_get_source_id_default:
 * @app: a #GsApp
 *
 * Gets the default source ID.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_source_id_default (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	if (priv->source_ids->len == 0)
		return NULL;
	return g_ptr_array_index (priv->source_ids, 0);
}

/**
 * gs_app_get_source_ids:
 * @app: a #GsApp
 *
 * Gets the list of source IDs.
 *
 * Returns: (element-type utf8) (transfer none): a list
 *
 * Since: 3.22
 **/
GPtrArray *
gs_app_get_source_ids (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->source_ids;
}

/**
 * gs_app_clear_source_ids:
 * @app: a #GsApp
 *
 * Clear the list of source IDs.
 *
 * Since: 3.22
 **/
void
gs_app_clear_source_ids (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	g_ptr_array_set_size (priv->source_ids, 0);
}

/**
 * gs_app_set_source_ids:
 * @app: a #GsApp
 * @source_ids: The source-id, e.g. ["lingmo-calculator;0.134;fedora"]
 *		or ["/home/hughsie/.local/share/applications/0ad.desktop"]
 *
 * This ID is used internally to the controlling plugin.
 *
 * Since: 3.22
 **/
void
gs_app_set_source_ids (GsApp *app, GPtrArray *source_ids)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_ptr_array (&priv->source_ids, source_ids);
}

/**
 * gs_app_add_source_id:
 * @app: a #GsApp
 * @source_id: a source ID, e.g. "lingmo-calculator;0.134;fedora"
 *
 * Adds a source ID to the application.
 *
 * Since: 3.22
 **/
void
gs_app_add_source_id (GsApp *app, const gchar *source_id)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	const gchar *tmp;
	guint i;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (source_id != NULL);

	/* only add if not already present */
	for (i = 0; i < priv->source_ids->len; i++) {
		tmp = g_ptr_array_index (priv->source_ids, i);
		if (g_strcmp0 (tmp, source_id) == 0)
			return;
	}
	g_ptr_array_add (priv->source_ids, g_strdup (source_id));
}

/**
 * gs_app_get_project_group:
 * @app: a #GsApp
 *
 * Gets a project group for the application.
 * Applications belonging to other project groups may not be shown in
 * this software center.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_project_group (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->project_group;
}

/**
 * gs_app_get_developer_name:
 * @app: a #GsApp
 *
 * Gets the developer name for the application.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_developer_name (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->developer_name;
}

/**
 * gs_app_set_project_group:
 * @app: a #GsApp
 * @project_group: The non-localized project group, e.g. "LINGMO" or "KDE"
 *
 * Sets a project group for the application.
 *
 * Since: 3.22
 **/
void
gs_app_set_project_group (GsApp *app, const gchar *project_group)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_str (&priv->project_group, project_group);
}

/**
 * gs_app_set_developer_name:
 * @app: a #GsApp
 * @developer_name: The developer name, e.g. "Richard Hughes"
 *
 * Sets a developer name for the application.
 *
 * Since: 3.22
 **/
void
gs_app_set_developer_name (GsApp *app, const gchar *developer_name)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_str (&priv->developer_name, developer_name);
}

static GtkIconTheme *
get_icon_theme (void)
{
	GtkIconTheme *theme;
	GdkDisplay *display = gdk_display_get_default ();

	if (display != NULL) {
		theme = g_object_ref (gtk_icon_theme_get_for_display (display));
	} else {
		const gchar *test_search_path;

		/* This fallback path is needed for the unit tests,
		 * which run without a screen, and in an environment
		 * where the XDG dir variables don’t point to the system
		 * datadir which contains the system icon theme. */
		theme = gtk_icon_theme_new ();

		test_search_path = g_getenv ("GS_SELF_TEST_ICON_THEME_PATH");
		if (test_search_path != NULL) {
			g_auto(GStrv) dirs = g_strsplit (test_search_path, ":", -1);
			gtk_icon_theme_set_search_path (theme, (const char * const *) dirs);
		}

		gtk_icon_theme_add_resource_path (theme, "/org/lingmo/Appstore/icons/");
	}

	return theme;
}

/**
 * gs_app_get_icon_for_size:
 * @app: a #GsApp
 * @size: size (width or height, square) of the icon to fetch, in device pixels
 * @scale: scale of the icon to fetch, typically from gtk_widget_get_scale_factor()
 * @fallback_icon_name: (nullable): name of an icon to load as a fallback if
 *    no other suitable one is found, or %NULL for no fallback
 *
 * Finds the most appropriate icon in the @app’s set of icons to be loaded at
 * the given @size×@scale to represent the application. This might be provided
 * by the backend at the given @size, or downsized from a larger icon provided
 * by the backend. The return value is guaranteed to be suitable for loading as
 * a pixbuf at @size, if it’s not %NULL.
 *
 * If an image at least @size pixels in width isn’t available, and
 * @fallback_icon_name has not been provided, %NULL will be returned. If
 * @fallback_icon_name has been provided, a #GIcon representing that will be
 * returned, and %NULL is guaranteed not to be returned.
 *
 * Icons which come from a remote server (over HTTP or HTTPS) will be returned
 * as a pointer into a local cache, which may not have been populated. You must
 * call gs_remote_icon_ensure_cached() on icons of type #GsRemoteIcon to
 * download them; this function will not do that for you.
 *
 * This function may do disk I/O or image resizing, but it will not do network
 * I/O to load a pixbuf. It should be acceptable to call this from a UI thread.
 *
 * Returns: (transfer full) (nullable): a #GIcon, or %NULL
 *
 * Since: 40
 */
GIcon *
gs_app_get_icon_for_size (GsApp       *app,
                          guint        size,
                          guint        scale,
                          const gchar *fallback_icon_name)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);
	g_return_val_if_fail (size > 0, NULL);
	g_return_val_if_fail (scale >= 1, NULL);

	g_debug ("Looking for icon for %s, at size %u×%u, with fallback %s",
		 gs_app_get_id (app), size, scale, fallback_icon_name);

	locker = g_mutex_locker_new (&priv->mutex);

	/* See if there’s an icon of the right size, or the first one which is too
	 * big which could be scaled down. Note that the icons array may be
	 * lazily created. */
	for (guint i = 0; priv->icons != NULL && i < priv->icons->len; i++) {
		GIcon *icon = priv->icons->pdata[i];
		g_autofree gchar *icon_str = g_icon_to_string (icon);
		guint icon_width = gs_icon_get_width (icon);
		guint icon_scale = gs_icon_get_scale (icon);

		g_debug ("\tConsidering icon of type %s (%s), width %u×%u",
			 G_OBJECT_TYPE_NAME (icon), icon_str, icon_width, icon_scale);

		/* To avoid excessive I/O, the loading of AppStream data does
		 * not verify the existence of cached icons, which we do now. */
		if (G_IS_FILE_ICON (icon)) {
			GFile *file = g_file_icon_get_file (G_FILE_ICON (icon));
			if (!g_file_query_exists (file, NULL)) {
				continue;
			}
		}

		/* Ignore icons with unknown width and skip over ones which
		 * are too small. */
		if (icon_width == 0 || icon_width * icon_scale < size * scale)
			continue;

		if (icon_width * icon_scale >= size * scale)
			return g_object_ref (icon);
	}

	/* Fallback to themed icons with no width set. Typically
	 * themed icons are available in any given size. */
	for (guint i = 0; priv->icons != NULL && i < priv->icons->len; i++) {
		GIcon *icon = priv->icons->pdata[i];
		guint icon_width = gs_icon_get_width (icon);

		if (icon_width == 0 && G_IS_THEMED_ICON (icon)) {
			g_autoptr(GtkIconTheme) theme = get_icon_theme ();
			if (gtk_icon_theme_has_gicon (theme, icon)) {
				g_debug ("Found themed icon");
				return g_object_ref (icon);
			}
		}
	}

	g_clear_pointer (&locker, g_mutex_locker_free);

	if (scale > 1) {
		g_debug ("Retrying at scale 1");
		return gs_app_get_icon_for_size (app, size, 1, fallback_icon_name);
	} else if (fallback_icon_name != NULL) {
		g_debug ("Using fallback icon %s", fallback_icon_name);
		return g_themed_icon_new (fallback_icon_name);
	} else {
		g_debug ("No icon found");
		return NULL;
	}
}

/**
 * gs_app_get_action_screenshot:
 * @app: a #GsApp
 *
 * Gets a screenshot for the pending user action.
 *
 * Returns: (transfer none) (nullable): a #AsScreenshot, or %NULL
 *
 * Since: 40
 **/
AsScreenshot *
gs_app_get_action_screenshot (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->action_screenshot;
}

/**
 * gs_app_get_icons:
 * @app: a #GsApp
 *
 * Gets the icons for the application.
 *
 * This will never return an empty array; it will always return either %NULL or
 * a non-empty array.
 *
 * Returns: (transfer none) (element-type GIcon) (nullable): an array of icons,
 *     or %NULL if there are no icons
 *
 * Since: 3.22
 *
 * Deprecated: 45: Use gs_app_dup_icons() or gs_app_has_icons() instead.
 **/
GPtrArray *
gs_app_get_icons (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->icons == NULL || priv->icons->len == 0)
		return NULL;

	return priv->icons;
}

/**
 * gs_app_dup_icons:
 * @app: a #GsApp
 *
 * Gets the icons for the application in a thread safe way.
 *
 * This will never return an empty array; it will always return either %NULL or
 * a non-empty array.
 *
 * Returns: (transfer container) (element-type GIcon) (nullable): an array of icons,
 *     or %NULL if there are no icons
 *
 * Since: 45
 **/
GPtrArray *
gs_app_dup_icons (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	GPtrArray *copy;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->icons == NULL || priv->icons->len == 0)
		return NULL;

	copy = g_ptr_array_new_full (priv->icons->len, g_object_unref);
	for (guint i = 0; i < priv->icons->len; i++) {
		g_ptr_array_add (copy, g_object_ref (g_ptr_array_index (priv->icons, i)));
	}

	return copy;
}

/**
 * gs_app_has_icons:
 * @app: a #GsApp
 *
 * Checks whether there are any icons set.
 *
 * Returns: %TRUE, when the @app has set any icons, %FALSE otherwise
 *
 * Since: 45
 **/
gboolean
gs_app_has_icons (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	locker = g_mutex_locker_new (&priv->mutex);

	return priv->icons != NULL && priv->icons->len > 0;
}

static gint
icon_sort_width_cb (gconstpointer a,
                    gconstpointer b)
{
	GIcon *icon_a = *((GIcon **) a);
	GIcon *icon_b = *((GIcon **) b);
	guint width_a = gs_icon_get_width (icon_a);
	guint width_b = gs_icon_get_width (icon_b);

	/* Sort unknown widths (0 value) to the end. */
	if (width_a == 0 && width_b == 0)
		return 0;
	else if (width_a == 0)
		return 1;
	else if (width_b == 0)
		return -1;
	else
		return width_a - width_b;
}

/**
 * gs_app_add_icon:
 * @app: a #GsApp
 * @icon: a #GIcon
 *
 * Adds an icon to use for the application.
 * If the first icon added cannot be loaded then the next one is tried.
 *
 * Since: 40
 **/
void
gs_app_add_icon (GsApp *app, GIcon *icon)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (G_IS_ICON (icon));

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->icons == NULL)
		priv->icons = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);

	g_ptr_array_add (priv->icons, g_object_ref (icon));

	/* Ensure the array is sorted by increasing width. */
	g_ptr_array_sort (priv->icons, icon_sort_width_cb);
}

/**
 * gs_app_remove_all_icons:
 * @app: a #GsApp
 *
 * Remove all icons from @app.
 *
 * Since: 40
 */
void
gs_app_remove_all_icons (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->icons != NULL)
		g_ptr_array_set_size (priv->icons, 0);
}

/**
 * gs_app_get_agreement:
 * @app: a #GsApp
 *
 * Gets the agreement text for the application.
 *
 * Returns: a string in AppStream description format, or %NULL for unset
 *
 * Since: 3.28
 **/
const gchar *
gs_app_get_agreement (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->agreement;
}

/**
 * gs_app_set_agreement:
 * @app: a #GsApp
 * @agreement: The agreement text, e.g. "<p>Foobar</p>"
 *
 * Sets the application end-user agreement (e.g. a EULA) in AppStream
 * description format.
 *
 * Since: 3.28
 **/
void
gs_app_set_agreement (GsApp *app, const gchar *agreement)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_str (&priv->agreement, agreement);
}

/**
 * gs_app_get_local_file:
 * @app: a #GsApp
 *
 * Gets the file that backs this application, for instance this might
 * be a local file in ~/Downloads that we are installing.
 *
 * Returns: (transfer none): a #GFile, or %NULL
 *
 * Since: 3.22
 **/
GFile *
gs_app_get_local_file (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->local_file;
}

/**
 * gs_app_set_local_file:
 * @app: a #GsApp
 * @local_file: a #GFile, or %NULL
 *
 * Sets the file that backs this application, for instance this might
 * be a local file in ~/Downloads that we are installing.
 *
 * Since: 3.22
 **/
void
gs_app_set_local_file (GsApp *app, GFile *local_file)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	g_set_object (&priv->local_file, local_file);
}

/**
 * gs_app_dup_content_rating:
 * @app: a #GsApp
 *
 * Gets the content rating for this application.
 *
 * Returns: (transfer full) (nullable): a #AsContentRating, or %NULL
 *
 * Since: 41
 **/
AsContentRating *
gs_app_dup_content_rating (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return (priv->content_rating != NULL) ? g_object_ref (priv->content_rating) : NULL;
}

/**
 * gs_app_set_content_rating:
 * @app: a #GsApp
 * @content_rating: a #AsContentRating, or %NULL
 *
 * Sets the content rating for this application.
 *
 * Since: 40
 **/
void
gs_app_set_content_rating (GsApp *app, AsContentRating *content_rating)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (g_set_object (&priv->content_rating, content_rating))
		gs_app_queue_notify (app, obj_props[PROP_CONTENT_RATING]);
}

/**
 * gs_app_get_runtime:
 * @app: a #GsApp
 *
 * Gets the runtime for the installed application.
 *
 * Returns: (transfer none): a #GsApp, or %NULL for unset
 *
 * Since: 3.22
 **/
GsApp *
gs_app_get_runtime (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->runtime;
}

/**
 * gs_app_set_runtime:
 * @app: a #GsApp
 * @runtime: a #GsApp
 *
 * Sets the runtime that the installed application requires.
 *
 * Since: 3.22
 **/
void
gs_app_set_runtime (GsApp *app, GsApp *runtime)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP (runtime));
	g_return_if_fail (app != runtime);
	locker = g_mutex_locker_new (&priv->mutex);
	g_set_object (&priv->runtime, runtime);

	/* The runtime adds to the main app’s sizes. */
	gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE]);
	gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES]);
}

/**
 * gs_app_set_action_screenshot:
 * @app: a #GsApp
 * @action_screenshot: (transfer none) (nullable): a #AsScreenshot, or %NULL
 *
 * Sets a screenshot used to represent the action.
 *
 * Since: 40
 **/
void
gs_app_set_action_screenshot (GsApp *app, AsScreenshot *action_screenshot)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	g_set_object (&priv->action_screenshot, action_screenshot);
}

typedef enum {
	GS_APP_VERSION_FIXUP_RELEASE		= 1,
	GS_APP_VERSION_FIXUP_DISTRO_SUFFIX	= 2,
	GS_APP_VERSION_FIXUP_GIT_SUFFIX		= 4,
	GS_APP_VERSION_FIXUP_LAST,
} GsAppVersionFixup;

/**
 * gs_app_get_ui_version:
 *
 * convert 1:1.6.2-7.fc17 into "Version 1.6.2"
 **/
static gchar *
gs_app_get_ui_version (const gchar *version, guint64 flags)
{
	guint i;
	gchar *new;
	gchar *f;

	/* nothing set */
	if (version == NULL)
		return NULL;

	/* first remove any epoch */
	for (i = 0; version[i] != '\0'; i++) {
		if (version[i] == ':') {
			version = &version[i+1];
			break;
		}
		if (!g_ascii_isdigit (version[i]))
			break;
	}

	/* then remove any distro suffix */
	new = g_strdup (version);
	if ((flags & GS_APP_VERSION_FIXUP_DISTRO_SUFFIX) > 0) {
		f = g_strstr_len (new, -1, ".fc");
		if (f != NULL)
			*f= '\0';
		f = g_strstr_len (new, -1, ".el");
		if (f != NULL)
			*f= '\0';
	}

	/* then remove any release */
	if ((flags & GS_APP_VERSION_FIXUP_RELEASE) > 0) {
		f = g_strrstr_len (new, -1, "-");
		if (f != NULL)
			*f= '\0';
	}

	/* then remove any git suffix */
	if ((flags & GS_APP_VERSION_FIXUP_GIT_SUFFIX) > 0) {
		f = g_strrstr_len (new, -1, ".2012");
		if (f != NULL)
			*f= '\0';
		f = g_strrstr_len (new, -1, ".2013");
		if (f != NULL)
			*f= '\0';
	}

	return new;
}

static void
gs_app_ui_versions_invalidate (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_free (priv->version_ui);
	g_free (priv->update_version_ui);
	priv->version_ui = NULL;
	priv->update_version_ui = NULL;
}

static void
gs_app_ui_versions_populate (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	guint i;
	guint64 flags[] = { GS_APP_VERSION_FIXUP_RELEASE |
			    GS_APP_VERSION_FIXUP_DISTRO_SUFFIX |
			    GS_APP_VERSION_FIXUP_GIT_SUFFIX,
			    GS_APP_VERSION_FIXUP_DISTRO_SUFFIX |
			    GS_APP_VERSION_FIXUP_GIT_SUFFIX,
			    GS_APP_VERSION_FIXUP_DISTRO_SUFFIX,
			    0 };

	/* try each set of bitfields in order */
	for (i = 0; flags[i] != 0; i++) {
		priv->version_ui = gs_app_get_ui_version (priv->version, flags[i]);
		priv->update_version_ui = gs_app_get_ui_version (priv->update_version, flags[i]);
		if (g_strcmp0 (priv->version_ui, priv->update_version_ui) != 0) {
			gs_app_queue_notify (app, obj_props[PROP_VERSION]);
			return;
		}
		gs_app_ui_versions_invalidate (app);
	}

	/* we tried, but failed */
	priv->version_ui = g_strdup (priv->version);
	priv->update_version_ui = g_strdup (priv->update_version);
}

/**
 * gs_app_get_version:
 * @app: a #GsApp
 *
 * Gets the exact version for the application.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_version (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->version;
}

/**
 * gs_app_get_version_ui:
 * @app: a #GsApp
 *
 * Gets a version string that can be displayed in a UI.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_version_ui (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	/* work out the two version numbers */
	if (priv->version != NULL &&
	    priv->version_ui == NULL) {
		gs_app_ui_versions_populate (app);
	}

	return priv->version_ui;
}

/**
 * gs_app_set_version:
 * @app: a #GsApp
 * @version: The version, e.g. "2:1.2.3.fc19"
 *
 * This saves the version after stripping out any non-friendly parts, such as
 * distro tags, git revisions and that kind of thing.
 *
 * Since: 3.22
 **/
void
gs_app_set_version (GsApp *app, const gchar *version)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (_g_set_str (&priv->version, version)) {
		gs_app_ui_versions_invalidate (app);
		gs_app_queue_notify (app, obj_props[PROP_VERSION]);
	}
}

/**
 * gs_app_get_summary:
 * @app: a #GsApp
 *
 * Gets the single-line description of the application.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_summary (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->summary;
}

/**
 * gs_app_set_summary:
 * @app: a #GsApp
 * @quality: a #GsAppQuality, e.g. %GS_APP_QUALITY_LOWEST
 * @summary: a string, e.g. "A graphical calculator for LINGMO"
 *
 * The medium length one-line localized name.
 *
 * Since: 3.22
 **/
void
gs_app_set_summary (GsApp *app, GsAppQuality quality, const gchar *summary)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* only save this if the data is sufficiently high quality */
	if (quality < priv->summary_quality)
		return;
	priv->summary_quality = quality;
	if (_g_set_str (&priv->summary, summary))
		gs_app_queue_notify (app, obj_props[PROP_SUMMARY]);
}

/**
 * gs_app_get_description:
 * @app: a #GsApp
 *
 * Gets the long multi-line description of the application.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_description (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->description;
}

/**
 * gs_app_set_description:
 * @app: a #GsApp
 * @quality: a #GsAppQuality, e.g. %GS_APP_QUALITY_LOWEST
 * @description: a string, e.g. "LINGMO Calculator is a graphical calculator for LINGMO..."
 *
 * Sets the long multi-line description of the application.
 *
 * Since: 3.22
 **/
void
gs_app_set_description (GsApp *app, GsAppQuality quality, const gchar *description)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* only save this if the data is sufficiently high quality */
	if (quality < priv->description_quality)
		return;
	priv->description_quality = quality;
	_g_set_str (&priv->description, description);
}

/**
 * gs_app_get_url:
 * @app: a #GsApp
 * @kind: a #AsUrlKind, e.g. %AS_URL_KIND_HOMEPAGE
 *
 * Gets a web address of a specific type.
 *
 * Returns: (nullable): a string, or %NULL for unset
 *
 * Since: 40
 **/
const gchar *
gs_app_get_url (GsApp *app, AsUrlKind kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->urls == NULL)
		return NULL;
	return g_hash_table_lookup (priv->urls, GINT_TO_POINTER (kind));
}

/**
 * gs_app_set_url:
 * @app: a #GsApp
 * @kind: a #AsUrlKind, e.g. %AS_URL_KIND_HOMEPAGE
 * @url: (nullable): a web URL, e.g. "http://www.hughsie.com/", or %NULL to
 *   unset the URL of this @kind
 *
 * Sets a web address of a specific type.
 *
 * Since: 40
 **/
void
gs_app_set_url (GsApp *app, AsUrlKind kind, const gchar *url)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	gboolean changed;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->urls == NULL)
		priv->urls = g_hash_table_new_full (g_direct_hash, g_direct_equal,
						    NULL, g_free);

	if (url != NULL)
		changed = g_hash_table_insert (priv->urls,
					       GINT_TO_POINTER (kind),
					       g_strdup (url));
	else
		changed = g_hash_table_remove (priv->urls,
					       GINT_TO_POINTER (kind));

	if (changed)
		gs_app_queue_notify (app, obj_props[PROP_URLS]);
}

/**
 * gs_app_get_url_missing:
 * @app: a #GsApp
 *
 * Gets a web address for the application with explanations
 * why it does not have an installation candidate.
 *
 * Returns: (nullable): a string, or %NULL for unset
 *
 * Since: 40
 **/
const gchar *
gs_app_get_url_missing (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return priv->url_missing;
}

/**
 * gs_app_set_url_missing:
 * @app: a #GsApp
 * @url: (nullable): a web URL, e.g. `http://www.packagekit.org/pk-package-not-found.html`, or %NULL
 *
 * Sets a web address containing explanations why this app
 * does not have an installation candidate.
 *
 * Since: 40
 **/
void
gs_app_set_url_missing (GsApp *app, const gchar *url)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);

	if (g_strcmp0 (priv->url_missing, url) == 0)
		return;
	g_free (priv->url_missing);
	priv->url_missing = g_strdup (url);
	gs_app_queue_notify (app, obj_props[PROP_URL_MISSING]);
}

/**
 * gs_app_get_launchable:
 * @app: a #GsApp
 * @kind: a #AsLaunchableKind, e.g. %AS_LAUNCHABLE_KIND_DESKTOP_ID
 *
 * Gets a launchable of a specific type.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 40
 **/
const gchar *
gs_app_get_launchable (GsApp *app, AsLaunchableKind kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return g_hash_table_lookup (priv->launchables,
				    as_launchable_kind_to_string (kind));
}

/**
 * gs_app_set_launchable:
 * @app: a #GsApp
 * @kind: a #AsLaunchableKind, e.g. %AS_LAUNCHABLE_KIND_DESKTOP_ID
 * @launchable: a way to launch, e.g. "org.lingmo.Sysprof2.desktop"
 *
 * Sets a launchable of a specific type.
 *
 * Since: 40
 **/
void
gs_app_set_launchable (GsApp *app, AsLaunchableKind kind, const gchar *launchable)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	gpointer current_value = NULL;
	const gchar *key;
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	key = as_launchable_kind_to_string (kind);
	if (g_hash_table_lookup_extended (priv->launchables, key, NULL, &current_value)) {
		if (g_strcmp0 ((const gchar *) current_value, launchable) != 0)
			g_debug ("Preventing app '%s' replace of %s's launchable '%s' with '%s'",
				 priv->name, key, (const gchar *) current_value, launchable);
	} else {
		g_hash_table_insert (priv->launchables,
				     (gpointer) as_launchable_kind_to_string (kind),
				     g_strdup (launchable));
	}
}

/**
 * gs_app_get_license:
 * @app: a #GsApp
 *
 * Gets the project license of the application.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_license (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->license;
}

/**
 * gs_app_get_license_is_free:
 * @app: a #GsApp
 *
 * Returns if the application is free software.
 *
 * Returns: %TRUE if the application is free software
 *
 * Since: 3.22
 **/
gboolean
gs_app_get_license_is_free (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return priv->license_is_free;
}

/**
 * gs_app_set_license:
 * @app: a #GsApp
 * @quality: a #GsAppQuality, e.g. %GS_APP_QUALITY_NORMAL
 * @license: a SPDX license string, e.g. "GPL-3.0 AND LGPL-2.0-or-later"
 *
 * Sets the project licenses used in the application.
 *
 * Since: 3.22
 **/
void
gs_app_set_license (GsApp *app, GsAppQuality quality, const gchar *license)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* only save this if the data is sufficiently high quality */
	if (quality <= priv->license_quality)
		return;
	if (license == NULL || *license == '\0')
		return;
	priv->license_quality = quality;

	priv->license_is_free = as_license_is_free_license (license);

	if (_g_set_str (&priv->license, license))
		gs_app_queue_notify (app, obj_props[PROP_LICENSE]);
}

/**
 * gs_app_get_summary_missing:
 * @app: a #GsApp
 *
 * Gets the one-line summary to use when this application is missing.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_summary_missing (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->summary_missing;
}

/**
 * gs_app_set_summary_missing:
 * @app: a #GsApp
 * @summary_missing: a string, or %NULL
 *
 * Sets the one-line summary to use when this application is missing.
 *
 * Since: 3.22
 **/
void
gs_app_set_summary_missing (GsApp *app, const gchar *summary_missing)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_str (&priv->summary_missing, summary_missing);
}

static gboolean
_gs_app_has_desktop_group (GsApp *app, const gchar *desktop_group)
{
	guint i;
	g_auto(GStrv) split = g_strsplit (desktop_group, "::", -1);
	for (i = 0; split[i] != NULL; i++) {
		if (!gs_app_has_category (app, split[i]))
			return FALSE;
	}
	return TRUE;
}

/**
 * gs_app_get_menu_path:
 * @app: a #GsApp
 *
 * Returns the menu path which is an array of path elements.
 * The resulting array is an internal structure and must not be
 * modified or freed.
 *
 * Returns: a %NULL-terminated array of strings
 *
 * Since: 3.22
 **/
gchar **
gs_app_get_menu_path (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	/* Lazy load. */
	if (priv->menu_path == NULL) {
		const gchar *strv[] = { "", NULL, NULL };
		const GsDesktopData *msdata;
		gboolean found = FALSE;

		/* find a top level category the app has */
		msdata = gs_desktop_get_data ();
		for (gsize i = 0; !found && msdata[i].id != NULL; i++) {
			const GsDesktopData *data = &msdata[i];
			for (gsize j = 0; !found && data->mapping[j].id != NULL; j++) {
				const GsDesktopMap *map = &data->mapping[j];
				g_autofree gchar *msgctxt = NULL;

				if (g_strcmp0 (map->id, "all") == 0)
					continue;
				if (g_strcmp0 (map->id, "featured") == 0)
					continue;
				msgctxt = g_strdup_printf ("Menu of %s", data->name);
				for (gsize k = 0; !found && map->fdo_cats[k] != NULL; k++) {
					const gchar *tmp = msdata[i].mapping[j].fdo_cats[k];
					if (_gs_app_has_desktop_group (app, tmp)) {
						strv[0] = g_dgettext (GETTEXT_PACKAGE, msdata[i].name);
						strv[1] = g_dpgettext2 (GETTEXT_PACKAGE, msgctxt,
							                msdata[i].mapping[j].name);
						found = TRUE;
						break;
					}
				}
			}
		}

		/* always set something to avoid keep searching for this */
		gs_app_set_menu_path (app, (gchar **) strv);
	}

	return priv->menu_path;
}

/**
 * gs_app_set_menu_path:
 * @app: a #GsApp
 * @menu_path: a %NULL-terminated array of strings
 *
 * Sets the new menu path. The menu path is an array of path elements.
 * This function creates a deep copy of the path.
 *
 * Since: 3.22
 **/
void
gs_app_set_menu_path (GsApp *app, gchar **menu_path)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_strv (&priv->menu_path, menu_path);
}

/**
 * gs_app_get_origin:
 * @app: a #GsApp
 *
 * Gets the origin for the application, e.g. "fedora".
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_origin (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->origin;
}

/**
 * gs_app_set_origin:
 * @app: a #GsApp
 * @origin: a string, or %NULL
 *
 * The origin is the original source of the application e.g. "fedora-updates"
 *
 * Since: 3.22
 **/
void
gs_app_set_origin (GsApp *app, const gchar *origin)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* same */
	if (g_strcmp0 (origin, priv->origin) == 0)
		return;

	/* trying to change */
	if (priv->origin != NULL && origin != NULL) {
		g_warning ("automatically prevented from changing "
			   "origin on %s from %s to %s!",
			   gs_app_get_unique_id_unlocked (app),
			   priv->origin, origin);
		return;
	}

	g_free (priv->origin);
	priv->origin = g_strdup (origin);

	/* no longer valid */
	priv->unique_id_valid = FALSE;
}

/**
 * gs_app_get_origin_appstream:
 * @app: a #GsApp
 *
 * Gets the appstream origin for the application, e.g. "fedora".
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.28
 **/
const gchar *
gs_app_get_origin_appstream (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->origin_appstream;
}

/**
 * gs_app_set_origin_appstream:
 * @app: a #GsApp
 * @origin_appstream: a string, or %NULL
 *
 * The appstream origin is the appstream source of the application e.g. "fedora"
 *
 * Since: 3.28
 **/
void
gs_app_set_origin_appstream (GsApp *app, const gchar *origin_appstream)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* same */
	if (g_strcmp0 (origin_appstream, priv->origin_appstream) == 0)
		return;

	g_free (priv->origin_appstream);
	priv->origin_appstream = g_strdup (origin_appstream);
}

/**
 * gs_app_get_origin_hostname:
 * @app: a #GsApp
 *
 * Gets the hostname of the origin used to install the application, e.g.
 * "fedoraproject.org" or "sdk.lingmo.org".
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_origin_hostname (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->origin_hostname;
}

/**
 * gs_app_set_origin_hostname:
 * @app: a #GsApp
 * @origin_hostname: a string, or %NULL
 *
 * The origin is the hostname of the source used to install the application
 * e.g. "fedoraproject.org"
 *
 * You can also use a full URL as @origin_hostname and this will be parsed and
 * the hostname extracted. This process will also remove any unnecessary DNS
 * prefixes like "download" or "mirrors".
 *
 * Since: 3.22
 **/
void
gs_app_set_origin_hostname (GsApp *app, const gchar *origin_hostname)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GUri) uri = NULL;
	guint i;
	const gchar *prefixes[] = { "download.", "mirrors.", NULL };

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* same */
	if (g_strcmp0 (origin_hostname, priv->origin_hostname) == 0)
		return;
	g_free (priv->origin_hostname);

	/* convert a URL */
	uri = g_uri_parse (origin_hostname, SOUP_HTTP_URI_FLAGS, NULL);
	if (uri != NULL)
		origin_hostname = g_uri_get_host (uri);

	/* remove some common prefixes */
	for (i = 0; prefixes[i] != NULL; i++) {
		if (g_str_has_prefix (origin_hostname, prefixes[i]))
			origin_hostname += strlen (prefixes[i]);
	}

	/* fallback for localhost */
	if (g_strcmp0 (origin_hostname, "") == 0)
		origin_hostname = "localhost";

	/* success */
	priv->origin_hostname = g_strdup (origin_hostname);
}

/**
 * gs_app_add_screenshot:
 * @app: a #GsApp
 * @screenshot: a #AsScreenshot
 *
 * Adds a screenshot to the application.
 *
 * Since: 40
 **/
void
gs_app_add_screenshot (GsApp *app, AsScreenshot *screenshot)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (AS_IS_SCREENSHOT (screenshot));

	locker = g_mutex_locker_new (&priv->mutex);
	g_ptr_array_add (priv->screenshots, g_object_ref (screenshot));
}

/**
 * gs_app_get_screenshots:
 * @app: a #GsApp
 *
 * Gets the list of screenshots.
 *
 * Returns: (element-type AsScreenshot) (transfer none): a list
 *
 * Since: 3.22
 **/
GPtrArray *
gs_app_get_screenshots (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->screenshots;
}

/**
 * gs_app_get_update_version:
 * @app: a #GsApp
 *
 * Gets the newest update version.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_update_version (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->update_version;
}

/**
 * gs_app_get_update_version_ui:
 * @app: a #GsApp
 *
 * Gets the update version for the UI.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_update_version_ui (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	/* work out the two version numbers */
	if (priv->update_version != NULL &&
	    priv->update_version_ui == NULL) {
		gs_app_ui_versions_populate (app);
	}

	return priv->update_version_ui;
}

static void
gs_app_set_update_version_internal (GsApp *app, const gchar *update_version)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	if (_g_set_str (&priv->update_version, update_version))
		gs_app_ui_versions_invalidate (app);
}

/**
 * gs_app_set_update_version:
 * @app: a #GsApp
 * @update_version: a string, e.g. "0.1.2.3"
 *
 * Sets the new version number of the update.
 *
 * Since: 3.22
 **/
void
gs_app_set_update_version (GsApp *app, const gchar *update_version)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	gs_app_set_update_version_internal (app, update_version);
	gs_app_queue_notify (app, obj_props[PROP_VERSION]);
}

/**
 * gs_app_get_update_details_markup:
 * @app: a #GsApp
 *
 * Gets the multi-line description for the update as a Pango markup.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 42.0
 **/
const gchar *
gs_app_get_update_details_markup (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->update_details_markup;
}

/**
 * gs_app_set_update_details_markup:
 * @app: a #GsApp
 * @markup: a Pango markup
 *
 * Sets the multi-line description for the update as markup.
 *
 * See: gs_app_set_update_details_text()
 *
 * Since: 42.0
 **/
void
gs_app_set_update_details_markup (GsApp *app,
				  const gchar *markup)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	priv->update_details_set = TRUE;
	_g_set_str (&priv->update_details_markup, markup);
}

/**
 * gs_app_set_update_details_text:
 * @app: a #GsApp
 * @text: a text without Pango markup
 *
 * Sets the multi-line description for the update as text,
 * escaping the @text to be safe for a Pango markup.
 *
 * See: gs_app_set_update_details_markup()
 *
 * Since: 42.0
 **/
void
gs_app_set_update_details_text (GsApp *app,
				const gchar *text)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	priv->update_details_set = TRUE;
	if (text == NULL) {
		_g_set_str (&priv->update_details_markup, NULL);
	} else {
		gchar *markup = g_markup_escape_text (text, -1);
		g_free (priv->update_details_markup);
		priv->update_details_markup = markup;
	}
}

/**
 * gs_app_get_update_details_set:
 * @app: a #GsApp
 *
 * Returns whether update details for the @app had been set. It does
 * not matter whether it was set to %NULL or an actual text.
 *
 * Returns: whether update details for the @app had been set
 *
 * Since: 45
 **/
gboolean
gs_app_get_update_details_set (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	locker = g_mutex_locker_new (&priv->mutex);
	return priv->update_details_set;
}

/**
 * gs_app_get_update_urgency:
 * @app: a #GsApp
 *
 * Gets the update urgency.
 *
 * Returns: a #AsUrgencyKind, or %AS_URGENCY_KIND_UNKNOWN for unset
 *
 * Since: 40
 **/
AsUrgencyKind
gs_app_get_update_urgency (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), AS_URGENCY_KIND_UNKNOWN);
	return priv->update_urgency;
}

/**
 * gs_app_set_update_urgency:
 * @app: a #GsApp
 * @update_urgency: a #AsUrgencyKind
 *
 * Sets the update urgency.
 *
 * Since: 40
 **/
void
gs_app_set_update_urgency (GsApp *app, AsUrgencyKind update_urgency)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	if (update_urgency == priv->update_urgency)
		return;
	priv->update_urgency = update_urgency;
}

/**
 * gs_app_dup_management_plugin:
 * @app: a #GsApp
 *
 * Gets the management plugin.
 *
 * This is some metadata about the application which gives which plugin should
 * handle the install, remove or upgrade actions.
 *
 * Returns: (nullable) (transfer full): the management plugin, or %NULL for unset
 *
 * Since: 42
 **/
GsPlugin *
gs_app_dup_management_plugin (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return g_weak_ref_get (&priv->management_plugin_weak);
}

/**
 * gs_app_has_management_plugin:
 * @app: a #GsApp
 * @plugin: (nullable) (transfer none): a #GsPlugin to check against, or %NULL
 *
 * Check whether the management plugin for @app is set to @plugin.
 *
 * If @plugin is %NULL, %TRUE is returned only if the @app has no management
 * plugin set.
 *
 * Returns: %TRUE if @plugin is the management plugin for @app, %FALSE otherwise
 * Since: 42
 */
gboolean
gs_app_has_management_plugin (GsApp    *app,
                              GsPlugin *plugin)
{
	g_autoptr(GsPlugin) app_plugin = gs_app_dup_management_plugin (app);
	return (app_plugin == plugin);
}

/**
 * gs_app_set_management_plugin:
 * @app: a #GsApp
 * @management_plugin: (nullable) (transfer none): a plugin, or %NULL
 *
 * The management plugin is the plugin that can handle doing install and remove
 * operations on the #GsApp.
 *
 * It is an error to attempt to change the management plugin once it has been
 * previously set or to try to use this function on a wildcard application.
 *
 * Since: 42
 **/
void
gs_app_set_management_plugin (GsApp    *app,
                              GsPlugin *management_plugin)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GsPlugin) old_plugin = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (management_plugin == NULL || GS_IS_PLUGIN (management_plugin));

	locker = g_mutex_locker_new (&priv->mutex);

	/* plugins cannot adopt wildcard packages */
	if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD)) {
		g_warning ("plugins should not set the management plugin on "
			   "%s to %s -- create a new GsApp in refine()!",
			   gs_app_get_unique_id_unlocked (app),
			   (management_plugin != NULL) ? gs_plugin_get_name (management_plugin) : "(null)");
		return;
	}

	/* same */
	old_plugin = g_weak_ref_get (&priv->management_plugin_weak);

	if (old_plugin == management_plugin)
		return;

	/* trying to change */
	if (old_plugin != NULL && management_plugin != NULL) {
		g_warning ("automatically prevented from changing "
			   "management plugin on %s from %s to %s!",
			   gs_app_get_unique_id_unlocked (app),
			   gs_plugin_get_name (old_plugin),
			   gs_plugin_get_name (management_plugin));
		return;
	}

	g_weak_ref_set (&priv->management_plugin_weak, management_plugin);
}

/**
 * gs_app_get_rating:
 * @app: a #GsApp
 *
 * Gets the percentage rating of the application, where 100 is 5 stars.
 *
 * Returns: a percentage, or -1 for unset
 *
 * Since: 3.22
 **/
gint
gs_app_get_rating (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), -1);
	return priv->rating;
}

/**
 * gs_app_set_rating:
 * @app: a #GsApp
 * @rating: a percentage, or -1 for invalid
 *
 * Gets the percentage rating of the application.
 *
 * Since: 3.22
 **/
void
gs_app_set_rating (GsApp *app, gint rating)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	if (rating == priv->rating)
		return;
	priv->rating = rating;
	gs_app_queue_notify (app, obj_props[PROP_RATING]);
}

/**
 * gs_app_get_review_ratings:
 * @app: a #GsApp
 *
 * Gets the review ratings.
 *
 * Returns: (element-type guint32) (transfer none): a list
 *
 * Since: 3.22
 **/
GArray *
gs_app_get_review_ratings (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->review_ratings;
}

/**
 * gs_app_set_review_ratings:
 * @app: a #GsApp
 * @review_ratings: (element-type guint32): a list
 *
 * Sets the review ratings.
 *
 * Since: 3.22
 **/
void
gs_app_set_review_ratings (GsApp *app, GArray *review_ratings)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_array (&priv->review_ratings, review_ratings);
}

/**
 * gs_app_get_reviews:
 * @app: a #GsApp
 *
 * Gets all the user-submitted reviews for the application.
 *
 * Returns: (element-type AsReview) (transfer none): the list of reviews
 *
 * Since: 3.22
 **/
GPtrArray *
gs_app_get_reviews (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->reviews;
}

/**
 * gs_app_add_review:
 * @app: a #GsApp
 * @review: a #AsReview
 *
 * Adds a user-submitted review to the application.
 *
 * Since: 40
 **/
void
gs_app_add_review (GsApp *app, AsReview *review)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (AS_IS_REVIEW (review));
	locker = g_mutex_locker_new (&priv->mutex);
	g_ptr_array_add (priv->reviews, g_object_ref (review));
}

/**
 * gs_app_remove_review:
 * @app: a #GsApp
 * @review: a #AsReview
 *
 * Removes a user-submitted review to the application.
 *
 * Since: 40
 **/
void
gs_app_remove_review (GsApp *app, AsReview *review)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	g_ptr_array_remove (priv->reviews, review);
}

/**
 * gs_app_get_provided:
 * @app: a #GsApp
 *
 * Gets all the provided item sets for the application.
 *
 * Returns: (element-type AsProvided) (transfer none): the list of provided items
 *
 * Since: 40
 **/
GPtrArray*
gs_app_get_provided (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->provided;
}

/**
 * gs_app_get_provided_for_kind:
 * @cpt: a #AsComponent instance.
 * @kind: kind of the provided item, e.g. %AS_PROVIDED_KIND_MIMETYPE
 *
 * Get an #AsProvided object for the given interface type, or %NULL if
 * none was found.
 *
 * Returns: (nullable) (transfer none): the #AsProvided
 *
 * Since: 40
 */
AsProvided*
gs_app_get_provided_for_kind (GsApp *app, AsProvidedKind kind)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	for (guint i = 0; i < priv->provided->len; i++) {
		AsProvided *prov = AS_PROVIDED (g_ptr_array_index (priv->provided, i));
		if (as_provided_get_kind (prov) == kind)
			return prov;
	}
	return NULL;
}

/**
 * gs_app_add_provided:
 * @app: a #GsApp
 * @kind: the kind of the provided item, e.g. %AS_PROVIDED_KIND_MEDIATYPE
 * @item: the item to add.
 *
 * Adds a provided items of the given kind to the application.
 *
 * Since: 40
 **/
void
gs_app_add_provided_item (GsApp *app, AsProvidedKind kind, const gchar *item)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	AsProvided *prov;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (item != NULL);
	g_return_if_fail (kind != AS_PROVIDED_KIND_UNKNOWN && kind < AS_PROVIDED_KIND_LAST);

	locker = g_mutex_locker_new (&priv->mutex);
	prov = gs_app_get_provided_for_kind (app, kind);
	if (prov == NULL) {
		prov = as_provided_new ();
		as_provided_set_kind (prov, kind);
		g_ptr_array_add (priv->provided, prov);
	}
	as_provided_add_item (prov, item);
}

/**
 * gs_app_get_size_download:
 * @app: A #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the download size, in bytes, or %NULL to ignore
 *
 * Get the values of #GsApp:size-download-type and #GsApp:size-download.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the download size. Otherwise, its value will be undefined.
 *
 * Returns: type of the download size
 * Since: 43
 **/
GsSizeType
gs_app_get_size_download (GsApp   *app,
                          guint64 *size_bytes_out)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	if (size_bytes_out != NULL)
		*size_bytes_out = (priv->size_download_type == GS_SIZE_TYPE_VALID) ? priv->size_download : 0;

	return priv->size_download_type;
}

/**
 * gs_app_set_size_download:
 * @app: a #GsApp
 * @size_type: type of the download size
 * @size_bytes: size in bytes
 *
 * Sets the download size of the application, not including any
 * required runtime.
 *
 * @size_bytes will be ignored unless @size_type is %GS_SIZE_TYPE_VALID.
 *
 * Since: 43
 **/
void
gs_app_set_size_download (GsApp      *app,
                          GsSizeType  size_type,
                          guint64     size_bytes)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	if (size_type != GS_SIZE_TYPE_VALID)
		size_bytes = 0;

	if (priv->size_download_type != size_type) {
		priv->size_download_type = size_type;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD_TYPE]);
	}

	if (priv->size_download != size_bytes) {
		priv->size_download = size_bytes;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD]);
	}
}

/* Add two sizes, accounting for their validity, and checking for overflow. This
 * is essentially `out_bytes = a_bytes + b_bytes` with additional checking.
 *
 * If either of @a_type or @b_type is %GS_SIZE_TYPE_UNKNOWN or
 * %GS_SIZE_TYPE_UNKNOWABLE, that type will be propagated to @out_type.
 *
 * If the sum of @a_bytes and @b_bytes exceeds %G_MAXUINT64, the result in
 * @out_bytes will silently be clamped to %G_MAXUINT64.
 *
 * The lifetime of @app must be at least as long as the lifetime of
 * @covered_uids, which allows us to avoid some string copies.
 */
static gboolean
add_sizes (GsApp      *app,
           GHashTable *covered_uids,
           GsSizeType  a_type,
           guint64     a_bytes,
           GsSizeType  b_type,
           guint64     b_bytes,
           GsSizeType *out_type,
           guint64    *out_bytes)
{
	g_return_val_if_fail (out_type != NULL, FALSE);
	g_return_val_if_fail (out_bytes != NULL, FALSE);

	if (app != NULL && covered_uids != NULL) {
		const gchar *id = gs_app_get_unique_id (app);
		if (id != NULL &&
		    !g_hash_table_add (covered_uids, (gpointer) id))
			return TRUE;
	}

	if (a_type == GS_SIZE_TYPE_VALID && b_type == GS_SIZE_TYPE_VALID) {
		*out_type = GS_SIZE_TYPE_VALID;
		if (!g_uint64_checked_add (out_bytes, a_bytes, b_bytes))
			*out_bytes = G_MAXUINT64;
		return TRUE;
	}

	*out_type = (a_type == GS_SIZE_TYPE_UNKNOWABLE || b_type == GS_SIZE_TYPE_UNKNOWABLE) ? GS_SIZE_TYPE_UNKNOWABLE : GS_SIZE_TYPE_UNKNOWN;
	*out_bytes = 0;

	return FALSE;
}

static GsSizeType
get_size_download_dependencies (GsApp *app,
				guint64 *size_bytes_out,
				GHashTable *covered_uids)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	GsSizeType size_type = GS_SIZE_TYPE_VALID;
	guint64 size_bytes = 0;

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	/* add the runtime if this is not installed */
	if (priv->runtime != NULL &&
	    gs_app_get_state (priv->runtime) == GS_APP_STATE_AVAILABLE) {
		GsSizeType runtime_size_download_type, runtime_size_download_dependencies_type;
		guint64 runtime_size_download_bytes, runtime_size_download_dependencies_bytes;

		runtime_size_download_type = gs_app_get_size_download (priv->runtime, &runtime_size_download_bytes);

		if (add_sizes (priv->runtime, covered_uids,
			       size_type, size_bytes,
			       runtime_size_download_type, runtime_size_download_bytes,
			       &size_type, &size_bytes)) {
			runtime_size_download_dependencies_type = get_size_download_dependencies (priv->runtime,
												  &runtime_size_download_dependencies_bytes,
												  covered_uids);

			add_sizes (NULL, NULL,
				   size_type, size_bytes,
				   runtime_size_download_dependencies_type, runtime_size_download_dependencies_bytes,
				   &size_type, &size_bytes);
		}
	}

	/* add related apps */
	for (guint i = 0; i < gs_app_list_length (priv->related); i++) {
		GsApp *app_related = gs_app_list_index (priv->related, i);
		GsSizeType related_size_download_type, related_size_download_dependencies_type;
		guint64 related_size_download_bytes, related_size_download_dependencies_bytes;

		related_size_download_type = gs_app_get_size_download (app_related, &related_size_download_bytes);

		if (!add_sizes (app_related, covered_uids,
				size_type, size_bytes,
				related_size_download_type, related_size_download_bytes,
				&size_type, &size_bytes))
			break;

		related_size_download_dependencies_type = get_size_download_dependencies (app_related,
											  &related_size_download_dependencies_bytes,
											  covered_uids);

		if (!add_sizes (NULL, NULL,
				size_type, size_bytes,
				related_size_download_dependencies_type, related_size_download_dependencies_bytes,
				&size_type, &size_bytes))
			break;
	}

	if (size_bytes_out != NULL)
		*size_bytes_out = (size_type == GS_SIZE_TYPE_VALID) ? size_bytes : 0;

	return size_type;
}

/**
 * gs_app_get_size_download_dependencies:
 * @app: A #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the download size of dependencies, in bytes, or %NULL to ignore
 *
 * Get the value of #GsApp:size-download-dependencies-type and
 * #GsApp:size-download-dependencies.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the download size of dependencies. Otherwise, its value will be
 * undefined.
 *
 * Returns: type of the download size of dependencies
 * Since: 43
 **/
GsSizeType
gs_app_get_size_download_dependencies (GsApp   *app,
                                       guint64 *size_bytes_out)
{
	g_autoptr(GHashTable) covered_uids = NULL;

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	covered_uids = g_hash_table_new_full ((GHashFunc) as_utils_data_id_hash, (GEqualFunc) as_utils_data_id_equal, NULL, NULL);

	return get_size_download_dependencies (app, size_bytes_out, covered_uids);
}

/**
 * gs_app_get_size_installed:
 * @app: a #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the installed size, in bytes, or %NULL to ignore
 *
 * Get the values of #GsApp:size-installed-type and #GsApp:size-installed.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the installed size. Otherwise, its value will be undefined.
 *
 * Returns: type of the installed size
 * Since: 43
 **/
GsSizeType
gs_app_get_size_installed (GsApp   *app,
                           guint64 *size_bytes_out)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	if (size_bytes_out != NULL)
		*size_bytes_out = (priv->size_installed_type == GS_SIZE_TYPE_VALID) ? priv->size_installed : 0;

	return priv->size_installed_type;
}

/**
 * gs_app_set_size_installed:
 * @app: a #GsApp
 * @size_type: type of the installed size
 * @size_bytes: size in bytes
 *
 * Sets the installed size of the application.
 *
 * @size_bytes will be ignored unless @size_type is %GS_SIZE_TYPE_VALID.
 *
 * Since: 43
 **/
void
gs_app_set_size_installed (GsApp      *app,
                           GsSizeType  size_type,
                           guint64     size_bytes)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	if (size_type != GS_SIZE_TYPE_VALID)
		size_bytes = 0;

	if (priv->size_installed_type != size_type) {
		priv->size_installed_type = size_type;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_INSTALLED_TYPE]);
	}

	if (priv->size_installed != size_bytes) {
		priv->size_installed = size_bytes;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_INSTALLED]);
	}
}

static GsSizeType
get_size_installed_dependencies (GsApp *app,
				 guint64 *size_bytes_out,
				 GHashTable *covered_uids)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	GsSizeType size_type = GS_SIZE_TYPE_VALID;
	guint64 size_bytes = 0;

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	/* add related apps */
	for (guint i = 0; i < gs_app_list_length (priv->related); i++) {
		GsApp *app_related = gs_app_list_index (priv->related, i);
		GsSizeType related_size_installed_type, related_size_installed_dependencies_type;
		guint64 related_size_installed_bytes, related_size_installed_dependencies_bytes;

		related_size_installed_type = gs_app_get_size_installed (app_related, &related_size_installed_bytes);

		if (!add_sizes (app_related, covered_uids,
				size_type, size_bytes,
				related_size_installed_type, related_size_installed_bytes,
				&size_type, &size_bytes))
			break;

		related_size_installed_dependencies_type = get_size_installed_dependencies (app_related,
											    &related_size_installed_dependencies_bytes,
											    covered_uids);

		if (!add_sizes (NULL, NULL,
				size_type, size_bytes,
				related_size_installed_dependencies_type, related_size_installed_dependencies_bytes,
				&size_type, &size_bytes))
			break;
	}

	if (size_bytes_out != NULL)
		*size_bytes_out = (size_type == GS_SIZE_TYPE_VALID) ? size_bytes : 0;

	return size_type;
}

/**
 * gs_app_get_size_installed_dependencies:
 * @app: a #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the installed size of dependencies, in bytes, or %NULL to ignore
 *
 * Get the values of #GsApp:size-installed-dependencies-type and
 * #GsApp:size-installed-dependencies.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the installed size of dependencies. Otherwise, its value will be
 * undefined.
 *
 * Returns: type of the installed size of dependencies
 * Since: 43
 **/
GsSizeType
gs_app_get_size_installed_dependencies (GsApp   *app,
                                        guint64 *size_bytes_out)
{
	g_autoptr(GHashTable) covered_uids = NULL;

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	covered_uids = g_hash_table_new_full ((GHashFunc) as_utils_data_id_hash, (GEqualFunc) as_utils_data_id_equal, NULL, NULL);

	return get_size_installed_dependencies (app, size_bytes_out, covered_uids);
}

/**
 * gs_app_get_size_user_data:
 * @app: A #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the user data size, in bytes, or %NULL to ignore
 *
 * Get the values of #GsApp:size-user-data-type and #GsApp:size-user-data.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the user data size. Otherwise, its value will be undefined.
 *
 * Returns: type of the user data size
 * Since: 43
 **/
GsSizeType
gs_app_get_size_user_data (GsApp   *app,
                           guint64 *size_bytes_out)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	if (size_bytes_out != NULL)
		*size_bytes_out = (priv->size_user_data_type == GS_SIZE_TYPE_VALID) ? priv->size_user_data : 0;

	return priv->size_user_data_type;
}

/**
 * gs_app_set_size_user_data:
 * @app: a #GsApp
 * @size_type: type of the user data size
 * @size_bytes: size in bytes
 *
 * Sets the user data size of the @app.
 *
 * @size_bytes will be ignored unless @size_type is %GS_SIZE_TYPE_VALID.
 *
 * Since: 43
 **/
void
gs_app_set_size_user_data (GsApp      *app,
                           GsSizeType  size_type,
                           guint64     size_bytes)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	if (size_type != GS_SIZE_TYPE_VALID)
		size_bytes = 0;

	if (priv->size_user_data_type != size_type) {
		priv->size_user_data_type = size_type;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_USER_DATA_TYPE]);
	}

	if (priv->size_user_data != size_bytes) {
		priv->size_user_data = size_bytes;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_USER_DATA]);
	}
}

/**
 * gs_app_get_size_cache_data:
 * @app: A #GsApp
 * @size_bytes_out: (optional) (out caller-allocates): return location for
 *   the cache data size, in bytes, or %NULL to ignore
 *
 * Get the values of #GsApp:size-cache-data-type and #GsApp:size-cache-data.
 *
 * If this returns %GS_SIZE_TYPE_VALID, @size_bytes_out (if non-%NULL) will be
 * set to the cache data size. Otherwise, its value will be undefined.
 *
 * Returns: type of the cache data size
 * Since: 43
 **/
GsSizeType
gs_app_get_size_cache_data (GsApp   *app,
                            guint64 *size_bytes_out)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), GS_SIZE_TYPE_UNKNOWN);

	if (size_bytes_out != NULL)
		*size_bytes_out = (priv->size_cache_data_type == GS_SIZE_TYPE_VALID) ? priv->size_cache_data : 0;

	return priv->size_cache_data_type;
}

/**
 * gs_app_set_size_cache_data:
 * @app: a #GsApp
 * @size_type: type of the cache data size
 * @size_bytes: size in bytes
 *
 * Sets the cache data size of the @app.
 *
 * @size_bytes will be ignored unless @size_type is %GS_SIZE_TYPE_VALID.
 *
 * Since: 43
 **/
void
gs_app_set_size_cache_data (GsApp      *app,
                            GsSizeType  size_type,
                            guint64     size_bytes)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_if_fail (GS_IS_APP (app));

	if (size_type != GS_SIZE_TYPE_VALID)
		size_bytes = 0;

	if (priv->size_cache_data_type != size_type) {
		priv->size_cache_data_type = size_type;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_CACHE_DATA_TYPE]);
	}

	if (priv->size_cache_data != size_bytes) {
		priv->size_cache_data = size_bytes;
		gs_app_queue_notify (app, obj_props[PROP_SIZE_CACHE_DATA]);
	}
}

/**
 * gs_app_get_metadata_item:
 * @app: a #GsApp
 * @key: a string, e.g. "fwupd::device-id"
 *
 * Gets some metadata for the application.
 * Is is expected that plugins namespace any plugin-specific metadata,
 * for example `fwupd::device-id`.
 *
 * Returns: a string, or %NULL for unset
 *
 * Since: 3.22
 **/
const gchar *
gs_app_get_metadata_item (GsApp *app, const gchar *key)
{
	GVariant *tmp;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	g_return_val_if_fail (key != NULL, NULL);
	tmp = gs_app_get_metadata_variant (app, key);
	if (tmp == NULL)
		return NULL;
	return g_variant_get_string (tmp, NULL);
}

/**
 * gs_app_set_metadata:
 * @app: a #GsApp
 * @key: a string, e.g. "fwupd::DeviceID"
 * @value: a string, e.g. "fubar"
 *
 * Sets some metadata for the application.
 * Is is expected that plugins namespace any plugin-specific metadata.
 *
 * Since: 3.22
 **/
void
gs_app_set_metadata (GsApp *app, const gchar *key, const gchar *value)
{
	g_autoptr(GVariant) tmp = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (key != NULL);
	if (value != NULL)
		tmp = g_variant_new_string (value);
	gs_app_set_metadata_variant (app, key, tmp);
}

/**
 * gs_app_get_metadata_variant:
 * @app: a #GsApp
 * @key: a string, e.g. "fwupd::device-id"
 *
 * Gets some metadata for the application.
 * Is is expected that plugins namespace any plugin-specific metadata.
 *
 * Returns: (transfer none) (nullable): a variant, or %NULL for unset
 *
 * Since: 3.26
 **/
GVariant *
gs_app_get_metadata_variant (GsApp *app, const gchar *key)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	g_return_val_if_fail (key != NULL, NULL);
	return g_hash_table_lookup (priv->metadata, key);
}

/**
 * gs_app_set_metadata_variant:
 * @app: a #GsApp
 * @key: a string, e.g. "fwupd::DeviceID"
 * @value: a #GVariant
 *
 * Sets some metadata for the application.
 * Is is expected that plugins namespace any plugin-specific metadata,
 * for example `fwupd::device-id`.
 *
 * Since: 3.26
 **/
void
gs_app_set_metadata_variant (GsApp *app, const gchar *key, GVariant *value)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	GVariant *found;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	/* if no value, then remove the key */
	if (value == NULL) {
		g_hash_table_remove (priv->metadata, key);
		return;
	}

	/* check we're not overwriting */
	found = g_hash_table_lookup (priv->metadata, key);
	if (found != NULL) {
		if (g_variant_equal (found, value))
			return;
		if (g_variant_type_equal (g_variant_get_type (value), G_VARIANT_TYPE_STRING) &&
		    g_variant_type_equal (g_variant_get_type (found), G_VARIANT_TYPE_STRING)) {
			g_debug ("tried overwriting %s key %s from %s to %s",
				 priv->id, key,
				 g_variant_get_string (found, NULL),
				 g_variant_get_string (value, NULL));
		} else {
			g_debug ("tried overwriting %s key %s (%s->%s)",
				 priv->id, key,
				 g_variant_get_type_string (found),
				 g_variant_get_type_string (value));
		}
		return;
	}
	g_hash_table_insert (priv->metadata, g_strdup (key), g_variant_ref (value));
}

/**
 * gs_app_dup_addons:
 * @app: a #GsApp
 *
 * Gets the list of addons for the application.
 *
 * Returns: (transfer full) (nullable): a list of addons, or %NULL if there are none
 *
 * Since: 43
 */
GsAppList *
gs_app_dup_addons (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return (priv->addons != NULL) ? g_object_ref (priv->addons) : NULL;
}

/**
 * gs_app_add_addons:
 * @app: a #GsApp
 * @addons: (transfer none) (not nullable): a list of #GsApps
 *
 * Adds zero or more addons to the list of application addons.
 *
 * Since: 43
 **/
void
gs_app_add_addons (GsApp     *app,
                   GsAppList *addons)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GsAppList) new_addons = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP_LIST (addons));

	if (gs_app_list_length (addons) == 0)
		return;

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->addons != NULL)
		new_addons = gs_app_list_copy (priv->addons);
	else
		new_addons = gs_app_list_new ();
	gs_app_list_add_list (new_addons, addons);

	g_set_object (&priv->addons, new_addons);
}

/**
 * gs_app_remove_addon:
 * @app: a #GsApp
 * @addon: a #GsApp
 *
 * Removes an addon from the list of application addons.
 *
 * Since: 3.22
 **/
void
gs_app_remove_addon (GsApp *app, GsApp *addon)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP (addon));
	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->addons != NULL)
		gs_app_list_remove (priv->addons, addon);
}

/**
 * gs_app_get_related:
 * @app: a #GsApp
 *
 * Gets any related applications.
 *
 * Returns: (transfer none): a list of applications
 *
 * Since: 3.22
 **/
GsAppList *
gs_app_get_related (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->related;
}

/**
 * gs_app_add_related:
 * @app: a #GsApp
 * @app2: a #GsApp
 *
 * Adds a related application.
 *
 * Since: 3.22
 **/
void
gs_app_add_related (GsApp *app, GsApp *app2)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	GsAppPrivate *priv2 = gs_app_get_instance_private (app2);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP (app2));

	locker = g_mutex_locker_new (&priv->mutex);

	/* if the app is updatable-live and any related app is not then
	 * degrade to the offline state */
	if (priv->state == GS_APP_STATE_UPDATABLE_LIVE &&
	    priv2->state == GS_APP_STATE_UPDATABLE)
		priv->state = priv2->state;

	gs_app_list_add (priv->related, app2);

	/* The related apps add to the main app’s sizes. */
	gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE]);
	gs_app_queue_notify (app, obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES]);
	gs_app_queue_notify (app, obj_props[PROP_SIZE_INSTALLED_DEPENDENCIES_TYPE]);
	gs_app_queue_notify (app, obj_props[PROP_SIZE_INSTALLED_DEPENDENCIES]);
}

/**
 * gs_app_get_history:
 * @app: a #GsApp
 *
 * Gets the history of this application.
 *
 * Returns: (transfer none): a list
 *
 * Since: 3.22
 **/
GsAppList *
gs_app_get_history (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->history;
}

/**
 * gs_app_add_history:
 * @app: a #GsApp
 * @app2: a #GsApp
 *
 * Adds a history item for this package.
 *
 * Since: 3.22
 **/
void
gs_app_add_history (GsApp *app, GsApp *app2)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP (app2));
	locker = g_mutex_locker_new (&priv->mutex);
	gs_app_list_add (priv->history, app2);
}

/**
 * gs_app_get_install_date:
 * @app: a #GsApp
 *
 * Gets the date that an application was installed.
 *
 * Returns: A UNIX epoch, or 0 for unset
 *
 * Since: 3.22
 **/
guint64
gs_app_get_install_date (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), 0);
	return priv->install_date;
}

/**
 * gs_app_set_install_date:
 * @app: a #GsApp
 * @install_date: an epoch, or %GS_APP_INSTALL_DATE_UNKNOWN
 *
 * Sets the date that an application was installed.
 *
 * Since: 3.22
 **/
void
gs_app_set_install_date (GsApp *app, guint64 install_date)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	if (install_date == priv->install_date)
		return;
	priv->install_date = install_date;
}

/**
 * gs_app_get_release_date:
 * @app: a #GsApp
 *
 * Gets the date that an application was released.
 *
 * Returns: A UNIX epoch, or 0 for unset
 *
 * Since: 3.40
 **/
guint64
gs_app_get_release_date (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), 0);
	return priv->release_date;
}

/**
 * gs_app_set_release_date:
 * @app: a #GsApp
 * @release_date: an epoch, or 0
 *
 * Sets the date that an application was released.
 *
 * Since: 3.40
 **/
void
gs_app_set_release_date (GsApp *app, guint64 release_date)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	if (release_date == priv->release_date)
		return;
	priv->release_date = release_date;

	gs_app_queue_notify (app, obj_props[PROP_RELEASE_DATE]);
}

/**
 * gs_app_is_installed:
 * @app: a #GsApp
 *
 * Gets whether the app is installed or not.
 *
 * Returns: %TRUE if the app is installed, %FALSE otherwise.
 *
 * Since: 3.22
 **/
gboolean
gs_app_is_installed (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return (priv->state == GS_APP_STATE_INSTALLED) ||
	       (priv->state == GS_APP_STATE_UPDATABLE) ||
	       (priv->state == GS_APP_STATE_UPDATABLE_LIVE) ||
	       (priv->state == GS_APP_STATE_REMOVING);
}

/**
 * gs_app_is_updatable:
 * @app: a #GsApp
 *
 * Gets whether the app is updatable or not.
 *
 * Returns: %TRUE if the app is updatable, %FALSE otherwise.
 *
 * Since: 3.22
 **/
gboolean
gs_app_is_updatable (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	if (priv->kind == AS_COMPONENT_KIND_OPERATING_SYSTEM)
		return TRUE;
	return (priv->state == GS_APP_STATE_UPDATABLE) ||
	       (priv->state == GS_APP_STATE_UPDATABLE_LIVE);
}

/**
 * gs_app_get_categories:
 * @app: a #GsApp
 *
 * Gets the list of categories for an application.
 *
 * Returns: (element-type utf8) (transfer none): a list
 *
 * Since: 3.22
 **/
GPtrArray *
gs_app_get_categories (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	return priv->categories;
}

/**
 * gs_app_has_category:
 * @app: a #GsApp
 * @category: a category ID, e.g. "AudioVideo"
 *
 * Checks if the application is in a specific category.
 *
 * Returns: %TRUE for success
 *
 * Since: 3.22
 **/
gboolean
gs_app_has_category (GsApp *app, const gchar *category)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	const gchar *tmp;
	guint i;

	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	/* find the category */
	for (i = 0; i < priv->categories->len; i++) {
		tmp = g_ptr_array_index (priv->categories, i);
		if (g_strcmp0 (tmp, category) == 0)
			return TRUE;
	}
	return FALSE;
}

/**
 * gs_app_set_categories:
 * @app: a #GsApp
 * @categories: a set of categories
 *
 * Set the list of categories for an application.
 *
 * Since: 3.22
 **/
void
gs_app_set_categories (GsApp *app, GPtrArray *categories)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (categories != NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_ptr_array (&priv->categories, categories);
}

/**
 * gs_app_add_category:
 * @app: a #GsApp
 * @category: a category ID, e.g. "AudioVideo"
 *
 * Adds a category ID to an application.
 *
 * Since: 3.22
 **/
void
gs_app_add_category (GsApp *app, const gchar *category)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (category != NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	if (gs_app_has_category (app, category))
		return;
	g_ptr_array_add (priv->categories, g_strdup (category));
}

/**
 * gs_app_remove_category:
 * @app: a #GsApp
 * @category: a category ID, e.g. "AudioVideo"
 *
 * Removes an category ID from an application, it exists.
 *
 * Returns: %TRUE for success
 *
 * Since: 3.24
 **/
gboolean
gs_app_remove_category (GsApp *app, const gchar *category)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	const gchar *tmp;
	guint i;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	locker = g_mutex_locker_new (&priv->mutex);

	for (i = 0; i < priv->categories->len; i++) {
		tmp = g_ptr_array_index (priv->categories, i);
		if (g_strcmp0 (tmp, category) != 0)
			continue;
		g_ptr_array_remove_index_fast (priv->categories, i);
		return TRUE;
	}
	return FALSE;
}

/**
 * gs_app_set_is_update_downloaded:
 * @app: a #GsApp
 * @is_update_downloaded: Whether a new update is already downloaded locally
 *
 * Sets if the new update is already downloaded for the app.
 *
 * Since: 3.36
 * Deprecated: 44: No longer supported.
 **/
void
gs_app_set_is_update_downloaded (GsApp *app, gboolean is_update_downloaded)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	priv->is_update_downloaded = is_update_downloaded;
}

/**
 * gs_app_get_is_update_downloaded:
 * @app: a #GsApp
 *
 * Gets if the new update is already downloaded for the app and
 * is locally available.
 *
 * Returns: (element-type gboolean): Whether a new update for the #GsApp is already downloaded.
 *
 * Since: 3.36
 * Deprecated: 44: No longer supported.
 **/
gboolean
gs_app_get_is_update_downloaded (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return priv->is_update_downloaded;
}

static void
calculate_key_colors (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GIcon) icon_small = NULL;
	g_autoptr(GdkPixbuf) pb_small = NULL;
	const gchar *overrides_str;

	/* Lazily create the array */
	if (priv->key_colors == NULL)
		priv->key_colors = g_array_new (FALSE, FALSE, sizeof (GdkRGBA));
	priv->user_key_colors = FALSE;

	/* Look for an override first. Parse and use it if possible. This is
	 * typically specified in the appdata for an app as:
	 * |[
	 * <component>
	 *   <custom>
	 *     <value key="LingmoAppstore::key-colors">[(124, 53, 77), (99, 16, 0)]</value>
	 *   </custom>
	 * </component>
	 * ]|
	 */
	overrides_str = gs_app_get_metadata_item (app, "LingmoAppstore::key-colors");
	if (overrides_str != NULL) {
		g_autoptr(GVariant) overrides = NULL;
		g_autoptr(GError) local_error = NULL;

		overrides = g_variant_parse (G_VARIANT_TYPE ("a(yyy)"),
					     overrides_str,
					     NULL,
					     NULL,
					     &local_error);

		if (overrides != NULL && g_variant_n_children (overrides) > 0) {
			GVariantIter iter;
			guint8 red, green, blue;

			g_variant_iter_init (&iter, overrides);
			while (g_variant_iter_loop (&iter, "(yyy)", &red, &green, &blue)) {
				GdkRGBA rgba;
				rgba.red = (gdouble) red / 255.0;
				rgba.green = (gdouble) green / 255.0;
				rgba.blue = (gdouble) blue / 255.0;
				rgba.alpha = 1.0;
				g_array_append_val (priv->key_colors, rgba);
			}

			priv->user_key_colors = TRUE;

			return;
		} else {
			g_warning ("Invalid value for LingmoAppstore::key-colors for %s: %s",
				   gs_app_get_id (app), local_error->message);
			/* fall through */
		}
	}

	/* Try and load the pixbuf. */
	icon_small = gs_app_get_icon_for_size (app, 32, 1, NULL);

	if (icon_small == NULL) {
		g_debug ("no pixbuf, so no key colors");
		return;
	} else if (G_IS_LOADABLE_ICON (icon_small)) {
		g_autoptr(GInputStream) icon_stream = g_loadable_icon_load (G_LOADABLE_ICON (icon_small), 32, NULL, NULL, NULL);
		if (icon_stream)
			pb_small = gdk_pixbuf_new_from_stream_at_scale (icon_stream, 32, 32, TRUE, NULL, NULL);
	} else if (G_IS_THEMED_ICON (icon_small)) {
		g_autoptr(GtkIconPaintable) icon_paintable = NULL;
		g_autoptr(GtkIconTheme) theme = get_icon_theme ();

		icon_paintable = gtk_icon_theme_lookup_by_gicon (theme, icon_small,
								 32, 1,
								 gtk_get_locale_direction (),
								 0);
		if (icon_paintable != NULL) {
			g_autoptr(GFile) file = NULL;
			g_autofree gchar *path = NULL;

			file = gtk_icon_paintable_get_file (icon_paintable);
			if (file != NULL)
				path = g_file_get_path (file);

			if (path != NULL) {
				pb_small = gdk_pixbuf_new_from_file_at_size (path, 32, 32, NULL);
			} else {
				const gchar *const *names = g_themed_icon_get_names (G_THEMED_ICON (icon_small));
				for (guint i = 0; names != NULL && names[i] != NULL && pb_small == NULL; i++) {
					g_autoptr(GError) local_error = NULL;
					g_autofree gchar *resource_path = NULL;
					resource_path = g_strconcat ("/org/lingmo/Appstore/icons/scalable/apps/", names[i], ".svg", NULL);
					pb_small = gdk_pixbuf_new_from_resource (resource_path, &local_error);
					if (pb_small == NULL)
						g_warning ("Failed to load icon from resource '%s': %s", resource_path, local_error != NULL ? local_error->message : "Unknown error");
				}
			}
		}

	} else {
		g_debug ("unsupported pixbuf, so no key colors");
		return;
	}

	if (pb_small == NULL) {
		g_debug ("pixbuf couldn’t be loaded, so no key colors");
		return;
	}

	/* get a list of key colors */
	g_clear_pointer (&priv->key_colors, g_array_unref);
	priv->key_colors = gs_calculate_key_colors (pb_small);
}

/**
 * gs_app_get_key_colors:
 * @app: a #GsApp
 *
 * Gets the key colors used in the application icon.
 *
 * Returns: (element-type GdkRGBA) (transfer none): a list
 *
 * Since: 40
 **/
GArray *
gs_app_get_key_colors (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	if (priv->key_colors == NULL)
		calculate_key_colors (app);

	return priv->key_colors;
}

/**
 * gs_app_set_key_colors:
 * @app: a #GsApp
 * @key_colors: (element-type GdkRGBA): a set of key colors
 *
 * Sets the key colors used in the application icon.
 *
 * Since: 40
 **/
void
gs_app_set_key_colors (GsApp *app, GArray *key_colors)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (key_colors != NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	priv->user_key_colors = FALSE;
	if (_g_set_array (&priv->key_colors, key_colors))
		gs_app_queue_notify (app, obj_props[PROP_KEY_COLORS]);
}

/**
 * gs_app_add_key_color:
 * @app: a #GsApp
 * @key_color: a #GdkRGBA
 *
 * Adds a key color used in the application icon.
 *
 * Since: 3.22
 **/
void
gs_app_add_key_color (GsApp *app, GdkRGBA *key_color)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (key_color != NULL);

	/* Lazily create the array */
	if (priv->key_colors == NULL)
		priv->key_colors = g_array_new (FALSE, FALSE, sizeof (GdkRGBA));

	priv->user_key_colors = FALSE;
	g_array_append_val (priv->key_colors, *key_color);
	gs_app_queue_notify (app, obj_props[PROP_KEY_COLORS]);
}

/**
 * gs_app_get_user_key_colors:
 * @app: a #GsApp
 *
 * Returns whether the key colors provided by gs_app_get_key_colors()
 * are set by the user (using `LingmoAppstore::key-colors`). %FALSE
 * means the colors have been calculated from the @app icon.
 *
 * Returns: whether the key colors have been provided by the user.
 *
 * Since: 42
 **/
gboolean
gs_app_get_user_key_colors (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return priv->user_key_colors;
}

/**
 * gs_app_add_kudo:
 * @app: a #GsApp
 * @kudo: a #GsAppKudo, e.g. %GS_APP_KUDO_MY_LANGUAGE
 *
 * Adds a kudo to the application.
 *
 * Since: 3.22
 **/
void
gs_app_add_kudo (GsApp *app, GsAppKudo kudo)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	if (kudo & GS_APP_KUDO_SANDBOXED_SECURE)
		kudo |= GS_APP_KUDO_SANDBOXED;
	priv->kudos |= kudo;
}

/**
 * gs_app_remove_kudo:
 * @app: a #GsApp
 * @kudo: a #GsAppKudo, e.g. %GS_APP_KUDO_MY_LANGUAGE
 *
 * Removes a kudo from the application.
 *
 * Since: 3.30
 **/
void
gs_app_remove_kudo (GsApp *app, GsAppKudo kudo)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	priv->kudos &= ~kudo;
}

/**
 * gs_app_has_kudo:
 * @app: a #GsApp
 * @kudo: a #GsAppKudo, e.g. %GS_APP_KUDO_MY_LANGUAGE
 *
 * Finds out if a kudo has been awarded by the application.
 *
 * Returns: %TRUE if the app has the specified kudo
 *
 * Since: 3.22
 **/
gboolean
gs_app_has_kudo (GsApp *app, GsAppKudo kudo)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return (priv->kudos & kudo) > 0;
}

/**
 * gs_app_get_kudos:
 * @app: a #GsApp
 *
 * Gets all the kudos the application has been awarded.
 *
 * Returns: the kudos, as a bitfield
 *
 * Since: 3.22
 **/
guint64
gs_app_get_kudos (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), 0);
	return priv->kudos;
}

/**
 * gs_app_get_kudos_percentage:
 * @app: a #GsApp
 *
 * Gets the kudos, as a percentage value.
 *
 * Returns: a percentage, with 0 for no kudos and a maximum of 100.
 *
 * Since: 3.22
 **/
guint
gs_app_get_kudos_percentage (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	guint percentage = 0;

	g_return_val_if_fail (GS_IS_APP (app), 0);

	if ((priv->kudos & GS_APP_KUDO_MY_LANGUAGE) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_RECENT_RELEASE) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_FEATURED_RECOMMENDED) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_HAS_KEYWORDS) > 0)
		percentage += 5;
	if ((priv->kudos & GS_APP_KUDO_HAS_SCREENSHOTS) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_HI_DPI_ICON) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_SANDBOXED) > 0)
		percentage += 20;
	if ((priv->kudos & GS_APP_KUDO_SANDBOXED_SECURE) > 0)
		percentage += 20;

	return MIN (percentage, 100);
}

/**
 * gs_app_get_to_be_installed:
 * @app: a #GsApp
 *
 * Gets if the application is queued for installation.
 *
 * This is only set for addons when the user has selected some addons to be
 * installed before installing the main application.
 * Plugins should check all the addons for this property when installing
 * main applications so that the chosen set of addons is also installed at the
 * same time. This is never set when applications do not have addons.
 *
 * Returns: %TRUE for success
 *
 * Since: 3.22
 **/
gboolean
gs_app_get_to_be_installed (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	return priv->to_be_installed;
}

/**
 * gs_app_set_to_be_installed:
 * @app: a #GsApp
 * @to_be_installed: if the app is due to be installed
 *
 * Sets if the application is queued for installation.
 *
 * Since: 3.22
 **/
void
gs_app_set_to_be_installed (GsApp *app, gboolean to_be_installed)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));

	priv->to_be_installed = to_be_installed;
}

/**
 * gs_app_has_quirk:
 * @app: a #GsApp
 * @quirk: a #GsAppQuirk, e.g. %GS_APP_QUIRK_COMPULSORY
 *
 * Finds out if an application has a specific quirk.
 *
 * Returns: %TRUE for success
 *
 * Since: 3.22
 **/
gboolean
gs_app_has_quirk (GsApp *app, GsAppQuirk quirk)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	return (priv->quirk & quirk) > 0;
}

/**
 * gs_app_add_quirk:
 * @app: a #GsApp
 * @quirk: a #GsAppQuirk, e.g. %GS_APP_QUIRK_COMPULSORY
 *
 * Adds a quirk to an application.
 *
 * Since: 3.22
 **/
void
gs_app_add_quirk (GsApp *app, GsAppQuirk quirk)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	/* same */
	if ((priv->quirk & quirk) > 0)
		return;

	locker = g_mutex_locker_new (&priv->mutex);
	priv->quirk |= quirk;
	gs_app_queue_notify (app, obj_props[PROP_QUIRK]);
}

/**
 * gs_app_remove_quirk:
 * @app: a #GsApp
 * @quirk: a #GsAppQuirk, e.g. %GS_APP_QUIRK_COMPULSORY
 *
 * Removes a quirk from an application.
 *
 * Since: 3.22
 **/
void
gs_app_remove_quirk (GsApp *app, GsAppQuirk quirk)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	/* same */
	if ((priv->quirk & quirk) == 0)
		return;

	locker = g_mutex_locker_new (&priv->mutex);
	priv->quirk &= ~quirk;
	gs_app_queue_notify (app, obj_props[PROP_QUIRK]);
}

/**
 * gs_app_set_match_value:
 * @app: a #GsApp
 * @match_value: a value
 *
 * Set a match quality value, where higher values correspond to a
 * "better" search match, and should be shown above lower results.
 *
 * Since: 3.22
 **/
void
gs_app_set_match_value (GsApp *app, guint match_value)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	priv->match_value = match_value;
}

/**
 * gs_app_get_match_value:
 * @app: a #GsApp
 *
 * Get a match quality value, where higher values correspond to a
 * "better" search match, and should be shown above lower results.
 *
 * Note: This value is only valid when processing the result set
 * and may be overwritten on subsequent searches if the plugin is using
 * a cache.
 *
 * Returns: a value, where higher is better
 *
 * Since: 3.22
 **/
guint
gs_app_get_match_value (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), 0);
	return priv->match_value;
}

/**
 * gs_app_set_priority:
 * @app: a #GsApp
 * @priority: a value
 *
 * Set a priority value.
 *
 * Since: 3.22
 **/
void
gs_app_set_priority (GsApp *app, guint priority)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_if_fail (GS_IS_APP (app));
	priv->priority = priority;
}

/**
 * gs_app_get_priority:
 * @app: a #GsApp
 *
 * Get a priority value, where higher values will be chosen where
 * multiple #GsApp's match a specific rule.
 *
 * Returns: a value, where higher is better
 *
 * Since: 3.22
 **/
guint
gs_app_get_priority (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), 0);

	/* If the priority hasn’t been explicitly set, fetch it from the app’s
	 * management plugin. */
	if (priv->priority == 0) {
		g_autoptr(GsPlugin) plugin = gs_app_dup_management_plugin (app);
		if (plugin != NULL)
			priv->priority = gs_plugin_get_priority (plugin);
	}

	return priv->priority;
}

/**
 * gs_app_get_cancellable:
 * @app: a #GsApp
 *
 * Get a cancellable to be used with operations related to the #GsApp. This is a
 * way for views to be able to cancel an on-going operation. If the #GCancellable
 * is canceled, it will be unreferenced and renewed before returning it, i.e. the
 * cancellable object will always be ready to use for new operations. So be sure
 * to keep a reference to it if you do more than just passing the cancellable to
 * a process.
 *
 * Returns: a #GCancellable
 *
 * Since: 3.28
 **/
GCancellable *
gs_app_get_cancellable (GsApp *app)
{
	g_autoptr(GCancellable) cancellable = NULL;
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->cancellable == NULL || g_cancellable_is_cancelled (priv->cancellable)) {
		cancellable = g_cancellable_new ();
		g_set_object (&priv->cancellable, cancellable);
	}
	return priv->cancellable;
}

/**
 * gs_app_peek_cancellable:
 * @app: a #GsApp
 *
 * Peek the current cancellable used by the @app. It's referenced for thread safety;
 * if not %NULL, free it with g_object_unref() when no longer needed.
 *
 * Returns: (nullable) (transfer full): the current cancellable, or %NULL
 *
 * Since: 44
 **/
GCancellable *
gs_app_peek_cancellable (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->cancellable)
		return g_object_ref (priv->cancellable);

	return NULL;
}

/**
 * gs_app_get_pending_action:
 * @app: a #GsApp
 *
 * Get the pending action for this #GsApp, or %NULL if no action is pending.
 *
 * Returns: the #GsAppAction of the @app.
 **/
GsPluginAction
gs_app_get_pending_action (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), GS_PLUGIN_ACTION_UNKNOWN);
	locker = g_mutex_locker_new (&priv->mutex);
	return priv->pending_action;
}

/**
 * gs_app_set_pending_action:
 * @app: a #GsApp
 * @action: a #GsPluginAction
 *
 * Set an action that is pending on this #GsApp.
 **/
void
gs_app_set_pending_action (GsApp *app,
			   GsPluginAction action)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	locker = g_mutex_locker_new (&priv->mutex);
	gs_app_set_pending_action_internal (app, action);
}

static void
gs_app_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsApp *app = GS_APP (object);
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	switch ((GsAppProperty) prop_id) {
	case PROP_ID:
		g_value_set_string (value, priv->id);
		break;
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_VERSION:
		g_value_set_string (value, priv->version);
		break;
	case PROP_SUMMARY:
		g_value_set_string (value, priv->summary);
		break;
	case PROP_DESCRIPTION:
		g_value_set_string (value, priv->description);
		break;
	case PROP_RATING:
		g_value_set_int (value, priv->rating);
		break;
	case PROP_KIND:
		g_value_set_uint (value, priv->kind);
		break;
	case PROP_SPECIAL_KIND:
		g_value_set_enum (value, priv->special_kind);
		break;
	case PROP_STATE:
		g_value_set_enum (value, priv->state);
		break;
	case PROP_PROGRESS:
		g_value_set_uint (value, priv->progress);
		break;
	case PROP_CAN_CANCEL_INSTALLATION:
		g_value_set_boolean (value, priv->allow_cancel);
		break;
	case PROP_INSTALL_DATE:
		g_value_set_uint64 (value, priv->install_date);
		break;
	case PROP_RELEASE_DATE:
		g_value_set_uint64 (value, priv->release_date);
		break;
	case PROP_QUIRK:
		g_value_set_flags (value, priv->quirk);
		break;
	case PROP_PENDING_ACTION:
		g_value_set_enum (value, priv->pending_action);
		break;
	case PROP_KEY_COLORS:
		g_value_set_boxed (value, gs_app_get_key_colors (app));
		break;
	case PROP_IS_UPDATE_DOWNLOADED:
		g_value_set_boolean (value, priv->is_update_downloaded);
		break;
	case PROP_URLS:
		g_value_set_boxed (value, priv->urls);
		break;
	case PROP_URL_MISSING:
		g_value_set_string (value, priv->url_missing);
		break;
	case PROP_CONTENT_RATING:
		g_value_set_object (value, priv->content_rating);
		break;
	case PROP_LICENSE:
		g_value_set_string (value, priv->license);
		break;
	case PROP_SIZE_CACHE_DATA_TYPE:
		g_value_set_enum (value, gs_app_get_size_cache_data (app, NULL));
		break;
	case PROP_SIZE_CACHE_DATA: {
		guint64 size_bytes;
		gs_app_get_size_cache_data (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_SIZE_DOWNLOAD_TYPE:
		g_value_set_enum (value, gs_app_get_size_download (app, NULL));
		break;
	case PROP_SIZE_DOWNLOAD: {
		guint64 size_bytes;
		gs_app_get_size_download (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE:
		g_value_set_enum (value, gs_app_get_size_download_dependencies (app, NULL));
		break;
	case PROP_SIZE_DOWNLOAD_DEPENDENCIES: {
		guint64 size_bytes;
		gs_app_get_size_download_dependencies (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_SIZE_INSTALLED_TYPE:
		g_value_set_enum (value, gs_app_get_size_installed (app, NULL));
		break;
	case PROP_SIZE_INSTALLED: {
		guint64 size_bytes;
		gs_app_get_size_installed (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_SIZE_INSTALLED_DEPENDENCIES_TYPE:
		g_value_set_enum (value, gs_app_get_size_installed_dependencies (app, NULL));
		break;
	case PROP_SIZE_INSTALLED_DEPENDENCIES: {
		guint64 size_bytes;
		gs_app_get_size_installed_dependencies (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_SIZE_USER_DATA_TYPE:
		g_value_set_enum (value, gs_app_get_size_user_data (app, NULL));
		break;
	case PROP_SIZE_USER_DATA: {
		guint64 size_bytes;
		gs_app_get_size_user_data (app, &size_bytes);
		g_value_set_uint64 (value, size_bytes);
		break;
	}
	case PROP_PERMISSIONS:
		g_value_take_object (value, gs_app_dup_permissions (app));
		break;
	case PROP_RELATIONS:
		g_value_take_boxed (value, gs_app_get_relations (app));
		break;
	case PROP_ORIGIN_UI:
		g_value_take_string (value, gs_app_dup_origin_ui (app, TRUE));
		break;
	case PROP_HAS_TRANSLATIONS:
		g_value_set_boolean (value, gs_app_get_has_translations (app));
		break;
	case PROP_ICONS_STATE:
		g_value_set_enum (value, priv->icons_state);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsApp *app = GS_APP (object);
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	switch ((GsAppProperty) prop_id) {
	case PROP_ID:
		gs_app_set_id (app, g_value_get_string (value));
		break;
	case PROP_NAME:
		gs_app_set_name (app,
				 GS_APP_QUALITY_UNKNOWN,
				 g_value_get_string (value));
		break;
	case PROP_VERSION:
		gs_app_set_version (app, g_value_get_string (value));
		break;
	case PROP_SUMMARY:
		gs_app_set_summary (app,
				    GS_APP_QUALITY_UNKNOWN,
				    g_value_get_string (value));
		break;
	case PROP_DESCRIPTION:
		gs_app_set_description (app,
					GS_APP_QUALITY_UNKNOWN,
					g_value_get_string (value));
		break;
	case PROP_RATING:
		gs_app_set_rating (app, g_value_get_int (value));
		break;
	case PROP_KIND:
		gs_app_set_kind (app, g_value_get_uint (value));
		break;
	case PROP_SPECIAL_KIND:
		gs_app_set_special_kind (app, g_value_get_enum (value));
		break;
	case PROP_STATE:
		gs_app_set_state_internal (app, g_value_get_enum (value));
		break;
	case PROP_PROGRESS:
		gs_app_set_progress (app, g_value_get_uint (value));
		break;
	case PROP_CAN_CANCEL_INSTALLATION:
		priv->allow_cancel = g_value_get_boolean (value);
		break;
	case PROP_INSTALL_DATE:
		gs_app_set_install_date (app, g_value_get_uint64 (value));
		break;
	case PROP_RELEASE_DATE:
		gs_app_set_release_date (app, g_value_get_uint64 (value));
		break;
	case PROP_QUIRK:
		priv->quirk = g_value_get_flags (value);
		break;
	case PROP_PENDING_ACTION:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_KEY_COLORS:
		gs_app_set_key_colors (app, g_value_get_boxed (value));
		break;
	case PROP_IS_UPDATE_DOWNLOADED:
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		gs_app_set_is_update_downloaded (app, g_value_get_boolean (value));
G_GNUC_END_IGNORE_DEPRECATIONS
		break;
	case PROP_URLS:
		/* Read only */
		g_assert_not_reached ();
		break;
	case PROP_URL_MISSING:
		gs_app_set_url_missing (app, g_value_get_string (value));
		break;
	case PROP_CONTENT_RATING:
		gs_app_set_content_rating (app, g_value_get_object (value));
		break;
	case PROP_LICENSE:
		/* Read-only */
		g_assert_not_reached ();
	case PROP_SIZE_CACHE_DATA_TYPE:
		gs_app_set_size_cache_data (app, g_value_get_enum (value), priv->size_cache_data);
		break;
	case PROP_SIZE_CACHE_DATA:
		gs_app_set_size_cache_data (app, priv->size_cache_data_type, g_value_get_uint64 (value));
		break;
	case PROP_SIZE_DOWNLOAD_TYPE:
		gs_app_set_size_download (app, g_value_get_enum (value), priv->size_download);
		break;
	case PROP_SIZE_DOWNLOAD:
		gs_app_set_size_download (app, priv->size_download_type, g_value_get_uint64 (value));
		break;
	case PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE:
	case PROP_SIZE_DOWNLOAD_DEPENDENCIES:
		/* Read-only */
		g_assert_not_reached ();
	case PROP_SIZE_INSTALLED_TYPE:
		gs_app_set_size_installed (app, g_value_get_enum (value), priv->size_installed);
		break;
	case PROP_SIZE_INSTALLED:
		gs_app_set_size_installed (app, priv->size_installed_type, g_value_get_uint64 (value));
		break;
	case PROP_SIZE_INSTALLED_DEPENDENCIES_TYPE:
	case PROP_SIZE_INSTALLED_DEPENDENCIES:
		/* Read-only */
		g_assert_not_reached ();
	case PROP_SIZE_USER_DATA_TYPE:
		gs_app_set_size_user_data (app, g_value_get_enum (value), priv->size_user_data);
		break;
	case PROP_SIZE_USER_DATA:
		gs_app_set_size_user_data (app, priv->size_user_data_type, g_value_get_uint64 (value));
		break;
	case PROP_PERMISSIONS:
		gs_app_set_permissions (app, g_value_get_object (value));
		break;
	case PROP_RELATIONS:
		gs_app_set_relations (app, g_value_get_boxed (value));
		break;
	case PROP_ORIGIN_UI:
		gs_app_set_origin_ui (app, g_value_get_string (value));
		break;
	case PROP_HAS_TRANSLATIONS:
		gs_app_set_has_translations (app, g_value_get_boolean (value));
		break;
	case PROP_ICONS_STATE:
		/* Read-only */
		g_assert_not_reached ();
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_dispose (GObject *object)
{
	GsApp *app = GS_APP (object);
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_clear_object (&priv->runtime);

	g_clear_pointer (&priv->addons, g_object_unref);
	g_clear_pointer (&priv->history, g_object_unref);
	g_clear_pointer (&priv->related, g_object_unref);
	g_clear_pointer (&priv->screenshots, g_ptr_array_unref);
	g_clear_pointer (&priv->review_ratings, g_array_unref);
	g_clear_pointer (&priv->reviews, g_ptr_array_unref);
	g_clear_pointer (&priv->provided, g_ptr_array_unref);
	g_clear_pointer (&priv->icons, g_ptr_array_unref);
	g_clear_pointer (&priv->version_history, g_ptr_array_unref);
	g_clear_pointer (&priv->relations, g_ptr_array_unref);
	g_weak_ref_clear (&priv->management_plugin_weak);

	G_OBJECT_CLASS (gs_app_parent_class)->dispose (object);
}

static void
gs_app_finalize (GObject *object)
{
	GsApp *app = GS_APP (object);
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_mutex_clear (&priv->mutex);
	g_free (priv->id);
	g_free (priv->unique_id);
	g_free (priv->branch);
	g_free (priv->name);
	g_free (priv->renamed_from);
	g_free (priv->url_missing);
	g_clear_pointer (&priv->urls, g_hash_table_unref);
	g_hash_table_unref (priv->launchables);
	g_free (priv->license);
	g_strfreev (priv->menu_path);
	g_free (priv->origin);
	g_free (priv->origin_ui);
	g_free (priv->origin_appstream);
	g_free (priv->origin_hostname);
	g_ptr_array_unref (priv->sources);
	g_ptr_array_unref (priv->source_ids);
	g_free (priv->project_group);
	g_free (priv->developer_name);
	g_free (priv->agreement);
	g_free (priv->version);
	g_free (priv->version_ui);
	g_free (priv->summary);
	g_free (priv->summary_missing);
	g_free (priv->description);
	g_free (priv->update_version);
	g_free (priv->update_version_ui);
	g_free (priv->update_details_markup);
	g_hash_table_unref (priv->metadata);
	g_ptr_array_unref (priv->categories);
	g_clear_pointer (&priv->key_colors, g_array_unref);
	g_clear_object (&priv->cancellable);
	g_clear_object (&priv->local_file);
	g_clear_object (&priv->content_rating);
	g_clear_object (&priv->action_screenshot);
	g_clear_object (&priv->update_permissions);
	g_clear_object (&priv->permissions);

	G_OBJECT_CLASS (gs_app_parent_class)->finalize (object);
}

static void
gs_app_class_init (GsAppClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->dispose = gs_app_dispose;
	object_class->finalize = gs_app_finalize;
	object_class->get_property = gs_app_get_property;
	object_class->set_property = gs_app_set_property;

	/**
	 * GsApp:id:
	 */
	obj_props[PROP_ID] = g_param_spec_string ("id", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:name:
	 */
	obj_props[PROP_NAME] = g_param_spec_string ("name", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:version:
	 */
	obj_props[PROP_VERSION] = g_param_spec_string ("version", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:summary:
	 */
	obj_props[PROP_SUMMARY] = g_param_spec_string ("summary", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:description:
	 */
	obj_props[PROP_DESCRIPTION] = g_param_spec_string ("description", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:rating:
	 */
	obj_props[PROP_RATING] = g_param_spec_int ("rating", NULL, NULL,
				  -1, 100, -1,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:kind:
	 */
	/* FIXME: Should use AS_TYPE_APP_KIND when it’s available */
	obj_props[PROP_KIND] = g_param_spec_uint ("kind", NULL, NULL,
				   AS_COMPONENT_KIND_UNKNOWN,
				   AS_COMPONENT_KIND_LAST,
				   AS_COMPONENT_KIND_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:special-kind:
	 *
	 * LINGMO Appstore specific occupation of the #GsApp entity
	 * that does not reflect a software type defined by AppStream.
	 *
	 * Since: 40
	 */
	obj_props[PROP_SPECIAL_KIND] = g_param_spec_enum ("special-kind", NULL, NULL,
					GS_TYPE_APP_SPECIAL_KIND,
					GS_APP_SPECIAL_KIND_NONE,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:state:
	 */
	obj_props[PROP_STATE] = g_param_spec_enum ("state", NULL, NULL,
				   GS_TYPE_APP_STATE,
				   GS_APP_STATE_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:progress:
	 *
	 * A percentage (0–100, inclusive) indicating the progress through the
	 * current task on this app. The value may otherwise be
	 * %GS_APP_PROGRESS_UNKNOWN if the progress is unknown or has a wide
	 * confidence interval.
	 */
	obj_props[PROP_PROGRESS] = g_param_spec_uint ("progress", NULL, NULL,
				   0, GS_APP_PROGRESS_UNKNOWN, GS_APP_PROGRESS_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:allow-cancel:
	 */
	obj_props[PROP_CAN_CANCEL_INSTALLATION] =
		g_param_spec_boolean ("allow-cancel", NULL, NULL, TRUE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:install-date:
	 */
	obj_props[PROP_INSTALL_DATE] = g_param_spec_uint64 ("install-date", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:release-date:
	 *
	 * Set to the release date of the application on the server. Can be 0,
	 * which means the release date is unknown.
	 *
	 * Since: 3.40
	 */
	obj_props[PROP_RELEASE_DATE] = g_param_spec_uint64 ("release-date", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:quirk:
	 */
	obj_props[PROP_QUIRK] = g_param_spec_flags ("quirk", NULL, NULL,
				     GS_TYPE_APP_QUIRK, GS_APP_QUIRK_NONE,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:pending-action:
	 */
	obj_props[PROP_PENDING_ACTION] = g_param_spec_enum ("pending-action", NULL, NULL,
				     GS_TYPE_PLUGIN_ACTION, GS_PLUGIN_ACTION_UNKNOWN,
				     G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:key-colors:
	 */
	obj_props[PROP_KEY_COLORS] = g_param_spec_boxed ("key-colors", NULL, NULL,
				    G_TYPE_ARRAY, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:is-update-downloaded:
	 *
	 * Deprecated: 44: No longer supported.
	 */
	obj_props[PROP_IS_UPDATE_DOWNLOADED] = g_param_spec_boolean ("is-update-downloaded", NULL, NULL,
					       FALSE,
					       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_DEPRECATED);

	/**
	 * GsApp:urls: (nullable) (element-type AsUrlKind utf8)
	 *
	 * The URLs associated with the app.
	 *
	 * This is %NULL if no URLs are available. If provided, it is a mapping
	 * from #AsUrlKind to the URLs.
	 *
	 * This property is read-only: use gs_app_set_url() to set URLs.
	 *
	 * Since: 41
	 */
	obj_props[PROP_URLS] =
		g_param_spec_boxed ("urls", NULL, NULL,
				    G_TYPE_HASH_TABLE,
				    G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:url-missing:
	 *
	 * A web URL pointing to explanations why this app
	 * does not have an installation candidate.
	 *
	 * Since: 40
	 */
	obj_props[PROP_URL_MISSING] = g_param_spec_string ("url-missing", NULL, NULL,
					NULL,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:content-rating: (nullable)
	 *
	 * The content rating for the app, which gives information on how
	 * suitable it is for different age ranges of user.
	 *
	 * This is %NULL if no content rating information is available.
	 *
	 * Since: 41
	 */
	obj_props[PROP_CONTENT_RATING] =
		g_param_spec_object ("content-rating", NULL, NULL,
				     /* FIXME: Use the get_type() function directly here to work
				      * around https://github.com/ximion/appstream/pull/318 */
				     as_content_rating_get_type (),
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:license: (nullable)
	 *
	 * The license for the app, which is typically its source code license.
	 *
	 * Use gs_app_set_license() to set this.
	 *
	 * This is %NULL if no licensing information is available.
	 *
	 * Since: 41
	 */
	obj_props[PROP_LICENSE] =
		g_param_spec_string ("license", NULL, NULL,
				     NULL,
				     G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-cache-data-type
	 *
	 * The type of #GsApp:size-cache-data.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_CACHE_DATA_TYPE] =
		g_param_spec_enum ("size-cache-data-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-cache-data
	 *
	 * The size on the disk for the cache data of the application.
	 *
	 * This is undefined if #GsApp:size-cache-data-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_CACHE_DATA] =
		g_param_spec_uint64 ("size-cache-data", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-download-type
	 *
	 * The type of #GsApp:size-download.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_DOWNLOAD_TYPE] =
		g_param_spec_enum ("size-download-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-download
	 *
	 * The size of the total download needed to either install or update
	 * this application, in bytes. If the app is partially downloaded, this
	 * is the number of bytes remaining to download.
	 *
	 * This is undefined if #GsApp:size-download-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * To get the runtime or other dependencies download size,
	 * use #GsApp:size-download-dependencies.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_DOWNLOAD] =
		g_param_spec_uint64 ("size-download", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-download-dependencies-type
	 *
	 * The type of #GsApp:size-download-dependencies.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES_TYPE] =
		g_param_spec_enum ("size-download-dependencies-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-download-dependencies
	 *
	 * The size of the total download needed to either install or update
	 * this application's dependencies, in bytes. If the dependencies are partially
	 * downloaded, this is the number of bytes remaining to download.
	 *
	 * This is undefined if #GsApp:size-download-dependencies-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_DOWNLOAD_DEPENDENCIES] =
		g_param_spec_uint64 ("size-download-dependencies", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-installed-type
	 *
	 * The type of #GsApp:size-installed.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_INSTALLED_TYPE] =
		g_param_spec_enum ("size-installed-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-installed
	 *
	 * The size of the application on disk, in bytes. If the application is
	 * not yet installed, this is the size it would need, once installed.
	 *
	 * This is undefined if #GsApp:size-installed-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * To get the application runtime or extensions installed sizes,
	 * use #GsApp:size-installed-dependencies.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_INSTALLED] =
		g_param_spec_uint64 ("size-installed", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-installed-dependencies-type
	 *
	 * The type of #GsApp:size-installed-dependencies.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_INSTALLED_DEPENDENCIES_TYPE] =
		g_param_spec_enum ("size-installed-dependencies-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-installed-dependencies
	 *
	 * The size of the application's dependencies on disk, in bytes. If the dependencies are
	 * not yet installed, this is the size it would need, once installed.
	 *
	 * This is undefined if #GsApp:size-installed-dependencies-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_INSTALLED_DEPENDENCIES] =
		g_param_spec_uint64 ("size-installed-dependencies", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-user-data-type
	 *
	 * The type of #GsApp:size-user-data.
	 *
	 * Since: 43
	 */
	obj_props[PROP_SIZE_USER_DATA_TYPE] =
		g_param_spec_enum ("size-user-data-type", NULL, NULL,
				   GS_TYPE_SIZE_TYPE, GS_SIZE_TYPE_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:size-user-data
	 *
	 * The size on the disk for the user data of the application.
	 *
	 * This is undefined if #GsApp:size-user-data-type is not
	 * %GS_SIZE_TYPE_VALID.
	 *
	 * Since: 41
	 */
	obj_props[PROP_SIZE_USER_DATA] =
		g_param_spec_uint64 ("size-user-data", NULL, NULL,
				     0, G_MAXUINT64, 0,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:permissions
	 *
	 * The permissions the app requires to run, as a #GsAppPermissions object.
	 *
	 * This is %NULL, if the permissions are unknown.
	 *
	 * Since: 43
	 */
	obj_props[PROP_PERMISSIONS] =
		g_param_spec_object ("permissions", NULL, NULL,
				     GS_TYPE_APP_PERMISSIONS,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:relations: (nullable) (element-type AsRelation)
	 *
	 * Relations between this app and other things. For example,
	 * requirements or recommendations that the computer have certain input
	 * devices to use the app (the app requires a touchscreen or gamepad),
	 * or that the screen is a certain size.
	 *
	 * %NULL is equivalent to an empty array. Relations of kind
	 * %AS_RELATION_KIND_REQUIRES are conjunctive, so each additional
	 * relation further restricts the set of computers which can run the
	 * app. Relations of kind %AS_RELATION_KIND_RECOMMENDS and
	 * %AS_RELATION_KIND_SUPPORTS are disjunctive.
	 *
	 * Since: 41
	 */
	obj_props[PROP_RELATIONS] =
		g_param_spec_boxed ("relations", NULL, NULL,
				    G_TYPE_PTR_ARRAY,
				    G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:origin-ui: (not nullable)
	 *
	 * The package origin, in a human readable format suitable for use in
	 * the UI. For example ‘Local file (RPM)’ or ‘Flathub (Flatpak)’.
	 *
	 * Since: 41
	 */
	obj_props[PROP_ORIGIN_UI] =
		g_param_spec_string ("origin-ui", NULL, NULL,
				     NULL,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:has-translations
	 *
	 * Whether the app has any information about provided translations. If
	 * this is %TRUE, the app provides information about the translations
	 * it ships. If %FALSE, the app does not provide any information (but
	 * might ship translations which aren’t mentioned).
	 *
	 * Since: 41
	 */
	obj_props[PROP_HAS_TRANSLATIONS] =
		g_param_spec_boolean ("has-translations", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	/**
	 * GsApp:icons-state:
	 *
	 * The state of the icons of this app. Notice that it is valid
	 * for the icon state to be %GS_APP_ICONS_STATE_AVAILABLE, and
	 * for there to be no icon for the app. This can happen, for
	 * example, if it downloads an icon, but the icon download has
	 * failed.
	 *
	 * Since: 44
	 */
	obj_props[PROP_ICONS_STATE] = g_param_spec_enum ("icons-state", NULL, NULL,
					GS_TYPE_APP_ICONS_STATE,
					GS_APP_ICONS_STATE_UNKNOWN,
					G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

static void
gs_app_init (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	priv->rating = -1;
	priv->sources = g_ptr_array_new_with_free_func (g_free);
	priv->source_ids = g_ptr_array_new_with_free_func (g_free);
	priv->categories = g_ptr_array_new_with_free_func (g_free);
	priv->related = gs_app_list_new ();
	priv->history = gs_app_list_new ();
	priv->screenshots = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	priv->reviews = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	priv->provided = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	priv->metadata = g_hash_table_new_full (g_str_hash,
	                                        g_str_equal,
	                                        g_free,
	                                        (GDestroyNotify) g_variant_unref);
	priv->launchables = g_hash_table_new_full (g_str_hash,
	                                           g_str_equal,
	                                           NULL,
	                                           g_free);
	priv->allow_cancel = TRUE;
	priv->size_download_type = GS_SIZE_TYPE_UNKNOWN;
	priv->size_installed_type = GS_SIZE_TYPE_UNKNOWN;
	priv->size_cache_data_type = GS_SIZE_TYPE_UNKNOWN;
	priv->size_user_data_type = GS_SIZE_TYPE_UNKNOWN;
	g_mutex_init (&priv->mutex);
}

/**
 * gs_app_new:
 * @id: an application ID, or %NULL, e.g. "org.lingmo.Appstore.desktop"
 *
 * Creates a new application object.
 *
 * The ID should only be set when the application ID (with optional prefix) is
 * known; it is perfectly valid to use gs_app_new() with an @id of %NULL, and
 * then relying on another plugin to set the @id using gs_app_set_id() based on
 * some other information.
 *
 * For instance, a #GsApp is created with no ID when returning results from the
 * packagekit plugin, but with the default source name set as the package name.
 * The source name is read by the appstream plugin, and if matched in the
 * AppStream XML the correct ID is set, along with other higher quality data
 * like the application icon and long description.
 *
 * Returns: a new #GsApp
 *
 * Since: 3.22
 **/
GsApp *
gs_app_new (const gchar *id)
{
	GsApp *app;
	app = g_object_new (GS_TYPE_APP,
			    "id", id,
			    NULL);
	return GS_APP (app);
}

/**
 * gs_app_set_from_unique_id:
 * @app: a #GsApp
 * @unique_id: an application unique ID, e.g.
 *	`system/flatpak/lingmo/desktop/org.lingmo.Appstore.desktop/master`
 *
 * Sets details on an application object.
 *
 * The unique ID will be parsed to set some information in the application such
 * as the scope, bundle kind, id, etc.
 *
 * Since: 3.26
 **/
void
gs_app_set_from_unique_id (GsApp *app, const gchar *unique_id, AsComponentKind kind)
{
	g_auto(GStrv) split = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (unique_id != NULL);

	if (kind != AS_COMPONENT_KIND_UNKNOWN)
		gs_app_set_kind (app, kind);

	split = g_strsplit (unique_id, "/", -1);
	if (g_strv_length (split) != 5)
		return;
	if (g_strcmp0 (split[0], "*") != 0)
		gs_app_set_scope (app, as_component_scope_from_string (split[0]));
	if (g_strcmp0 (split[1], "*") != 0)
		gs_app_set_bundle_kind (app, as_bundle_kind_from_string (split[1]));
	if (g_strcmp0 (split[2], "*") != 0)
		gs_app_set_origin (app, split[2]);
	if (g_strcmp0 (split[3], "*") != 0)
		gs_app_set_id (app, split[3]);
	if (g_strcmp0 (split[4], "*") != 0)
		gs_app_set_branch (app, split[4]);
}

/**
 * gs_app_new_from_unique_id:
 * @unique_id: an application unique ID, e.g.
 *	`system/flatpak/lingmo/desktop/org.lingmo.Appstore.desktop/master`
 *
 * Creates a new application object.
 *
 * The unique ID will be parsed to set some information in the application such
 * as the scope, bundle kind, id, etc. Unlike gs_app_new(), it cannot take a
 * %NULL argument.
 *
 * Returns: a new #GsApp
 *
 * Since: 3.22
 **/
GsApp *
gs_app_new_from_unique_id (const gchar *unique_id)
{
	GsApp *app;
	g_return_val_if_fail (unique_id != NULL, NULL);
	app = gs_app_new (NULL);
	gs_app_set_from_unique_id (app, unique_id, AS_COMPONENT_KIND_UNKNOWN);
	return app;
}

/**
 * gs_app_dup_origin_ui:
 * @app: a #GsApp
 * @with_packaging_format: %TRUE, to include also packaging format
 *
 * Gets the package origin that's suitable for UI use, i.e. the value of
 * #GsApp:origin-ui.
 *
 * Returns: (not nullable) (transfer full): The package origin for UI use
 *
 * Since: 43
 **/
gchar *
gs_app_dup_origin_ui (GsApp *app,
		      gboolean with_packaging_format)
{
	GsAppPrivate *priv;
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GsOsRelease) os_release = NULL;
	const gchar *origin_str = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	/* use the distro name for official packages */
	if (gs_app_has_quirk (app, GS_APP_QUIRK_PROVENANCE) &&
	    gs_app_get_kind (app) != AS_COMPONENT_KIND_REPOSITORY) {
		os_release = gs_os_release_new (NULL);
		if (os_release != NULL) {
			origin_str = gs_os_release_get_vendor_name (os_release);
			if (origin_str == NULL)
				origin_str = gs_os_release_get_name (os_release);
		}
	}

	priv = gs_app_get_instance_private (app);
	locker = g_mutex_locker_new (&priv->mutex);

	if (!origin_str) {
		origin_str = priv->origin_ui;

		if (origin_str == NULL || origin_str[0] == '\0') {
			/* use "Local file" rather than the filename for local files */
			if (gs_app_get_state (app) == GS_APP_STATE_AVAILABLE_LOCAL ||
			    gs_app_get_local_file (app) != NULL)
				origin_str = _("Local file");
			else if (g_strcmp0 (gs_app_get_origin (app), "flathub") == 0)
				origin_str = "Flathub";
			else if (g_strcmp0 (gs_app_get_origin (app), "flathub-beta") == 0)
				origin_str = "Flathub Beta";
			else
				origin_str = gs_app_get_origin (app);
		}
	}

	if (with_packaging_format) {
		g_autofree gchar *packaging_format = NULL;

		packaging_format = gs_app_get_packaging_format (app);

		if (packaging_format) {
			/* TRANSLATORS: the first %s is replaced with an origin name;
			   the second %s is replaced with the packaging format.
			   Example string: "Local file (RPM)" */
			return g_strdup_printf (_("%s (%s)"), origin_str, packaging_format);
		}
	}

	return g_strdup (origin_str);
}

/**
 * gs_app_set_origin_ui:
 * @app: a #GsApp
 * @origin_ui: (not nullable): the new origin UI
 *
 * Set the value of #GsApp:origin-ui.
 */
void
gs_app_set_origin_ui (GsApp *app,
		      const gchar *origin_ui)
{
	GsAppPrivate *priv;
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));

	priv = gs_app_get_instance_private (app);
	locker = g_mutex_locker_new (&priv->mutex);

	if (origin_ui && !*origin_ui)
		origin_ui = NULL;

	if (g_strcmp0 (priv->origin_ui, origin_ui) == 0)
		return;

	g_free (priv->origin_ui);
	priv->origin_ui = g_strdup (origin_ui);
	gs_app_queue_notify (app, obj_props[PROP_ORIGIN_UI]);
}

/**
 * gs_app_get_packaging_format:
 * @app: a #GsApp
 *
 * Gets the packaging format, e.g. 'RPM' or 'Flatpak'.
 *
 * Returns: The packaging format
 *
 * Since: 3.32
 **/
gchar *
gs_app_get_packaging_format (GsApp *app)
{
	AsBundleKind bundle_kind;
	const gchar *bundle_kind_ui;
	const gchar *packaging_format;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	/* does the app have packaging format set? */
	packaging_format = gs_app_get_metadata_item (app, "LingmoAppstore::PackagingFormat");
	if (packaging_format != NULL)
		return g_strdup (packaging_format);

	/* fall back to bundle kind */
	bundle_kind = gs_app_get_bundle_kind (app);
	switch (bundle_kind) {
	case AS_BUNDLE_KIND_UNKNOWN:
		bundle_kind_ui = NULL;
		break;
	case AS_BUNDLE_KIND_LIMBA:
		bundle_kind_ui = "Limba";
		break;
	case AS_BUNDLE_KIND_FLATPAK:
		bundle_kind_ui = "Flatpak";
		break;
	case AS_BUNDLE_KIND_SNAP:
		bundle_kind_ui = "Snap";
		break;
	case AS_BUNDLE_KIND_PACKAGE:
		bundle_kind_ui = _("Package");
		break;
	case AS_BUNDLE_KIND_CABINET:
		bundle_kind_ui = "Cabinet";
		break;
	case AS_BUNDLE_KIND_APPIMAGE:
		bundle_kind_ui = "AppImage";
		break;
	default:
		g_warning ("unhandled bundle kind %s", as_bundle_kind_to_string (bundle_kind));
		bundle_kind_ui = as_bundle_kind_to_string (bundle_kind);
	}

	return g_strdup (bundle_kind_ui);
}

/**
 * gs_app_get_packaging_format_raw:
 * @app: a #GsApp
 *
 * Similar to gs_app_get_packaging_format(), but it does not return a newly
 * allocated string and the value is not suitable for the UI. Depending on
 * the plugin, it can be "deb", "flatpak", "package", "RPM", "snap", ....
 *
 * Returns: The raw value of the packaging format
 *
 * Since: 41
 **/
const gchar *
gs_app_get_packaging_format_raw (GsApp *app)
{
	const gchar *packaging_format;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	packaging_format = gs_app_get_metadata_item (app, "LingmoAppstore::PackagingFormat");
	if (packaging_format != NULL)
		return packaging_format;

	return as_bundle_kind_to_string (gs_app_get_bundle_kind (app));
}

/**
 * gs_app_subsume_metadata:
 * @app: a #GsApp
 * @donor: another #GsApp
 *
 * Copies any metadata from @donor to @app.
 *
 * Since: 3.32
 **/
void
gs_app_subsume_metadata (GsApp *app, GsApp *donor)
{
	GsAppPrivate *priv = gs_app_get_instance_private (donor);
	g_autoptr(GList) keys = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (GS_IS_APP (donor));

	keys = g_hash_table_get_keys (priv->metadata);
	for (GList *l = keys; l != NULL; l = l->next) {
		const gchar *key = l->data;
		GVariant *tmp = gs_app_get_metadata_variant (donor, key);
		if (gs_app_get_metadata_variant (app, key) != NULL)
			continue;
		gs_app_set_metadata_variant (app, key, tmp);
	}
}

/**
 * gs_app_dup_permissions:
 * @app: a #GsApp
 *
 * Get a reference to the @app permissions. The returned value can
 * be %NULL, when the app's permissions are unknown. Free the returned pointer,
 * if not %NULL, with g_object_unref(), when no longer needed.
 *
 * Returns: (nullable) (transfer full): referenced #GsAppPermissions,
 *    or %NULL
 *
 * Since: 43
 **/
GsAppPermissions *
gs_app_dup_permissions (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return priv->permissions ? g_object_ref (priv->permissions) : NULL;
}

/**
 * gs_app_set_permissions:
 * @app: a #GsApp
 * @permissions: (nullable) (transfer none): a #GsAppPermissions, or %NULL
 *
 * Set permissions for the @app. The @permissions is referenced,
 * if not %NULL.
 *
 * Note the @permissions need to be sealed.
 *
 * Since: 43
 **/
void
gs_app_set_permissions (GsApp *app,
			GsAppPermissions *permissions)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (permissions == NULL || gs_app_permissions_is_sealed (permissions));

	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->permissions == permissions)
		return;
	g_clear_object (&priv->permissions);
	if (permissions != NULL)
		priv->permissions = g_object_ref (permissions);
	gs_app_queue_notify (app, obj_props[PROP_PERMISSIONS]);
}

/**
 * gs_app_dup_update_permissions:
 * @app: a #GsApp
 *
 * Get a reference to the update permissions. The returned value can
 * be %NULL, when no update permissions had been set. Free
 * the returned pointer, if not %NULL, with g_object_unref(), when
 * no longer needed.
 *
 * Returns: (nullable) (transfer full): referenced #GsAppPermissions,
 *    or %NULL
 *
 * Since: 43
 **/
GsAppPermissions *
gs_app_dup_update_permissions (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);
	locker = g_mutex_locker_new (&priv->mutex);
	return priv->update_permissions ? g_object_ref (priv->update_permissions) : NULL;
}

/**
 * gs_app_set_update_permissions:
 * @app: a #GsApp
 * @update_permissions: (nullable) (transfer none): a #GsAppPermissions, or %NULL
 *
 * Set update permissions for the @app, that is, the permissions, which change
 * in an update or similar reasons. The @update_permissions is referenced,
 * if not %NULL.
 *
 * Note the @update_permissions need to be sealed.
 *
 * Since: 43
 **/
void
gs_app_set_update_permissions (GsApp *app,
			       GsAppPermissions *update_permissions)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (update_permissions == NULL || gs_app_permissions_is_sealed (update_permissions));
	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->update_permissions != update_permissions) {
		g_clear_object (&priv->update_permissions);
		if (update_permissions != NULL)
			priv->update_permissions = g_object_ref (update_permissions);
	}
}

/**
 * gs_app_get_version_history:
 * @app: a #GsApp
 *
 * Gets the list of past releases for an application (including the latest
 * one).
 *
 * Returns: (element-type AsRelease) (transfer container) (nullable): a list, or
 *     %NULL if the version history is not known
 *
 * Since: 41
 **/
GPtrArray *
gs_app_get_version_history (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);
	if (priv->version_history == NULL)
		return NULL;
	return g_ptr_array_ref (priv->version_history);
}

/**
 * gs_app_set_version_history:
 * @app: a #GsApp
 * @version_history: (element-type AsRelease) (nullable): a set of entries
 *   representing the version history, or %NULL if none are known
 *
 * Set the list of past releases for an application (including the latest one).
 *
 * Since: 40
 **/
void
gs_app_set_version_history (GsApp *app, GPtrArray *version_history)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_return_if_fail (GS_IS_APP (app));

	if (version_history != NULL && version_history->len == 0)
		version_history = NULL;

	locker = g_mutex_locker_new (&priv->mutex);
	_g_set_ptr_array (&priv->version_history, version_history);
}

/**
 * gs_app_ensure_icons_downloaded:
 * @app: a #GsApp
 * @soup_session: a #SoupSession
 * @maximum_icon_size: maximum icon size
 * @cancellable: (nullable): optional #GCancellable object
 *
 * Ensure all remote icons in the @app's icons are locally cached.
 *
 * Since: 41
 **/
void
gs_app_ensure_icons_downloaded (GsApp *app,
				SoupSession *soup_session,
				guint maximum_icon_size,
				GCancellable *cancellable)
{
	GsAppPrivate *priv;
	g_autoptr(GMutexLocker) locker = NULL;
	GPtrArray *icons;
	guint i;

	g_return_if_fail (GS_IS_APP (app));

	priv = gs_app_get_instance_private (app);
	locker = g_mutex_locker_new (&priv->mutex);

	/* process all icons */
	icons = priv->icons;

	for (i = 0; icons != NULL && i < icons->len; i++) {
		GIcon *icon = g_ptr_array_index (icons, i);
		g_autoptr(GError) error_local = NULL;

		/* Only remote icons need to be cached. */
		if (!GS_IS_REMOTE_ICON (icon))
			continue;

		if (!gs_remote_icon_ensure_cached (GS_REMOTE_ICON (icon),
						   soup_session,
						   maximum_icon_size,
						   cancellable,
						   &error_local)) {
			/* we failed, but keep going */
			g_debug ("failed to cache icon for %s: %s",
				 gs_app_get_id (app),
				 error_local->message);
		}
	}
}

/**
 * gs_app_get_relations:
 * @app: a #GsApp
 *
 * Gets the value of #GsApp:relations. %NULL is equivalent to an empty array.
 *
 * The returned array should not be modified.
 *
 * Returns: (transfer container) (element-type AsRelation) (nullable): the value of
 *     #GsApp:relations, or %NULL
 * Since: 41
 */
GPtrArray *
gs_app_get_relations (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_val_if_fail (GS_IS_APP (app), NULL);

	locker = g_mutex_locker_new (&priv->mutex);
	return (priv->relations != NULL) ? g_ptr_array_ref (priv->relations) : NULL;
}

/**
 * gs_app_add_relation:
 * @app: a #GsApp
 * @relation: (transfer none) (not nullable): a new #AsRelation to add to the app
 *
 * Adds @relation to #GsApp:relations. @relation must have all its properties
 * set already.
 *
 * Since: 41
 */
void
gs_app_add_relation (GsApp      *app,
                     AsRelation *relation)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));
	g_return_if_fail (AS_IS_RELATION (relation));

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->relations == NULL)
		priv->relations = g_ptr_array_new_with_free_func (g_object_unref);
	g_ptr_array_add (priv->relations, g_object_ref (relation));

	gs_app_queue_notify (app, obj_props[PROP_RELATIONS]);
}

/**
 * gs_app_set_relations:
 * @app: a #GsApp
 * @relations: (element-type AsRelation) (nullable) (transfer none): a new set
 *     of relations for #GsApp:relations; %NULL represents an empty array
 *
 * Set #GsApp:relations to @relations, replacing its previous value. %NULL is
 * equivalent to an empty array.
 *
 * Since: 41
 */
void
gs_app_set_relations (GsApp     *app,
                      GPtrArray *relations)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;
	g_autoptr(GPtrArray) old_relations = NULL;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (relations == NULL && priv->relations == NULL)
		return;

	if (priv->relations != NULL)
		old_relations = g_steal_pointer (&priv->relations);

	if (relations != NULL)
		priv->relations = g_ptr_array_ref (relations);

	gs_app_queue_notify (app, obj_props[PROP_RELATIONS]);
}

/**
 * gs_app_get_has_translations:
 * @app: a #GsApp
 *
 * Get the value of #GsApp:has-translations.
 *
 * Returns: %TRUE if the app has translation metadata, %FALSE otherwise
 * Since: 41
 */
gboolean
gs_app_get_has_translations (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	return priv->has_translations;
}

/**
 * gs_app_set_has_translations:
 * @app: a #GsApp
 * @has_translations: %TRUE if the app has translation metadata, %FALSE otherwise
 *
 * Set the value of #GsApp:has-translations.
 *
 * Since: 41
 */
void
gs_app_set_has_translations (GsApp    *app,
                             gboolean  has_translations)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->has_translations == has_translations)
		return;

	priv->has_translations = has_translations;
	gs_app_queue_notify (app, obj_props[PROP_HAS_TRANSLATIONS]);
}

/**
 * gs_app_is_downloaded:
 * @app: a #GsApp
 *
 * Returns whether the @app is downloaded for updates or not,
 * considering also its dependencies.
 *
 * Returns: %TRUE, when the @app is downloaded, %FALSE otherwise
 *
 * Since: 43
 **/
gboolean
gs_app_is_downloaded (GsApp *app)
{
	GsSizeType size_type;
	guint64 size_bytes = 0;

	g_return_val_if_fail (GS_IS_APP (app), FALSE);

	if (!gs_app_has_quirk (app, GS_APP_QUIRK_IS_PROXY)) {
		size_type = gs_app_get_size_download (app, &size_bytes);
		if (size_type != GS_SIZE_TYPE_VALID || size_bytes != 0)
			return FALSE;
	}

	size_type = gs_app_get_size_download_dependencies (app, &size_bytes);
	if (size_type != GS_SIZE_TYPE_VALID || size_bytes != 0)
		return FALSE;

	return TRUE;
}

/**
 * gs_app_get_icons_state:
 * @app: a #GsApp
 *
 * Returns the state of the icons of @app.
 *
 * Returns: a #GsAppIconsState
 *
 * Since: 44
 **/
GsAppIconsState
gs_app_get_icons_state (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);

	g_return_val_if_fail (GS_IS_APP (app), GS_APP_ICONS_STATE_UNKNOWN);

	return priv->icons_state;
}

/**
 * gs_app_set_icons_state:
 * @app: a #GsApp
 * @icons_state: a #GsAppIconsState
 *
 * Sets the app icons state of @app.
 *
 * Since: 44
 **/
void
gs_app_set_icons_state (GsApp           *app,
                        GsAppIconsState  icons_state)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_autoptr(GMutexLocker) locker = NULL;

	g_return_if_fail (GS_IS_APP (app));

	locker = g_mutex_locker_new (&priv->mutex);

	if (priv->icons_state == icons_state)
		return;

	priv->icons_state = icons_state;
	gs_app_queue_notify (app, obj_props[PROP_ICONS_STATE]);
}

/**
 * gs_app_is_application:
 * @app: a #GsApp
 *
 * Returns whether the @app is an application, not a "generic" software.
 *
 * Returns: whether the @app is an application, not a "generic" software
 *
 * Since: 45
 **/
gboolean
gs_app_is_application (GsApp *app)
{
	GsAppPrivate *priv = gs_app_get_instance_private (app);
	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	return priv->kind == AS_COMPONENT_KIND_DESKTOP_APP ||
	       priv->kind == AS_COMPONENT_KIND_CONSOLE_APP ||
	       priv->kind == AS_COMPONENT_KIND_WEB_APP;
}
