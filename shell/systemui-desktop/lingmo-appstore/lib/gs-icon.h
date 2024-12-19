/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation, Inc
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <appstream.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

guint		 gs_icon_get_width			(GIcon			 *icon);
void		 gs_icon_set_width			(GIcon			 *icon,
							 guint			  width);
guint		 gs_icon_get_height			(GIcon			 *icon);
void		 gs_icon_set_height			(GIcon			 *icon,
							 guint			  height);
guint		 gs_icon_get_scale			(GIcon			 *icon);
void		 gs_icon_set_scale			(GIcon			 *icon,
							 guint			  scale);

GIcon		*gs_icon_new_for_appstream_icon		(AsIcon			 *appstream_icon);

G_END_DECLS
