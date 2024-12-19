/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2011-2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct {
	const gchar	*id;
	const gchar	*name;
	const gchar	*fdo_cats[16];
} GsDesktopMap;

typedef struct {
	const gchar	*id;
	const GsDesktopMap *mapping;
	const gchar	*name;
	const gchar	*icon;
	gint		 score;
} GsDesktopData;

const GsDesktopData	*gs_desktop_get_data		(void);

/**
 * GS_DESKTOP_DATA_N_ENTRIES:
 *
 * Number of entries in the array returned by gs_desktop_get_data(). This is
 * static and guaranteed to be up to date. It’s intended to be used when
 * defining static arrays which need to be the same size as the array returned
 * by gs_desktop_get_data().
 *
 * Since: 40
 */
#define GS_DESKTOP_DATA_N_ENTRIES 12

G_END_DECLS
