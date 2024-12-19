/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <glib/gi18n.h>
#include <lingmo-appstore.h>

#include "gs-plugin-generic-updates.h"

struct _GsPluginGenericUpdates
{
	GsPlugin		 parent;
};

G_DEFINE_TYPE (GsPluginGenericUpdates, gs_plugin_generic_updates, GS_TYPE_PLUGIN)

static void
gs_plugin_generic_updates_init (GsPluginGenericUpdates *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "appstream");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "packagekit");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "rpm-ostree");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_BEFORE, "icons");
}

static gboolean
gs_plugin_generic_updates_merge_os_update (GsApp *app)
{
	/* this is only for grouping system-installed packages */
	if (gs_app_get_bundle_kind (app) != AS_BUNDLE_KIND_PACKAGE ||
	    gs_app_get_scope (app) != AS_COMPONENT_SCOPE_SYSTEM)
		return FALSE;

	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_GENERIC:
	case AS_COMPONENT_KIND_REPOSITORY:
	case AS_COMPONENT_KIND_SERVICE:
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

static GsApp *
gs_plugin_generic_updates_get_os_update (GsPlugin *plugin)
{
	GsApp *app;
	const gchar *id = "org.lingmo.Appstore.OsUpdate";
	g_autoptr(GIcon) ic = NULL;

	/* create new */
	app = gs_app_new (id);
	gs_app_add_quirk (app, GS_APP_QUIRK_IS_PROXY);
	gs_app_set_management_plugin (app, plugin);
	gs_app_set_special_kind (app, GS_APP_SPECIAL_KIND_OS_UPDATE);
	gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);
	gs_app_set_name (app,
			 GS_APP_QUALITY_NORMAL,
			 /* TRANSLATORS: this is a group of updates that are not
			  * packages and are not shown in the main list */
			 _("System Updates"));
	gs_app_set_summary (app,
			    GS_APP_QUALITY_NORMAL,
			    /* TRANSLATORS: this is a longer description of the
			     * "System Updates" string */
			    _("General system updates, such as security or bug fixes, and performance improvements."));
	gs_app_set_description (app,
				GS_APP_QUALITY_NORMAL,
				gs_app_get_summary (app));
	ic = g_themed_icon_new ("system-component-os-updates");
	gs_app_add_icon (app, ic);

	return app;
}

static void
gs_plugin_generic_updates_refine_async (GsPlugin            *plugin,
                                        GsAppList           *list,
                                        GsPluginRefineFlags  flags,
                                        GCancellable        *cancellable,
                                        GAsyncReadyCallback  callback,
                                        gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GsAppList) os_updates = gs_app_list_new ();
	AsUrgencyKind max_urgency = AS_URGENCY_KIND_UNKNOWN;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_generic_updates_refine_async);

	/* not from get_updates() */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_DETAILS) == 0 &&
	    (flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_UPDATE_SEVERITY) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* do we have any packages left that are not apps? */
	for (guint i = 0; i < gs_app_list_length (list); i++) {
		GsApp *app_tmp = gs_app_list_index (list, i);
		if (gs_app_has_quirk (app_tmp, GS_APP_QUIRK_IS_WILDCARD))
			continue;
		if (gs_plugin_generic_updates_merge_os_update (app_tmp)) {
			if (max_urgency < gs_app_get_update_urgency (app_tmp))
				max_urgency = gs_app_get_update_urgency (app_tmp);
			gs_app_list_add (os_updates, app_tmp);
		}
	}
	if (gs_app_list_length (os_updates) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	/* create new meta object */
	app = gs_plugin_generic_updates_get_os_update (plugin);
	gs_app_set_update_urgency (app, max_urgency);
	for (guint i = 0; i < gs_app_list_length (os_updates); i++) {
		GsApp *app_tmp = gs_app_list_index (os_updates, i);
		gs_app_add_related (app, app_tmp);
		gs_app_list_remove (list, app_tmp);
	}
	gs_app_list_add (list, app);

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_generic_updates_refine_finish (GsPlugin      *plugin,
                                         GAsyncResult  *result,
                                         GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_generic_updates_class_init (GsPluginGenericUpdatesClass *klass)
{
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	plugin_class->refine_async = gs_plugin_generic_updates_refine_async;
	plugin_class->refine_finish = gs_plugin_generic_updates_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_GENERIC_UPDATES;
}
