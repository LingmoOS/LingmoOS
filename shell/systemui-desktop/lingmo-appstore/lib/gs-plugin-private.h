/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gio/gio.h>
#include <glib-object.h>
#include <gmodule.h>

#include "gs-plugin.h"

G_BEGIN_DECLS

GsPlugin	*gs_plugin_new				(GDBusConnection *session_bus_connection,
							 GDBusConnection *system_bus_connection);
GsPlugin	*gs_plugin_create			(const gchar	*filename,
							 GDBusConnection *session_bus_connection,
							 GDBusConnection *system_bus_connection,
							 GError		**error);
const gchar	*gs_plugin_error_to_string		(GsPluginError	 error);
const gchar	*gs_plugin_action_to_string		(GsPluginAction	 action);
GsPluginAction	 gs_plugin_action_from_string		(const gchar	*action);
const gchar	*gs_plugin_action_to_function_name	(GsPluginAction	 action);

void		 gs_plugin_set_scale			(GsPlugin	*plugin,
							 guint		 scale);
guint		 gs_plugin_get_order			(GsPlugin	*plugin);
void		 gs_plugin_set_order			(GsPlugin	*plugin,
							 guint		 order);
guint		 gs_plugin_get_priority			(GsPlugin	*plugin);
void		 gs_plugin_set_priority			(GsPlugin	*plugin,
							 guint		 priority);
void		 gs_plugin_set_name			(GsPlugin	*plugin,
							 const gchar	*name);
void		 gs_plugin_set_language			(GsPlugin	*plugin,
							 const gchar	*language);
void		 gs_plugin_set_auth_array		(GsPlugin	*plugin,
							 GPtrArray	*auth_array);
GPtrArray	*gs_plugin_get_rules			(GsPlugin	*plugin,
							 GsPluginRule	 rule);
gpointer	 gs_plugin_get_symbol			(GsPlugin	*plugin,
							 const gchar	*function_name);
void		 gs_plugin_interactive_inc		(GsPlugin	*plugin);
void		 gs_plugin_interactive_dec		(GsPlugin	*plugin);
gchar		*gs_plugin_refine_flags_to_string	(GsPluginRefineFlags refine_flags);
void		 gs_plugin_set_network_monitor		(GsPlugin		*plugin,
							 GNetworkMonitor	*monitor);

G_END_DECLS
