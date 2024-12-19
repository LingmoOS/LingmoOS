/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015-2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "gs-page.h"

G_BEGIN_DECLS

#define GS_TYPE_EXTRAS_PAGE (gs_extras_page_get_type ())

G_DECLARE_FINAL_TYPE (GsExtrasPage, gs_extras_page, GS, EXTRAS_PAGE, GsPage)

typedef enum {
	GS_EXTRAS_PAGE_MODE_UNKNOWN,
	GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_FILES,
	GS_EXTRAS_PAGE_MODE_INSTALL_PROVIDE_FILES,
	GS_EXTRAS_PAGE_MODE_INSTALL_PACKAGE_NAMES,
	GS_EXTRAS_PAGE_MODE_INSTALL_MIME_TYPES,
	GS_EXTRAS_PAGE_MODE_INSTALL_FONTCONFIG_RESOURCES,
	GS_EXTRAS_PAGE_MODE_INSTALL_GSTREAMER_RESOURCES,
	GS_EXTRAS_PAGE_MODE_INSTALL_PLASMA_RESOURCES,
	GS_EXTRAS_PAGE_MODE_INSTALL_PRINTER_DRIVERS,
	GS_EXTRAS_PAGE_MODE_LAST
} GsExtrasPageMode;

const gchar		*gs_extras_page_mode_to_string		(GsExtrasPageMode	  mode);
GsExtrasPage		*gs_extras_page_new			(void);
void			 gs_extras_page_search			(GsExtrasPage		 *self,
								 const gchar 		 *mode,
								 gchar			**resources,
								 const gchar             *desktop_id,
								 const gchar		 *ident);

G_END_DECLS
