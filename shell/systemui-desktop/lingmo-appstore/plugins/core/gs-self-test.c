/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Joaquim Rocha <jrocha@endlessm.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gstdio.h>

#include "lingmo-appstore-private.h"

#include "gs-appstream.h"
#include "gs-test.h"

const gchar * const allowlist[] = {
	"appstream",
	"generic-updates",
	"icons",
	"os-release",
	NULL
};

static void
gs_plugins_core_search_repo_name_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app_tmp = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	const gchar *keywords[2] = { NULL, };

	/* drop all caches */
	gs_utils_rmtree (g_getenv ("GS_SELF_TEST_CACHEDIR"), NULL);
	gs_test_reinitialise_plugin_loader (plugin_loader, allowlist, NULL);

	/* force this app to be installed */
	app_tmp = gs_plugin_loader_app_create (plugin_loader, "*/*/yellow/arachne.desktop/*", NULL, &error);
	g_assert_no_error (error);
	g_assert_nonnull (app_tmp);
	gs_app_set_state (app_tmp, GS_APP_STATE_INSTALLED);

	/* get search result based on addon keyword */
	keywords[0] = "yellow";
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_match_value,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);

	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert_nonnull (list);

	/* make sure there is at least one entry, the parent app */
	g_assert_cmpint (gs_app_list_length (list), >=, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "arachne.desktop");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
}

static void
gs_plugins_core_os_release_func (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsApp) app3 = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;

	/* drop all caches */
	gs_utils_rmtree (g_getenv ("GS_SELF_TEST_CACHEDIR"), NULL);
	gs_test_reinitialise_plugin_loader (plugin_loader, allowlist, NULL);

	/* refine system application */
	app = gs_plugin_loader_get_system_app (plugin_loader, NULL, &error);
	g_assert_no_error (error);
	g_assert_nonnull (app);
	plugin_job = gs_plugin_job_refine_new_for_app (app,
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_URL |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_VERSION);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert_true (ret);

	/* make sure there is valid content */
	g_assert_cmpstr (gs_app_get_id (app), ==, "org.fedoraproject.fedora-25");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);
	g_assert_cmpstr (gs_app_get_name (app), ==, "Fedora");
	g_assert_cmpstr (gs_app_get_version (app), ==, "25");
	g_assert_cmpstr (gs_app_get_url (app, AS_URL_KIND_HOMEPAGE), ==,
			 "https://fedoraproject.org/");
	g_assert_cmpstr (gs_app_get_metadata_item (app, "LingmoAppstore::CpeName"), ==,
			 "cpe:/o:fedoraproject:fedora:25");

	/* this comes from appstream */
	g_assert_cmpstr (gs_app_get_summary (app), ==, "Fedora Workstation");

	/* check we can get this by the old name too */
	app3 = gs_plugin_loader_get_system_app (plugin_loader, NULL, &error);
	g_assert_no_error (error);
	g_assert_nonnull (app3);
	g_assert_true (app3 == app);
}

static void
gs_plugins_core_generic_updates_func (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	GsApp *os_update;
	GsAppList *related;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsPluginJob) plugin_job2 = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app1 = NULL;
	g_autoptr(GsApp) app2 = NULL;
	g_autoptr(GsApp) app_wildcard = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsAppList) list_wildcard = NULL;
	GsAppList *result_list;
	GsAppList *result_list_wildcard;

	/* drop all caches */
	gs_utils_rmtree (g_getenv ("GS_SELF_TEST_CACHEDIR"), NULL);
	gs_test_reinitialise_plugin_loader (plugin_loader, allowlist, NULL);

	/* create a list with generic apps */
	list = gs_app_list_new ();
	app1 = gs_app_new ("package1");
	app2 = gs_app_new ("package2");
	gs_app_set_kind (app1, AS_COMPONENT_KIND_GENERIC);
	gs_app_set_kind (app2, AS_COMPONENT_KIND_GENERIC);
	gs_app_set_bundle_kind (app1, AS_BUNDLE_KIND_PACKAGE);
	gs_app_set_bundle_kind (app2, AS_BUNDLE_KIND_PACKAGE);
	gs_app_set_scope (app1, AS_COMPONENT_SCOPE_SYSTEM);
	gs_app_set_scope (app2, AS_COMPONENT_SCOPE_SYSTEM);
	gs_app_set_state (app1, GS_APP_STATE_UPDATABLE);
	gs_app_set_state (app2, GS_APP_STATE_UPDATABLE);
	gs_app_add_source (app1, "package1");
	gs_app_add_source (app2, "package2");
	gs_app_list_add (list, app1);
	gs_app_list_add (list, app2);

	/* refine to make the generic-updates plugin merge them into a single OsUpdate item */
	plugin_job = gs_plugin_job_refine_new (list, GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert_true (ret);

	/* make sure there is one entry, the os update */
	result_list = gs_plugin_job_refine_get_result_list (GS_PLUGIN_JOB_REFINE (plugin_job));
	g_assert_cmpint (gs_app_list_length (result_list), ==, 1);
	os_update = gs_app_list_index (result_list, 0);

	/* make sure the os update is valid */
	g_assert_cmpstr (gs_app_get_id (os_update), ==, "org.lingmo.Appstore.OsUpdate");
	g_assert_cmpint (gs_app_get_kind (os_update), ==, AS_COMPONENT_KIND_GENERIC);
	g_assert_cmpint (gs_app_get_special_kind (os_update), ==, GS_APP_SPECIAL_KIND_OS_UPDATE);
	g_assert_true (gs_app_has_quirk (os_update, GS_APP_QUIRK_IS_PROXY));

	/* must have two related apps, the ones we added earlier */
	related = gs_app_get_related (os_update);
	g_assert_cmpint (gs_app_list_length (related), ==, 2);

	/* another test to make sure that we don't get an OsUpdate item created for wildcard apps */
	list_wildcard = gs_app_list_new ();
	app_wildcard = gs_app_new ("nosuchapp.desktop");
	gs_app_add_quirk (app_wildcard, GS_APP_QUIRK_IS_WILDCARD);
	gs_app_set_kind (app_wildcard, AS_COMPONENT_KIND_GENERIC);
	gs_app_list_add (list_wildcard, app_wildcard);
	plugin_job2 = gs_plugin_job_refine_new (list_wildcard, GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job2, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert_true (ret);
	result_list_wildcard = gs_plugin_job_refine_get_result_list (GS_PLUGIN_JOB_REFINE (plugin_job2));

	/* no OsUpdate item created */
	for (guint i = 0; i < gs_app_list_length (result_list_wildcard); i++) {
		GsApp *app_tmp = gs_app_list_index (result_list_wildcard, i);
		g_assert_cmpint (gs_app_get_kind (app_tmp), !=, AS_COMPONENT_KIND_GENERIC);
		g_assert_cmpint (gs_app_get_special_kind (app_tmp), !=, GS_APP_SPECIAL_KIND_OS_UPDATE);
		g_assert_false (gs_app_has_quirk (app_tmp, GS_APP_QUIRK_IS_PROXY));
	}
}

int
main (int argc, char **argv)
{
	g_autofree gchar *tmp_root = NULL;
	gboolean ret;
	int retval;
	g_autofree gchar *os_release_filename = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar *xml;

	/* While we use %G_TEST_OPTION_ISOLATE_DIRS to create temporary directories
	 * for each of the tests, we want to use the system MIME registry, assuming
	 * that it exists and correctly has shared-mime-info installed. */
	g_content_type_set_mime_dirs (NULL);

	/* Similarly, add the system-wide icon theme path before it’s
	 * overwritten by %G_TEST_OPTION_ISOLATE_DIRS. */
	gs_test_expose_icon_theme_paths ();

	gs_test_init (&argc, &argv);

	/* Use a common cache directory for all tests, since the appstream
	 * plugin uses it and cannot be reinitialised for each test. */
	tmp_root = g_dir_make_tmp ("lingmo-appstore-core-test-XXXXXX", NULL);
	g_assert_nonnull (tmp_root);
	g_setenv ("GS_SELF_TEST_CACHEDIR", tmp_root, TRUE);

	os_release_filename = gs_test_get_filename (TESTDATADIR, "os-release");
	g_assert_nonnull (os_release_filename);
	g_setenv ("GS_SELF_TEST_OS_RELEASE_FILENAME", os_release_filename, TRUE);

	/* fake some data */
	xml = "<?xml version=\"1.0\"?>\n"
		"<components origin=\"yellow\" version=\"0.9\">\n"
		"  <component type=\"desktop\">\n"
		"    <id>arachne.desktop</id>\n"
		"    <name>test</name>\n"
		"    <summary>Test</summary>\n"
		"    <icon type=\"stock\">system-file-manager</icon>\n"
		"    <pkgname>arachne</pkgname>\n"
		"  </component>\n"
		"  <component type=\"os-upgrade\">\n"
		"    <id>org.fedoraproject.fedora-25</id>\n"
		"    <name>Fedora</name>\n"
		"    <summary>Fedora Workstation</summary>\n"
		"    <pkgname>fedora-release</pkgname>\n"
		"  </component>\n"
		"  <info>\n"
		"    <scope>user</scope>\n"
		"  </info>\n"
		"</components>\n";
	g_setenv ("GS_SELF_TEST_APPSTREAM_XML", xml, TRUE);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert_true (ret);

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/core/search-repo-name",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_core_search_repo_name_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/core/os-release",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_core_os_release_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/core/generic-updates",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_core_generic_updates_func);
	retval = g_test_run ();

	/* Clean up. */
	gs_utils_rmtree (tmp_root, NULL);

	return retval;
}
