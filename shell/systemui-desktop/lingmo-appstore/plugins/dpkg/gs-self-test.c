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
gs_plugins_dpkg_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;
	g_autofree gchar *fn = NULL;
	g_autoptr(GFile) file = NULL;

	/* no dpkg, abort */
	if (!gs_plugin_loader_get_enabled (plugin_loader, "dpkg")) {
		g_test_skip ("not enabled");
		return;
	}

	/* load local file */
	fn = gs_test_get_filename (TESTDATADIR, "chiron-1.1-1.deb");
	g_assert (fn != NULL);
	file = g_file_new_for_path (fn);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_FILE_TO_APP,
					 "file", file,
					 NULL);
	app = gs_plugin_loader_job_process_app (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (app != NULL);
	g_assert_cmpstr (gs_app_get_source_default (app), ==, "chiron");
	g_assert_cmpstr (gs_app_get_url (app, AS_URL_KIND_HOMEPAGE), ==, "http://127.0.0.1/");
	g_assert_cmpstr (gs_app_get_name (app), ==, "chiron");
	g_assert_cmpstr (gs_app_get_version (app), ==, "1.1-1");
	g_assert_cmpstr (gs_app_get_summary (app), ==, "Single line synopsis");
	g_assert_cmpstr (gs_app_get_description (app), ==,
			 "This is the first paragraph in the example "
			 "package control file.\nThis is the second paragraph.");
	g_assert (gs_app_get_local_file (app) != NULL);
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"dpkg",
		NULL
	};

	/* While we use %G_TEST_OPTION_ISOLATE_DIRS to create temporary directories
	 * for each of the tests, we want to use the system MIME registry, assuming
	 * that it exists and correctly has shared-mime-info installed. */
	g_content_type_set_mime_dirs (NULL);

	gs_test_init (&argc, &argv);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/dpkg",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dpkg_func);

	return g_test_run ();
}
