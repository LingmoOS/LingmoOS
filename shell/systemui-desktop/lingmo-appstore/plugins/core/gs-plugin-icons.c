/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2014 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include <libsoup/soup.h>
#include <string.h>

#include <lingmo-appstore.h>

#include "gs-plugin-icons.h"

/*
 * SECTION:
 * Loads remote icons and converts them into local cached ones.
 *
 * It is provided so that each plugin handling icons does not
 * have to handle the download and caching functionality.
 *
 * FIXME: This plugin will eventually go away. Currently it only exists as the
 * plugin threading code is a convenient way of ensuring that loading the remote
 * icons happens in a worker thread.
 */

struct _GsPluginIcons
{
	GsPlugin	parent;

	GMutex		 mutex;  /* protects @icon_downloader **/
	GsIconDownloader	*icon_downloader; /* (owned) */
	SoupSession	*soup_session;  /* (owned) */
};

G_DEFINE_TYPE (GsPluginIcons, gs_plugin_icons, GS_TYPE_PLUGIN)

static void
gs_plugin_icons_init (GsPluginIcons *self)
{
	/* needs remote icons downloaded */
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "appstream");
	gs_plugin_add_rule (GS_PLUGIN (self), GS_PLUGIN_RULE_RUN_AFTER, "epiphany");
}

static void
gs_plugin_icons_dispose (GObject *object)
{
	GsPluginIcons *self = GS_PLUGIN_ICONS (object);

	g_clear_object (&self->icon_downloader);
	g_clear_object (&self->soup_session);

	G_OBJECT_CLASS (gs_plugin_icons_parent_class)->dispose (object);
}

static void
gs_plugin_icons_setup_async (GsPlugin            *plugin,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
	GsPluginIcons *self = GS_PLUGIN_ICONS (plugin);
	g_autoptr(GTask) task = NULL;
	guint maximum_icon_size_px;


	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_icons_setup_async);

	g_mutex_init (&self->mutex);
	self->soup_session = gs_build_soup_session ();

	/* Currently a 160px icon is needed for #GsFeatureTile, at most. */
	maximum_icon_size_px = 160 * gs_plugin_get_scale (plugin);
	self->icon_downloader = gs_icon_downloader_new (self->soup_session, maximum_icon_size_px);

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_icons_setup_finish (GsPlugin      *plugin,
                              GAsyncResult  *result,
                              GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void shutdown_cb (GObject      *source_object,
                         GAsyncResult *result,
                         gpointer      user_data);

static void
gs_plugin_icons_shutdown_async (GsPlugin            *plugin,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
	GsPluginIcons *self = GS_PLUGIN_ICONS (plugin);
	g_autoptr(GTask) task = NULL;

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_icons_shutdown_async);

	/* Stop the icon downloader. */
	gs_icon_downloader_shutdown_async (self->icon_downloader, cancellable,
					   shutdown_cb, g_steal_pointer (&task));
}

static void
shutdown_cb (GObject      *source_object,
             GAsyncResult *result,
             gpointer      user_data)
{
	g_autoptr(GsIconDownloader) icon_downloader = NULL;
	g_autoptr(GTask) task = G_TASK (user_data);
	GsPluginIcons *self = g_task_get_source_object (task);
	g_autoptr(GError) local_error = NULL;

	g_clear_object (&self->soup_session);
	icon_downloader = g_steal_pointer (&self->icon_downloader);
	if (!gs_icon_downloader_shutdown_finish (icon_downloader, result, &local_error)) {
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_icons_shutdown_finish (GsPlugin      *plugin,
                                 GAsyncResult  *result,
                                 GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static gboolean
refine_app_unlocked (GsPluginIcons        *self,
                     GsApp                *app,
                     GsPluginRefineFlags   flags,
                     gboolean              interactive,
                     GCancellable         *cancellable,
                     GError              **error)
{
	/* not required */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON) == 0)
		return TRUE;

	gs_icon_downloader_queue_app (self->icon_downloader, app, interactive);

	return TRUE;
}

static void
gs_plugin_icons_refine_async (GsPlugin            *plugin,
                              GsAppList           *list,
                              GsPluginRefineFlags  flags,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
	GsPluginIcons *self = GS_PLUGIN_ICONS (plugin);
	g_autoptr(GTask) task = NULL;
	gboolean interactive = gs_plugin_has_flags (GS_PLUGIN (self), GS_PLUGIN_FLAGS_INTERACTIVE);

	task = g_task_new (plugin, cancellable, callback, user_data);
	g_task_set_source_tag (task, gs_plugin_icons_refine_async);

	/* nothing to do here */
	if ((flags & GS_PLUGIN_REFINE_FLAGS_REQUIRE_ICON) == 0) {
		g_task_return_boolean (task, TRUE);
		return;
	}

	{
		g_autoptr(GMutexLocker) locker = g_mutex_locker_new (&self->mutex);

		for (guint i = 0; i < gs_app_list_length (list); i++) {
			g_autoptr(GError) local_error = NULL;
			GsApp *app = gs_app_list_index (list, i);

			if (!refine_app_unlocked (self, app, flags, interactive, cancellable,
						  &local_error)) {
				g_task_return_error (task, g_steal_pointer (&local_error));
				return;
			}
		}
	}

	g_task_return_boolean (task, TRUE);
}

static gboolean
gs_plugin_icons_refine_finish (GsPlugin      *plugin,
                               GAsyncResult  *result,
                               GError       **error)
{
	return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gs_plugin_icons_class_init (GsPluginIconsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GsPluginClass *plugin_class = GS_PLUGIN_CLASS (klass);

	object_class->dispose = gs_plugin_icons_dispose;

	plugin_class->setup_async = gs_plugin_icons_setup_async;
	plugin_class->setup_finish = gs_plugin_icons_setup_finish;
	plugin_class->shutdown_async = gs_plugin_icons_shutdown_async;
	plugin_class->shutdown_finish = gs_plugin_icons_shutdown_finish;
	plugin_class->refine_async = gs_plugin_icons_refine_async;
	plugin_class->refine_finish = gs_plugin_icons_refine_finish;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_ICONS;
}
