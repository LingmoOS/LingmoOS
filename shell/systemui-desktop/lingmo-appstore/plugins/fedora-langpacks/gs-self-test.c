/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2019 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gstdio.h>

#include "lingmo-appstore-private.h"

#include "gs-test.h"

static void
gs_plugins_fedora_langpacks_func (GsPluginLoader *plugin_loader)
{
	g_autofree gchar *cachefn = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsOsRelease) os_release = NULL;

	os_release = gs_os_release_new (NULL);
	if (g_strcmp0 (gs_os_release_get_id (os_release), "fedora") != 0) {
		g_test_skip ("not on fedora");
		return;
	}

	/* start with a clean slate */
	cachefn = gs_utils_get_cache_filename ("langpacks", "langpacks-pt_BR",
					       GS_UTILS_CACHE_FLAG_WRITEABLE |
					       GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					       &error);
	g_assert_no_error (error);
	g_unlink (cachefn);

	/* get langpacks result based on locale */
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_GET_LANGPACKS,
					 "search", "pt_BR.UTF-8",
					 "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
					 NULL);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	g_assert_nonnull (list);
	g_assert_no_error (error);

	/* check if we have just one app in the list */
	g_assert_cmpint (gs_app_list_length (list), ==, 1);

	/* check app's source and kind */
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_source_default (app), ==, "langpacks-pt_BR");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_LOCALIZATION);
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"fedora-langpacks",
		NULL
	};

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
	g_test_add_data_func ("/lingmo-appstore/plugins/fedora-langpacks",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_fedora_langpacks_func);
	return g_test_run ();
}
