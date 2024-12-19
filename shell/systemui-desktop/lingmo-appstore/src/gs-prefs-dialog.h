/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_PREFS_DIALOG (gs_prefs_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsPrefsDialog, gs_prefs_dialog, GS, PREFS_DIALOG, AdwPreferencesWindow)

GtkWidget	*gs_prefs_dialog_new		(GtkWindow	*parent,
						 GsPluginLoader	*plugin_loader);

G_END_DECLS
