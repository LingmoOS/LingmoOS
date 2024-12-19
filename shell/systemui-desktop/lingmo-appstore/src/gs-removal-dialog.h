/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>

#include "lingmo-appstore-private.h"

G_BEGIN_DECLS

#define GS_TYPE_REMOVAL_DIALOG (gs_removal_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsRemovalDialog, gs_removal_dialog, GS, REMOVAL_DIALOG, AdwWindow)

GtkWidget	*gs_removal_dialog_new				(void);
void		 gs_removal_dialog_show_upgrade_removals	(GsRemovalDialog	 *self,
								 GsApp			 *upgrade);

G_END_DECLS
