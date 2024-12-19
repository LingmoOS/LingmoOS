/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_VENDOR (gs_vendor_get_type ())

G_DECLARE_FINAL_TYPE (GsVendor, gs_vendor, GS, VENDOR, GObject)

typedef enum
{
	GS_VENDOR_URL_TYPE_CODEC,
	GS_VENDOR_URL_TYPE_FONT,
	GS_VENDOR_URL_TYPE_MIME,
	GS_VENDOR_URL_TYPE_HARDWARE,
	GS_VENDOR_URL_TYPE_DEFAULT
} GsVendorUrlType;

GsVendor	*gs_vendor_new				(void);
gchar		*gs_vendor_get_not_found_url		(GsVendor		*vendor,
							 GsVendorUrlType	 type);

G_END_DECLS
