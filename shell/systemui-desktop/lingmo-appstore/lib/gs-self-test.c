/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "lingmo-appstore-private.h"

#include "gs-debug.h"
#include "gs-test.h"

static gboolean
gs_app_list_filter_cb (GsApp *app, gpointer user_data)
{
	if (g_strcmp0 (gs_app_get_id (app), "a") == 0)
		return FALSE;
	if (g_strcmp0 (gs_app_get_id (app), "c") == 0)
		return FALSE;
	return TRUE;
}

static void
gs_utils_url_func (void)
{
	g_autofree gchar *path1 = NULL;
	g_autofree gchar *path2 = NULL;
	g_autofree gchar *path3 = NULL;
	g_autofree gchar *scheme1 = NULL;
	g_autofree gchar *scheme2 = NULL;

	scheme1 = gs_utils_get_url_scheme ("appstream://gimp.desktop");
	g_assert_cmpstr (scheme1, ==, "appstream");
	scheme2 = gs_utils_get_url_scheme ("appstream:gimp.desktop");
	g_assert_cmpstr (scheme2, ==, "appstream");

	path1 = gs_utils_get_url_path ("appstream://gimp.desktop");
	g_assert_cmpstr (path1, ==, "gimp.desktop");
	path2 = gs_utils_get_url_path ("appstream:gimp.desktop");
	g_assert_cmpstr (path2, ==, "gimp.desktop");
	path3 = gs_utils_get_url_path ("apt:/gimp");
	g_assert_cmpstr (path3, ==, "gimp");
}

static void
gs_utils_wilson_func (void)
{
	g_assert_cmpint ((gint64) gs_utils_get_wilson_rating (0, 0, 0, 0, 0), ==, -1);
	g_assert_cmpint ((gint64) gs_utils_get_wilson_rating (0, 0, 0, 0, 400), ==, 100);
	g_assert_cmpint ((gint64) gs_utils_get_wilson_rating (10, 0, 0, 0, 400), ==, 98);
	g_assert_cmpint ((gint64) gs_utils_get_wilson_rating (0, 0, 0, 0, 1), ==, 76);
	g_assert_cmpint ((gint64) gs_utils_get_wilson_rating (5, 4, 20, 100, 400), ==, 93);
}

static void
gs_os_release_func (void)
{
	g_autofree gchar *fn = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsOsRelease) os_release = NULL;

	fn = gs_test_get_filename (TESTDATADIR, "tests/os-release");
	g_assert (fn != NULL);
	g_setenv ("GS_SELF_TEST_OS_RELEASE_FILENAME", fn, TRUE);

	os_release = gs_os_release_new (&error);
	g_assert_no_error (error);
	g_assert (os_release != NULL);
	g_assert_cmpstr (gs_os_release_get_id (os_release), ==, "fedora");
	g_assert_cmpstr (gs_os_release_get_name (os_release), ==, "Fedora");
	g_assert_cmpstr (gs_os_release_get_version (os_release), ==, "25 (Workstation Edition)");
	g_assert_cmpstr (gs_os_release_get_version_id (os_release), ==, "25");
	g_assert_cmpstr (gs_os_release_get_pretty_name (os_release), ==, "Fedora 25 (Workstation Edition)");
}

static void
gs_utils_append_kv_func (void)
{
	g_autoptr(GString) str = g_string_new (NULL);

	/* normal */
	gs_utils_append_key_value (str, 5, "key", "val");
	g_assert_cmpstr (str->str, ==, "key:  val\n");

	/* oversize */
	g_string_truncate (str, 0);
	gs_utils_append_key_value (str, 5, "longkey", "val");
	g_assert_cmpstr (str->str, ==, "longkey: val\n");

	/* null key */
	g_string_truncate (str, 0);
	gs_utils_append_key_value (str, 5, NULL, "val");
	g_assert_cmpstr (str->str, ==, "      val\n");

	/* zero align key */
	g_string_truncate (str, 0);
	gs_utils_append_key_value (str, 0, "key", "val");
	g_assert_cmpstr (str->str, ==, "key: val\n");
}

static void
gs_utils_cache_func (void)
{
	g_autofree gchar *fn1 = NULL;
	g_autofree gchar *fn2 = NULL;
	g_autoptr(GError) error = NULL;

	fn1 = gs_utils_get_cache_filename ("test",
					   "http://www.foo.bar/baz",
					   GS_UTILS_CACHE_FLAG_WRITEABLE |
					   GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					   &error);
	g_assert_no_error (error);
	g_assert_cmpstr (fn1, !=, NULL);
	g_assert (g_str_has_prefix (fn1, g_get_user_cache_dir ()));
	g_assert (g_str_has_suffix (fn1, "test/baz"));

	fn2 = gs_utils_get_cache_filename ("test",
					   "http://www.foo.bar/baz",
					   GS_UTILS_CACHE_FLAG_WRITEABLE |
					   GS_UTILS_CACHE_FLAG_USE_HASH |
					   GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					   &error);
	g_assert_no_error (error);
	g_assert_cmpstr (fn2, !=, NULL);
	g_assert (g_str_has_prefix (fn2, g_get_user_cache_dir ()));
	g_assert (g_str_has_suffix (fn2, "test/295099f59d12b3eb0b955325fcb699cd23792a89-baz"));
}

static void
gs_utils_error_func (void)
{
	g_autofree gchar *app_id = NULL;
	g_autofree gchar *origin_id = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = gs_app_new ("gimp.desktop");
	g_autoptr(GsApp) origin = gs_app_new ("gimp-repo");

	for (guint i = 0; i < GS_PLUGIN_ERROR_LAST; i++)
		g_assert (gs_plugin_error_to_string (i) != NULL);

	/* noop */
	gs_utils_error_add_app_id (&error, app);
	gs_utils_error_add_origin_id (&error, origin);

	g_set_error (&error,
		     GS_PLUGIN_ERROR,
		     GS_PLUGIN_ERROR_DOWNLOAD_FAILED,
		     "failed");
	g_assert_cmpstr (error->message, ==, "failed");
	gs_utils_error_add_app_id (&error, app);
	gs_utils_error_add_origin_id (&error, origin);
	g_assert_cmpstr (error->message, ==, "[*/*/*/gimp-repo/*] {*/*/*/gimp.desktop/*} failed");

	/* find and strip any unique IDs from the error message */
	for (guint i = 0; i < 2; i++) {
		if (app_id == NULL)
			app_id = gs_utils_error_strip_app_id (error);
		if (origin_id == NULL)
			origin_id = gs_utils_error_strip_origin_id (error);
	}

	g_assert_cmpstr (app_id, ==, "*/*/*/gimp.desktop/*");
	g_assert_cmpstr (origin_id, ==, "*/*/*/gimp-repo/*");
	g_assert_cmpstr (error->message, ==, "failed");
}

static void
async_result_cb (GObject      *source_object,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GAsyncResult **result_out = user_data;

	g_assert (*result_out == NULL);
	*result_out = g_object_ref (result);
	g_main_context_wakeup (g_main_context_get_thread_default ());
}

static void
gs_plugin_download_rewrite_func (void)
{
	g_autofree gchar *css = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GAsyncResult) result = NULL;
	g_autoptr(GMainContext) context = g_main_context_new ();
	g_autoptr(GMainContextPusher) context_pusher = g_main_context_pusher_new (context);
	const gchar *resource = "background:\n"
				" url('file://" DATADIR "/lingmo-appstore/featured-maps.png')\n"
				" url('file://" DATADIR "/lingmo-appstore/featured-maps-bg.png')\n"
				" bottom center / contain no-repeat;\n";

	/* only when installed */
	if (!g_file_test (DATADIR "/lingmo-appstore/featured-maps.png", G_FILE_TEST_EXISTS)) {
		g_test_skip ("not installed");
		return;
	}

	/* test rewrite */
	gs_download_rewrite_resource_async (resource, NULL, async_result_cb, &result);

	while (result == NULL)
		g_main_context_iteration (context, TRUE);

	css = gs_download_rewrite_resource_finish (result, &error);
	g_assert_no_error (error);
	g_assert (css != NULL);
}

static void
gs_plugin_func (void)
{
	GsAppList *list;
	GsAppList *list_dup;
	GsAppList *list_remove;
	GsApp *app;

	/* check enums converted */
	for (guint i = 0; i < GS_PLUGIN_ACTION_LAST; i++) {
		const gchar *tmp = gs_plugin_action_to_string (i);
		if (tmp == NULL)
			g_critical ("failed to convert %u", i);
		g_assert_cmpint (gs_plugin_action_from_string (tmp), ==, i);
	}
	for (guint i = 1; i < GS_PLUGIN_ACTION_LAST; i++) {
		const gchar *tmp = gs_plugin_action_to_function_name (i);
		if (tmp == NULL) {
			/* These do not have function, they exist only for better error messages. */
			if (i == GS_PLUGIN_ACTION_INSTALL_REPO ||
			    i == GS_PLUGIN_ACTION_REMOVE_REPO ||
			    i == GS_PLUGIN_ACTION_ENABLE_REPO ||
			    i == GS_PLUGIN_ACTION_DISABLE_REPO)
				continue;
			g_critical ("failed to convert %u", i);
		}
	}

	/* add a couple of duplicate IDs */
	app = gs_app_new ("a");
	list = gs_app_list_new ();
	gs_app_list_add (list, app);
	g_object_unref (app);

	/* test refcounting */
	g_assert_cmpstr (gs_app_get_id (gs_app_list_index (list, 0)), ==, "a");
	list_dup = gs_app_list_copy (list);
	g_object_unref (list);
	g_assert_cmpint (gs_app_list_length (list_dup), ==, 1);
	g_assert_cmpstr (gs_app_get_id (gs_app_list_index (list_dup, 0)), ==, "a");
	g_object_unref (list_dup);

	/* test removing objects */
	app = gs_app_new ("a");
	list_remove = gs_app_list_new ();
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	app = gs_app_new ("b");
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	app = gs_app_new ("c");
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 3);
	gs_app_list_filter (list_remove, gs_app_list_filter_cb, NULL);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 1);
	g_assert_cmpstr (gs_app_get_id (gs_app_list_index (list_remove, 0)), ==, "b");

	/* test removing duplicates at runtime */
	app = gs_app_new ("b");
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	app = gs_app_new ("b");
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 1);
	g_assert_cmpstr (gs_app_get_id (gs_app_list_index (list_remove, 0)), ==, "b");
	g_object_unref (list_remove);

	/* test removing duplicates when lazy-loading */
	list_remove = gs_app_list_new ();
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	gs_app_set_id (app, "e");
	g_object_unref (app);
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	gs_app_set_id (app, "e");
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 2);
	gs_app_list_filter_duplicates (list_remove, GS_APP_LIST_FILTER_FLAG_NONE);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 1);
	g_object_unref (list_remove);

	/* test removing duplicates when some apps have no app ID */
	list_remove = gs_app_list_new ();
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	g_object_unref (app);
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	gs_app_set_id (app, "e");
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 3);
	gs_app_list_filter_duplicates (list_remove, GS_APP_LIST_FILTER_FLAG_NONE);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 3);
	g_object_unref (list_remove);

	/* remove lazy-loaded app */
	list_remove = gs_app_list_new ();
	app = gs_app_new (NULL);
	gs_app_list_add (list_remove, app);
	gs_app_list_remove (list_remove, app);
	g_assert_cmpint (gs_app_list_length (list_remove), ==, 0);
	g_object_unref (app);
	g_object_unref (list_remove);

	/* respect priority when deduplicating */
	list = gs_app_list_new ();
	app = gs_app_new ("e");
	gs_app_set_unique_id (app, "user/foo/*/e/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 0);
	g_object_unref (app);
	app = gs_app_new ("e");
	gs_app_set_unique_id (app, "user/bar/*/e/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 99);
	g_object_unref (app);
	app = gs_app_new ("e");
	gs_app_set_unique_id (app, "user/baz/*/e/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 50);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 3);
	gs_app_list_filter_duplicates (list, GS_APP_LIST_FILTER_FLAG_KEY_ID);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_assert_cmpstr (gs_app_get_unique_id (gs_app_list_index (list, 0)), ==, "user/bar/*/e/*");
	g_object_unref (list);

	/* respect priority (using name and version) when deduplicating */
	list = gs_app_list_new ();
	app = gs_app_new ("e");
	gs_app_add_source (app, "foo");
	gs_app_set_version (app, "1.2.3");
	gs_app_set_unique_id (app, "user/foo/repo/*/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 0);
	g_object_unref (app);
	app = gs_app_new ("e");
	gs_app_add_source (app, "foo");
	gs_app_set_version (app, "1.2.3");
	gs_app_set_unique_id (app, "user/foo/repo-security/*/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 99);
	g_object_unref (app);
	app = gs_app_new ("e");
	gs_app_add_source (app, "foo");
	gs_app_set_version (app, "1.2.3");
	gs_app_set_unique_id (app, "user/foo/repo-universe/*/*");
	gs_app_list_add (list, app);
	gs_app_set_priority (app, 50);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 3);
	gs_app_list_filter_duplicates (list, GS_APP_LIST_FILTER_FLAG_KEY_ID |
					     GS_APP_LIST_FILTER_FLAG_KEY_SOURCE |
					     GS_APP_LIST_FILTER_FLAG_KEY_VERSION);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_assert_cmpstr (gs_app_get_unique_id (gs_app_list_index (list, 0)), ==, "user/foo/repo-security/*/*");
	g_object_unref (list);

	/* prefer installed apps */
	list = gs_app_list_new ();
	app = gs_app_new ("e");
	gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	gs_app_set_unique_id (app, "user/foo/*/e/*");
	gs_app_set_priority (app, 0);
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("e");
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
	gs_app_set_unique_id (app, "user/bar/*/e/*");
	gs_app_set_priority (app, 100);
	gs_app_list_add (list, app);
	g_object_unref (app);
	gs_app_list_filter_duplicates (list,
				       GS_APP_LIST_FILTER_FLAG_KEY_ID |
				       GS_APP_LIST_FILTER_FLAG_PREFER_INSTALLED);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_assert_cmpstr (gs_app_get_unique_id (gs_app_list_index (list, 0)), ==, "user/foo/*/e/*");
	g_object_unref (list);

	/* use the provides ID to dedupe */
	list = gs_app_list_new ();
	app = gs_app_new ("gimp.desktop");
	gs_app_set_unique_id (app, "user/fedora/*/gimp.desktop/*");
	gs_app_set_priority (app, 0);
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("org.gimp.GIMP");
	gs_app_add_provided_item (app,
				  AS_PROVIDED_KIND_ID,
				  "gimp.desktop");
	gs_app_set_unique_id (app, "user/flathub/*/org.gimp.GIMP/*");
	gs_app_set_priority (app, 100);
	gs_app_list_add (list, app);
	g_object_unref (app);
	gs_app_list_filter_duplicates (list, GS_APP_LIST_FILTER_FLAG_KEY_ID_PROVIDES);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_assert_cmpstr (gs_app_get_unique_id (gs_app_list_index (list, 0)), ==,
			 "user/flathub/*/org.gimp.GIMP/*");
	g_object_unref (list);

	/* use globs when adding */
	list = gs_app_list_new ();
	app = gs_app_new ("b");
	gs_app_set_unique_id (app, "a/b/c/d/e");
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("b");
	gs_app_set_unique_id (app, "a/b/c/*/e");
	gs_app_list_add (list, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_assert_cmpstr (gs_app_get_id (gs_app_list_index (list, 0)), ==, "b");
	g_object_unref (list);

	/* lookup with a wildcard */
	list = gs_app_list_new ();
	app = gs_app_new ("b");
	gs_app_set_unique_id (app, "a/b/c/d/e");
	gs_app_list_add (list, app);
	g_object_unref (app);
	g_assert (gs_app_list_lookup (list, "a/b/c/d/e") != NULL);
	g_assert (gs_app_list_lookup (list, "a/b/c/d/*") != NULL);
	g_assert (gs_app_list_lookup (list, "*/b/c/d/e") != NULL);
	g_assert (gs_app_list_lookup (list, "x/x/x/x/x") == NULL);
	g_object_unref (list);

	/* allow duplicating a wildcard */
	list = gs_app_list_new ();
	app = gs_app_new ("gimp.desktop");
	gs_app_add_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("gimp.desktop");
	gs_app_set_unique_id (app, "system/flatpak/*/gimp.desktop/stable");
	gs_app_list_add (list, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 2);
	g_object_unref (list);

	/* allow duplicating a wildcard */
	list = gs_app_list_new ();
	app = gs_app_new ("gimp.desktop");
	gs_app_add_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("gimp.desktop");
	gs_app_add_quirk (app, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_list_add (list, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	g_object_unref (list);

	/* add a list to a list */
	list = gs_app_list_new ();
	list_dup = gs_app_list_new ();
	app = gs_app_new ("a");
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("b");
	gs_app_list_add (list_dup, app);
	g_object_unref (app);
	gs_app_list_add_list (list, list_dup);
	g_assert_cmpint (gs_app_list_length (list), ==, 2);
	g_assert_cmpint (gs_app_list_length (list_dup), ==, 1);
	g_object_unref (list);
	g_object_unref (list_dup);

	/* remove apps from the list */
	list = gs_app_list_new ();
	app = gs_app_new ("a");
	gs_app_list_add (list, app);
	gs_app_list_remove (list, app);
	g_object_unref (app);
	g_assert_cmpint (gs_app_list_length (list), ==, 0);
	g_object_unref (list);

	/* truncate list */
	list = gs_app_list_new ();
	app = gs_app_new ("a");
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("b");
	gs_app_list_add (list, app);
	g_object_unref (app);
	app = gs_app_new ("c");
	gs_app_list_add (list, app);
	g_object_unref (app);
	g_assert (!gs_app_list_has_flag (list, GS_APP_LIST_FLAG_IS_TRUNCATED));
	g_assert_cmpint (gs_app_list_get_size_peak (list), ==, 3);
	gs_app_list_truncate (list, 3);
	g_assert_cmpint (gs_app_list_length (list), ==, 3);
	g_assert (gs_app_list_has_flag (list, GS_APP_LIST_FLAG_IS_TRUNCATED));
	g_assert_cmpint (gs_app_list_get_size_peak (list), ==, 3);
	gs_app_list_truncate (list, 2);
	g_assert_cmpint (gs_app_list_length (list), ==, 2);
	gs_app_list_truncate (list, 1);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	gs_app_list_truncate (list, 0);
	g_assert_cmpint (gs_app_list_length (list), ==, 0);
	g_assert_cmpint (gs_app_list_get_size_peak (list), ==, 3);
	g_object_unref (list);
}

static gpointer
gs_app_thread_cb (gpointer data)
{
	GsApp *app = GS_APP (data);
	for (guint i = 0; i < 10000; i++) {
		g_assert_cmpstr (gs_app_get_unique_id (app), !=, NULL);
		gs_app_set_branch (app, "master");
		g_assert_cmpstr (gs_app_get_unique_id (app), !=, NULL);
		gs_app_set_branch (app, "stable");
	}
	return NULL;
}

static void
gs_app_thread_func (gconstpointer user_data)
{
	GsDebug *debug = GS_DEBUG ((void *)user_data);
	GThread *thread1;
	GThread *thread2;
	g_autoptr(GsApp) app = gs_app_new ("gimp.desktop");

	/* try really hard to cause a threading problem */
	gs_debug_set_verbose (debug, FALSE);
	thread1 = g_thread_new ("thread1", gs_app_thread_cb, app);
	thread2 = g_thread_new ("thread2", gs_app_thread_cb, app);
	g_thread_join (thread1); /* consumes the reference  */
	g_thread_join (thread2);
	gs_debug_set_verbose (debug, TRUE);
}

static void
gs_app_unique_id_func (void)
{
	g_autoptr(GsApp) app = gs_app_new (NULL);
	g_autofree gchar *data_id = NULL;
	const gchar *unique_id;

	unique_id = "system/flatpak/lingmo/org.lingmo.Appstore/master";
	gs_app_set_from_unique_id (app, unique_id, AS_COMPONENT_KIND_DESKTOP_APP);
	g_assert (GS_IS_APP (app));
	g_assert_cmpint (gs_app_get_scope (app), ==, AS_COMPONENT_SCOPE_SYSTEM);
	g_assert_cmpint (gs_app_get_bundle_kind (app), ==, AS_BUNDLE_KIND_FLATPAK);
	g_assert_cmpstr (gs_app_get_origin (app), ==, "lingmo");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
	g_assert_cmpstr (gs_app_get_id (app), ==, "org.lingmo.Appstore");
	g_assert_cmpstr (gs_app_get_branch (app), ==, "master");

	/* test conversions from 6-part IDs */
	data_id = gs_utils_unique_id_compat_convert (unique_id);
	g_assert_cmpstr (data_id, ==, unique_id);
	g_clear_pointer (&data_id, g_free);

	data_id = gs_utils_unique_id_compat_convert ("not a unique ID");
	g_assert_null (data_id);

	data_id = gs_utils_unique_id_compat_convert ("system/flatpak/lingmo/desktop-app/org.lingmo.Appstore/master");
	g_assert_cmpstr (data_id, ==, unique_id);
	g_clear_pointer (&data_id, g_free);
}

static void
gs_app_addons_func (void)
{
	g_autoptr(GsApp) app = gs_app_new ("test.desktop");
	g_autoptr(GsApp) addon = NULL;
	g_autoptr(GsAppList) addons_list = NULL;

	/* create, add then drop ref, so @app has the only refcount of addon */
	addon = gs_app_new ("test.desktop");
	addons_list = gs_app_list_new ();
	gs_app_list_add (addons_list, addon);

	gs_app_add_addons (app, addons_list);

	gs_app_remove_addon (app, addon);
}

static void
gs_app_func (void)
{
	g_autoptr(GsApp) app = NULL;

	app = gs_app_new ("lingmo-appstore.desktop");
	g_assert (GS_IS_APP (app));
	g_assert_cmpstr (gs_app_get_id (app), ==, "lingmo-appstore.desktop");

	/* check we clean up the version, but not at the expense of having
	 * the same string as the update version */
	gs_app_set_version (app, "2.8.6-3.fc20");
	gs_app_set_update_version (app, "2.8.6-4.fc20");
	g_assert_cmpstr (gs_app_get_version (app), ==, "2.8.6-3.fc20");
	g_assert_cmpstr (gs_app_get_update_version (app), ==, "2.8.6-4.fc20");
	g_assert_cmpstr (gs_app_get_version_ui (app), ==, "2.8.6-3");
	g_assert_cmpstr (gs_app_get_update_version_ui (app), ==, "2.8.6-4");

	/* check the quality stuff works */
	gs_app_set_name (app, GS_APP_QUALITY_NORMAL, "dave");
	g_assert_cmpstr (gs_app_get_name (app), ==, "dave");
	gs_app_set_name (app, GS_APP_QUALITY_LOWEST, "brian");
	g_assert_cmpstr (gs_app_get_name (app), ==, "dave");
	gs_app_set_name (app, GS_APP_QUALITY_HIGHEST, "hugh");
	g_assert_cmpstr (gs_app_get_name (app), ==, "hugh");

	/* check non-transient state saving */
	gs_app_set_state (app, GS_APP_STATE_INSTALLED);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);
	gs_app_set_state (app, GS_APP_STATE_REMOVING);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_REMOVING);
	gs_app_set_state_recover (app); // simulate an error
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);

	/* try again */
	gs_app_set_state (app, GS_APP_STATE_REMOVING);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_REMOVING);
	gs_app_set_state_recover (app); // simulate an error
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);

	/* correctly parse URL */
	gs_app_set_origin_hostname (app, "https://mirrors.fedoraproject.org/metalink");
	g_assert_cmpstr (gs_app_get_origin_hostname (app), ==, "fedoraproject.org");
	gs_app_set_origin_hostname (app, "file:///home/hughsie");
	g_assert_cmpstr (gs_app_get_origin_hostname (app), ==, "localhost");

	/* check setting the progress */
	gs_app_set_progress (app, 42);
	g_assert_cmpuint (gs_app_get_progress (app), ==, 42);
	gs_app_set_progress (app, 0);
	g_assert_cmpuint (gs_app_get_progress (app), ==, 0);
	gs_app_set_progress (app, GS_APP_PROGRESS_UNKNOWN);
	g_assert_cmpuint (gs_app_get_progress (app), ==, GS_APP_PROGRESS_UNKNOWN);
	g_assert_false ((gint) 0 <= (gint) GS_APP_PROGRESS_UNKNOWN && GS_APP_PROGRESS_UNKNOWN <= 100);

	/* check pending action */
	g_assert_cmpuint (gs_app_get_pending_action (app), ==, GS_PLUGIN_ACTION_UNKNOWN);
	gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);
	gs_app_set_state (app, GS_APP_STATE_INSTALLING);
	g_assert_cmpuint (gs_app_get_pending_action (app), ==, GS_PLUGIN_ACTION_UNKNOWN);
	gs_app_set_state_recover (app);

	/* check pending action */
	g_assert_cmpuint (gs_app_get_pending_action (app), ==, GS_PLUGIN_ACTION_UNKNOWN);
	gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);
	gs_app_set_state (app, GS_APP_STATE_DOWNLOADING);
	g_assert_cmpuint (gs_app_get_pending_action (app), ==, GS_PLUGIN_ACTION_UNKNOWN);
	gs_app_set_state_recover (app);
}

static void
gs_app_progress_clamping_func (void)
{
	g_autoptr(GsApp) app = NULL;

	if (g_test_subprocess ()) {
		app = gs_app_new ("lingmo-appstore.desktop");
		gs_app_set_progress (app, 142);
		g_assert_cmpuint (gs_app_get_progress (app), ==, 100);
	} else {
		g_test_trap_subprocess (NULL, 0, 0);
		g_test_trap_assert_failed ();
		g_test_trap_assert_stderr ("*cannot set 142% for *, setting instead: 100%*");
	}
}

static void
gs_app_list_wildcard_dedupe_func (void)
{
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GsApp) app1 = gs_app_new ("app");
	g_autoptr(GsApp) app2 = gs_app_new ("app");

	gs_app_add_quirk (app1, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_list_add (list, app1);
	gs_app_add_quirk (app2, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_list_add (list, app2);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
}

static void
gs_app_list_func (void)
{
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GsApp) app1 = gs_app_new ("app1");
	g_autoptr(GsApp) app2 = gs_app_new ("app2");

	/* turn on */
	gs_app_list_add_flag (list, GS_APP_LIST_FLAG_WATCH_APPS);

	g_assert_cmpint (gs_app_list_get_progress (list), ==, 0);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_UNKNOWN);
	gs_app_list_add (list, app1);
	gs_app_set_progress (app1, 75);
	gs_app_set_state (app1, GS_APP_STATE_AVAILABLE);
	gs_app_set_state (app1, GS_APP_STATE_INSTALLING);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 75);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_INSTALLING);

	gs_app_set_state (app1, GS_APP_STATE_UNKNOWN);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_UNKNOWN);
	gs_app_set_state (app1, GS_APP_STATE_AVAILABLE);
	gs_app_set_state (app1, GS_APP_STATE_DOWNLOADING);
	gs_app_set_progress (app1, 80);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 80);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_DOWNLOADING);
	gs_app_set_progress (app1, 90);
	gs_app_set_state (app1, GS_APP_STATE_INSTALLING);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 90);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_INSTALLING);

	/* return back the progress expected by the below code */
	gs_app_set_progress (app1, 75);

	gs_app_list_add (list, app2);
	gs_app_set_progress (app2, 25);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 50);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_INSTALLING);

	gs_app_list_remove (list, app1);
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 25);
	g_assert_cmpint (gs_app_list_get_state (list), ==, GS_APP_STATE_UNKNOWN);
}

static void
gs_app_list_performance_func (void)
{
	g_autoptr(GPtrArray) apps = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GTimer) timer = NULL;

	/* create a few apps */
	for (guint i = 0; i < 500; i++) {
		g_autofree gchar *id = g_strdup_printf ("%03u.desktop", i);
		g_ptr_array_add (apps, gs_app_new (id));
	}

	/* add them to the list */
	timer = g_timer_new ();
	for (guint i = 0; i < apps->len; i++) {
		GsApp *app = g_ptr_array_index (apps, i);
		gs_app_list_add (list, app);
	}
	g_print ("%.2fms ", g_timer_elapsed (timer, NULL) * 1000);
}

static void
gs_app_list_related_func (void)
{
	g_autoptr(GsAppList) list = gs_app_list_new ();
	g_autoptr(GsApp) app = gs_app_new ("app");
	g_autoptr(GsApp) related = gs_app_new ("related");

	/* turn on */
	gs_app_list_add_flag (list,
			      GS_APP_LIST_FLAG_WATCH_APPS |
			      GS_APP_LIST_FLAG_WATCH_APPS_RELATED);
	gs_app_add_related (app, related);
	gs_app_list_add (list, app);

	gs_app_set_progress (app, 75);
	gs_app_set_progress (related, 25);
	gs_test_flush_main_context ();
	g_assert_cmpint (gs_app_list_get_progress (list), ==, 50);
}

int
main (int argc, char **argv)
{
	g_autoptr(GsDebug) debug = gs_debug_new (NULL, TRUE, FALSE);

	gs_test_init (&argc, &argv);

	/* tests go here */
	g_test_add_func ("/lingmo-appstore/lib/utils{url}", gs_utils_url_func);
	g_test_add_func ("/lingmo-appstore/lib/utils{wilson}", gs_utils_wilson_func);
	g_test_add_func ("/lingmo-appstore/lib/utils{error}", gs_utils_error_func);
	g_test_add_func ("/lingmo-appstore/lib/utils{cache}", gs_utils_cache_func);
	g_test_add_func ("/lingmo-appstore/lib/utils{append-kv}", gs_utils_append_kv_func);
	g_test_add_func ("/lingmo-appstore/lib/os-release", gs_os_release_func);
	g_test_add_func ("/lingmo-appstore/lib/app", gs_app_func);
	g_test_add_func ("/lingmo-appstore/lib/app/progress-clamping", gs_app_progress_clamping_func);
	g_test_add_func ("/lingmo-appstore/lib/app{addons}", gs_app_addons_func);
	g_test_add_func ("/lingmo-appstore/lib/app{unique-id}", gs_app_unique_id_func);
	g_test_add_data_func ("/lingmo-appstore/lib/app{thread}", debug, gs_app_thread_func);
	g_test_add_func ("/lingmo-appstore/lib/app{list}", gs_app_list_func);
	g_test_add_func ("/lingmo-appstore/lib/app{list-wildcard-dedupe}", gs_app_list_wildcard_dedupe_func);
	g_test_add_func ("/lingmo-appstore/lib/app{list-performance}", gs_app_list_performance_func);
	g_test_add_func ("/lingmo-appstore/lib/app{list-related}", gs_app_list_related_func);
	g_test_add_func ("/lingmo-appstore/lib/plugin", gs_plugin_func);
	g_test_add_func ("/lingmo-appstore/lib/plugin{download-rewrite}", gs_plugin_download_rewrite_func);

	return g_test_run ();
}
