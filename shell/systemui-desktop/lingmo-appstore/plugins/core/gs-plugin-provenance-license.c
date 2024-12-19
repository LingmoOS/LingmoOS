/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016 Matthias Klumpp <mak@debian.org>
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <lingmo-appstore.h>

#include "gs-plugin-provenance-license.h"

/*
 * SECTION:
 * Marks the application as Free Appstore if it comes from an origin
 * that is recognized as being DFSGish-free.
 *
 * This plugin executes entirely in the main thread.
 */

struct _GsPluginProvenanceLicense {
	GsPlugin		 parent;

	GSettings		*settings;
	gchar			**sources;
	gchar			*license_id;
};

G_DEFINE_TYPE (GsPluginProvenanceLicense, gs_plugin_provenance_license, GS_TYPE_PLUGIN)

static gchar **
gs_plugin_provenance_license_get_sources (GsPluginProvenanceLicense *self)
{
	const gchar *tmp;

	tmp = g_getenv ("GS_SELF_TEST_PROVENANCE_LICENSE_SOURCES");
	if (tmp != NULL) {
		g_debug ("using custom provenance_license sources of %s", tmp);
		return g_strsplit (tmp, ",", -1);
	}
	return g_settings_get_strv (self->settings, "free-repos");
}

static gchar *
gs_plugin_provenance_license_get_id (GsPluginProvenanceLicense *self)
{
	const gchar *tmp;
	g_autofree gchar *url = NULL;

	tmp = g_getenv ("GS_SELF_TEST_PROVENANCE_LICENSE_URL");
	if (tmp != NULL) {
		g_debug ("using custom license generic sources of %s", tmp);
		url = g_strdup (tmp);
	} else {
		url = g_settings_get_string (self->settings, "free-repos-url");
		if (url == NULL)
			return g_strdup ("LicenseRef-free");
	}
	return g_strdup_printf ("LicenseRef-free=%s", url);
}

static void
gs_plugin_provenance_license_changed_cb (GSettings   *settings,
                                         const gchar *key,
                                         gpointer     user_data)
{
	GsPluginProvenanceLicense *self = GS_PLUGIN_PROVENANCE_LICENSE (user_data);

	if (g_strcmp0 (key, "free-repos") == 0) {
		g_strfreev (self->sources);
		self->sources = gs_plugin_provenance_license_get_sources (self);
	}
	if (g_strcmp0 (key, "free-repos-url") == 0) {
		g_free (self->license_id);
		self->license_id = gs_plugin_provenance_license_get_id (self);
	}
}

static void
gs_plugin_provenance_license_init (GsPluginProvenanceLicense *self)
{
	self->settings = g_settings_new ("org.lingmo.appstore");
	g_signal_connect (self->settings, "changed",
			  G_CALLBACK (gs_plugin_provenance_license_changed_cb), self);
	self->sources = gs_plugin_provenance_license_get_sources (self);
	self->license_id = gs_plugin_provenance_license_get_id (self);

	/* need this set */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "provenance");
}

static void
gs_plugin_provenance_license_dispose (GObject *object)
{
	GsPluginProvenanceLicense *self = GS_PLUGIN_PROVENANCE_LICENSE (object);

	g_clear_pointer (&self->sources, g_strfreev);
	g_clear_pointer (&self->license_id, g_free);
	g_clear_object (&self->settings);

	G_OBJECT_CLASS (gs_plugin_provenance_license_parent_class)->dispose (object);
}

static gboolean
refine_app (GsPluginProvenanceLicense  *self,
            GsApp                      *app,
            GsPluginRefineFlags         flags,
            GCancellable               *cancellable,
            GError                    **error)
{
	const gchar *origin;

	/* not required */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE) == 0)
		return TRUE;

	/* no provenance */
	if (!gs_app_has_quirk (app, GS_APP_QUIRK_PROVENANCE))
		return TRUE;

	/* nothing to search */
	if (self->sources == NULL || self->sources[0] == NULL)
		return TRUE;

	/* simple case */
	origin = gs_app_get_origin (app);
	if (origin != NULL && gs_utils_strv_fnmatch (self->sources, origin))
		gs_app_set_license (app, GS_APP_QUALITY_NORMAL, self->license_id);

	return TRUE;
}

static void
gs_plugin_provenance_license_refine_async (GsPlugin            *plugin,
                                           GsAppList           *list,
                                           GsPluginRefineFlags  flags,
                                           GCancellable        *cancellable,
                                           GAsyncReadyCallback  callback,
                                           gpointer             user_data)
{
	GsPluginProvenanceLicense *self = GS_PLUGIN_PROVENANCE_LICENSE (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_provenance_license_refine_async);

	/* nothing to do here */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_LICENSE) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* nothing to search */
	if (self->sources == NULL || self->sources[0] == NULL) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app = gs_app_list_index (list, i);
		if (!refine_app (self, app, flags, cancellable, &local_error)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_provenance_license_refine_finish (GsPlugin      *plugin,
                                            GAsyncResult  *result,
                                            GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_provenance_license_class_init (GsPluginProvenanceLicenseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_provenance_license_dispose;

	plugin_class->refine_async = gs_plugin_provenance_license_refine_async;
	plugin_class->refine_finish = gs_plugin_provenance_license_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_PROVENANCE_LICENSE;
}
