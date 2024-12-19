/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Canonical Ltd
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <snapd-glib/snapd-glib.h>

#include "config.h"

#include "lingmo-appstore-private.h"

#include "gs-test.h"

static gboolean snap_installed = FALSE;

SnapdAuthData *
snapd_login_sync (const gchar *username, const gchar *password, const gchar *otp,
		  GCancellable *cancellable, GError **error)
{
	return snapd_auth_data_new ("macaroon", NULL);
}

SnapdClient *
snapd_client_new (void)
{
	/* use a dummy object - we intercept all snapd-glib calls */
	return g_object_new (G_TYPE_OBJECT, NULL);
}

void
snapd_client_set_allow_interaction (SnapdClient *client, gboolean allow_interaction)
{
}

void
snapd_client_set_auth_data (SnapdClient *client, SnapdAuthData *auth_data)
{
}

gboolean
snapd_client_connect_sync (SnapdClient *client, GCancellable *cancellable, GError **error)
{
	/* skip connection */
	return TRUE;
}

const gchar *
snapd_client_get_user_agent (SnapdClient *client)
{
	return "snapd-glib/0.0.1";
}

void
snapd_client_set_user_agent (SnapdClient *client, const gchar *user_agent)
{
}

SnapdSystemInformation *
snapd_client_get_system_information_sync (SnapdClient *client, GCancellable *cancellable, GError **error)
{
	g_autoptr(GHashTable) sandbox_features = g_hash_table_new (g_str_hash, g_str_equal);
	return g_object_new (SNAPD_TYPE_SYSTEM_INFORMATION,
			     "version", "2.31",
			     "confinement", SNAPD_SYSTEM_CONFINEMENT_STRICT,
			     "sandbox-features", sandbox_features,
			     NULL);
}

static SnapdSnap *
make_snap (const gchar *name, SnapdSnapStatus status)
{
	gchar *common_ids[] = { NULL };
	g_autoptr(GDateTime) install_date = NULL;
	g_autoptr(GPtrArray) apps = NULL;
	g_autoptr(GPtrArray) media = NULL;
	SnapdMedia *m;

	install_date = g_date_time_new_utc (2017, 1, 2, 11, 23, 58);

	apps = g_ptr_array_new_with_free_func (g_object_unref);

	media = g_ptr_array_new_with_free_func (g_object_unref);
	m = g_object_new (SNAPD_TYPE_MEDIA,
			  "type", "screenshot",
			  "url", "http://example.com/screenshot1.jpg",
			  "width", 640,
			  "height", 480,
			  NULL);
	g_ptr_array_add (media, m);
	m = g_object_new (SNAPD_TYPE_MEDIA,
			  "type", "screenshot",
			  "url", "http://example.com/screenshot2.jpg",
			  "width", 1024,
			  "height", 768,
			  NULL);
	g_ptr_array_add (media, m);

	return g_object_new (SNAPD_TYPE_SNAP,
			     "apps", status == SNAPD_SNAP_STATUS_INSTALLED ? apps : NULL,
			     "common-ids", common_ids,
			     "description", "DESCRIPTION",
			     "download-size", status == SNAPD_SNAP_STATUS_AVAILABLE ? 500 : 0,
			     "icon", status == SNAPD_SNAP_STATUS_AVAILABLE ? NULL : "/icon",
			     "id", name,
			     "install-date", status == SNAPD_SNAP_STATUS_INSTALLED ? install_date : NULL,
			     "installed-size", status == SNAPD_SNAP_STATUS_INSTALLED ? 1000 : 0,
			     "media", status == SNAPD_SNAP_STATUS_AVAILABLE ? media : NULL,
			     "name", name,
			     "status", status,
			     "snap-type", SNAPD_SNAP_TYPE_APP,
			     "summary", "SUMMARY",
			     "version", "VERSION",
			     NULL);
}

GPtrArray *
snapd_client_get_snaps_sync (SnapdClient *client,
			     SnapdGetSnapsFlags flags, gchar **names,
			     GCancellable *cancellable, GError **error)
{
	GPtrArray *snaps;

	snaps = g_ptr_array_new_with_free_func (g_object_unref);
	if (snap_installed)
		g_ptr_array_add (snaps, make_snap ("snap", SNAPD_SNAP_STATUS_INSTALLED));

	return snaps;
}

SnapdSnap *
snapd_client_get_snap_sync (SnapdClient *client,
			    const gchar *name,
			    GCancellable *cancellable, GError **error)
{
	if (snap_installed) {
		return make_snap ("snap", SNAPD_SNAP_STATUS_INSTALLED);
	} else {
		g_set_error_literal (error, SNAPD_ERROR, SNAPD_ERROR_NOT_INSTALLED, "not installed");
		return NULL;
	}
}

SnapdIcon *
snapd_client_get_icon_sync (SnapdClient *client,
			    const gchar *name,
			    GCancellable *cancellable, GError **error)
{
	g_autoptr(GBytes) data = NULL;
	/* apparently this is the smallest valid PNG file (1x1) */
	const gchar png_data[67] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
				     0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
				     0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
				     0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
				     0x89, 0x00, 0x00, 0x00, 0x0A, 0x49, 0x44, 0x41,
				     0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
				     0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00,
				     0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
				     0x42, 0x60, 0x82 };

	data = g_bytes_new (png_data, 67);
	return g_object_new (SNAPD_TYPE_ICON,
			     "mime-type", "image/png",
			     "data", data,
			     NULL);
}

gboolean
snapd_client_get_connections_sync (SnapdClient *client,
				   GPtrArray **established, GPtrArray **undesired,
				   GPtrArray **plugs, GPtrArray **slots,
				   GCancellable *cancellable, GError **error)
{
	if (plugs)
		*plugs = g_ptr_array_new_with_free_func (g_object_unref);
	if (slots)
		*slots = g_ptr_array_new_with_free_func (g_object_unref);
	return TRUE;
}

GPtrArray *
snapd_client_find_section_sync (SnapdClient *client,
				SnapdFindFlags flags,
				const gchar *section, const gchar *query,
				gchar **suggested_currency,
				GCancellable *cancellable, GError **error)
{
	GPtrArray *snaps;

	snaps = g_ptr_array_new_with_free_func (g_object_unref);
	g_ptr_array_add (snaps, make_snap ("snap", SNAPD_SNAP_STATUS_AVAILABLE));

	return snaps;
}

gboolean
snapd_client_install2_sync (SnapdClient *client,
			    SnapdInstallFlags flags,
			    const gchar *name, const gchar *channel, const gchar *revision,
			    SnapdProgressCallback progress_callback, gpointer progress_callback_data,
			    GCancellable *cancellable, GError **error)
{
	g_autoptr(SnapdChange) change = NULL;
	g_autoptr(GPtrArray) tasks = NULL;
	SnapdTask *task;

	g_assert_cmpstr (name, ==, "snap");
	g_assert (channel == NULL);

	tasks = g_ptr_array_new_with_free_func (g_object_unref);
	task = g_object_new (SNAPD_TYPE_TASK,
			     "progress-done", 0,
			     "progress-total", 1,
			     NULL);
	g_ptr_array_add (tasks, task);
	change = g_object_new (SNAPD_TYPE_CHANGE,
			       "tasks", tasks,
			       NULL);
	progress_callback (client, change, NULL, progress_callback_data);

	snap_installed = TRUE;
	return TRUE;
}

gboolean
snapd_client_remove_sync (SnapdClient *client,
			  const gchar *name,
			  SnapdProgressCallback progress_callback, gpointer progress_callback_data,
			  GCancellable *cancellable, GError **error)
{
	g_autoptr(SnapdChange) change = NULL;
	g_autoptr(GPtrArray) tasks = NULL;
	SnapdTask *task;

	g_assert_cmpstr (name, ==, "snap");

	tasks = g_ptr_array_new_with_free_func (g_object_unref);
	task = g_object_new (SNAPD_TYPE_TASK,
			     "progress-done", 0,
			     "progress-total", 1,
			     NULL);
	g_ptr_array_add (tasks, task);
	change = g_object_new (SNAPD_TYPE_CHANGE,
			       "tasks", tasks,
			       NULL);
	progress_callback (client, change, NULL, progress_callback_data);

	snap_installed = FALSE;
	return TRUE;
}

static void
gs_plugins_snap_test_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppList) apps = NULL;
	gboolean ret;
	GsApp *app;
	GPtrArray *screenshots, *images;
	AsScreenshot *screenshot;
	AsImage *image;
	g_autoptr(GIcon) icon = NULL;
	g_autoptr(GInputStream) icon_stream = NULL;
	g_autoptr(GdkPixbuf) pixbuf = NULL;
	g_autoptr(GError) error = NULL;
	GsSizeType size_installed_type, size_download_type;
	guint64 size_installed_bytes, size_download_bytes;
	const gchar *keywords[] = { NULL, };
	g_autoptr(GsAppQuery) query = NULL;

	/* no snap, abort */
	if (!gs_plugin_loader_get_enabled (plugin_loader, "snap")) {
		g_test_skip ("not enabled");
		return;
	}

	keywords[0] = "snap";
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
						  GS_PLUGIN_REFINE_FLAGS_REQUIRE_SCREENSHOTS,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_match_value,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	apps = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	g_assert_no_error (error);
	g_assert (apps != NULL);
	g_assert_cmpint (gs_app_list_length (apps), ==, 1);
	app = gs_app_list_index (apps, 0);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_AVAILABLE);
	g_assert_cmpstr (gs_app_get_name (app), ==, "snap");
	g_assert_cmpstr (gs_app_get_version (app), ==, "VERSION");
	g_assert_cmpstr (gs_app_get_summary (app), ==, "SUMMARY");
	g_assert_cmpstr (gs_app_get_description (app), ==, "DESCRIPTION");
	screenshots = gs_app_get_screenshots (app);
	g_assert_cmpint (screenshots->len, ==, 2);
	screenshot = g_ptr_array_index (screenshots, 0);
	images = as_screenshot_get_images (screenshot);
	g_assert_cmpint (images->len, ==, 1);
	image = g_ptr_array_index (images, 0);
	g_assert_cmpstr (as_image_get_url (image), ==, "http://example.com/screenshot1.jpg");
	g_assert_cmpint (as_image_get_width (image), ==, 640);
	g_assert_cmpint (as_image_get_height (image), ==, 480);
	screenshot = g_ptr_array_index (screenshots, 1);
	images = as_screenshot_get_images (screenshot);
	g_assert_cmpint (images->len, ==, 1);
	image = g_ptr_array_index (images, 0);
	g_assert_cmpstr (as_image_get_url (image), ==, "http://example.com/screenshot2.jpg");
	g_assert_cmpint (as_image_get_width (image), ==, 1024);
	g_assert_cmpint (as_image_get_height (image), ==, 768);
	icon = gs_app_get_icon_for_size (app, 64, 1, NULL);
	g_assert_null (icon);

	size_installed_type = gs_app_get_size_installed (app, &size_installed_bytes);
	g_assert_cmpint (size_installed_type, ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpuint (size_installed_bytes, ==, 0);

	size_download_type = gs_app_get_size_download (app, &size_download_bytes);
	g_assert_cmpint (size_download_type, ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpuint (size_download_bytes, ==, 500);

	g_assert_cmpint (gs_app_get_install_date (app), ==, 0);

	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_INSTALL,
					 "app", app,
					 "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
					 NULL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);

	size_installed_type = gs_app_get_size_installed (app, &size_installed_bytes);
	g_assert_cmpint (size_installed_type, ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpuint (size_installed_bytes, ==, 1000);

	g_assert_cmpint (gs_app_get_install_date (app), ==, g_date_time_to_unix (g_date_time_new_utc (2017, 1, 2, 11, 23, 58)));

	icon = gs_app_get_icon_for_size (app, 128, 1, NULL);
	g_assert_nonnull (icon);
	g_assert_true (G_IS_LOADABLE_ICON (icon));
	icon_stream = g_loadable_icon_load (G_LOADABLE_ICON (icon), 128, NULL, NULL, &error);
	g_assert_no_error (error);
	g_assert_nonnull (icon_stream);
	pixbuf = gdk_pixbuf_new_from_stream (icon_stream, NULL, &error);
	g_assert_no_error (error);
	g_assert_nonnull (pixbuf);
	g_assert_cmpint (gdk_pixbuf_get_width (pixbuf), ==, 128);
	g_assert_cmpint (gdk_pixbuf_get_height (pixbuf), ==, 128);

	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_REMOVE,
					 "app", app,
					 NULL);
	gs_test_flush_main_context ();
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	g_assert_no_error (error);
	g_assert (ret);
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"snap",
		NULL
	};

	gs_test_init (&argc, &argv);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR_CORE);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/snap/test",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_snap_test_func);
	return g_test_run ();
}
