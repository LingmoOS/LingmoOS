/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <fwupd.h>
#include <fcntl.h>
#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <lingmo-appstore.h>

#include "gs-fwupd-app.h"
#include "gs-metered.h"

#include "gs-plugin-fwupd.h"

/*
 * SECTION:
 * Queries for new firmware and schedules it to be installed as required.
 *
 * This plugin calls UpdatesChanged() if any updatable devices are
 * added or removed or if a device has been updated live.
 *
 * Since fwupd is a daemon accessible over D-Bus, this plugin basically
 * translates every job into one or more D-Bus calls, and all the real work is
 * done in the fwupd daemon. FIXME: This means the plugin can therefore execute
 * entirely in the main thread, making asynchronous D-Bus calls, once all the
 * vfuncs have been ported.
 */

struct _GsPluginFwupd {
	GsPlugin		 parent;

	FwupdClient		*client;
	GsApp			*app_current;
	GsApp			*cached_origin;
	GHashTable		*cached_sources; /* (nullable) (owned) (element-type utf8 GsApp); sources by id, each value is weak reffed */
	GMutex			 cached_sources_mutex;
};

G_DEFINE_TYPE (GsPluginFwupd, gs_plugin_fwupd, GS_TYPE_PLUGIN)

static void
cached_sources_weak_ref_cb (gpointer user_data,
			    GObject *object)
{
	GsPluginFwupd *self = user_data;
	GHashTableIter iter;
	gpointer key, value;
	g_autoptr(GMutexLocker) locker = NULL;

	locker = g_mutex_locker_new (&self->cached_sources_mutex);

	g_assert (self->cached_sources != NULL);

	g_hash_table_iter_init (&iter, self->cached_sources);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		GObject *repo_object = value;
		if (repo_object == object) {
			g_hash_table_iter_remove (&iter);
			if (!g_hash_table_size (self->cached_sources))
				g_clear_pointer (&self->cached_sources, g_hash_table_unref);
			break;
		}
	}
}

static void
gs_plugin_fwupd_error_convert (GError **perror)
{
	GError *error = perror != NULL ? *perror : NULL;

	/* not set */
	if (error == NULL)
		return;

	/* already correct */
	if (error->domain == GS_PLUGIN_ERROR)
		return;

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gio (perror))
		return;

	/* this are allowed for low-level errors */
	if (gs_utils_error_convert_gdbus (perror))
		return;

	/* custom to this plugin */
	if (error->domain == FWUPD_ERROR) {
		switch (error->code) {
		case FWUPD_ERROR_ALREADY_PENDING:
		case FWUPD_ERROR_INVALID_FILE:
		case FWUPD_ERROR_NOT_SUPPORTED:
			error->code = GS_PLUGIN_ERROR_NOT_SUPPORTED;
			break;
		case FWUPD_ERROR_AUTH_FAILED:
			error->code = GS_PLUGIN_ERROR_AUTH_INVALID;
			break;
		case FWUPD_ERROR_SIGNATURE_INVALID:
			error->code = GS_PLUGIN_ERROR_NO_SECURITY;
			break;
		case FWUPD_ERROR_AC_POWER_REQUIRED:
			error->code = GS_PLUGIN_ERROR_AC_POWER_REQUIRED;
			break;
		case FWUPD_ERROR_BATTERY_LEVEL_TOO_LOW:
			error->code = GS_PLUGIN_ERROR_BATTERY_LEVEL_TOO_LOW;
			break;
		default:
			error->code = GS_PLUGIN_ERROR_FAILED;
			break;
		}
	} else {
		g_warning ("can't reliably fixup error from domain %s",
			   g_quark_to_string (error->domain));
		error->code = GS_PLUGIN_ERROR_FAILED;
	}
	error->domain = GS_PLUGIN_ERROR;
}

static void
gs_plugin_fwupd_init (GsPluginFwupd *self)
{
	self->client = fwupd_client_new ();
	g_mutex_init (&self->cached_sources_mutex);

	/* set name of MetaInfo file */
	gs_plugin_set_appstream_id (GS_PLUGIN (self), "org.lingmo.Appstore.Plugin.Fwupd");
}

static void
gs_plugin_fwupd_dispose (GObject *object)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (object);

	g_clear_object (&self->cached_origin);
	g_clear_object (&self->client);

	if (self->cached_sources != NULL) {
		GHashTableIter iter;
		gpointer value;

		g_hash_table_iter_init (&iter, self->cached_sources);
		while (g_hash_table_iter_next (&iter, NULL, &value)) {
			GObject *app_repo = value;
			g_object_weak_unref (app_repo, cached_sources_weak_ref_cb, self);
		}

		g_clear_pointer (&self->cached_sources, g_hash_table_unref);
	}

	G_OBJECT_CLASS (gs_plugin_fwupd_parent_class)->dispose (object);
}

static void
gs_plugin_fwupd_finalize (GObject *object)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (object);

	g_mutex_clear (&self->cached_sources_mutex);

	G_OBJECT_CLASS (gs_plugin_fwupd_parent_class)->finalize (object);
}

void
gs_plugin_adopt_app (GsPlugin *plugin, GsApp *app)
{
	if (gs_app_get_kind (app) == AS_COMPONENT_KIND_FIRMWARE)
		gs_app_set_management_plugin (app, plugin);
}

static void
gs_plugin_fwupd_changed_cb (FwupdClient *client, GsPlugin *plugin)
{
}

static void
gs_plugin_fwupd_device_changed_cb (FwupdClient *client,
				   FwupdDevice *dev,
				   GsPlugin *plugin)
{
	/* limit number of UI refreshes */
	if (!fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_SUPPORTED)) {
		g_debug ("%s changed (not supported) so ignoring",
			 fwupd_device_get_id (dev));
		return;
	}

	/* If the flag is set the device matches something in the
	 * metadata as therefor is worth refreshing the update list */
	g_debug ("%s changed (supported) so reloading",
		 fwupd_device_get_id (dev));
	gs_plugin_updates_changed (plugin);
}

static void
gs_plugin_fwupd_notify_percentage_cb (GObject    *object,
                                      GParamSpec *pspec,
                                      gpointer    user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (user_data);

	/* nothing in progress */
	if (self->app_current == NULL) {
		g_debug ("fwupd percentage: %u%%",
			 fwupd_client_get_percentage (self->client));
		return;
	}
	g_debug ("fwupd percentage for %s: %u%%",
		 gs_app_get_unique_id (self->app_current),
		 fwupd_client_get_percentage (self->client));
	gs_app_set_progress (self->app_current,
			     fwupd_client_get_percentage (self->client));
}

static void
gs_plugin_fwupd_notify_status_cb (GObject    *object,
                                  GParamSpec *pspec,
                                  gpointer    user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (user_data);

	/* nothing in progress */
	if (self->app_current == NULL) {
		g_debug ("fwupd status: %s",
			 fwupd_status_to_string (fwupd_client_get_status (self->client)));
		return;
	}

	g_debug ("fwupd status for %s: %s",
		 gs_app_get_unique_id (self->app_current),
		 fwupd_status_to_string (fwupd_client_get_status (self->client)));
	switch (fwupd_client_get_status (self->client)) {
	case FWUPD_STATUS_DECOMPRESSING:
	case FWUPD_STATUS_DEVICE_RESTART:
	case FWUPD_STATUS_DEVICE_WRITE:
	case FWUPD_STATUS_DEVICE_VERIFY:
		gs_app_set_state (self->app_current, GS_APP_STATE_INSTALLING);
		break;
	case FWUPD_STATUS_IDLE:
		g_clear_object (&self->app_current);
		break;
	default:
		break;
	}
}

static gchar *
gs_plugin_fwupd_get_file_checksum (const gchar *filename,
				   GChecksumType checksum_type,
				   GError **error)
{
	gsize len;
	g_autofree gchar *data = NULL;

	if (!g_file_get_contents (filename, &data, &len, error)) {
		gs_utils_error_convert_gio (error);
		return NULL;
	}
	return g_compute_checksum_for_data (checksum_type, (const guchar *)data, len);
}

static void setup_connect_cb (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data);
static void setup_features_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);

static void
gs_plugin_fwupd_setup_async (GsPlugin            *plugin,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fwupd_setup_async);

	/* connect a proxy */
	fwupd_client_connect_async (self->client, cancellable, setup_connect_cb,
				    g_steal_pointer (&task));
}

static void
setup_connect_cb (GObject      *source_object,
                  GAsyncResult *result,
                  gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFwupd *self = g_task_get_source_object (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) local_error = NULL;

	if (!fwupd_client_connect_finish (self->client, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* send our implemented feature set */
	fwupd_client_set_feature_flags_async (self->client,
#if FWUPD_CHECK_VERSION(1, 8, 1)
					      FWUPD_FEATURE_FLAG_SHOW_PROBLEMS |
#endif
					      FWUPD_FEATURE_FLAG_REQUESTS |
					      FWUPD_FEATURE_FLAG_UPDATE_ACTION |
					      FWUPD_FEATURE_FLAG_DETACH_ACTION,
					      cancellable, setup_features_cb,
					      g_steal_pointer (&task));
}

static void
setup_features_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFwupd *self = g_task_get_source_object (task);
	GsPlugin *plugin = GS_PLUGIN (self);
	g_autoptr(GError) local_error = NULL;

	if (!fwupd_client_set_feature_flags_finish (self->client, result, &local_error))
		g_debug ("Failed to set front-end features: %s", local_error->message);
	g_clear_error (&local_error);

	/* we know the runtime daemon version now */
	fwupd_client_set_user_agent_for_package (self->client, PACKAGE_NAME, PACKAGE_VERSION);
	if (!fwupd_client_ensure_networking (self->client, &local_error)) {
		gs_plugin_fwupd_error_convert (&local_error);
		g_prefix_error (&local_error, "Failed to setup networking: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* add source */
	self->cached_origin = gs_app_new (gs_plugin_get_name (plugin));
	gs_app_set_kind (self->cached_origin, AS_COMPONENT_KIND_REPOSITORY);
	gs_app_set_bundle_kind (self->cached_origin, AS_BUNDLE_KIND_CABINET);
	gs_app_set_management_plugin (self->cached_origin, plugin);

	/* add the source to the plugin cache which allows us to match the
	 * unique ID to a GsApp when creating an event */
	gs_plugin_cache_add (plugin,
			     gs_app_get_unique_id (self->cached_origin),
			     self->cached_origin);

	/* register D-Bus errors */
	fwupd_error_quark ();
	g_signal_connect (self->client, "changed",
			  G_CALLBACK (gs_plugin_fwupd_changed_cb), plugin);
	g_signal_connect (self->client, "device-added",
			  G_CALLBACK (gs_plugin_fwupd_device_changed_cb), plugin);
	g_signal_connect (self->client, "device-removed",
			  G_CALLBACK (gs_plugin_fwupd_device_changed_cb), plugin);
	g_signal_connect (self->client, "device-changed",
			  G_CALLBACK (gs_plugin_fwupd_device_changed_cb), plugin);
	g_signal_connect (self->client, "notify::percentage",
			  G_CALLBACK (gs_plugin_fwupd_notify_percentage_cb), self);
	g_signal_connect (self->client, "notify::status",
			  G_CALLBACK (gs_plugin_fwupd_notify_status_cb), self);

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fwupd_setup_finish (GsPlugin      *plugin,
                              GAsyncResult  *result,
                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static GsApp *
gs_plugin_fwupd_new_app_from_device (GsPlugin *plugin, FwupdDevice *dev)
{
	FwupdRelease *rel = fwupd_device_get_release_default (dev);
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	GsApp *app;
	g_autofree gchar *id = NULL;
	g_autoptr(GIcon) icon = NULL;

	/* older versions of fwups didn't record this for historical devices */
	if (fwupd_release_get_appstream_id (rel) == NULL)
		return NULL;

	/* get from cache */
	id = gs_utils_build_unique_id (AS_COMPONENT_SCOPE_SYSTEM,
				       AS_BUNDLE_KIND_UNKNOWN,
				       NULL, /* origin */
				       fwupd_release_get_appstream_id (rel),
				       NULL);
	app = gs_plugin_cache_lookup (plugin, id);
	if (app == NULL) {
		app = gs_app_new (id);
		gs_plugin_cache_add (plugin, id, app);
	}

	/* default stuff */
	gs_app_set_kind (app, AS_COMPONENT_KIND_FIRMWARE);
	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_CABINET);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
	gs_app_add_quirk (app, GS_APP_QUIRK_DO_NOT_AUTO_UPDATE);
	gs_app_set_management_plugin (app, plugin);
	gs_app_add_category (app, "System");
	gs_fwupd_app_set_device_id (app, fwupd_device_get_id (dev));

	/* create icon */
	icon = g_themed_icon_new ("system-component-firmware");
	gs_app_add_icon (app, icon);
	gs_fwupd_app_set_from_device (app, self->client, dev);
	gs_fwupd_app_set_from_release (app, rel);

	if (fwupd_release_get_appstream_id (rel) != NULL)
		gs_app_set_id (app, fwupd_release_get_appstream_id (rel));

	/* the same as we have already */
	if (g_strcmp0 (fwupd_device_get_version (dev),
		       fwupd_release_get_version (rel)) == 0) {
		g_warning ("same firmware version as installed");
	}

	return app;
}

static gchar *
gs_plugin_fwupd_build_device_id (FwupdDevice *dev)
{
	g_autofree gchar *tmp = g_strdup (fwupd_device_get_id (dev));
	g_strdelimit (tmp, "/", '_');
	return g_strdup_printf ("org.fwupd.%s.device", tmp);
}

static GsApp *
gs_plugin_fwupd_new_app_from_device_raw (GsPlugin *plugin, FwupdDevice *device)
{
	GPtrArray *icons;
	g_autofree gchar *id = NULL;
	g_autoptr(GsApp) app = NULL;

	/* create a GsApp based on the device, not the release */
	id = gs_plugin_fwupd_build_device_id (device);
	app = gs_app_new (id);
	gs_app_set_kind (app, AS_COMPONENT_KIND_FIRMWARE);
	gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
	gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
	gs_app_add_quirk (app, GS_APP_QUIRK_DO_NOT_AUTO_UPDATE);
	gs_app_set_version (app, fwupd_device_get_version (device));
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, fwupd_device_get_name (device));
	gs_app_set_summary (app, GS_APP_QUALITY_LOWEST, fwupd_device_get_summary (device));
	gs_app_set_origin (app, fwupd_device_get_vendor (device));
	gs_fwupd_app_set_device_id (app, fwupd_device_get_id (device));
	gs_app_set_management_plugin (app, plugin);

	/* create icon */
	icons = fwupd_device_get_icons (device);
	for (guint j = 0; j < icons->len; j++) {
		const gchar *icon_str = g_ptr_array_index (icons, j);
		g_autoptr(GIcon) icon = NULL;
		if (g_str_has_prefix (icon_str, "/")) {
			g_autoptr(GFile) icon_file = g_file_new_for_path (icon_str);
			icon = g_file_icon_new (icon_file);
		} else {
			icon = g_themed_icon_new (icon_str);
		}
		gs_app_add_icon (app, icon);
	}
	return g_steal_pointer (&app);
}

static GsApp *
gs_plugin_fwupd_new_app (GsPlugin *plugin, FwupdDevice *dev, GError **error)
{
	FwupdRelease *rel = fwupd_device_get_release_default (dev);
	GPtrArray *checksums;
	GPtrArray *locations = fwupd_release_get_locations (rel);
	const gchar *update_uri = NULL;
	g_autofree gchar *basename = NULL;
	g_autofree gchar *filename_cache = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GsApp) app = NULL;

	/* update unsupported */
	app = gs_plugin_fwupd_new_app_from_device (plugin, dev);
	if (gs_app_get_state (app) != GS_APP_STATE_UPDATABLE_LIVE) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "%s [%s] cannot be updated",
			     gs_app_get_name (app), gs_app_get_id (app));
		return NULL;
	}

	/* some missing */
	if (gs_app_get_id (app) == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "fwupd: No id for firmware");
		return NULL;
	}
	if (gs_app_get_version (app) == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "fwupd: No version! for %s!", gs_app_get_id (app));
		return NULL;
	}
	if (gs_app_get_update_version (app) == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NOT_SUPPORTED,
			     "fwupd: No update-version! for %s!", gs_app_get_id (app));
		return NULL;
	}
	checksums = fwupd_release_get_checksums (rel);
	if (checksums->len == 0) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_NO_SECURITY,
			     "%s [%s] (%s) has no checksums, ignoring as unsafe",
			     gs_app_get_name (app),
			     gs_app_get_id (app),
			     gs_app_get_update_version (app));
		return NULL;
	}

	/* typically the first URI will be the main HTTP mirror, and we
	 * don't have the capability to use an IPFS/IPNS URL anyway */
	if (locations->len > 0)
		update_uri = g_ptr_array_index (locations, 0);

	if (update_uri == NULL) {
		g_set_error (error,
			     GS_PLUGIN_ERROR,
			     GS_PLUGIN_ERROR_INVALID_FORMAT,
			     "no location available for %s [%s]",
			     gs_app_get_name (app), gs_app_get_id (app));
		return NULL;
	}

	/* does the firmware already exist in the cache? */
	basename = g_path_get_basename (update_uri);
	filename_cache = gs_utils_get_cache_filename ("fwupd",
						      basename,
						      GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
						      error);
	if (filename_cache == NULL)
		return NULL;

	/* delete the file if the checksum does not match */
	if (g_file_test (filename_cache, G_FILE_TEST_EXISTS)) {
		const gchar *checksum_tmp = NULL;
		g_autofree gchar *checksum = NULL;

		/* we can migrate to something better than SHA1 when the LVFS
		 * starts producing metadata with multiple hash types */
		checksum_tmp = fwupd_checksum_get_by_kind (checksums,
							   G_CHECKSUM_SHA1);
		if (checksum_tmp == NULL) {
			g_set_error (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_INVALID_FORMAT,
				     "No valid checksum for %s",
				     filename_cache);
		}
		checksum = gs_plugin_fwupd_get_file_checksum (filename_cache,
							      G_CHECKSUM_SHA1,
							      error);
		if (checksum == NULL)
			return NULL;
		if (g_strcmp0 (checksum_tmp, checksum) != 0) {
			g_set_error (error,
				     GS_PLUGIN_ERROR,
				     GS_PLUGIN_ERROR_INVALID_FORMAT,
				     "%s does not match checksum, expected %s got %s",
				     filename_cache, checksum_tmp, checksum);
			g_unlink (filename_cache);
			return NULL;
		}
	}

	/* already downloaded, so overwrite */
	if (g_file_test (filename_cache, G_FILE_TEST_EXISTS))
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);

	/* actually add the application */
	file = g_file_new_for_path (filename_cache);
	gs_app_set_local_file (app, file);
	return g_steal_pointer (&app);
}

gboolean
gs_plugin_add_updates_historical (GsPlugin *plugin,
				  GsAppList *list,
				  GCancellable *cancellable,
				  GError **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GPtrArray) devices = NULL;

	devices = fwupd_client_get_devices (self->client, cancellable, &error_local);
	if (devices == NULL) {
		if (g_error_matches (error_local,
				     FWUPD_ERROR,
				     FWUPD_ERROR_NOTHING_TO_DO))
			return TRUE;
		if (g_error_matches (error_local,
				     FWUPD_ERROR,
				     FWUPD_ERROR_NOT_FOUND))
			return TRUE;
		g_propagate_error (error, g_steal_pointer (&error_local));
		gs_plugin_fwupd_error_convert (error);
		return FALSE;
	}

	for (guint i = 0; i < devices->len; i++) {
		FwupdDevice *idev = g_ptr_array_index (devices, i);
		g_autoptr(FwupdDevice) dev = NULL;
		g_autoptr(GsApp) app = NULL;

		/* get historical updates */
		dev = fwupd_client_get_results (self->client,
						fwupd_device_get_id (idev),
						cancellable,
						&error_local);
		if (dev == NULL) {
			if (g_error_matches (error_local,
					     FWUPD_ERROR,
					     FWUPD_ERROR_NOTHING_TO_DO)) {
				g_clear_error (&error_local);
				continue;
			}
			if (g_error_matches (error_local,
					     FWUPD_ERROR,
					     FWUPD_ERROR_NOT_FOUND)) {
				g_clear_error (&error_local);
				continue;
			}
			g_propagate_error (error, g_steal_pointer (&error_local));
			gs_plugin_fwupd_error_convert (error);
			return FALSE;
		}

		/* parse */
		app = gs_plugin_fwupd_new_app_from_device (plugin, dev);
		if (app != NULL) {
			gs_app_list_add (list, app);
		} else {
			g_debug ("updates historical: failed to build result for '%s' (%s)",
				 fwupd_device_get_name (dev),
				 fwupd_device_get_id (dev));
		}
	}

	return TRUE;
}

gboolean
gs_plugin_add_updates (GsPlugin *plugin,
		       GsAppList *list,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GPtrArray) devices = NULL;

	/* get current list of updates */
	devices = fwupd_client_get_devices (self->client, cancellable, &error_local);
	if (devices == NULL) {
		if (g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOTHING_TO_DO) ||
		    g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOT_SUPPORTED) ||
		    g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOT_FOUND)) {
			g_debug ("no devices (%s)", error_local->message);
			return TRUE;
		}
		g_debug ("Failed to get devices: %s", error_local->message);
		return TRUE;
	}
	for (guint i = 0; i < devices->len; i++) {
		FwupdDevice *dev = g_ptr_array_index (devices, i);
		FwupdRelease *rel_newest;
		g_autoptr(GError) error_local2 = NULL;
		g_autoptr(GPtrArray) rels = NULL;
		g_autoptr(GsApp) app = NULL;

		/* locked device that needs unlocking */
		if (fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_LOCKED)) {
			app = gs_plugin_fwupd_new_app_from_device_raw (plugin, dev);
			gs_fwupd_app_set_is_locked (app, TRUE);
			gs_app_list_add (list, app);
			continue;
		}

		/* not going to have results, so save a D-Bus round-trip */
		if (!fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_SUPPORTED))
			continue;

		/* get the releases for this device and filter for validity */
		rels = fwupd_client_get_upgrades (self->client,
						  fwupd_device_get_id (dev),
						  cancellable, &error_local2);
		if (rels == NULL) {
			if (g_error_matches (error_local2,
					     FWUPD_ERROR,
					     FWUPD_ERROR_NOTHING_TO_DO)) {
				g_debug ("no updates for %s", fwupd_device_get_id (dev));
				continue;
			}
			if (g_error_matches (error_local2,
					     FWUPD_ERROR,
					     FWUPD_ERROR_NOT_SUPPORTED)) {
				g_debug ("not supported for %s", fwupd_device_get_id (dev));
				continue;
			}
			g_warning ("failed to get upgrades for %s: %s]",
				   fwupd_device_get_id (dev),
				   error_local2->message);
			continue;
		}

		/* normal device update */
		rel_newest = g_ptr_array_index (rels, 0);
		fwupd_device_add_release (dev, rel_newest);
		app = gs_plugin_fwupd_new_app (plugin, dev, &error_local2);
		if (app == NULL) {
			g_debug ("%s", error_local2->message);
			continue;
		}

		/* add update descriptions for all releases inbetween */
		if (rels->len > 1) {
			g_autoptr(GString) update_desc = g_string_new (NULL);
			for (guint j = 0; j < rels->len; j++) {
				FwupdRelease *rel = g_ptr_array_index (rels, j);
				g_autofree gchar *desc = NULL;
				if (fwupd_release_get_description (rel) == NULL)
					continue;
#if AS_CHECK_VERSION(1, 0, 0)
				desc = as_markup_convert (fwupd_release_get_description (rel), AS_MARKUP_KIND_TEXT, NULL);
#else
				desc = as_markup_convert_simple (fwupd_release_get_description (rel), NULL);
#endif
				if (desc == NULL)
					continue;
				g_string_append_printf (update_desc,
							"Version %s:\n%s\n\n",
							fwupd_release_get_version (rel),
							desc);
			}
			if (update_desc->len > 2) {
				g_string_truncate (update_desc, update_desc->len - 2);
				gs_app_set_update_details_text (app, update_desc->str);
			}
		}
		gs_app_list_add (list, app);
	}
	return TRUE;
}

static gboolean
remote_cache_is_expired (FwupdRemote *remote,
                         guint64      cache_age_secs)
{
	/* check cache age */
	if (cache_age_secs > 0) {
		guint64 age = fwupd_remote_get_age (remote);
		if (age < cache_age_secs) {
			g_debug ("fwupd remote is only %" G_GUINT64_FORMAT " seconds old, so ignoring refresh", age);
			return FALSE;
		}
	}

	return TRUE;
}

typedef struct {
	/* Input data. */
	guint64 cache_age_secs;

	/* In-progress state. */
	guint n_operations_pending;
	GError *error;  /* (owned) (nullable) */
} RefreshMetadataData;

static void
refresh_metadata_data_free (RefreshMetadataData *data)
{
	g_clear_error (&data->error);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (RefreshMetadataData, refresh_metadata_data_free)

static void get_remotes_cb (GObject      *source_object,
                            GAsyncResult *result,
                            gpointer      user_data);
static void refresh_remote_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);
static void finish_refresh_metadata_op (GTask *task);

static void
gs_plugin_fwupd_refresh_metadata_async (GsPlugin                     *plugin,
                                        guint64                       cache_age_secs,
                                        GsPluginRefreshMetadataFlags  flags,
                                        GCancellable                 *cancellable,
                                        GAsyncReadyCallback           callback,
                                        gpointer                      user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(RefreshMetadataData) data = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fwupd_refresh_metadata_async);

	data = g_new0 (RefreshMetadataData, 1);
	data->cache_age_secs = cache_age_secs;
	g_task_set_task_data (task, g_steal_pointer (&data), (GDestroyNotify) refresh_metadata_data_free);

	/* get the list of enabled remotes */
	fwupd_client_get_remotes_async (self->client, cancellable, get_remotes_cb, g_steal_pointer (&task));
}

static void
get_remotes_cb (GObject      *source_object,
                GAsyncResult *result,
                gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	RefreshMetadataData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) error_local = NULL;
	g_autoptr(GPtrArray) remotes = NULL;

	remotes = fwupd_client_get_remotes_finish (client, result, &error_local);

	if (remotes == NULL) {
		g_debug ("No remotes found: %s", error_local ? error_local->message : "Unknown error");
		if (g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOTHING_TO_DO) ||
		    g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOT_SUPPORTED) ||
		    g_error_matches (error_local, FWUPD_ERROR, FWUPD_ERROR_NOT_FOUND)) {
			g_task_return_boolean (task, TRUE);
			return;
		}

		gs_plugin_fwupd_error_convert (&error_local);
		g_task_return_error (task, g_steal_pointer (&error_local));
		return;
	}

	/* Refresh each of the remotes in parallel. Keep the pending operation
	 * count incremented until all operations have been started, so that
	 * the overall operation doesn’t complete too early. */
	data->n_operations_pending = 1;

	for (guint i = 0; i < remotes->len; i++) {
		FwupdRemote *remote = g_ptr_array_index (remotes, i);
		gboolean is_enabled;

		#if FWUPD_CHECK_VERSION(1, 9, 4)
		is_enabled = fwupd_remote_has_flag (remote, FWUPD_REMOTE_FLAG_ENABLED);
		#else
		is_enabled = fwupd_remote_get_enabled (remote);
		#endif

		if (!is_enabled)
			continue;
		if (fwupd_remote_get_kind (remote) != FWUPD_REMOTE_KIND_DOWNLOAD)
			continue;
		if (!remote_cache_is_expired (remote, data->cache_age_secs))
			continue;

		data->n_operations_pending++;
		#if FWUPD_CHECK_VERSION(2, 0, 0)
		fwupd_client_refresh_remote_async (client, remote, FWUPD_CLIENT_DOWNLOAD_FLAG_NONE, cancellable,
						   refresh_remote_cb, g_object_ref (task));
		#elif FWUPD_CHECK_VERSION(1, 9, 4)
		fwupd_client_refresh_remote2_async (client, remote, FWUPD_CLIENT_DOWNLOAD_FLAG_NONE, cancellable,
						    refresh_remote_cb, g_object_ref (task));
		#else
		fwupd_client_refresh_remote_async (client, remote, cancellable,
						   refresh_remote_cb, g_object_ref (task));
		#endif
	}

	finish_refresh_metadata_op (task);
}

static void
refresh_remote_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	RefreshMetadataData *data = g_task_get_task_data (task);
	g_autoptr(GError) local_error = NULL;

	if (!fwupd_client_refresh_remote_finish (client, result, &local_error)) {
		gs_plugin_fwupd_error_convert (&local_error);
		if (data->error == NULL)
			data->error = g_steal_pointer (&local_error);
		else
			g_debug ("Another remote refresh error: %s", local_error->message);
	}

	finish_refresh_metadata_op (task);
}

static void
finish_refresh_metadata_op (GTask *task)
{
	RefreshMetadataData *data = g_task_get_task_data (task);

	g_assert (data->n_operations_pending > 0);
	data->n_operations_pending--;

	if (data->n_operations_pending == 0) {
		if (data->error != NULL)
			g_task_return_error (task, g_steal_pointer (&data->error));
		else
			g_task_return_boolean (task, TRUE);
	}
}

static gboolean
gs_plugin_fwupd_refresh_metadata_finish (GsPlugin      *plugin,
                                         GAsyncResult  *result,
                                         GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

typedef struct {
	GsApp *app;  /* (owned) (not nullable) */
	GFile *local_file;  /* (owned) (not nullable) */
	gpointer schedule_entry_handle;  /* (nullable) (owned) */
} DownloadData;

static void
download_data_free (DownloadData *data)
{
	/* Should have been explicitly removed from the scheduler by now. */
	g_assert (data->schedule_entry_handle == NULL);

	g_clear_object (&data->app);
	g_clear_object (&data->local_file);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DownloadData, download_data_free)

static void download_schedule_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data);
static void download_download_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data);
static void download_replace_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      user_data);

static void
gs_plugin_fwupd_download_async (GsPluginFwupd       *self,
                                GsApp               *app,
                                gboolean             interactive,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
	GFile *local_file;
	g_autoptr(GTask) task = NULL;
	DownloadData *data;
	g_autoptr(DownloadData) data_owned = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fwupd_download_async);

	/* not set */
	local_file = gs_app_get_local_file (app);
	if (local_file == NULL) {
		g_task_return_new_error (task,
					 GS_PLUGIN_ERROR,
					 GS_PLUGIN_ERROR_FAILED,
					 "not enough data for fwupd");
		return;
	}

	data = data_owned = g_new0 (DownloadData, 1);
	data->app = g_object_ref (app);
	data->local_file = g_object_ref (local_file);
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) download_data_free);

	/* Check the cancellable, since the error return for
	 * g_file_query_exists() is the same as file-not-exists. */
	if (g_task_return_error_if_cancelled (task))
		return;

	/* If the file exists already, return early */
	if (g_file_query_exists (local_file, cancellable)) {
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, 0);
		g_task_return_boolean (task, TRUE);
		return;
	}

	gs_app_set_state (app, GS_APP_STATE_DOWNLOADING);

	if (!interactive) {
		gs_metered_block_on_download_scheduler_async (gs_metered_build_scheduler_parameters_for_app (app),
							      cancellable, download_schedule_cb, g_steal_pointer (&task));
	} else {
		download_schedule_cb (NULL, NULL, g_steal_pointer (&task));
	}
}

static void
download_schedule_cb (GObject      *source_object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFwupd *self = g_task_get_source_object (task);
	DownloadData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	const gchar *uri = gs_fwupd_app_get_update_uri (data->app);
	g_autoptr(GError) local_error = NULL;

	if (result != NULL &&
	    !gs_metered_block_on_download_scheduler_finish (result, &data->schedule_entry_handle, &local_error)) {
		g_warning ("Failed to block on download scheduler: %s",
			   local_error->message);
		g_clear_error (&local_error);
	}

	/* Download the firmware contents. */
	fwupd_client_download_bytes_async (self->client,
					   uri,
					   FWUPD_CLIENT_DOWNLOAD_FLAG_NONE,
					   cancellable,
					   download_download_cb,
					   g_steal_pointer (&task));
}

static void
download_download_cb (GObject      *source_object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	DownloadData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GBytes) bytes = NULL;
	g_autoptr(GError) local_error = NULL;

	bytes = fwupd_client_download_bytes_finish (client, result, &local_error);
	if (bytes == NULL) {
		gs_app_set_state_recover (data->app);
		gs_plugin_fwupd_error_convert (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Now write to the file. */
	g_file_replace_contents_bytes_async (data->local_file, bytes, NULL, FALSE,
					     G_FILE_CREATE_NONE,
					     cancellable,
					     download_replace_cb,
					     g_steal_pointer (&task));
}

static void
download_replace_cb (GObject      *source_object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
	GFile *local_file = G_FILE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	DownloadData *data = g_task_get_task_data (task);
	gboolean download_success;
	g_autoptr(GError) local_error = NULL;

	download_success = g_file_replace_contents_finish (local_file, result, NULL, &local_error);

	/* Fire this call off into the void, it’s not worth tracking it.
	 * Don’t pass a cancellable in, as the download may have been cancelled. */
	if (data->schedule_entry_handle != NULL)
		gs_metered_remove_from_download_scheduler_async (data->schedule_entry_handle, NULL, NULL, NULL);

	gs_app_set_state_recover (data->app);

	if (!download_success) {
		gs_plugin_fwupd_error_convert (&local_error);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	gs_app_set_size_download (data->app, GS_SIZE_TYPE_VALID, 0);

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fwupd_download_finish (GsPluginFwupd  *self,
                                 GAsyncResult   *result,
                                 GError        **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

typedef struct {
	GsPluginAppNeedsUserActionCallback app_needs_user_action_callback;
	gpointer app_needs_user_action_data;
	GsApp *app;  /* (owned) (not nullable) */
	gboolean interactive;
	GFile *local_file;  /* (owned) (not nullable) */
	const gchar *device_id;  /* (not nullable) */
} InstallData;

static void
install_data_free (InstallData *data)
{
	g_clear_object (&data->app);
	g_clear_object (&data->local_file);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (InstallData, install_data_free)

static void install_install_cb (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data);
static void install_delete_cb (GObject      *source_object,
                               GAsyncResult *result,
                               gpointer      user_data);
static void install_get_device_cb (GObject      *source_object,
                                   GAsyncResult *result,
                                   gpointer      user_data);
static void install_device_request_cb (FwupdClient  *client,
                                       FwupdRequest *request,
                                       GTask        *task);

static void
gs_plugin_fwupd_install_async (GsPluginFwupd                      *self,
                               GsApp                              *app,
                               gboolean                            interactive,
                               GsPluginAppNeedsUserActionCallback  app_needs_user_action_callback,
                               gpointer                            app_needs_user_action_data,
                               GCancellable                       *cancellable,
                               GAsyncReadyCallback                 callback,
                               gpointer                            user_data)
{
	FwupdInstallFlags install_flags = 0;
	GFile *local_file;
	g_autoptr(GTask) task = NULL;
	InstallData *data;
	g_autoptr(InstallData) data_owned = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fwupd_install_async);

	/* This function assumes that the file has already been downloaded and
	 * cached at @local_file. */
	local_file = gs_app_get_local_file (app);
	if (local_file == NULL) {
		g_task_return_new_error (task,
					 GS_PLUGIN_ERROR,
					 GS_PLUGIN_ERROR_FAILED,
					 "not enough data for fwupd");
		return;
	}

	data = data_owned = g_new0 (InstallData, 1);
	data->app_needs_user_action_callback = app_needs_user_action_callback;
	data->app_needs_user_action_data = app_needs_user_action_data;
	data->app = g_object_ref (app);
	data->interactive = interactive;
	data->local_file = g_object_ref (local_file);
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) install_data_free);

	/* limit to single device? */
	data->device_id = gs_fwupd_app_get_device_id (app);
	if (data->device_id == NULL)
		data->device_id = FWUPD_DEVICE_ID_ANY;

	/* watch for FwupdRequest */
	g_signal_connect (self->client, "device-request", G_CALLBACK (install_device_request_cb), task);

	/* Store the app pointer for getting status and progress updates from
	 * the daemon.
	 *
	 * FIXME: This only supports one operation in parallel, so progress
	 * reporting with gs_app_set_progress() will get a little confused if
	 * there are multiple firmware updates being applied. We need more API
	 * from libfwupd to improve on this; see
	 * https://github.com/fwupd/fwupd/issues/5522. */
	g_set_object (&self->app_current, app);

	/* only offline supported */
	if (gs_app_get_metadata_item (app, "fwupd::OnlyOffline") != NULL)
		install_flags |= FWUPD_INSTALL_FLAG_OFFLINE;

	gs_app_set_state (app, GS_APP_STATE_INSTALLING);

	fwupd_client_install_async (self->client, data->device_id,
				    g_file_peek_path (local_file), install_flags,
				    cancellable,
				    install_install_cb, g_steal_pointer (&task));
}

static void
install_device_request_cb (FwupdClient *client, FwupdRequest *request, GTask *task)
{
	GsPluginFwupd *self = g_task_get_source_object (task);
	InstallData *data = g_task_get_task_data (task);
	g_autoptr(AsScreenshot) ss = as_screenshot_new ();
	g_autofree gchar *str = fwupd_request_to_string (request);

	/* check the device ID is correct */
	g_debug ("got FwupdRequest: %s", str);
	if (g_strcmp0 (data->device_id, FWUPD_DEVICE_ID_ANY) != 0 &&
	    g_strcmp0 (data->device_id, fwupd_request_get_device_id (request)) != 0) {
		g_warning ("received request for %s, but updating %s",
			   fwupd_request_get_device_id (request),
			   data->device_id);
		return;
	}

	/* image is optional, caption is required */
	if (fwupd_request_get_image (request) != NULL) {
		g_autoptr(AsImage) im = as_image_new ();
		as_image_set_kind (im, AS_IMAGE_KIND_SOURCE);
		as_image_set_url (im, fwupd_request_get_image (request));
		as_screenshot_add_image (ss, im);
	}
	as_screenshot_set_kind (ss, AS_SCREENSHOT_KIND_DEFAULT);
	as_screenshot_set_caption (ss, fwupd_request_get_message (request), NULL);

	/* require the dialog */
	if (fwupd_request_get_kind (request) == FWUPD_REQUEST_KIND_POST) {
		gs_app_add_quirk (data->app, GS_APP_QUIRK_NEEDS_USER_ACTION);
		gs_app_set_action_screenshot (data->app, ss);
	} else if (data->app_needs_user_action_callback != NULL) {
		data->app_needs_user_action_callback (GS_PLUGIN (self),
						      data->app,
						      ss,
						      data->app_needs_user_action_data);
	}
}

static void
install_install_cb (GObject      *source_object,
                    GAsyncResult *result,
                    gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFwupd *self = g_task_get_source_object (task);
	InstallData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) local_error = NULL;

	/* no longer handling requests */
	g_signal_handlers_disconnect_by_func (client, G_CALLBACK (install_device_request_cb), task);

	if (!fwupd_client_install_finish (client, result, &local_error)) {
		g_autoptr(GsPluginEvent) event = NULL;

		/* show the user this failed */
		gs_plugin_fwupd_error_convert (&local_error);
		event = gs_plugin_event_new ("app", self->app_current,
					     "error", local_error,
					     NULL);
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
		if (data->interactive)
			gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
		gs_plugin_report_event (GS_PLUGIN (self), event);

		gs_app_set_state_recover (data->app);

		/* this error code *has* to be cancelled to *not* show the reboot dialog */
		g_task_return_new_error (task,
					 GS_PLUGIN_ERROR,
					 GS_PLUGIN_ERROR_CANCELLED,
					 "%s", local_error->message);
		return;
	}

	gs_app_set_state (data->app, GS_APP_STATE_INSTALLED);

	/* delete the file from the cache */
	g_file_delete_async (data->local_file, G_PRIORITY_DEFAULT, cancellable,
			     install_delete_cb, g_steal_pointer (&task));
}

static void
install_delete_cb (GObject      *source_object,
                   GAsyncResult *result,
                   gpointer      user_data)
{
	GFile *local_file = G_FILE (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	GsPluginFwupd *self = g_task_get_source_object (task);
	InstallData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) local_error = NULL;

	if (!g_file_delete_finish (local_file, result, &local_error) &&
	    !g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_clear_error (&local_error);

	/* does the device have an update message? */
	fwupd_client_get_device_by_id_async (self->client,
					     data->device_id,
					     cancellable,
					     install_get_device_cb,
					     g_steal_pointer (&task));
}

static void
install_get_device_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(GTask) task = g_steal_pointer (&user_data);
	g_autoptr(FwupdDevice) dev = NULL;
	g_autoptr(GError) local_error = NULL;

	dev = fwupd_client_get_device_by_id_finish (client, result, &local_error);
	if (dev == NULL) {
		/* NOTE: this is probably entirely fine; some devices do not
		 * re-enumerate until replugged manually or the machine is
		 * rebooted -- and the metadata to know that is only available
		 * in a too-new-to-depend-on fwupd version */
		g_debug ("failed to find device after install: %s", local_error->message);
	}

	/* success */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fwupd_install_finish (GsPluginFwupd  *self,
                                GAsyncResult   *result,
                                GError        **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_fwupd_modify_source_ready_cb (GObject *source_object,
					GAsyncResult *result,
					gpointer user_data)
{
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GTask) task = user_data;
	GsPluginFwupd *self = g_task_get_source_object (task);
	GsApp *repository = g_task_get_task_data (task);

	if (!fwupd_client_modify_remote_finish (FWUPD_CLIENT (source_object), result, &local_error)) {
		gs_app_set_state_recover (repository);
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	if (gs_app_get_state (repository) == GS_APP_STATE_INSTALLING)
		gs_app_set_state (repository, GS_APP_STATE_INSTALLED);
	else if (gs_app_get_state (repository) == GS_APP_STATE_REMOVING)
		gs_app_set_state (repository, GS_APP_STATE_AVAILABLE);

	gs_plugin_repository_changed (GS_PLUGIN (self), repository);

	g_task_return_boolean (task, TRUE);
}

static void
gs_plugin_fwupd_modify_source_async (GsPluginFwupd      *self,
				     GsApp              *repository,
				     gboolean            enabled,
				     GCancellable       *cancellable,
				     GAsyncReadyCallback callback,
				     gpointer            user_data)
{
	g_autoptr(GTask) task = NULL;
	const gchar *remote_id;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_task_data (task, g_object_ref (repository), g_object_unref);
	g_task_set_source_tag (task, gs_plugin_fwupd_modify_source_async);

	if (!gs_app_has_management_plugin (repository, GS_PLUGIN (self))) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* source -> remote */
	g_assert (gs_app_get_kind (repository) == AS_COMPONENT_KIND_REPOSITORY);

	remote_id = gs_app_get_metadata_item (repository, "fwupd::remote-id");
	if (remote_id == NULL) {
		g_task_return_new_error (task,
					 GS_PLUGIN_ERROR,
					 GS_PLUGIN_ERROR_FAILED,
					 "not enough data for fwupd %s",
					 gs_app_get_unique_id (repository));
		return;
	}
	gs_app_set_state (repository, enabled ?
	                  GS_APP_STATE_INSTALLING : GS_APP_STATE_REMOVING);
	fwupd_client_modify_remote_async (self->client,
	                                  remote_id,
	                                  "Enabled",
	                                  enabled ? "true" : "false",
	                                  cancellable,
					  gs_plugin_fwupd_modify_source_ready_cb,
					  g_steal_pointer (&task));
}

static gboolean
gs_plugin_fwupd_modify_source_finish (GsPluginFwupd *self,
				      GAsyncResult  *result,
				      GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
async_result_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GAsyncResult **result_out = user_data;

	g_assert (result_out != NULL && *result_out == NULL);
	*result_out = g_object_ref (result);
	g_main_context_wakeup (g_main_context_get_thread_default ());
}

gboolean
gs_plugin_app_install (GsPlugin *plugin,
		       GsApp *app,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	gboolean interactive = gs_plugin_has_flags (plugin, GS_PLUGIN_FLAGS_INTERACTIVE);
	g_autoptr(GMainContext) context = g_main_context_new ();
	g_autoptr(GMainContextPusher) pusher = g_main_context_pusher_new (context);
	g_autoptr(GAsyncResult) result = NULL;

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (app, plugin))
		return TRUE;

	/* source -> remote, handled by dedicated function */
	g_return_val_if_fail (gs_app_get_kind (app) != AS_COMPONENT_KIND_REPOSITORY, FALSE);

	/* Download the file first. */
	gs_plugin_fwupd_download_async (self, app, interactive, cancellable, async_result_cb, &result);
	while (result == NULL)
		g_main_context_iteration (context, TRUE);

	if (!gs_plugin_fwupd_download_finish (self, result, error))
		return FALSE;

	g_clear_object (&result);

	/* FIXME: Connect #GsPluginAppNeedsUserActionCallback when this function
	 * is ported to the new #GsPluginJob subclasses. */
	gs_plugin_fwupd_install_async (self, app, interactive, NULL, NULL, cancellable, async_result_cb, &result);
	while (result == NULL)
		g_main_context_iteration (context, TRUE);

	return gs_plugin_fwupd_install_finish (self, result, error);
}

typedef struct {
	/* Input data. */
	guint n_apps;
	GsPluginUpdateAppsFlags flags;
	GsPluginProgressCallback progress_callback;
	gpointer progress_user_data;
	GsPluginAppNeedsUserActionCallback app_needs_user_action_callback;
	gpointer app_needs_user_action_data;

	/* In-progress data. */
	guint n_pending_ops;
	GError *saved_error;  /* (owned) (nullable) */
} UpdateAppsData;

static void
update_apps_data_free (UpdateAppsData *data)
{
	/* Error should have been propagated by now, and all pending ops completed. */
	g_assert (data->saved_error == NULL);
	g_assert (data->n_pending_ops == 0);

	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (UpdateAppsData, update_apps_data_free)

typedef struct {
	GTask *task;  /* (owned) */
	GsApp *app;  /* (owned) */
	guint index;  /* zero-based */
} UpdateSingleAppData;

static void
update_single_app_data_free (UpdateSingleAppData *data)
{
	g_clear_object (&data->app);
	g_clear_object (&data->task);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (UpdateSingleAppData, update_single_app_data_free)

static void update_app_download_cb (GObject      *source_object,
                                    GAsyncResult *result,
                                    gpointer      user_data);
static void update_app_unlock_cb (GObject      *source_object,
                                  GAsyncResult *result,
                                  gpointer      user_data);
static void update_app_install_cb (GObject      *source_object,
                                   GAsyncResult *result,
                                   gpointer      user_data);
static void finish_update_apps_op (GTask  *task,
                                   GError *error);

static void
gs_plugin_fwupd_update_apps_async (GsPlugin                           *plugin,
                                   GsAppList                          *apps,
                                   GsPluginUpdateAppsFlags             flags,
                                   GsPluginProgressCallback            progress_callback,
                                   gpointer                            progress_user_data,
                                   GsPluginAppNeedsUserActionCallback  app_needs_user_action_callback,
                                   gpointer                            app_needs_user_action_data,
                                   GCancellable                       *cancellable,
                                   GAsyncReadyCallback                 callback,
                                   gpointer                            user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = (flags & GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE);
	UpdateAppsData *data;
	g_autoptr(UpdateAppsData) data_owned = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_fwupd_update_apps_async);

	data = data_owned = g_new0 (UpdateAppsData, 1);
	data->flags = flags;
	data->progress_callback = progress_callback;
	data->progress_user_data = progress_user_data;
	data->app_needs_user_action_callback = app_needs_user_action_callback;
	data->app_needs_user_action_data = app_needs_user_action_data;
	data->n_apps = gs_app_list_length (apps);
	g_task_set_task_data (task, g_steal_pointer (&data_owned), (GDestroyNotify) update_apps_data_free);

	/* Start a load of operations in parallel to download the firmware
	 * updates for all the apps. When each download is complete, start the
	 * install process for it in parallel with whatever downloads and
	 * installs are going on for the other apps.
	 *
	 * When all installs are finished for all apps, finish_update_apps_op()
	 * will return success/error for the overall #GTask. */
	data->n_pending_ops = 1;

	for (guint i = 0; i < gs_app_list_length (apps); i++) {
		GsApp *app = gs_app_list_index (apps, i);
		g_autoptr(UpdateSingleAppData) app_data = NULL;

		/* only process this app if was created by this plugin */
		if (!gs_app_has_management_plugin (app, plugin))
			continue;

		app_data = g_new0 (UpdateSingleAppData, 1);
		app_data->index = i;
		app_data->task = g_object_ref (task);
		app_data->app = g_object_ref (app);

		data->n_pending_ops++;
		if (!(flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD)) {
			gs_plugin_fwupd_download_async (self, app, interactive, cancellable, update_app_download_cb, g_steal_pointer (&app_data));
		} else {
			update_app_download_cb (G_OBJECT (self), NULL, g_steal_pointer (&app_data));
		}
	}

	finish_update_apps_op (task, NULL);
}

static void
update_app_download_cb (GObject      *source_object,
                        GAsyncResult *result,
                        gpointer      user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (source_object);
	g_autoptr(UpdateSingleAppData) app_data = g_steal_pointer (&user_data);
	GTask *task = app_data->task;
	UpdateAppsData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	g_autoptr(GError) local_error = NULL;

	if (result != NULL &&
	    !gs_plugin_fwupd_download_finish (self, result, &local_error)) {
		finish_update_apps_op (task, g_steal_pointer (&local_error));
		return;
	}

	if (!(data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_NO_APPLY)) {
		/* locked devices need unlocking, rather than installing */
		if (gs_fwupd_app_get_is_locked (app_data->app)) {
			const gchar *device_id = gs_fwupd_app_get_device_id (app_data->app);

			if (device_id == NULL) {
				finish_update_apps_op (task, g_error_new (GS_PLUGIN_ERROR,
									  GS_PLUGIN_ERROR_INVALID_FORMAT,
									  "not enough data for fwupd unlock"));
				return;
			}

			fwupd_client_unlock_async (self->client, device_id,
						   cancellable,
						   update_app_unlock_cb,
						   g_steal_pointer (&app_data));
		} else {
			update_app_unlock_cb (G_OBJECT (self->client), NULL, g_steal_pointer (&app_data));
		}
	} else {
		/* Not applying the update, so finish the operation now. */
		finish_update_apps_op (task, NULL);
	}
}

static void
update_app_unlock_cb (GObject      *source_object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
	FwupdClient *client = FWUPD_CLIENT (source_object);
	g_autoptr(UpdateSingleAppData) app_data = g_steal_pointer (&user_data);
	GTask *task = app_data->task;
	UpdateAppsData *data = g_task_get_task_data (task);
	GCancellable *cancellable = g_task_get_cancellable (task);
	GsPluginFwupd *self = g_task_get_source_object (task);
	GsApp *app = app_data->app;
	g_autoptr(GError) local_error = NULL;

	if (result != NULL &&
	    !fwupd_client_unlock_finish (client, result, &local_error)) {
		gs_plugin_fwupd_error_convert (&local_error);
		finish_update_apps_op (task, g_steal_pointer (&local_error));
		return;
	}

	/* update means install */
	gs_plugin_fwupd_install_async (self, app,
				       (data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE),
				       data->app_needs_user_action_callback,
				       data->app_needs_user_action_data,
				       cancellable,
				       update_app_install_cb,
				       g_steal_pointer (&app_data));
}

static void
update_app_install_cb (GObject      *source_object,
                       GAsyncResult *result,
                       gpointer      user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (source_object);
	g_autoptr(UpdateSingleAppData) app_data = g_steal_pointer (&user_data);
	GTask *task = app_data->task;
	UpdateAppsData *data = g_task_get_task_data (task);
	g_autoptr(GError) local_error = NULL;

	if (!gs_plugin_fwupd_install_finish (self, result, &local_error)) {
		gs_plugin_fwupd_error_convert (&local_error);
		finish_update_apps_op (task, g_steal_pointer (&local_error));
		return;
	}

	/* Simple progress reporting. */
	if (data->progress_callback != NULL) {
		data->progress_callback (GS_PLUGIN (self),
					 100 * ((gdouble) (app_data->index + 1) / data->n_apps),
					 data->progress_user_data);
	}

	/* App successfully updated. */
	finish_update_apps_op (task, NULL);
}

/* @error is (transfer full) if non-%NULL */
static void
finish_update_apps_op (GTask  *task,
                       GError *error)
{
	GsPluginFwupd *self = g_task_get_source_object (task);
	UpdateAppsData *data = g_task_get_task_data (task);
	g_autoptr(GError) error_owned = g_steal_pointer (&error);

	/* Report certain errors to the user directly. Any errors which we
	 * return from the `update_apps_async()` vfunc are logged but not
	 * displayed in the UI as the #GsPluginJobUpdateApps code can’t know
	 * which errors are understandable by users and which aren’t. */
	if (g_error_matches (error_owned, FWUPD_ERROR, FWUPD_ERROR_NEEDS_USER_ACTION)) {
		g_autoptr(GError) event_error = NULL;
		g_autoptr(GsPluginEvent) event = NULL;

		event_error = g_error_copy (error_owned);
		g_prefix_error_literal (&event_error, _("Firmware update could not be applied: "));
		gs_plugin_fwupd_error_convert (&event_error);

		event = gs_plugin_event_new ("app", self->app_current,
					     "error", event_error,
					     NULL);
		gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_WARNING);
		if (data->flags & GS_PLUGIN_UPDATE_APPS_FLAGS_INTERACTIVE)
			gs_plugin_event_add_flag (event, GS_PLUGIN_EVENT_FLAG_INTERACTIVE);
		gs_plugin_report_event (GS_PLUGIN (self), event);
	}

	if (error_owned != NULL && data->saved_error == NULL)
		data->saved_error = g_steal_pointer (&error_owned);
	else if (error_owned != NULL)
		g_debug ("Additional error while updating apps: %s", error_owned->message);

	g_assert (data->n_pending_ops > 0);
	data->n_pending_ops--;

	if (data->n_pending_ops > 0)
		return;

	/* Get the results of the parallel ops. */
	if (data->saved_error != NULL)
		g_task_return_error (task, g_steal_pointer (&data->saved_error));
	else
		g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_fwupd_update_apps_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gs_plugin_file_to_app (GsPlugin *plugin,
		       GsAppList *list,
		       GFile *file,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autofree gchar *content_type = NULL;
	g_autofree gchar *filename = NULL;
	g_autoptr(GPtrArray) devices = NULL;
	const gchar *mimetypes[] = {
		"application/vnd.ms-cab-compressed",
		NULL };

	/* does this match any of the mimetypes we support */
	content_type = gs_utils_get_content_type (file, cancellable, error);
	if (content_type == NULL)
		return FALSE;
	if (!g_strv_contains (mimetypes, content_type))
		return TRUE;

	/* get results */
	filename = g_file_get_path (file);
	devices = fwupd_client_get_details (self->client,
					    filename,
					    cancellable,
					    error);
	if (devices == NULL) {
		gs_plugin_fwupd_error_convert (error);
		return FALSE;
	}
	for (guint i = 0; i < devices->len; i++) {
		FwupdDevice *dev = g_ptr_array_index (devices, i);
		g_autoptr(GsApp) app = NULL;

		/* create each app */
		app = gs_plugin_fwupd_new_app_from_device (plugin, dev);

		/* we *might* have no update view for local files */
		gs_app_set_version (app, gs_app_get_update_version (app));
		gs_app_set_description (app, GS_APP_QUALITY_LOWEST,
					gs_app_get_update_details_markup (app));
		gs_app_list_add (list, app);
	}
	return TRUE;
}

gboolean
gs_plugin_add_sources (GsPlugin *plugin,
		       GsAppList *list,
		       GCancellable *cancellable,
		       GError **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GPtrArray) remotes = NULL;
	g_autoptr(GMutexLocker) locker = NULL;

	/* find all remotes */
	remotes = fwupd_client_get_remotes (self->client, cancellable, error);
	if (remotes == NULL)
		return FALSE;
	locker = g_mutex_locker_new (&self->cached_sources_mutex);
	if (self->cached_sources == NULL)
		self->cached_sources = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	for (guint i = 0; i < remotes->len; i++) {
		FwupdRemote *remote = g_ptr_array_index (remotes, i);
		g_autofree gchar *id = NULL;
		g_autoptr(GsApp) app = NULL;

		/* ignore these, they're built in */
		if (fwupd_remote_get_kind (remote) != FWUPD_REMOTE_KIND_DOWNLOAD)
			continue;

		/* create something that we can use to enable/disable */
		id = g_strdup_printf ("org.fwupd.%s.remote", fwupd_remote_get_id (remote));
		app = g_hash_table_lookup (self->cached_sources, id);
		if (app == NULL) {
			gboolean is_enabled;

			#if FWUPD_CHECK_VERSION(1, 9, 4)
			is_enabled = fwupd_remote_has_flag (remote, FWUPD_REMOTE_FLAG_ENABLED);
			#else
			is_enabled = fwupd_remote_get_enabled (remote);
			#endif

			app = gs_app_new (id);
			gs_app_set_kind (app, AS_COMPONENT_KIND_REPOSITORY);
			gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
			gs_app_set_state (app, is_enabled ?
					  GS_APP_STATE_INSTALLED : GS_APP_STATE_AVAILABLE);
			gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
			gs_app_set_name (app, GS_APP_QUALITY_LOWEST,
					 fwupd_remote_get_title (remote));
			gs_app_set_agreement (app, fwupd_remote_get_agreement (remote));
			gs_app_set_url (app, AS_URL_KIND_HOMEPAGE,
					fwupd_remote_get_metadata_uri (remote));
			gs_app_set_metadata (app, "fwupd::remote-id",
					     fwupd_remote_get_id (remote));
			gs_app_set_management_plugin (app, plugin);
			gs_app_set_metadata (app, "LingmoAppstore::PackagingFormat", "fwupd");
			gs_app_set_metadata (app, "LingmoAppstore::SortKey", "800");
			gs_app_set_origin_ui (app, _("Firmware"));
			g_hash_table_insert (self->cached_sources, g_strdup (id), app);
			g_object_weak_ref (G_OBJECT (app), cached_sources_weak_ref_cb, self);
		} else {
			g_object_ref (app);
			/* The repo-related apps are those installed; due to re-using
			   cached app, make sure the list is populated from fresh data. */
			gs_app_list_remove_all (gs_app_get_related (app));
		}
		gs_app_list_add (list, app);
	}
	return TRUE;
}

static void
gs_plugin_fwupd_enable_repository_remote_refresh_ready_cb (GObject      *source_object,
							   GAsyncResult *result,
							   gpointer      user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GError) local_error = NULL;

	if (!fwupd_client_refresh_remote_finish (FWUPD_CLIENT (source_object), result, &local_error))
		g_debug ("Failed to refresh remote after enable: %s", local_error ? local_error->message : "Unknown error");

	/* Silently ignore refresh errors */
	g_task_return_boolean (task, TRUE);
}

static void
gs_plugin_fwupd_enable_repository_get_remotes_ready_cb (GObject      *source_object,
							GAsyncResult *result,
							gpointer      user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GPtrArray) remotes = NULL;
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (g_task_get_source_object (task));
	GsApp *repository = g_task_get_task_data (task);
	const gchar *remote_id;
	guint cache_age = 1;

	remotes = fwupd_client_get_remotes_finish (FWUPD_CLIENT (source_object), result, &local_error);
	if (remotes == NULL) {
		g_debug ("No remotes found after remote enable: %s", local_error ? local_error->message : "Unknown error");
		/* Silently ignore refresh errors */
		g_task_return_boolean (task, TRUE);
		return;
	}

	remote_id = gs_app_get_metadata_item (repository, "fwupd::remote-id");
	g_assert (remote_id != NULL);

	for (guint i = 0; i < remotes->len; i++) {
		FwupdRemote *remote = g_ptr_array_index (remotes, i);
		if (g_strcmp0 (remote_id, fwupd_remote_get_id (remote)) == 0) {
			gboolean is_enabled;
			#if FWUPD_CHECK_VERSION(1, 9, 4)
			is_enabled = fwupd_remote_has_flag (remote, FWUPD_REMOTE_FLAG_ENABLED);
			#else
			is_enabled = fwupd_remote_get_enabled (remote);
			#endif
			if (is_enabled &&
			    fwupd_remote_get_kind (remote) != FWUPD_REMOTE_KIND_LOCAL &&
			    !remote_cache_is_expired (remote, cache_age)) {
				GCancellable *cancellable = g_task_get_cancellable (task);
				#if FWUPD_CHECK_VERSION(2, 0, 0)
				fwupd_client_refresh_remote_async (self->client, remote, FWUPD_CLIENT_DOWNLOAD_FLAG_NONE, cancellable,
								   gs_plugin_fwupd_enable_repository_remote_refresh_ready_cb,
								   g_steal_pointer (&task));
				#elif FWUPD_CHECK_VERSION(1, 9, 4)
				fwupd_client_refresh_remote2_async (self->client, remote, FWUPD_CLIENT_DOWNLOAD_FLAG_NONE, cancellable,
								    gs_plugin_fwupd_enable_repository_remote_refresh_ready_cb,
								    g_steal_pointer (&task));
				#else
				fwupd_client_refresh_remote_async (self->client, remote, cancellable,
								   gs_plugin_fwupd_enable_repository_remote_refresh_ready_cb,
								   g_steal_pointer (&task));
				#endif
				return;
			}
			break;
		}
	}

	g_task_return_boolean (task, TRUE);
}

static void
gs_plugin_fwupd_enable_repository_ready_cb (GObject	 *source_object,
					    GAsyncResult *result,
					    gpointer	  user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GError) local_error = NULL;
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (g_task_get_source_object (task));
	GCancellable *cancellable = g_task_get_cancellable (task);

	if (!gs_plugin_fwupd_modify_source_finish (self, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* This can fail silently, it's only to update necessary caches, to provide
	 * up-to-date information after the successful repository enable/install. */
	fwupd_client_get_remotes_async (self->client,
					cancellable,
					gs_plugin_fwupd_enable_repository_get_remotes_ready_cb,
					g_steal_pointer (&task));
}

static void
gs_plugin_fwupd_enable_repository_async (GsPlugin                     *plugin,
					 GsApp			      *repository,
                                         GsPluginManageRepositoryFlags flags,
                                         GCancellable		      *cancellable,
                                         GAsyncReadyCallback	       callback,
                                         gpointer		       user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_task_data (task, g_object_ref (repository), g_object_unref);
	g_task_set_source_tag (task, gs_plugin_fwupd_enable_repository_async);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (repository, plugin)) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	gs_plugin_fwupd_modify_source_async (self, repository, TRUE, cancellable,
		gs_plugin_fwupd_enable_repository_ready_cb, g_steal_pointer (&task));
}

static gboolean
gs_plugin_fwupd_enable_repository_finish (GsPlugin      *plugin,
					  GAsyncResult  *result,
					  GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_fwupd_disable_repository_async (GsPlugin                     *plugin,
					  GsApp			      *repository,
                                          GsPluginManageRepositoryFlags flags,
                                          GCancellable		      *cancellable,
                                          GAsyncReadyCallback	       callback,
                                          gpointer		       user_data)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);

	/* only process this app if was created by this plugin */
	if (!gs_app_has_management_plugin (repository, plugin)) {
		g_autoptr(GTask) task = NULL;

		task = g_task_new (self, cancellable, callback, user_data);
		g_task_set_source_tag (task, gs_plugin_fwupd_disable_repository_async);
		g_task_return_boolean (task, TRUE);
		return;
	}

	gs_plugin_fwupd_modify_source_async (self, repository, FALSE, cancellable, callback, user_data);
}

static gboolean
gs_plugin_fwupd_disable_repository_finish (GsPlugin      *plugin,
					   GAsyncResult  *result,
					   GError       **error)
{
	GsPluginFwupd *self = GS_PLUGIN_FWUPD (plugin);
	return gs_plugin_fwupd_modify_source_finish (self, result, error);
}

static void
gs_plugin_fwupd_class_init (GsPluginFwupdClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_fwupd_dispose;
	object_class->finalize = gs_plugin_fwupd_finalize;

	plugin_class->setup_async = gs_plugin_fwupd_setup_async;
	plugin_class->setup_finish = gs_plugin_fwupd_setup_finish;
	plugin_class->refresh_metadata_async = gs_plugin_fwupd_refresh_metadata_async;
	plugin_class->refresh_metadata_finish = gs_plugin_fwupd_refresh_metadata_finish;
	plugin_class->enable_repository_async = gs_plugin_fwupd_enable_repository_async;
	plugin_class->enable_repository_finish = gs_plugin_fwupd_enable_repository_finish;
	plugin_class->disable_repository_async = gs_plugin_fwupd_disable_repository_async;
	plugin_class->disable_repository_finish = gs_plugin_fwupd_disable_repository_finish;
	plugin_class->update_apps_async = gs_plugin_fwupd_update_apps_async;
	plugin_class->update_apps_finish = gs_plugin_fwupd_update_apps_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_FWUPD;
}
