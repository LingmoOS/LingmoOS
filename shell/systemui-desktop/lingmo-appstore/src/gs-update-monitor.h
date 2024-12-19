/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-application.h"
#include "gs-shell.h"

G_BEGIN_DECLS

#define GS_TYPE_UPDATE_MONITOR (gs_update_monitor_get_type ())

G_DECLARE_FINAL_TYPE (GsUpdateMonitor, gs_update_monitor, GS, UPDATE_MONITOR, GObject)

GsUpdateMonitor	*gs_update_monitor_new			(GsApplication	*app,
							 GsPluginLoader	*plugin_loader);
void		 gs_update_monitor_autoupdate		(GsUpdateMonitor *monitor);
void		 gs_update_monitor_show_error		(GsUpdateMonitor *monitor,
							 GtkWindow	*window);

G_END_DECLS
