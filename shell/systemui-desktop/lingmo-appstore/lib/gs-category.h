/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

#include "gs-desktop-data.h"

G_BEGIN_DECLS

#define GS_TYPE_CATEGORY (gs_category_get_type ())

G_DECLARE_FINAL_TYPE (GsCategory, gs_category, GS, CATEGORY, GObject)

GsCategory	*gs_category_new_for_desktop_data	(const GsDesktopData	*data);

const gchar	*gs_category_get_id		(GsCategory	*category);
GsCategory	*gs_category_get_parent		(GsCategory	*category);

const gchar	*gs_category_get_name		(GsCategory	*category);
const gchar	*gs_category_get_icon_name	(GsCategory	*category);
gint		 gs_category_get_score		(GsCategory	*category);

GPtrArray	*gs_category_get_desktop_groups	(GsCategory	*category);
gboolean	 gs_category_has_desktop_group	(GsCategory	*category,
						 const gchar	*desktop_group);

GsCategory	*gs_category_find_child		(GsCategory	*category,
						 const gchar	*id);
GPtrArray	*gs_category_get_children	(GsCategory	*category);

guint		 gs_category_get_size		(GsCategory	*category);
void		 gs_category_increment_size	(GsCategory	*category,
						 guint		 value);

G_END_DECLS
