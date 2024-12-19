/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <lingmo-appstore.h>

#include "gs-plugin-os-release.h"

struct _GsPluginOsRelease
{
	GsPlugin		 parent;

	GsApp			*app_system;
};

G_DEFINE_TYPE (GsPluginOsRelease, gs_plugin_os_release, GS_TYPE_PLUGIN)

static void
gs_plugin_os_release_dispose (GObject *object)
{
	GsPluginOsRelease *self = GS_PLUGIN_OS_RELEASE (object);

	g_clear_object (&self->app_system);

	G_OBJECT_CLASS (gs_plugin_os_release_parent_class)->dispose (object);
}

static void
gs_plugin_os_release_init (GsPluginOsRelease *self)
{
	self->app_system = gs_app_new ("system");
	gs_app_set_kind (self->app_system, AS_COMPONENT_KIND_OPERATING_SYSTEM);
	gs_app_set_state (self->app_system, GS_APP_STATE_INSTALLED);
}

static void
gs_plugin_os_release_setup_async (GsPlugin            *plugin,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
	GsPluginOsRelease *self = GS_PLUGIN_OS_RELEASE (plugin);
	g_autoptr(GTask) task = NULL;
	const gchar *cpe_name;
	const gchar *home_url;
	const gchar *name;
	const gchar *version;
	const gchar *os_id;
	g_autoptr(GsOsRelease) os_release = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_os_release_setup_async);

	/* parse os-release, wherever it may be */
	os_release = gs_os_release_new (&local_error);
	if (os_release == NULL) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	cpe_name = gs_os_release_get_cpe_name (os_release);
	if (cpe_name != NULL)
		gs_app_set_metadata (self->app_system, "LingmoAppstore::CpeName", cpe_name);
	name = gs_os_release_get_name (os_release);
	if (name != NULL)
		gs_app_set_name (self->app_system, GS_APP_QUALITY_LOWEST, name);
	version = gs_os_release_get_version_id (os_release);
	if (version != NULL)
		gs_app_set_version (self->app_system, version);

	os_id = gs_os_release_get_id (os_release);

	/* use libsoup to convert a URL */
	home_url = gs_os_release_get_home_url (os_release);
	if (home_url != NULL) {
		g_autoptr(GUri) uri = NULL;

		/* homepage */
		gs_app_set_url (self->app_system, AS_URL_KIND_HOMEPAGE, home_url);

		/* Build ID from the reverse-DNS URL and the ID and version. */
		uri = g_uri_parse (home_url, SOUP_HTTP_URI_FLAGS, NULL);
		if (uri != NULL) {
			g_auto(GStrv) split = NULL;
			const gchar *home_host = g_uri_get_host (uri);
			split = g_strsplit_set (home_host, ".", -1);
			if (g_strv_length (split) >= 2) {
				g_autofree gchar *id = NULL;
				id = g_strdup_printf ("%s.%s.%s-%s",
						      split[1],
						      split[0],
						      (os_id != NULL) ? os_id : "unnamed",
						      (version != NULL) ? version : "unversioned");
				gs_app_set_id (self->app_system, id);
			}
		}
	}

	/* success */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_os_release_setup_finish (GsPlugin      *plugin,
                                   GAsyncResult  *result,
                                   GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_os_release_refine_async (GsPlugin            *plugin,
                                   GsAppList           *list,
                                   GsPluginRefineFlags  flags,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
	GsPluginOsRelease *self = GS_PLUGIN_OS_RELEASE (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_os_release_refine_async);

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);

		/* match meta-id */
		if (gs_app_has_quirk (app, GS_APP_QUIRK_IS_WILDCARD) &&
		    g_strcmp0 (gs_app_get_id (app), "system") == 0) {
			/* copy over interesting metadata */
			if (gs_app_get_install_date (app) != 0 &&
			    gs_app_get_install_date (self->app_system) == 0) {
				gs_app_set_install_date (self->app_system,
					                 gs_app_get_install_date (app));
			}

			gs_app_list_add (list, self->app_system);
			break;
		}
	}

	/* success */
	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_os_release_refine_finish (GsPlugin      *plugin,
                                    GAsyncResult  *result,
                                    GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_os_release_class_init (GsPluginOsReleaseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_os_release_dispose;

	plugin_class->setup_async = gs_plugin_os_release_setup_async;
	plugin_class->setup_finish = gs_plugin_os_release_setup_finish;
	plugin_class->refine_async = gs_plugin_os_release_refine_async;
	plugin_class->refine_finish = gs_plugin_os_release_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_OS_RELEASE;
}
