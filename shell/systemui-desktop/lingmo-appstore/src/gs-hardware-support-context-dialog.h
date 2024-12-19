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

#define GS_TYPE_HARDWARE_SUPPORT_CONTEXT_DIALOG (gs_hardware_support_context_dialog_get_type ())

G_DECLARE_FINAL_TYPE (GsHardwareSupportContextDialog, gs_hardware_support_context_dialog, GS, HARDWARE_SUPPORT_CONTEXT_DIALOG, GsInfoWindow)

GsHardwareSupportContextDialog	*gs_hardware_support_context_dialog_new		(GsApp				*app);

GsApp				*gs_hardware_support_context_dialog_get_app	(GsHardwareSupportContextDialog	*self);
void				 gs_hardware_support_context_dialog_set_app	(GsHardwareSupportContextDialog	*self,
										 GsApp				*app);

void gs_hardware_support_context_dialog_get_control_support (GdkDisplay     *display,
                                                             GPtrArray      *relations,
                                                             gboolean       *any_control_relations_set_out,
                                                             AsRelationKind *control_relations,
                                                             gboolean       *has_touchscreen_out,
                                                             gboolean       *has_keyboard_out,
                                                             gboolean       *has_mouse_out);

GdkMonitor *gs_hardware_support_context_dialog_get_largest_monitor (GdkDisplay *display);
void gs_hardware_support_context_dialog_get_display_support (GdkMonitor     *monitor,
                                                             GPtrArray      *relations,
                                                             gboolean       *any_display_relations_set_out,
                                                             gboolean       *desktop_match_out,
                                                             AsRelationKind *desktop_relation_kind_out,
                                                             gboolean       *mobile_match_out,
                                                             AsRelationKind *mobile_relation_kind_out,
                                                             gboolean       *other_match_out,
                                                             AsRelationKind *other_relation_kind_out);

G_END_DECLS
