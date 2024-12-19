/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2024 Jonathan Kang <jonathankang@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <config.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <json-glib/json-glib.h>
#include <lingmo-appstore.h>

#include "gs-plugin-opensuse-distro-upgrade.h"

#define OPENSUSE_DISTRO_UPGRADE_API_URI "https://get.opensuse.org/api/v0/distributions.json"

struct _GsPluginOpensuseDistroUpgrade {
	GsPlugin Parent;

	gchar *os_name;
	gchar *os_version;
	guint64 upgrade_weight;

	gchar *cachefn;
	GFileMonitor *cachefn_monitor;
	GsApp *cached_origin;
	gboolean is_valid;
	GPtrArray *distros;
};

typedef enum {
	DISTRO_UPGRADE_ITEM_STATE_ALPHA,
	DISTRO_UPGRADE_ITEM_STATE_BETA,
	DISTRO_UPGRADE_ITEM_STATE_STABLE,
	DISTRO_UPGRADE_ITEM_STATE_EOL,
	DISTRO_UPGRADE_ITEM_STATE_LAST,
} DistroUpgradeItemState;

typedef struct {
	gchar *name;
	gchar *version;
	DistroUpgradeItemState state;
	guint upgrade_weight;
} DistroUpgradeItem;

G_DEFINE_TYPE (GsPluginOpensuseDistroUpgrade, gs_plugin_opensuse_distro_upgrade, GS_TYPE_PLUGIN)

static gboolean
is_valid_upgrade (GsPluginOpensuseDistroUpgrade *self,
                  DistroUpgradeItem *item)
{
	guint diff;

	diff = item->upgrade_weight - self->upgrade_weight;
	if (diff == 1)
		return TRUE;
	else
		return FALSE;
}

static GsApp *
create_upgrade_app (GsPluginOpensuseDistroUpgrade *self,
                    DistroUpgradeItem             *item)
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

	cache_key = g_strdup_printf ("leap-%s", item->version);
	app = gs_plugin_cache_lookup (GS_PLUGIN (self), cache_key);
	if (app != NULL)
		return app;

	app_id = g_strdup_printf ("org.openSUSE.Leap-%s", item->version);
	app_version = g_strdup (item->version);

	/* icon from disk */
	icon_file = g_file_new_for_path ("/usr/share/pixmaps/distribution-logos/square-symbolic.svg");
	ic = g_file_icon_new (icon_file);

	app = gs_app_new (app_id);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
	gs_app_set_kind (app, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, item->name);
	gs_app_set_summary (app, GS_APP_QUALITY_NORMAL,
			    _("Upgrade for the latest features, performance and stability improvements."));
	gs_app_set_version (app, app_version);
	gs_app_set_size_installed (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_app_set_size_download (app, GS_SIZE_TYPE_UNKNOWABLE, 0);
	gs_app_set_license (app, GS_APP_QUALITY_LOWEST, "LicenseRef-free");
	gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);
	gs_app_add_quirk (app, GS_APP_QUIRK_PROVENANCE);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_REVIEWABLE);
	gs_app_add_icon (app, ic);

	/* Save it in the cache. */
	gs_plugin_cache_add (GS_PLUGIN (self), cache_key, app);

	return app;
}

static void
distro_upgrade_item_destroy (DistroUpgradeItem *item)
{
	g_free (item->name);
	g_free (item->version);
	g_free (item);
}

static GPtrArray *
load_json (GsPluginOpensuseDistroUpgrade  *self,
           GError                        **error)
{
	JsonArray *distros;
	JsonNode *root_node;
	JsonObject *root = NULL;
	g_autoptr(JsonParser) parser = NULL;
	g_autoptr(GPtrArray) new_distros = NULL;

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

	distros = json_object_get_array_member (root, "Leap");
	if (distros == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "No array member named \"Leap\".");
		return NULL;
	}

	new_distros = g_ptr_array_new_with_free_func ((GDestroyNotify) distro_upgrade_item_destroy);
	for (guint i = 0; i < json_array_get_length (distros); i++) {
		DistroUpgradeItem *item;
		DistroUpgradeItemState state;
		JsonObject *distro;
		const gchar *name;
		const gchar *version;
		const gchar *state_str;
		guint upgrade_weight;

		distro = json_array_get_object_element (distros, i);
		if (distro == NULL)
			continue;

		name = json_object_get_string_member (distro, "name");
		if (name == NULL)
			continue;

		version = json_object_get_string_member (distro, "version");
		if (version == NULL)
			continue;

		state_str = json_object_get_string_member (distro, "state");

		if (state_str == NULL)
			continue;
		if (g_strcmp0 (state_str, "Alpha") == 0)
			state = DISTRO_UPGRADE_ITEM_STATE_ALPHA;
		else if (g_strcmp0 (state_str, "Beta") == 0)
			state = DISTRO_UPGRADE_ITEM_STATE_BETA;
		else if (g_strcmp0 (state_str, "Stable") == 0)
			state = DISTRO_UPGRADE_ITEM_STATE_STABLE;
		else if (g_strcmp0 (state_str, "EOL") == 0)
			state = DISTRO_UPGRADE_ITEM_STATE_EOL;
		else
			continue;

		/* Do not care about versions that are end-of-life. */
		if (state == DISTRO_UPGRADE_ITEM_STATE_EOL)
			continue;

		upgrade_weight = json_object_get_int_member (distro, "upgrade-weight");

		/* Set upgrade weight for current OS. */
		if (g_strcmp0 (self->os_version, version) == 0)
			self->upgrade_weight = upgrade_weight;

		/* add item */
		item = g_new0 (DistroUpgradeItem, 1);
		item->name = g_strdup (name);
		item->version = g_strdup (version);
		item->state = state;
		item->upgrade_weight = upgrade_weight;
		g_ptr_array_add (new_distros, item);
	}

	/* success */
	self->is_valid = TRUE;

	return g_steal_pointer (&new_distros);
}

static void
download_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	SoupSession *soup_session = SOUP_SESSION (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginOpensuseDistroUpgrade *self = g_task_get_source_object (task);
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

	/* Distro upgrade list is not yet prepared. */
	self->is_valid = FALSE;

	g_task_return_boolean (task, TRUE);
}

static void
refresh_cache_async (GsPluginOpensuseDistroUpgrade *self,
                     guint64                        cache_age_secs,
                     GCancellable                  *cancellable,
                     GAsyncReadyCallback            callback,
                     gpointer                       user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(GFile) output_file = g_file_new_for_path (self->cachefn);
	g_autoptr(SoupSession) soup_session = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, refresh_cache_async);

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
	soup_session = gs_build_soup_session ();

	gs_download_file_async (soup_session,
				OPENSUSE_DISTRO_UPGRADE_API_URI,
				output_file,
				G_PRIORITY_LOW,
				NULL, NULL,  /* FIXME: progress reporting */
				cancellable,
				download_cb,
				g_steal_pointer (&task));
}

static gboolean
refresh_cache_finish (GsPluginOpensuseDistroUpgrade  *self,
                      GAsyncResult                   *result,
                      GError                        **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
ensure_refresh_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GPtrArray) distros = NULL;
	g_autoptr(GError) local_error = NULL;

	if (!refresh_cache_finish (self, result, &local_error)) {
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

/* This will return a strong reference to the latest distros
 * #GPtrArray. The caller should use this in their computation. */
static void
ensure_cache_async (GsPluginOpensuseDistroUpgrade *self,
                    GCancellable                  *cancellable,
                    GAsyncReadyCallback            callback,
                    gpointer                       user_data)
{
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, ensure_cache_async);

	/* already done */
	if (self->is_valid) {
		g_task_return_pointer (task, g_ptr_array_ref (self->distros), (GDestroyNotify) g_ptr_array_unref);
		return;
	}

	/* Ensure there is any data, no matter how old. This can download from
	 * the network if needed. */
	refresh_cache_async (self, G_MAXUINT, cancellable,
			     ensure_refresh_cb, g_steal_pointer (&task));
}

static GPtrArray *
ensure_cache_finish (GsPluginOpensuseDistroUpgrade  *self,
                     GAsyncResult                    *result,
                     GError                         **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static void
list_distro_upgrades_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data)
{
	gboolean show_prerelease;
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(GPtrArray) distros = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GSettings) settings = NULL;
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (source_object);

	/* Only interested in stable versions. */
	settings = g_settings_new ("org.lingmo.appstore");
	show_prerelease = g_settings_get_boolean (settings, "show-upgrade-prerelease");

	distros = ensure_cache_finish (self, result, &local_error);
	if (distros == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_clear_pointer (&self->distros, g_ptr_array_unref);
	self->distros = g_ptr_array_ref (distros);

	/* are any distros upgradable */
	list = gs_app_list_new ();

	for (guint i = 0; i < distros->len; i++) {
		DistroUpgradeItem *item;

		item = g_ptr_array_index (distros, i);
		if (show_prerelease ||
		    (!show_prerelease && item->state == DISTRO_UPGRADE_ITEM_STATE_STABLE))
			if (is_valid_upgrade (self, item)) {
				g_autoptr(GsApp) app = NULL;

				app = create_upgrade_app (self, item);
				gs_app_list_add (list, app);

				break;
			}
	}

	g_task_return_pointer (task, g_steal_pointer (&list), g_object_unref);
}

static void
gs_plugin_opensuse_distro_upgrade_list_distro_upgrades_async (GsPlugin *plugin,
                                                              GsPluginListDistroUpgradesFlags flags,
                                                              GCancellable *cancellable,
                                                              GAsyncReadyCallback callback,
                                                              gpointer user_data)
{
	g_autoptr(GTask) task = NULL;
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (plugin);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_opensuse_distro_upgrade_list_distro_upgrades_async);

	/* Ensure valid data is loaded. */
	ensure_cache_async (self, cancellable, list_distro_upgrades_cb, g_steal_pointer (&task));
}

static GsAppList *
gs_plugin_opensuse_distro_upgrade_list_distro_upgrades_finish (GsPlugin *plugin,
                                                               GAsyncResult *result,
                                                               GError **error)
{
	return g_task_propagate_pointer (G_TASK (result), error);
}

static void
gs_plugin_opensuse_distro_upgrade_dispose (GObject *object)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (object);

	g_clear_object (&self->cachefn_monitor);
	g_clear_object (&self->cached_origin);

	G_OBJECT_CLASS (gs_plugin_opensuse_distro_upgrade_parent_class)->dispose (object);
}

static void
gs_plugin_opensuse_distro_upgrade_finalize (GObject *object)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (object);

	g_clear_pointer (&self->distros, g_ptr_array_unref);
	g_clear_pointer (&self->os_name, g_free);
	g_clear_pointer (&self->os_version, g_free);
	g_clear_pointer (&self->cachefn, g_free);

	G_OBJECT_CLASS (gs_plugin_opensuse_distro_upgrade_parent_class)->finalize (object);
}

static void
file_changed_cb (GFileMonitor *monitor,
                 GFile *file,
                 GFile *other_file,
                 GFileMonitorEvent event_type,
                 gpointer user_data)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (user_data);

	self->is_valid = FALSE;

	g_debug ("cache file changed, so reloading upgrades list");
	gs_plugin_updates_changed (GS_PLUGIN (self));
}

static void
gs_plugin_opensuse_distro_upgrade_setup_async (GsPlugin *plugin,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer user_data)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (plugin);
	g_autoptr(GFile) file = NULL;
	g_autoptr(GsOsRelease) os_release = NULL;
	g_autoptr(GTask) task = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_opensuse_distro_upgrade_setup_async);

	/* get the file to cache */
	self->cachefn = gs_utils_get_cache_filename ("opensuse-distro-upgrade",
						     "distributions.json",
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
			  G_CALLBACK (file_changed_cb), plugin);

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
	self->os_version = g_strdup (gs_os_release_get_version_id (os_release));
        if (self->os_version == NULL) {
                g_task_return_new_error (task, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_INVALID_FORMAT,
					 "OS release had no version ID");
		return;
        }

	/* add source */
	self->cached_origin = gs_app_new (gs_plugin_get_name (plugin));
	gs_app_set_kind (self->cached_origin, AS_COMPONENT_KIND_REPOSITORY);
	gs_app_set_origin_hostname (self->cached_origin,
				    OPENSUSE_DISTRO_UPGRADE_API_URI);
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
gs_plugin_opensuse_distro_upgrade_setup_finish (GsPlugin *plugin,
                                                GAsyncResult *result,
                                                GError **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_opensuse_distro_upgrade_refresh_metadata_async (GsPlugin                     *plugin,
                                                          guint64                       cache_age_secs,
                                                          GsPluginRefreshMetadataFlags  flags,
                                                          GCancellable                 *cancellable,
                                                          GAsyncReadyCallback           callback,
                                                          gpointer                      user_data)
{
	GsPluginOpensuseDistroUpgrade *self = GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (plugin);
	refresh_cache_async (self, cache_age_secs, cancellable, callback, user_data);
}

static gboolean
gs_plugin_opensuse_distro_upgrade_refresh_metadata_finish (GsPlugin      *plugin,
                                                           GAsyncResult  *result,
                                                           GError       **error)
{
	return refresh_cache_finish (GS_PLUGIN_OPENSUSE_DISTRO_UPGRADE (plugin),
				     result,
				     error);
}

static void
gs_plugin_opensuse_distro_upgrade_init (GsPluginOpensuseDistroUpgrade *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	/* Check if we are running openSUSE Leap. */
	if (!gs_plugin_check_distro_id (plugin, "opensuse-leap")) {
		gs_plugin_set_enabled (plugin, FALSE);
		g_debug ("Disabling itself as it's only supported in openSUSE Leap", gs_plugin_get_name (plugin));
		return;
	}

	self->distros = g_ptr_array_new_with_free_func ((GDestroyNotify) distro_upgrade_item_destroy);
}

static void
gs_plugin_opensuse_distro_upgrade_class_init (GsPluginOpensuseDistroUpgradeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_opensuse_distro_upgrade_dispose;
	object_class->finalize = gs_plugin_opensuse_distro_upgrade_finalize;

	plugin_class->setup_async = gs_plugin_opensuse_distro_upgrade_setup_async;
	plugin_class->setup_finish = gs_plugin_opensuse_distro_upgrade_setup_finish;
	plugin_class->refresh_metadata_async = gs_plugin_opensuse_distro_upgrade_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_opensuse_distro_upgrade_refresh_metadata_finish;
	plugin_class->list_distro_upgrades_async = gs_plugin_opensuse_distro_upgrade_list_distro_upgrades_async;
	plugin_class->list_distro_upgrades_finish = gs_plugin_opensuse_distro_upgrade_list_distro_upgrades_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_OPENSUSE_DISTRO_UPGRADE;
}
