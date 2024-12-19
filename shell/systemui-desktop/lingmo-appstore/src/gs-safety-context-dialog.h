/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "gs-app.h"
#include "gs-info-window.h"

G_BEGIN_DECLS

#define GS_TYPE_SAFETY_CONTEXT_DIALOG (gs_safety_context_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsSafetyContextDialog, gs_safety_context_dialog, GS, SAFETY_CONTEXT_DIALOG, GsInfoWindow)

GsSafetyContextDialog	*gs_safety_context_dialog_new		(GsApp			*app);

GsApp			*gs_safety_context_dialog_get_app	(GsSafetyContextDialog	*self);
void			 gs_safety_context_dialog_set_app	(GsSafetyContextDialog	*self,
								 GsApp			*app);

G_END_DECLS
