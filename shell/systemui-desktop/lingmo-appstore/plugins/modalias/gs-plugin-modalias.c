/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <fnmatch.h>
#include <gudev/gudev.h>

#include <lingmo-appstore.h>

#include "gs-plugin-modalias.h"

struct _GsPluginModalias {
	GsPlugin		 parent;

	GUdevClient		*client;
	GPtrArray		*devices;
};

G_DEFINE_TYPE (GsPluginModalias, gs_plugin_modalias, GS_TYPE_PLUGIN)

static void
gs_plugin_modalias_uevent_cb (GUdevClient *client,
                              const gchar *action,
                              GUdevDevice *device,
                              gpointer     user_data)
{
	GsPluginModalias *self = GS_PLUGIN_MODALIAS (user_data);

	if (g_strcmp0 (action, "add") == 0 ||
	    g_strcmp0 (action, "remove") == 0) {
		g_debug ("invalidating devices as '%s' sent action '%s'",
			 g_udev_device_get_sysfs_path (device),
			 action);
		g_ptr_array_set_size (self->devices, 0);
	}
}

static void
gs_plugin_modalias_init (GsPluginModalias *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "appstream");
	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_BEFORE, "icons");

	self->devices = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
	self->client = g_udev_client_new (NULL);
	g_signal_connect (self->client, "uevent",
			  G_CALLBACK (gs_plugin_modalias_uevent_cb), self);
}

static void
gs_plugin_modalias_dispose (GObject *object)
{
	GsPluginModalias *self = GS_PLUGIN_MODALIAS (object);

	g_clear_object (&self->client);
	g_clear_pointer (&self->devices, g_ptr_array_unref);

	G_OBJECT_CLASS (gs_plugin_modalias_parent_class)->dispose (object);
}

static void
gs_plugin_modalias_ensure_devices (GsPluginModalias *self)
{
	g_autoptr(GList) list = NULL;

	/* already set */
	if (self->devices->len > 0)
		return;

	/* get the devices, and assume ownership of each */
	list = g_udev_client_query_by_subsystem (self->client, NULL);
	for (GList *l = list; l != NULL; l = l->next) {
		GUdevDevice *device = G_UDEV_DEVICE (l->data);
		if (g_udev_device_get_sysfs_attr (device, "modalias") == NULL) {
			g_object_unref (device);
			continue;
		}
		g_ptr_array_add (self->devices, device);
	}
	g_debug ("%u devices with modalias", self->devices->len);
}

static gboolean
gs_plugin_modalias_matches (GsPluginModalias *self,
                            const gchar      *modalias)
{
	gs_plugin_modalias_ensure_devices (self);
	for (guint i = 0; i < self->devices->len; i++) {
		GUdevDevice *device = g_ptr_array_index (self->devices, i);
		const gchar *modalias_tmp;

		/* get the (optional) device modalias */
		modalias_tmp = g_udev_device_get_sysfs_attr (device, "modalias");
		if (modalias_tmp == NULL)
			continue;
		if (fnmatch (modalias, modalias_tmp, 0) == 0) {
			g_debug ("matched %s against %s", modalias_tmp, modalias);
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
refine_app (GsPluginModalias     *self,
	    GsApp                *app,
	    GsPluginRefineFlags   flags,
	    GCancellable         *cancellable,
	    GError              **error)
{
	GPtrArray *provided;
	guint i;

	/* not required */
	if (gs_app_has_icons (app))
		return TRUE;
	if (gs_app_get_kind (app) != AS_COMPONENT_KIND_DRIVER)
		return TRUE;

	/* do any of the modaliases match any installed hardware */
	provided = gs_app_get_provided (app);
	for (i = 0 ; i < provided->len; i++) {
		GPtrArray *items;
		AsProvided *prov = g_ptr_array_index (provided, i);
		if (as_provided_get_kind (prov) != AS_PROVIDED_KIND_MODALIAS)
			continue;
		items = as_provided_get_items (prov);
		for (guint j = 0; j < items->len; j++) {
			if (gs_plugin_modalias_matches (self, (const gchar*) g_ptr_array_index (items, j))) {
				g_autoptr(GIcon) ic = NULL;
				ic = g_themed_icon_new ("emblem-system-symbolic");
				gs_app_add_icon (app, ic);
				gs_app_add_quirk (app, GS_APP_QUIRK_NOT_LAUNCHABLE);
				break;
			}
		}
	}
	return TRUE;
}

static void
gs_plugin_modalias_refine_async (GsPlugin            *plugin,
                                 GsAppList           *list,
                                 GsPluginRefineFlags  flags,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
	GsPluginModalias *self = GS_PLUGIN_MODALIAS (plugin);
	g_autoptr(GTask) task = NULL;
	g_autoptr(GError) local_error = NULL;

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_modalias_refine_async);

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
gs_plugin_modalias_refine_finish (GsPlugin      *plugin,
                                  GAsyncResult  *result,
                                  GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_modalias_class_init (GsPluginModaliasClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_modalias_dispose;

	plugin_class->refine_async = gs_plugin_modalias_refine_async;
	plugin_class->refine_finish = gs_plugin_modalias_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_MODALIAS;
}
