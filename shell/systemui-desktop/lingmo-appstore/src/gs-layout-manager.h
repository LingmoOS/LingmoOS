/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_LAYOUT_MANAGER (gs_layout_manager_get_type ())
G_DECLARE_DERIVABLE_TYPE (GsLayoutManager, gs_layout_manager, GS, LAYOUT_MANAGER, GtkLayoutManager)

struct _GsLayoutManagerClass {
	GtkLayoutManagerClass parent_class;
};

GtkLayoutManager *
		gs_layout_manager_new		(void);

G_END_DECLS
