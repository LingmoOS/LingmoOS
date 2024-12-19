/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2014-2018 Kalev Lember <klember@redhat.com>
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Authors:
 *  - Richard Hughes <richard@hughsie.com>
 *  - Kalev Lember <klember@redhat.com>
 *  - Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

GArray	*gs_calculate_key_colors	(GdkPixbuf	*pixbuf);

G_END_DECLS
