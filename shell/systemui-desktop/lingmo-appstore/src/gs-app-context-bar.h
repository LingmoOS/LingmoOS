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

G_BEGIN_DECLS

#define GS_TYPE_APP_CONTEXT_BAR (gs_app_context_bar_get_type ())

G_DECLARE_FINAL_TYPE (GsAppContextBar, gs_app_context_bar, GS, APP_CONTEXT_BAR, GtkBox)

GtkWidget	*gs_app_context_bar_new		(GsApp			*app);

GsApp		*gs_app_context_bar_get_app	(GsAppContextBar	*self);
void		 gs_app_context_bar_set_app	(GsAppContextBar	*self,
						 GsApp			*app);

G_END_DECLS
