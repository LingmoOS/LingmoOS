/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gstdio.h>
#include <locale.h>

#include "lingmo-appstore-private.h"

#include "gs-test.h"

const gchar * const allowlist[] = {
	"appstream",
	"dummy",
	"generic-updates",
	"hardcoded-blocklist",
	"icons",
	"provenance",
	"provenance-license",
	NULL
};

static guint _status_changed_cnt = 0;

static void
gs_plugin_loader_status_changed_cb (GsPluginLoader *plugin_loader,
				    GsApp *app,
				    GsPluginStatus status,
				    gpointer user_data)
{
	_status_changed_cnt++;
}

static void
gs_plugins_dummy_install_func (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;
	GsPlugin *plugin;

	/* install */
	app = gs_app_new ("chiron.desktop");
	plugin = gs_plugin_loader_find_plugin (plugin_loader, "dummy");
	gs_app_set_management_plugin (app, plugin);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_INSTALL,
					 "app", app,
					 NULL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);

	/* remove */
	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_REMOVE,
					 "app", app,
					 NULL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_AVAILABLE);
}

static void
gs_plugins_dummy_error_func (GsPluginLoader *plugin_loader)
{
	const GError *app_error;
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GPtrArray) events = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginEvent) event = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	GsPlugin *plugin;

	/* drop all caches */
	gs_utils_rmtree (g_getenv ("GS_SELF_TEST_CACHEDIR"), NULL);
	gs_test_reinitialise_plugin_loader (plugin_loader, allowlist, NULL);

	/* update, which should cause an error to be emitted */
	app = gs_app_new ("chiron.desktop");
	plugin = gs_plugin_loader_find_plugin (plugin_loader, "dummy");
	gs_app_set_management_plugin (app, plugin);
	gs_app_set_state (app, GS_APP_STATE_AVAILABLE);

	list = gs_app_list_new ();
	gs_app_list_add (list, app);

	plugin_job = gs_plugin_job_update_apps_new (list, GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);

	/* get last active event */
	event = gs_plugin_loader_get_event_default (plugin_loader);
	g_assert (event != NULL);
	g_assert (gs_plugin_event_get_app (event) == app);

	/* check all the events */
	events = gs_plugin_loader_get_events (plugin_loader);
	g_assert_cmpint (events->len, ==, 1);
	event = g_ptr_array_index (events, 0);
	g_assert (gs_plugin_event_get_app (event) == app);
	app_error = gs_plugin_event_get_error (event);
	g_assert (app_error != NULL);
	g_assert_error (app_error,
			GS_PLUGIN_ERROR,
			GS_PLUGIN_ERROR_DOWNLOAD_FAILED);
}

static void
gs_plugins_dummy_refine_func (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	GsPlugin *plugin;

	/* get the extra bits */
	app = gs_app_new ("chiron.desktop");
	plugin = gs_plugin_loader_find_plugin (plugin_loader, "dummy");
	gs_app_set_management_plugin (app, plugin);
	plugin_job = gs_plugin_job_refine_new_for_app (app,
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
						       GS_PLUGIN_REFINE_FLAGS_REQUIRE_URL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);

	g_assert_cmpstr (gs_app_get_license (app), ==, "GPL-2.0-or-later");
	g_assert_cmpstr (gs_app_get_description (app), !=, NULL);
	g_assert_cmpstr (gs_app_get_url (app, AS_URL_KIND_HOMEPAGE), ==, "http://www.test.org/");
}

static void
gs_plugins_dummy_metadata_quirks (GsPluginLoader *plugin_loader)
{
	gboolean ret;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	GsPlugin *plugin;

	/* get the extra bits */
	app = gs_app_new ("chiron.desktop");
	plugin = gs_plugin_loader_find_plugin (plugin_loader, "dummy");
	gs_app_set_management_plugin (app, plugin);
	plugin_job = gs_plugin_job_refine_new_for_app (app, GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);

	g_assert_cmpstr (gs_app_get_description (app), !=, NULL);

	/* check the not-launchable quirk */

	g_assert (!gs_app_has_quirk(app, GS_APP_QUIRK_NOT_LAUNCHABLE));

	gs_app_set_metadata (app, "LingmoAppstore::quirks::not-launchable", "true");

	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_refine_new_for_app (app, GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);

	g_assert (gs_app_has_quirk(app, GS_APP_QUIRK_NOT_LAUNCHABLE));

	gs_app_set_metadata (app, "LingmoAppstore::quirks::not-launchable", NULL);
	gs_app_set_metadata (app, "LingmoAppstore::quirks::not-launchable", "false");

	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_refine_new_for_app (app, GS_PLUGIN_REFINE_FLAGS_REQUIRE_DESCRIPTION);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);

	g_assert (!gs_app_has_quirk(app, GS_APP_QUIRK_NOT_LAUNCHABLE));
}

static void
gs_plugins_dummy_key_colors_func (GsPluginLoader *plugin_loader)
{
	GArray *array;
	gboolean ret;
	guint i;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GError) error = NULL;

	/* get the extra bits */
	app = gs_app_new ("chiron.desktop");
	plugin_job = gs_plugin_job_refine_new_for_app (app, GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	array = gs_app_get_key_colors (app);
	g_assert_cmpint (array->len, <=, 3);
	g_assert_cmpint (array->len, >, 0);

	/* check values are in range */
	for (i = 0; i < array->len; i++) {
		const GdkRGBA *kc = &g_array_index (array, GdkRGBA, i);
		g_assert_cmpfloat (kc->red, >=, 0.f);
		g_assert_cmpfloat (kc->red, <=, 1.f);
		g_assert_cmpfloat (kc->green, >=, 0.f);
		g_assert_cmpfloat (kc->green, <=, 1.f);
		g_assert_cmpfloat (kc->blue, >=, 0.f);
		g_assert_cmpfloat (kc->blue, <=, 1.f);
		g_assert_cmpfloat (kc->alpha, >=, 0.f);
		g_assert_cmpfloat (kc->alpha, <=, 1.f);
	}
}

static void
gs_plugins_dummy_updates_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get the updates list */
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_GET_UPDATES,
					 "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
							 GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS,
					 NULL);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there are three entries */
	g_assert_cmpint (gs_app_list_length (list), ==, 3);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "chiron.desktop");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_UPDATABLE_LIVE);
	g_assert_cmpstr (gs_app_get_update_details_markup (app), ==, "Do not crash when using libvirt.");
	g_assert_cmpint (gs_app_get_update_urgency (app), ==, AS_URGENCY_KIND_HIGH);

	/* get the virtual non-apps OS update */
	app = gs_app_list_index (list, 2);
	g_assert_cmpstr (gs_app_get_id (app), ==, "org.lingmo.Appstore.OsUpdate");
	g_assert_cmpstr (gs_app_get_name (app), ==, "System Updates");
	g_assert_cmpstr (gs_app_get_summary (app), ==, "General system updates, such as security or bug fixes, and performance improvements.");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_GENERIC);
	g_assert_cmpint (gs_app_get_special_kind (app), ==, GS_APP_SPECIAL_KIND_OS_UPDATE);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_UPDATABLE);
	g_assert_cmpint (gs_app_list_length (gs_app_get_related (app)), ==, 2);

	/* get the virtual non-apps OS update */
	app = gs_app_list_index (list, 1);
	g_assert_cmpstr (gs_app_get_id (app), ==, "proxy.desktop");
	g_assert (gs_app_has_quirk (app, GS_APP_QUIRK_IS_PROXY));
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_UPDATABLE_LIVE);
	g_assert_cmpint (gs_app_list_length (gs_app_get_related (app)), ==, 2);
}

static void
gs_plugins_dummy_distro_upgrades_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get the updates list */
	plugin_job = gs_plugin_job_list_distro_upgrades_new (GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE,
							     GS_PLUGIN_REFINE_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there is one entry */
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "org.fedoraproject.release-rawhide.upgrade");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_AVAILABLE);

	/* this should be set with a higher priority by AppStream */
	g_assert_cmpstr (gs_app_get_summary (app), ==, "Release specific tagline");

	/* download the update */
	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
					 "app", app,
					 NULL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_UPDATABLE);

	/* trigger the update */
	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPGRADE_TRIGGER,
					 "app", app,
					 NULL);
	ret = gs_plugin_loader_job_action (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (ret);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_UPDATABLE);
}

static gboolean
filter_valid_cb (GsApp    *app,
                 gpointer  user_data)
{
	return gs_plugin_loader_app_is_valid (app, GS_PLUGIN_REFINE_FLAGS_NONE);
}

static void
gs_plugins_dummy_installed_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	GsApp *addon;
	g_autoptr(GsAppList) addons = NULL;
	g_autofree gchar *menu_path = NULL;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GIcon) icon = NULL;
	GsPluginRefineFlags refine_flags;

	/* get installed packages */
	refine_flags = (GS_PLUGIN_REFINE_FLAGS_REQUIRE_ORIGIN |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_ADDONS |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_KUDOS |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_CATEGORIES |
			GS_PLUGIN_REFINE_FLAGS_REQUIRE_PROVENANCE);

	query = gs_app_query_new ("is-installed", GS_APP_QUERY_TRISTATE_TRUE,
				  "refine-flags", refine_flags,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "filter-func", filter_valid_cb,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there is one entry */
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "zeus.desktop");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
	g_assert_cmpint (gs_app_get_state (app), ==, GS_APP_STATE_INSTALLED);
	g_assert_cmpstr (gs_app_get_name (app), ==, "Zeus");
	g_assert_cmpstr (gs_app_get_source_default (app), ==, "zeus");
	icon = gs_app_get_icon_for_size (app, 48, 1, NULL);
	g_assert_nonnull (icon);
	g_assert_true (G_IS_THEMED_ICON (icon));
	g_clear_object (&icon);

	/* check various bitfields */
	g_assert (gs_app_has_quirk (app, GS_APP_QUIRK_PROVENANCE));
	g_assert_cmpstr (gs_app_get_license (app), ==, "GPL-2.0-or-later");
	g_assert (gs_app_get_license_is_free (app));

	/* check kudos */
	g_assert_true (gs_app_has_kudo (app, GS_APP_KUDO_MY_LANGUAGE));

	/* check categories */
	g_assert (gs_app_has_category (app, "Player"));
	g_assert (gs_app_has_category (app, "AudioVideo"));
	g_assert (!gs_app_has_category (app, "ImageProcessing"));
	g_assert (gs_app_get_menu_path (app) != NULL);
	menu_path = g_strjoinv ("->", gs_app_get_menu_path (app));
	g_assert_cmpstr (menu_path, ==, "Create->Music Players");

	/* check addon */
	addons = gs_app_dup_addons (app);
	g_assert_nonnull (addons);
	g_assert_cmpint (gs_app_list_length (addons), ==, 1);
	addon = gs_app_list_index (addons, 0);
	g_assert_cmpstr (gs_app_get_id (addon), ==, "zeus-spell.addon");
	g_assert_cmpint (gs_app_get_kind (addon), ==, AS_COMPONENT_KIND_ADDON);
	g_assert_cmpint (gs_app_get_state (addon), ==, GS_APP_STATE_AVAILABLE);
	g_assert_cmpstr (gs_app_get_name (addon), ==, "Spell Check");
	g_assert_cmpstr (gs_app_get_source_default (addon), ==, "zeus-spell");
	g_assert_cmpstr (gs_app_get_license (addon), ==,
			 "LicenseRef-free=https://www.debian.org/");
	/* The app has a non-existent icon */
	g_assert_true (gs_app_has_icons (addon));
	icon = gs_app_get_icon_for_size (addon, 48, 1, NULL);
	g_assert_null (icon);
}

static void
gs_plugins_dummy_search_func (GsPluginLoader *plugin_loader)
{
	GsApp *app;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	const gchar *keywords[2] = { NULL, };

	/* get search result based on addon keyword */
	keywords[0] = "zeus";
	query = gs_app_query_new ("keywords", keywords,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_match_value,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there is at least one entry, the parent app, which must be first */
	g_assert_cmpint (gs_app_list_length (list), >=, 1);
	app = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app), ==, "zeus.desktop");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
}

static void
gs_plugins_dummy_search_alternate_func (GsPluginLoader *plugin_loader)
{
	GsApp *app_tmp;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppQuery) query = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* get search result based on addon keyword */
	app = gs_app_new ("zeus.desktop");
	query = gs_app_query_new ("alternate-of", app,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
				  "dedupe-flags", GS_PLUGIN_JOB_DEDUPE_FLAGS_DEFAULT,
				  "sort-func", gs_utils_app_sort_priority,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list != NULL);

	/* make sure there is the original app, and the alternate */
	g_assert_cmpint (gs_app_list_length (list), ==, 2);
	app_tmp = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app_tmp), ==, "chiron.desktop");
	g_assert_cmpint (gs_app_get_kind (app_tmp), ==, AS_COMPONENT_KIND_DESKTOP_APP);
	app_tmp = gs_app_list_index (list, 1);
	g_assert_cmpstr (gs_app_get_id (app_tmp), ==, "zeus.desktop");
	g_assert_cmpint (gs_app_get_kind (app_tmp), ==, AS_COMPONENT_KIND_DESKTOP_APP);
}

static void
gs_plugins_dummy_url_to_app_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_URL_TO_APP,
					 "search", "dummy://chiron.desktop",
					 "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
					 NULL);
	app = gs_plugin_loader_job_process_app (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (app != NULL);
	g_assert_cmpstr (gs_app_get_id (app), ==, "chiron.desktop");
	g_assert_cmpint (gs_app_get_kind (app), ==, AS_COMPONENT_KIND_DESKTOP_APP);
}

static void
gs_plugins_dummy_plugin_cache_func (GsPluginLoader *plugin_loader)
{
	GsApp *app1;
	GsApp *app2;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list1 = NULL;
	g_autoptr(GsAppList) list2 = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* ensure we get the same results back from calling the methods twice */
	plugin_job = gs_plugin_job_list_distro_upgrades_new (GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE,
							     GS_PLUGIN_REFINE_FLAGS_NONE);
	list1 = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list1 != NULL);
	g_assert_cmpint (gs_app_list_length (list1), ==, 1);
	app1 = gs_app_list_index (list1, 0);

	g_object_unref (plugin_job);
	plugin_job = gs_plugin_job_list_distro_upgrades_new (GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE,
							     GS_PLUGIN_REFINE_FLAGS_NONE);
	list2 = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list2 != NULL);
	g_assert_cmpint (gs_app_list_length (list2), ==, 1);
	app2 = gs_app_list_index (list2, 0);

	/* make sure there is one GObject */
	g_assert_cmpstr (gs_app_get_id (app1), ==, gs_app_get_id (app2));
	g_assert (app1 == app2);
}

static void
gs_plugins_dummy_wildcard_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GError) error = NULL;
	g_autoptr(GsAppList) list1 = NULL;
	g_autoptr(GsAppList) list2 = NULL;
	const gchar *expected_apps2[] = { "chiron.desktop", "zeus.desktop", NULL };
	g_autoptr(GsPluginJob) plugin_job = NULL;
	g_autoptr(GsAppQuery) query = NULL;

	/* use the plugin's default curated list, indicated by setting max-results=5 */
	query = gs_app_query_new ("is-curated", GS_APP_QUERY_TRISTATE_TRUE,
				  "max-results", 5,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);

	list1 = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list1 != NULL);
	g_assert_cmpint (gs_app_list_length (list1), ==, 1);
	g_object_unref (plugin_job);
	g_object_unref (query);

	/* use the plugin’s second list, indicated by setting max-results=6 */
	query = gs_app_query_new ("is-curated", GS_APP_QUERY_TRISTATE_TRUE,
				  "max-results", 6,
				  "refine-flags", GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON,
				  NULL);
	plugin_job = gs_plugin_job_list_apps_new (query, GS_PLUGIN_LIST_APPS_FLAGS_NONE);

	list2 = gs_plugin_loader_job_process (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	g_assert_no_error (error);
	g_assert (list2 != NULL);

	g_assert_cmpint (gs_app_list_length (list2), ==, g_strv_length ((gchar **) expected_apps2));

	for (guint i = 0; i < gs_app_list_length (list2); ++i) {
		GsApp *app = gs_app_list_index (list2, i);
		g_assert (g_strv_contains (expected_apps2, gs_app_get_id (app)));
	}
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

static void
gs_plugins_dummy_limit_parallel_ops_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsAppList) list = NULL;
        GsApp *app1 = NULL;
	g_autoptr(GsApp) app2 = NULL;
	g_autoptr(GsApp) app3 = NULL;
	GsPlugin *plugin;
	g_autoptr(GsPluginJob) plugin_job1 = NULL;
	g_autoptr(GsPluginJob) plugin_job2 = NULL;
	g_autoptr(GsPluginJob) plugin_job3 = NULL;
	g_autoptr(GMainContext) context = NULL;
	g_autoptr(GAsyncResult) result1 = NULL;
	g_autoptr(GAsyncResult) result2 = NULL;
	g_autoptr(GAsyncResult) result3 = NULL;
	g_autoptr(GError) local_error = NULL;

	/* drop all caches */
	gs_utils_rmtree (g_getenv ("GS_SELF_TEST_CACHEDIR"), NULL);
	gs_test_reinitialise_plugin_loader (plugin_loader, allowlist, NULL);

	/* get the updates list */
	plugin_job1 = gs_plugin_job_list_distro_upgrades_new (GS_PLUGIN_LIST_DISTRO_UPGRADES_FLAGS_NONE,
							      GS_PLUGIN_REFINE_FLAGS_NONE);
	list = gs_plugin_loader_job_process (plugin_loader, plugin_job1, NULL, &local_error);
	gs_test_flush_main_context ();
	g_assert_no_error (local_error);
	g_assert (list != NULL);
	g_assert_cmpint (gs_app_list_length (list), ==, 1);
	app1 = gs_app_list_index (list, 0);
	g_assert_cmpstr (gs_app_get_id (app1), ==, "org.fedoraproject.release-rawhide.upgrade");
	g_assert_cmpint (gs_app_get_kind (app1), ==, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	g_assert_cmpint (gs_app_get_state (app1), ==, GS_APP_STATE_AVAILABLE);

	/* allow only one operation at a time */
	gs_plugin_loader_set_max_parallel_ops (plugin_loader, 1);

	app2 = gs_app_new ("chiron.desktop");
	plugin = gs_plugin_loader_find_plugin (plugin_loader, "dummy");
	gs_app_set_management_plugin (app2, plugin);
	gs_app_set_state (app2, GS_APP_STATE_AVAILABLE);

	/* use "proxy" prefix so the update function succeeds... */
	app3 = gs_app_new ("proxy-zeus.desktop");
	gs_app_set_management_plugin (app3, plugin);
	gs_app_set_state (app3, GS_APP_STATE_UPDATABLE_LIVE);

	context = g_main_context_new ();
	g_main_context_push_thread_default (context);

	/* call a few operations at the "same time" */

	/* download an upgrade */
	g_object_unref (plugin_job1);
	plugin_job1 = gs_plugin_job_newv (GS_PLUGIN_ACTION_UPGRADE_DOWNLOAD,
					  "app", app1,
					  NULL);
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job1,
					    NULL,
					    async_result_cb,
					    &result1);

	/* install an app */
	plugin_job2 = gs_plugin_job_newv (GS_PLUGIN_ACTION_INSTALL,
					  "app", app2,
					  NULL);
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job2,
					    NULL,
					    async_result_cb,
					    &result2);

	/* update an app */
	list = gs_app_list_new ();
	gs_app_list_add (list, app3);
	plugin_job3 = gs_plugin_job_update_apps_new (list, GS_PLUGIN_UPDATE_APPS_FLAGS_NO_DOWNLOAD);
	gs_plugin_loader_job_process_async (plugin_loader,
					    plugin_job3,
					    NULL,
					    async_result_cb,
					    &result3);

	/* wait for all operations to finish */
	while (result1 == NULL || result2 == NULL || result3 == NULL)
		g_main_context_iteration (context, TRUE);

	g_main_context_pop_thread_default (context);

	gs_test_flush_main_context ();

	gs_plugin_loader_job_action_finish (plugin_loader, result1, &local_error);
	g_assert_no_error (local_error);

	gs_plugin_loader_job_action_finish (plugin_loader, result2, &local_error);
	g_assert_no_error (local_error);

	gs_plugin_loader_job_action_finish (plugin_loader, result3, &local_error);
	g_assert_no_error (local_error);

	g_assert_cmpint (gs_app_get_state (app1), ==, GS_APP_STATE_UPDATABLE);
	g_assert_cmpint (gs_app_get_state (app2), ==, GS_APP_STATE_INSTALLED);
	g_assert_cmpint (gs_app_get_state (app3), ==, GS_APP_STATE_INSTALLED);

	/* set the default max parallel ops */
	gs_plugin_loader_set_max_parallel_ops (plugin_loader, 0);
}

static void
gs_plugins_dummy_app_size_calc_func (GsPluginLoader *loader)
{
	g_autoptr(GsApp) app1 = NULL;
	g_autoptr(GsApp) app2 = NULL;
	g_autoptr(GsApp) runtime = NULL;
	guint64 value = 0;

	app1 = gs_app_new ("app1");
	gs_app_set_state (app1, GS_APP_STATE_AVAILABLE);
	gs_app_set_size_download (app1, GS_SIZE_TYPE_VALID, 1);
	gs_app_set_size_installed (app1, GS_SIZE_TYPE_VALID, 1000);
	g_assert_cmpint (gs_app_get_size_download (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);
	g_assert_cmpint (gs_app_get_size_installed (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);

	app2 = gs_app_new ("app2");
	gs_app_set_state (app2, GS_APP_STATE_AVAILABLE);
	gs_app_set_size_download (app2, GS_SIZE_TYPE_VALID, 20);
	gs_app_set_size_installed (app2, GS_SIZE_TYPE_VALID, 20000);
	g_assert_cmpint (gs_app_get_size_download (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);
	g_assert_cmpint (gs_app_get_size_installed (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);

	runtime = gs_app_new ("runtime");
	gs_app_set_state (runtime, GS_APP_STATE_AVAILABLE);
	gs_app_set_size_download (runtime, GS_SIZE_TYPE_VALID, 300);
	gs_app_set_size_installed (runtime, GS_SIZE_TYPE_VALID, 300000);
	g_assert_cmpint (gs_app_get_size_download (runtime, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 300);
	g_assert_cmpint (gs_app_get_size_download_dependencies (runtime, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);
	g_assert_cmpint (gs_app_get_size_installed (runtime, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 300000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (runtime, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);

	gs_app_set_runtime (app1, runtime);
	g_assert_cmpint (gs_app_get_size_download (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 300);
	g_assert_cmpint (gs_app_get_size_installed (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);

	gs_app_set_runtime (app2, runtime);
	g_assert_cmpint (gs_app_get_size_download (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 300);
	g_assert_cmpint (gs_app_get_size_installed (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);

	gs_app_add_related (app1, app2);
	g_assert_cmpint (gs_app_get_size_download (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 320);
	g_assert_cmpint (gs_app_get_size_installed (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 1000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app1, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20000);

	g_assert_cmpint (gs_app_get_size_download (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20);
	g_assert_cmpint (gs_app_get_size_download_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 300);
	g_assert_cmpint (gs_app_get_size_installed (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 20000);
	g_assert_cmpint (gs_app_get_size_installed_dependencies (app2, &value), ==, GS_SIZE_TYPE_VALID);
	g_assert_cmpint (value, ==, 0);
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

	/* While we use %G_TEST_OPTION_ISOLATE_DIRS to create temporary directories
	 * for each of the tests, we want to use the system MIME registry, assuming
	 * that it exists and correctly has shared-mime-info installed. */
	g_content_type_set_mime_dirs (NULL);

	/* Force the GTK resources to be registered, needed for fallback icons. */
	gtk_init_check ();

	/* Similarly, add the system-wide icon theme path before it’s
	 * overwritten by %G_TEST_OPTION_ISOLATE_DIRS. */
	gs_test_expose_icon_theme_paths ();

	gs_test_init (&argc, &argv);
	g_setenv ("GS_XMLB_VERBOSE", "1", TRUE);

	/* set all the things required as a dummy test harness */
	setlocale (LC_MESSAGES, "en_GB.UTF-8");
	g_setenv ("GS_SELF_TEST_DUMMY_ENABLE", "1", TRUE);
	g_setenv ("GS_SELF_TEST_PROVENANCE_SOURCES", "london*,boston", TRUE);
	g_setenv ("GS_SELF_TEST_PROVENANCE_LICENSE_SOURCES", "london*,boston", TRUE);
	g_setenv ("GS_SELF_TEST_PROVENANCE_LICENSE_URL", "https://www.debian.org/", TRUE);

	/* Use a common cache directory for all tests, since the appstream
	 * plugin uses it and cannot be reinitialised for each test. */
	tmp_root = g_dir_make_tmp ("lingmo-appstore-dummy-test-XXXXXX", NULL);
	g_assert (tmp_root != NULL);
	g_setenv ("GS_SELF_TEST_CACHEDIR", tmp_root, TRUE);

	xml = g_strdup ("<?xml version=\"1.0\"?>\n"
		"<components version=\"0.9\">\n"
		"  <component type=\"desktop\">\n"
		"    <id>chiron.desktop</id>\n"
		"    <name>Chiron</name>\n"
		"    <pkgname>chiron</pkgname>\n"
		"  </component>\n"
		"  <component type=\"desktop\">\n"
		"    <id>zeus.desktop</id>\n"
		"    <name>Zeus</name>\n"
		"    <summary>A teaching application</summary>\n"
		"    <pkgname>zeus</pkgname>\n"
		"    <icon type=\"stock\">system-component-driver</icon>\n"
		"    <categories>\n"
		"      <category>AudioVideo</category>\n"
		"      <category>Player</category>\n"
		"    </categories>\n"
		"    <languages>\n"
		"      <lang percentage=\"100\">en_GB</lang>\n"
		"    </languages>\n"
		"  </component>\n"
		"  <component type=\"desktop\">\n"
		"    <id>mate-spell.desktop</id>\n"
		"    <name>Spell</name>\n"
		"    <summary>A spelling application for MATE</summary>\n"
		"    <pkgname>mate-spell</pkgname>\n"
		"    <icon type=\"stock\">system-component-driver</icon>\n"
		"    <project_group>MATE</project_group>\n"
		"  </component>\n"
		"  <component type=\"addon\">\n"
		"    <id>zeus-spell.addon</id>\n"
		"    <extends>zeus.desktop</extends>\n"
		"    <name>Spell Check</name>\n"
		"    <summary>Check the spelling when teaching</summary>\n"
		"    <pkgname>zeus-spell</pkgname>\n"
		"    <icon type=\"stock\">non-existent</icon>\n"
		"  </component>\n"
		"  <component type=\"desktop\">\n"
		"    <id>Uninstall Zeus.desktop</id>\n"
		"    <name>Uninstall Zeus</name>\n"
		"    <summary>Uninstall the teaching application</summary>\n"
		"    <icon type=\"stock\">system-component-driver</icon>\n"
		"  </component>\n"
		"  <component type=\"os-upgrade\">\n"
		"    <id>org.fedoraproject.release-rawhide.upgrade</id>\n"
		"    <name>Fedora Rawhide</name>\n"
		"    <summary>Release specific tagline</summary>\n"
		"    <pkgname>fedora-release</pkgname>\n"
		"  </component>\n"
		"  <info>\n"
		"    <scope>user</scope>\n"
		"  </info>\n"
		"</components>\n");
	g_setenv ("GS_SELF_TEST_APPSTREAM_XML", xml, TRUE);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	g_signal_connect (plugin_loader, "status-changed",
			  G_CALLBACK (gs_plugin_loader_status_changed_cb), NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR_CORE);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert (ret);
	g_assert (!gs_plugin_loader_get_enabled (plugin_loader, "notgoingtoexist"));
	g_assert (gs_plugin_loader_get_enabled (plugin_loader, "appstream"));
	g_assert (gs_plugin_loader_get_enabled (plugin_loader, "dummy"));

	/* plugin tests go here */
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/wildcard",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_wildcard_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/plugin-cache",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_plugin_cache_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/key-colors",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_key_colors_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/search",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_search_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/search-alternate",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_search_alternate_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/url-to-app",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_url_to_app_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/install",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_install_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/error",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_error_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/installed",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_installed_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/refine",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_refine_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/updates",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_updates_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/distro-upgrades",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_distro_upgrades_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/metadata-quirks",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_metadata_quirks);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/limit-parallel-ops",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_limit_parallel_ops_func);
	g_test_add_data_func ("/lingmo-appstore/plugins/dummy/app-size-calc",
			      plugin_loader,
			      (GTestDataFunc) gs_plugins_dummy_app_size_calc_func);
	retval = g_test_run ();

	/* Clean up. */
	gs_utils_rmtree (tmp_root, NULL);

	return retval;
}
