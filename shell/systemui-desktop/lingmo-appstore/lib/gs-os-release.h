/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <glib-object.h>

#include "gs-app.h"

G_BEGIN_DECLS

#define GS_TYPE_OS_RELEASE (gs_os_release_get_type ())

G_DECLARE_FINAL_TYPE (GsOsRelease, gs_os_release, GS, OS_RELEASE, GObject)

GsOsRelease		*gs_os_release_new			(GError		**error);
const gchar		*gs_os_release_get_name			(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_version		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_id			(GsOsRelease	*os_release);
const gchar *	  const *gs_os_release_get_id_like		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_version_id		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_pretty_name		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_cpe_name		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_distro_codename	(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_home_url		(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_logo			(GsOsRelease	*os_release);
const gchar		*gs_os_release_get_vendor_name		(GsOsRelease	*os_release);

G_END_DECLS
