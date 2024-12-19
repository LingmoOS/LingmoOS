/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016-2018 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <json-glib/json-glib.h>
#include <lingmo-appstore.h>

#include "gs-plugin-fedora-pkgdb-collections.h"

/*
 * SECTION:
 * Queries the list of Fedora package collections.
 *
 * This plugin downloads a file and performs some basic parsing on it. It
 * executes entirely in the main thread, and therefore does not require any
 * locking.
 */

#define FEDORA_PKGDB_COLLECTIONS_API_URI "https://admin.fedoraproject.org/pkgdb/api/collections/"

struct _GsPluginFedoraPkgdbCollections {
	GsPlugin	 parent;

	/* Only set at setup time, then read only: */
	gchar		*cachefn;  /* (owned) (not nullable) */
	GFileMonitor	*cachefn_monitor;  /* (owned) (not nullable) */
	gchar		*os_name;  /* (owned) (not nullable) */
	guint64		 os_version;
	GsApp		*cached_origin;  /* (owned) (not nullable) */
	GSettings	*settings;  /* (owned) (not nullable) */

	/* Contents may vary throughout the plugin’s lifetime: */
	gboolean	 is_valid;
	GPtrArray	*distros;  /* (owned) (not nullable) (element-type PkgdbItem) */
};

G_DEFINE_TYPE (GsPluginFedoraPkgdbCollections, gs_plugin_fedora_pkgdb_collections, GS_TYPE_PLUGIN)

typedef enum {
	PKGDB_ITEM_STATUS_ACTIVE,
	PKGDB_ITEM_STATUS_DEVEL,
	PKGDB_ITEM_STATUS_EOL,
	PKGDB_ITEM_STATUS_LAST
} PkgdbItemStatus;

typedef struct {
	gchar			*name;
	PkgdbItemStatus		 status;
	guint			 version;
} PkgdbItem;

static void
_pkgdb_item_free (PkgdbItem *item)
{
	g_free (item->name);
	g_slice_free (PkgdbItem, item);
}

static void
gs_plugin_fedora_pkgdb_collections_init (GsPluginFedoraPkgdbCollections *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	/* check that we are running on Fedora */
	if (!gs_plugin_check_distro_id (plugin, "fedora")) {
		gs_plugin_set_enabled (plugin, FALSE);
		g_debug ("disabling itself as we're not Fedora");
		return;
	}
	self->distros = g_ptr_array_new_with_free_func ((GDestroyNotify) _pkgdb_item_free);
	self->settings = g_settings_new ("org.lingmo.appstore");

	/* old name */
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_CONFLICTS, "fedora-distro-upgrades");
}

static void
gs_plugin_fedora_pkgdb_collections_dispose (GObject *object)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (object);

	g_clear_object (&self->cachefn_monitor);
	g_clear_object (&self->cached_origin);
	g_clear_object (&self->settings);

	G_OBJECT_CLASS (gs_plugin_fedora_pkgdb_collections_parent_class)->dispose (object);
}

static void
gs_plugin_fedora_pkgdb_collections_finalize (GObject *object)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (object);

	g_clear_pointer (&self->distros, g_ptr_array_unref);
	g_clear_pointer (&self->os_name, g_free);
	g_clear_pointer (&self->cachefn, g_free);

	G_OBJECT_CLASS (gs_plugin_fedora_pkgdb_collections_parent_class)->finalize (object);
}

/* Runs in the main thread. */
static void
_file_changed_cb (GFileMonitor *monitor,
		  GFile *file, GFile *other_file,
		  GFileMonitorEvent event_type,
		  gpointer user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (user_data);

	g_debug ("cache file changed, so reloading upgrades list");
	gs_plugin_updates_changed (GS_PLUGIN (self));

	self->is_valid = FALSE;
}

static void
gs_plugin_fedora_pkgdb_collections_setup_async (GsPlugin            *plugin,
                                                GCancellable        *cancellable,
                                                GAsyncReadyCallback  callback,
                                                gpointer             user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (plugin);
	const gchar *verstr = NULL;
	gchar *endptr = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GsOsRelease) os_release = NULL;
	g_autoptr(GTask) task = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fedora_pkgdb_collections_setup_async);

	/* get the file to cache */
	self->cachefn = gs_utils_get_cache_filename ("fedora-pkgdb-collections",
						     "fedora.json",
						     GS_UTILS_CACHE_FLAG_WRITEABLE |
						     GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
						     &local_error);
	if (self->cachefn == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* watch this in case it is changed by the user */
	file = g_file_new_for_path (self->cachefn);
	self->cachefn_monitor = g_file_monitor (file,
						G_FILE_MONITOR_NONE,
						cancellable,
						&local_error);
	if (self->cachefn_monitor == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_signal_connect (self->cachefn_monitor, "changed",
			  G_CALLBACK (_file_changed_cb), plugin);

	/* read os-release for the current versions */
	os_release = gs_os_release_new (&local_error);
	if (os_release == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	self->os_name = g_strdup (gs_os_release_get_name (os_release));
	if (self->os_name == NULL) {
		g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_INVALID_FORMAT,
					 "OS release had no name");
		return;
	}

	verstr = gs_os_release_get_version_id (os_release);
	if (verstr == NULL) {
		g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_INVALID_FORMAT,
					 "OS release had no version ID");
		return;
	}

	/* parse the version */
	self->os_version = g_ascii_strtoull (verstr, &endptr, 10);
	if (endptr == verstr || self->os_version > G_MAXUINT) {
		g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_INVALID_FORMAT,
					 "Failed parse VERSION_ID: %s", verstr);
		return;
	}

	/* add source */
	self->cached_origin = gs_app_new (gs_plugin_get_name (plugin));
	gs_app_set_kind (self->cached_origin, AS_COMPONENT_KIND_REPOSITORY);
	gs_app_set_origin_hostname (self->cached_origin,
				    FEDORA_PKGDB_COLLECTIONS_API_URI);
	gs_app_set_management_plugin (self->cached_origin, plugin);

	/* add the source to the plugin cache which allows us to match the
	 * unique ID to a GsApp when creating an event */
	gs_plugin_cache_add (plugin,
			     gs_app_get_unique_id (self->cached_origin),
			     self->cached_origin);

	/* success */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fedora_pkgdb_collections_setup_finish (GsPlugin      *plugin,
                                                 GAsyncResult  *result,
                                                 GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void download_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data);

static void
_refresh_cache_async (GsPluginFedoraPkgdbCollections *self,
                      guint64                         cache_age_secs,
                      GCancellable                   *cancellable,
                      GAsyncReadyCallback             callback,
                      gpointer                        user_data)
{
	GsPlugin *plugin = GS_PLUGIN (self);
	g_autoptr(GsApp) app_dl = gs_app_new (gs_plugin_get_name (plugin));
	g_autoptr(GTask) task = NULL;
	g_autoptr(GFile) output_file = g_file_new_for_path (self->cachefn);
	g_autoptr(SoupSession) soup_session = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, _refresh_cache_async);

	/* check cache age */
	if (cache_age_secs > 0) {
		guint64 tmp = gs_utils_get_file_age (output_file);
		if (tmp < cache_age_secs) {
			g_debug ("%s is only %" G_GUINT64_FORMAT " seconds old",
				 self->cachefn, tmp);
			g_task_return_boolean (task, TRUE);
			return;
		}
	}

	/* download new file */
	gs_app_set_summary_missing (app_dl,
				    /* TRANSLATORS: status text when downloading */
				    _("Downloading upgrade information…"));

	soup_session = gs_build_soup_session ();

	gs_download_file_async (soup_session,
				FEDORA_PKGDB_COLLECTIONS_API_URI,
				output_file,
				G_PRIORITY_LOW,
				NULL, NULL,  /* FIXME: progress reporting */
				cancellable,
				download_cb,
				g_steal_pointer (&task));
}

static void
download_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	SoupSession *soup_session = SOUP_SESSION (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFedoraPkgdbCollections *self = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	if (!gs_download_file_finish (soup_session, result, &local_error) &&
	    !g_error_matches (local_error, GS_DOWNLOAD_ERROR, GS_DOWNLOAD_ERROR_NOT_MODIFIED)) {
		g_autoptr(GError) wrapped_error = NULL;

		/* Wrap in a GsPluginError. */
		g_set_error_literal (&wrapped_error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_DOWNLOAD_FAILED,
				     local_error->message);

		gs_utils_error_add_origin_id (&wrapped_error, self->cached_origin);
		g_task_return_error (task, g_steal_pointer (&wrapped_error));
		return;
	}

	/* success */
	self->is_valid = FALSE;

	g_task_return_boolean (task, TRUE);
}

static gboolean
_refresh_cache_finish (GsPluginFedoraPkgdbCollections  *self,
                       GAsyncResult                    *result,
                       GError                         **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_fedora_pkgdb_collections_refresh_metadata_async (GsPlugin                     *plugin,
                                                           guint64                       cache_age_secs,
                                                           GsPluginRefreshMetadataFlags  flags,
                                                           GCancellable                 *cancellable,
                                                           GAsyncReadyCallback           callback,
                                                           gpointer                      user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (plugin);
	_refresh_cache_async (self, cache_age_secs, cancellable, callback, user_data);
}

static gboolean
gs_plugin_fedora_pkgdb_collections_refresh_metadata_finish (GsPlugin      *plugin,
                                                            GAsyncResult  *result,
                                                            GError       **error)
{
	return _refresh_cache_finish (GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (plugin),
				      result,
				      error);
}

static gchar *
_get_upgrade_css_background (guint version)
{
	g_autofree gchar *version_str = g_strdup_printf ("%u", version);
	g_autofree gchar *filename0 = NULL;
	g_autofree gchar *filename1 = NULL;
	g_autofree gchar *filename2 = NULL;

	/* Check the standard location. */
	filename0 = gs_utils_get_upgrade_background (version_str);
	if (filename0 != NULL)
		return g_strdup_printf ("url('file://%s')", filename0);

	/* Fedora-specific locations. Deprecated. */
	filename1 = g_strdup_printf ("/usr/share/backgrounds/f%u/default/standard/f%u.png", version, version);
	if (g_file_test (filename1, G_FILE_TEST_EXISTS))
		return g_strdup_printf ("url('file://%s')", filename1);

	filename2 = g_strdup_printf ("/usr/share/lingmo-appstore/backgrounds/f%u.png", version);
	if (g_file_test (filename2, G_FILE_TEST_EXISTS))
		return g_strdup_printf ("url('file://%s')", filename2);

	return NULL;
}

static gint
_sort_items_cb (gconstpointer a, gconstpointer b)
{
	PkgdbItem *item_a = *((PkgdbItem **) a);
	PkgdbItem *item_b = *((PkgdbItem **) b);

	if (item_a->version > item_b->version)
		return 1;
	if (item_a->version < item_b->version)
		return -1;
	return 0;
}

static GsApp *
_create_upgrade_from_info (GsPluginFedoraPkgdbCollections *self,
                           PkgdbItem                      *item)
{
	GsApp *app;
	g_autofree gchar *app_id = NULL;
	g_autofree gchar *app_version = NULL;
	g_autofree gchar *background = NULL;
	g_autofree gchar *cache_key = NULL;
	g_autofree gchar *css = NULL;
	g_autofree gchar *url = NULL;
	g_autoptr(GFile) icon_file = NULL;
	g_autoptr(GIcon) ic = NULL;

	/* search in the cache */
	cache_key = g_strdup_printf ("release-%u", item->version);
	app = gs_plugin_cache_lookup (GS_PLUGIN (self), cache_key);
	if (app != NULL)
		return app;

	app_id = g_strdup_printf ("org.fedoraproject.fedora-%u", item->version);
	app_version = g_strdup_printf ("%u", item->version);

	/* icon from disk */
	icon_file = g_file_new_for_path ("/usr/share/pixmaps/fedora-logo-sprite.png");
	ic = g_file_icon_new (icon_file);

	/* create */
	app = gs_app_new (app_id);
	if (item->status == PKGDB_ITEM_STATUS_EOL)
		gs_app_set_state (app, GS_APP_STATE_UNAVAILABLE);
	else
		gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
	gs_app_set_kind (app, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, item->name);
	gs_app_set_summary (app, GS_APP_QUALITY_LOWEST,
			    /* TRANSLATORS: this is a title for Fedora distro upgrades */
			    _("Upgrade for the latest features, performance and stability improvements."));
	gs_app_set_version (app, app_version);
	gs_app_set_size_installed (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_app_set_license (app, GS_APP_QUALITY_LOWEST, "LicenseRef-free");
	gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
	gs_app_add_quirk (app, GS_APP_QUIRK_PROVENANCE);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_REVIEWABLE);
	gs_app_add_icon (app, ic);

	/* show a Fedora magazine article for the release */
	url = g_strdup_printf ("https://fedoramagazine.org/whats-new-fedora-workstation-%u",
			       item->version);
	gs_app_set_url (app, AS_URL_KIND_HOMEPAGE, url);

	/* use a fancy background if possible, and suppress the border which is
	 * shown by default; the background image is designed to be borderless */
	background = _get_upgrade_css_background (item->version);
	if (background != NULL) {
		css = g_strdup_printf ("background: %s;"
				       "background-position: top;"
				       "background-size: 100%% 100%%;"
				       "color: white;"
				       "border-width: 0;",
				       background);
		gs_app_set_metadata (app, "LingmoAppstore::UpgradeBanner-css", css);
	}

	/* save in the cache */
	gs_plugin_cache_add (GS_PLUGIN (self), cache_key, app);

	/* success */
	return app;
}

static gboolean
_is_valid_upgrade (GsPluginFedoraPkgdbCollections *self,
                   PkgdbItem                      *item)
{
	/* only interested in upgrades to the same distro */
	if (g_strcmp0 (item->name, self->os_name) != 0)
		return FALSE;

	/* only interested in newer versions, but not more than N+2 */
	if (item->version <= self->os_version ||
	    item->version > self->os_version + 2)
		return FALSE;

	/* ignore End-Of-Life upgrades */
	if (item->status == PKGDB_ITEM_STATUS_EOL)
		return FALSE;

	/* only interested in non-devel distros */
	if (!g_settings_get_boolean (self->settings, "show-upgrade-prerelease")) {
		if (item->status == PKGDB_ITEM_STATUS_DEVEL)
			return FALSE;
	}

	/* success */
	return TRUE;
}

static GPtrArray *
load_json (GsPluginFedoraPkgdbCollections  *self,
           GError                         **error)
{
	JsonArray *collections;
	JsonNode *root_node;
	JsonObject *root = NULL;
	g_autoptr(JsonParser) parser = NULL;
	g_autoptr(GPtrArray) new_distros = NULL;

	new_distros = g_ptr_array_new_with_free_func ((GDestroyNotify) _pkgdb_item_free);
	parser = json_parser_new_immutable ();

	if (!json_parser_load_from_mapped_file (parser, self->cachefn, error))
		return NULL;

	root_node = json_parser_get_root (parser);
	if (root_node != NULL && JSON_NODE_HOLDS_OBJECT (root_node))
		root = json_node_get_object (root_node);
	if (root == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "no root object");
		return NULL;
	}

	collections = json_object_get_array_member (root, "collections");
	if (collections == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "no collections object");
		return NULL;
	}

	for (guint i = 0; i < json_array_get_length (collections); i++) {
		PkgdbItem *item;
		JsonObject *collection;
		PkgdbItemStatus status;
		const gchar *name;
		const gchar *status_str;
		const gchar *version_str;
		gchar *endptr = NULL;
		guint64 version;

		collection = json_array_get_object_element (collections, i);
		if (collection == NULL)
			continue;

		name = json_object_get_string_member (collection, "name");
		if (name == NULL)
			continue;

		status_str = json_object_get_string_member (collection, "status");
		if (status_str == NULL)
			continue;

		if (g_strcmp0 (status_str, "Active") == 0)
			status = PKGDB_ITEM_STATUS_ACTIVE;
		else if (g_strcmp0 (status_str, "Under Development") == 0)
			status = PKGDB_ITEM_STATUS_DEVEL;
		else if (g_strcmp0 (status_str, "EOL") == 0)
			status = PKGDB_ITEM_STATUS_EOL;
		else
			continue;

		version_str = json_object_get_string_member (collection, "version");
		if (version_str == NULL)
			continue;

		version = g_ascii_strtoull (version_str, &endptr, 10);
		if (endptr == version_str || version > G_MAXUINT)
			continue;

		/* add item */
		item = g_slice_new0 (PkgdbItem);
		item->name = g_strdup (name);
		item->status = status;
		item->version = (guint) version;
		g_ptr_array_add (new_distros, item);
	}

	/* ensure in correct order */
	g_ptr_array_sort (new_distros, _sort_items_cb);

	/* success */
	g_clear_pointer (&self->distros, g_ptr_array_unref);
	self->distros = g_ptr_array_ref (new_distros);
	self->is_valid = TRUE;

	return g_steal_pointer (&new_distros);
}

static void ensure_refresh_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);

/* This will return a strong reference to the latest distros
 * #GPtrArray. The caller should use this in their computation. */
static void
_ensure_cache_async (GsPluginFedoraPkgdbCollections *self,
                     GCancellable                   *cancellable,
                     GAsyncReadyCallback             callback,
                     gpointer                        user_data)
{
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, _ensure_cache_async);

	/* already done */
	if (self->is_valid) {
		g_task_return_pointer (task, g_ptr_array_ref (self->distros), (GDestroyNotify) g_ptr_array_unref);
		return;
	}

	/* Ensure there is any data, no matter how old. This can download from
	 * the network if needed. */
	_refresh_cache_async (self, G_MAXUINT, cancellable,
			      ensure_refresh_cb, g_steal_pointer (&task));
}

static void
ensure_refresh_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GPtrArray) distros = NULL;
	g_autoptr(GError) local_error = NULL;

	if (!_refresh_cache_finish (self, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	distros = load_json (self, &local_error);
	if (distros == NULL) {
		g_autoptr(GFile) cache_file = g_file_new_for_path (self->cachefn);

		g_debug ("Failed to load cache file ‘%s’, deleting it", self->cachefn);
		g_file_delete (cache_file, NULL, NULL);

		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_pointer (task, g_steal_pointer (&distros), (GDestroyNotify) g_ptr_array_unref);
}

static GPtrArray *
_ensure_cache_finish (GsPluginFedoraPkgdbCollections  *self,
                      GAsyncResult                    *result,
                      GError                         **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static void list_distro_upgrades_cb (GObject      *source_object,
                                     GAsyncResult *result,
                                     gpointer      user_data);

static void
gs_plugin_fedora_pkgdb_collections_list_distro_upgrades_async (GsPlugin                        *plugin,
                                                               GsPluginListDistroUpgradesFlags  flags,
                                                               GCancellable                    *cancellable,
                                                               GAsyncReadyCallback              callback,
                                                               gpointer                         user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fedora_pkgdb_collections_list_distro_upgrades_async);

	/* Ensure valid data is loaded. */
	_ensure_cache_async (self, cancellable, list_distro_upgrades_cb, g_steal_pointer (&task));
}

static void
list_distro_upgrades_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GPtrArray) distros = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) local_error = NULL;

	distros = _ensure_cache_finish (self, result, &local_error);
	if (distros == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* are any distros upgradable */
	list = gs_app_list_new ();

	for (guint i = 0; i < distros->len; i++) {
		PkgdbItem *item = g_ptr_array_index (distros, i);
		if (_is_valid_upgrade (self, item)) {
			g_autoptr(GsApp) app = NULL;
			app = _create_upgrade_from_info (self, item);
			gs_app_list_add (list, app);
		}
	}

	g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
}

static GsAppList *
gs_plugin_fedora_pkgdb_collections_list_distro_upgrades_finish (GsPlugin      *plugin,
                                                                GAsyncResult  *result,
                                                                GError       **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static gboolean
refine_app (GsPluginFedoraPkgdbCollections *self,
            GPtrArray                      *distros,
            GsApp                          *app,
            GsPluginRefineFlags             flags,
            GCancellable                   *cancellable,
            GError                        **error)
{
	PkgdbItem *item = NULL;
	guint64 app_version = 0;

	/* not for us */
	if (gs_app_get_kind (app) != AS_COMPONENT_KIND_OPERATING_SYSTEM)
		return TRUE;

	if (gs_app_get_version (app) != NULL)
		app_version = g_ascii_strtoull (gs_app_get_version (app), NULL, 10);

	/* system updates and system upgrades are the same kind, only different version */
	if (app_version == 0 || app_version == self->os_version)
		return TRUE;

	/* find item */
	for (guint i = 0; i < distros->len; i++) {
		item = g_ptr_array_index (distros, i);
		if (item->version == self->os_version &&
		    g_ascii_strcasecmp (item->name, self->os_name) == 0)
			break;
		item = NULL;
	}

	/* no information for this release */
	if (item == NULL)
		return TRUE;

	/* fix the state */
	if (item->status == PKGDB_ITEM_STATUS_EOL)
		gs_app_set_state (app, GS_APP_STATE_UNAVAILABLE);

	return TRUE;
}

static void refine_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data);

static void
gs_plugin_fedora_pkgdb_collections_refine_async (GsPlugin            *plugin,
                                                 GsAppList           *list,
                                                 GsPluginRefineFlags  flags,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean refine_needed = FALSE;

	task = gs_plugin_refine_data_new_task (plugin, list, flags, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fedora_pkgdb_collections_refine_async);

	/* Check if any of the apps actually need to be refined by this plugin,
	 * before potentially updating the collections file from the internet. */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_OPERATING_SYSTEM) {
			refine_needed = TRUE;
			break;
		}
	}

	if (!refine_needed) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* ensure valid data is loaded */
	_ensure_cache_async (self, cancellable, refine_cb, g_steal_pointer (&task));
}

static void
refine_cb (GObject      *source_object,
           GAsyncResult *result,
           gpointer      user_data)
{
	GsPluginFedoraPkgdbCollections *self = GS_PLUGIN_FEDORA_PKGDB_COLLECTIONS (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginRefineData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GPtrArray) distros = NULL;
	g_autoptr(GError) local_error = NULL;

	distros = _ensure_cache_finish (self, result, &local_error);
	if (distros == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	for (guint i = 0; i < gs_app_list_length (data->list); i++) {
		GsApp *app = gs_app_list_index (data->list, i);
		if (!refine_app (self, distros, app, data->flags, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fedora_pkgdb_collections_refine_finish (GsPlugin      *plugin,
                                                  GAsyncResult  *result,
                                                  GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_fedora_pkgdb_collections_class_init (GsPluginFedoraPkgdbCollectionsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_fedora_pkgdb_collections_dispose;
	object_class->finalize = gs_plugin_fedora_pkgdb_collections_finalize;

	plugin_class->setup_async = gs_plugin_fedora_pkgdb_collections_setup_async;
	plugin_class->setup_finish = gs_plugin_fedora_pkgdb_collections_setup_finish;
	plugin_class->refine_async = gs_plugin_fedora_pkgdb_collections_refine_async;
	plugin_class->refine_finish = gs_plugin_fedora_pkgdb_collections_refine_finish;
	plugin_class->refresh_metadata_async = gs_plugin_fedora_pkgdb_collections_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_fedora_pkgdb_collections_refresh_metadata_finish;
	plugin_class->list_distro_upgrades_async = gs_plugin_fedora_pkgdb_collections_list_distro_upgrades_async;
	plugin_class->list_distro_upgrades_finish = gs_plugin_fedora_pkgdb_collections_list_distro_upgrades_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_FEDORA_PKGDB_COLLECTIONS;
}
