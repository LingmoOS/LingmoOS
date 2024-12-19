/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2021 Matthew Leeds <mwleeds@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "lingmo-appstore-private.h"

#include "gs-test.h"
#include "gs-dynamic-launcher-portal-iface.h"
#include "gs-epiphany-generated.h"

#include <libglib-testing/dbus-queue.h>

/* This is run in a worker thread */
static void
epiphany_and_portal_mock_server_cb (GtDBusQueue *queue,
				    gpointer     user_data)
{
	{
		g_autoptr(GDBusMethodInvocation) invocation = NULL;
		g_autoptr(GVariant) properties_variant = NULL;
		const char *property_interface;
		invocation = gt_dbus_queue_assert_pop_message (queue,
							       "/org/lingmo/Epiphany/WebAppProvider",
							       "org.freedesktop.DBus.Properties",
							       "GetAll", "(&s)",
							       &property_interface);
		g_assert_cmpstr (property_interface, ==, "org.lingmo.Epiphany.WebAppProvider");
		properties_variant = g_variant_new_parsed ("({'Version': <@u 1>},)");
		g_dbus_method_invocation_return_value (invocation, g_steal_pointer (&properties_variant));
	}
	{
		g_autoptr(GDBusMethodInvocation) invocation = NULL;
		g_autoptr(GVariant) properties_variant = NULL;
		const char *property_interface, *props_dict;
		invocation = gt_dbus_queue_assert_pop_message (queue,
							       "/org/freedesktop/portal/desktop",
							       "org.freedesktop.DBus.Properties",
							       "GetAll", "(&s)",
							       &property_interface);
		g_assert_cmpstr (property_interface, ==, "org.freedesktop.portal.DynamicLauncher");
		props_dict = "({'version': <@u 1>,'SupportedLauncherTypes': <@u 3>},)";
		properties_variant = g_variant_new_parsed (props_dict);
		g_dbus_method_invocation_return_value (invocation, g_steal_pointer (&properties_variant));
	}
	{
		g_autoptr(GDBusMethodInvocation) invocation = NULL;
		const char *installed_apps[] = {"org.lingmo.Epiphany.WebApp_e9d0e1e4b0a10856aa3b38d9eb4375de4070d043.desktop", NULL};
		invocation = gt_dbus_queue_assert_pop_message (queue,
							       "/org/lingmo/Epiphany/WebAppProvider",
							       "org.lingmo.Epiphany.WebAppProvider",
							       "GetInstalledApps", "()", NULL);
		g_dbus_method_invocation_return_value (invocation, g_variant_new ("(^as)", installed_apps));
	}
}

static GtDBusQueue *
bus_set_up (void)
{
	g_autoptr(GError) local_error = NULL;
	g_autoptr(GtDBusQueue) queue = NULL;

	queue = gt_dbus_queue_new ();

	gt_dbus_queue_connect (queue, &local_error);
	g_assert_no_error (local_error);

	gt_dbus_queue_own_name (queue, "org.freedesktop.portal.Desktop");

	gt_dbus_queue_export_object (queue,
				     "/org/freedesktop/portal/desktop",
				     (GDBusInterfaceInfo *) &org_freedesktop_portal_dynamic_launcher_interface,
				     &local_error);
	g_assert_no_error (local_error);

	gt_dbus_queue_own_name (queue, "org.lingmo.Epiphany.WebAppProvider");

	gt_dbus_queue_export_object (queue,
				     "/org/lingmo/Epiphany/WebAppProvider",
				     gs_ephy_web_app_provider_interface_info (),
				     &local_error);
	g_assert_no_error (local_error);

	gt_dbus_queue_set_server_func (queue, epiphany_and_portal_mock_server_cb,
				       NULL);

	return g_steal_pointer (&queue);
}

static void
gs_plugins_epiphany_func (GsPluginLoader *plugin_loader)
{
	g_assert_true (gs_plugin_loader_get_enabled (plugin_loader, "epiphany"));
}

static char *
create_fake_desktop_file (const char *app_id)
{
	g_autofree char *contents = NULL;
	g_autoptr(GError) error = NULL;
	g_autofree char *desktop_path = NULL;
	g_autofree char *icon_path = NULL;

	/* Use an icon we already have locally */
	icon_path = gs_test_get_filename (TESTDATADIR, "icons/hicolor/scalable/org.lingmo.Appstore.svg");
	g_assert (icon_path != NULL);

	/* Use true instead of epiphany in Exec and TryExec; otherwise
	 * g_desktop_app_info_new() in the plugin code will look for an
	 * epiphany binary and fail.
	 */
	contents = g_strdup_printf ("[Desktop Entry]\n"
		"Name=Pinafore\n"
		"Exec=true --application-mode \"--profile=/home/nobody/.local/share/%s\" https://pinafore.social/\n"
		"StartupNotify=true\n"
		"Terminal=false\n"
		"Type=Application\n"
		"Categories=LINGMO;GTK;\n"
		"Icon=%s\n"
		"StartupWMClass=%s\n"
		"X-Purism-FormFactor=Workstation;Mobile;\n"
		"TryExec=true\n",
		app_id, icon_path, app_id);

	desktop_path = g_strconcat (g_get_user_data_dir (), G_DIR_SEPARATOR_S,
				    "applications", G_DIR_SEPARATOR_S,
				    app_id, ".desktop", NULL);

	g_debug ("Creating a fake desktop file at path: %s", desktop_path);
	gs_mkdir_parent (desktop_path, &error);
	g_assert_no_error (error);
	g_file_set_contents (desktop_path, contents, -1, &error);
	g_assert_no_error (error);

	return g_steal_pointer (&desktop_path);
}

static void
gs_plugins_epiphany_installed_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GIcon) icon = NULL;
	g_autoptr(GsAppList) list = NULL;
	GsApp *app;
	const char *app_id = "org.lingmo.Epiphany.WebApp_e9d0e1e4b0a10856aa3b38d9eb4375de4070d043";
	const char *metainfo_app_id = "org.lingmo.Appstore.WebApp_e636aa5f2069f6e9c02deccc7b65f43da7985e32.desktop";
	const char *launchable_app_id;
	g_autofree char *app_id_desktop = NULL;
	g_autofree char *desktop_path = NULL;
	g_autofree char *origin_ui = NULL;

	app_id_desktop = g_strdup_printf ("%s.desktop", app_id);
	desktop_path = create_fake_desktop_file (app_id);

	query = gs_app_query_new ("is-installed", GS_APP_QUERY_TRISTATE_TRUE,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert_nonnull (list);

	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, metainfo_app_id);
	launchable_app_id = gs_app_get_launchable (app, AS_LAUNCHABLE_KIND_DESKTOP_ID);
	g_assert_cmpstr (launchable_app_id, ==, app_id_desktop);
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_WEB_APP);
	g_assert_cmpint (gs_app_get_scope (app), ==, AS_COMPONENT_SCOPE_USER);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);
	g_assert_cmpstr (gs_app_get_name (app), ==, "Pinafore");
	g_assert_cmpstr (gs_app_get_summary (app), ==, "pinafore.social");
	g_assert_cmpstr (gs_app_get_origin (app), ==, "lingmo-web");
	origin_ui = gs_app_dup_origin_ui (app, TRUE);
	g_assert_cmpstr (origin_ui, ==, "Pinafore (Web App)");
	icon = gs_app_get_icon_for_size (app, 4096, 1, NULL);
	g_assert_nonnull (icon);
	g_clear_object (&icon);

	gs_utils_unlink (desktop_path, NULL);
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	g_autoptr(GtDBusQueue) queue = NULL;
	int res;
	const gchar *allowlist[] = {
		"epiphany",
		"icons",
		NULL
	};

	gs_test_init (&argc, &argv);
	g_setenv ("GS_XMLB_VERBOSE", "1", TRUE);

	/* Set up mock D-Bus services for the Epiphany WebAppProvider and the
	 * DynamicLauncher portal
	 */
	queue = bus_set_up ();

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (gt_dbus_queue_get_client_connection (queue), NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR_CORE);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert_true (ret);

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/epiphany/enabled",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_epiphany_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/epiphany/installed",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_epiphany_installed_func);

	res = g_test_run ();
	gt_dbus_queue_disconnect (queue, TRUE);
	return res;
}
