/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "lingmo-appstore-private.h"

#include "gs-test.h"

static void
gs_plugins_modalias_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	const gchar *keywords[2] = { NULL, };
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get search result based on addon keyword */
	keywords[0] = "colorhug2";
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_match_value,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there is one entry, the parent app */
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "com.hughski.ColorHug2.driver");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DRIVER);
	g_assert (gs_app_has_category (app, "Addon"));
	g_assert (gs_app_has_category (app, "Driver"));
}

int
main (int argc, char **argv)
{
	g_autofree gchar *tmp_root = NULL;
	gboolean ret;
	int retval;
	g_autofree gchar *xml = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"appstream",
		"dummy",
		"modalias",
		NULL
	};

	gs_test_init (&argc, &argv);
	g_setenv ("GS_SELF_TEST_DUMMY_ENABLE", "1", TRUE);

	xml = g_strdup_printf ("<?xml version=\"1.0\"?>\n"
		"<components version=\"0.9\">\n"
		"  <component type=\"driver\">\n"
		"    <id>com.hughski.ColorHug2.driver</id>\n"
		"    <name>ColorHug2</name>\n"
		"    <summary>ColorHug2 Colorimeter Driver</summary>\n"
		"    <pkgname>colorhug-client</pkgname>\n"
		"    <provides>\n"
		"      <modalias>pci:*</modalias>\n"
		"    </provides>\n"
		"  </component>\n"
		"  <info>\n"
		"    <scope>system</scope>\n"
		"  </info>\n"
		"</components>\n");
	g_setenv ("GS_SELF_TEST_APPSTREAM_XML", xml, TRUE);

	/* Use a common cache directory for all tests, since the appstream
	 * plugin uses it and cannot be reinitialised for each test. */
	tmp_root = g_dir_make_tmp ("lingmo-appstore-modalias-test-XXXXXX", NULL);
	g_assert (tmp_root != NULL);
	g_setenv ("GS_SELF_TEST_CACHEDIR", tmp_root, TRUE);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR_CORE);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR_DUMMY);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/modalias",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_modalias_func);

	retval = g_test_run ();

	/* Clean up. */
	gs_utils_rmtree (tmp_root, NULL);

	return retval;
}
