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
gs_plugins_repos_func (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get the extra bits */
	app = gs_app_new ("testrepos.desktop");
	gs_app_set_origin (app, "utopia");
	gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
	plugin_job = gs_plugin_job_refine_new_for_app (app, GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN_HOSTNAME);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpstr (gs_app_get_origin_hostname (app), ==, "people.freedesktop.org");
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autofree gchar *reposdir = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"repos",
		NULL
	};

	gs_test_init (&argc, &argv);

	/* dummy data */
	reposdir = gs_test_get_filename (TESTDATADIR, "yum.repos.d");
	g_assert (reposdir != NULL);
	g_setenv ("GS_SELF_TEST_REPOS_DIR", reposdir, TRUE);

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
	g_test_add_data_func ("/lingmo-appstore/plugins/repos",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_repos_func);

	return g_test_run ();
}
