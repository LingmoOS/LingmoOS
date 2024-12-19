/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2007-2017 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2020 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-app.h"
#include "gs-category.h"
#include "gs-category-manager.h"
#include "gs-odrs-provider.h"
#include "gs-plugin-event.h"
#include "gs-plugin.h"

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_LOADER		(gs_plugin_loader_get_type ())
G_DECLARE_FINAL_TYPE (GsPluginLoader, gs_plugin_loader, GS, PLUGIN_LOADER, GObject)

#include "gs-job-manager.h"
#include "gs-plugin-job.h"

GsPluginLoader	*gs_plugin_loader_new			(GDBusConnection *session_bus_connection,
							 GDBusConnection *system_bus_connection);
void		 gs_plugin_loader_job_process_async	(GsPluginLoader	*plugin_loader,
							 GsPluginJob	*plugin_job,
							 GCancellable	*cancellable,
							 GAsyncReadyCallback callback,
							 gpointer	 user_data);
GsAppList	*gs_plugin_loader_job_process_finish	(GsPluginLoader	*plugin_loader,
							 GAsyncResult	*res,
							 GError		**error);
gboolean	 gs_plugin_loader_job_action_finish	(GsPluginLoader	*plugin_loader,
							 GAsyncResult	*res,
							 GError		**error);
void		 gs_plugin_loader_setup_async		(GsPluginLoader	*plugin_loader,
							 const gchar * const *allowlist,
							 const gchar * const *blocklist,
							 GCancellable	*cancellable,
							 GAsyncReadyCallback callback,
							 gpointer	 user_data);
gboolean	 gs_plugin_loader_setup_finish		(GsPluginLoader	*plugin_loader,
							 GAsyncResult	*result,
							 GError		**error);

void		 gs_plugin_loader_shutdown		(GsPluginLoader	*plugin_loader,
							 GCancellable	*cancellable);

void		 gs_plugin_loader_dump_state		(GsPluginLoader	*plugin_loader);
gboolean	 gs_plugin_loader_get_enabled		(GsPluginLoader	*plugin_loader,
							 const gchar	*plugin_name);
void		 gs_plugin_loader_add_location		(GsPluginLoader	*plugin_loader,
							 const gchar	*location);
guint		 gs_plugin_loader_get_scale		(GsPluginLoader	*plugin_loader);
void		 gs_plugin_loader_set_scale		(GsPluginLoader	*plugin_loader,
							 guint		 scale);
GsAppList	*gs_plugin_loader_get_pending		(GsPluginLoader	*plugin_loader);
gboolean	 gs_plugin_loader_get_allow_updates	(GsPluginLoader	*plugin_loader);
gboolean	 gs_plugin_loader_get_network_available	(GsPluginLoader *plugin_loader);
gboolean	 gs_plugin_loader_get_network_metered	(GsPluginLoader *plugin_loader);
gboolean	 gs_plugin_loader_get_power_saver	(GsPluginLoader *plugin_loader);
gboolean	 gs_plugin_loader_get_game_mode		(GsPluginLoader *plugin_loader);
gboolean	 gs_plugin_loader_get_plugin_supported	(GsPluginLoader	*plugin_loader,
							 const gchar	*function_name);

GPtrArray	*gs_plugin_loader_get_plugins		(GsPluginLoader	*plugin_loader);

void		 gs_plugin_loader_add_event		(GsPluginLoader *plugin_loader,
							 GsPluginEvent	*event);
GPtrArray	*gs_plugin_loader_get_events		(GsPluginLoader	*plugin_loader);
GsPluginEvent	*gs_plugin_loader_get_event_default	(GsPluginLoader	*plugin_loader);
void		 gs_plugin_loader_remove_events		(GsPluginLoader	*plugin_loader);

void		 gs_plugin_loader_app_create_async	(GsPluginLoader	*plugin_loader,
							 const gchar	*unique_id,
							 GCancellable	*cancellable,
							 GAsyncReadyCallback callback,
							 gpointer	 user_data);
GsApp		*gs_plugin_loader_app_create_finish	(GsPluginLoader	*plugin_loader,
							 GAsyncResult	*res,
							 GError		**error);
void		 gs_plugin_loader_get_system_app_async	(GsPluginLoader	*plugin_loader,
							 GCancellable	*cancellable,
							 GAsyncReadyCallback callback,
							 gpointer	 user_data);
GsApp		*gs_plugin_loader_get_system_app_finish	(GsPluginLoader	*plugin_loader,
							 GAsyncResult	*res,
							 GError		**error);
GsOdrsProvider	*gs_plugin_loader_get_odrs_provider	(GsPluginLoader	*plugin_loader);

/* only useful from the self tests */
void		 gs_plugin_loader_clear_caches		(GsPluginLoader	*plugin_loader);
GsPlugin	*gs_plugin_loader_find_plugin		(GsPluginLoader	*plugin_loader,
							 const gchar	*plugin_name);
void            gs_plugin_loader_set_max_parallel_ops  (GsPluginLoader *plugin_loader,
                                                        guint           max_ops);

GsJobManager	*gs_plugin_loader_get_job_manager	(GsPluginLoader	*plugin_loader);

GsCategoryManager *gs_plugin_loader_get_category_manager (GsPluginLoader *plugin_loader);
void		 gs_plugin_loader_claim_error		(GsPluginLoader *plugin_loader,
							 GsPlugin *plugin,
							 GsPluginAction action,
							 GsApp *app,
							 gboolean interactive,
							 const GError *error);
void		 gs_plugin_loader_claim_job_error	(GsPluginLoader *plugin_loader,
							 GsPlugin *plugin,
							 GsPluginJob *job,
							 const GError *error);

gboolean	 gs_plugin_loader_app_is_valid		(GsApp *app,
							 GsPluginRefineFlags flags);
gboolean	 gs_plugin_loader_app_is_compatible	(GsPluginLoader *plugin_loader,
							 GsApp *app);

void		 gs_plugin_loader_run_adopt		(GsPluginLoader *plugin_loader,
							 GsAppList *list);
void		 gs_plugin_loader_emit_updates_changed	(GsPluginLoader *self);

G_END_DECLS
