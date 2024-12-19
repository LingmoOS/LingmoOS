/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2014-2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_UPDATE_DIALOG (gs_update_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsUpdateDialog, gs_update_dialog, GS, UPDATE_DIALOG, AdwWindow)

GtkWidget	*gs_update_dialog_new				(GsPluginLoader		*plugin_loader);
GtkWidget	*gs_update_dialog_new_for_app			(GsPluginLoader		*plugin_loader,
								 GsApp			*app);

G_END_DECLS
