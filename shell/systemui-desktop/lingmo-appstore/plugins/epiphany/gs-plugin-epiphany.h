/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Matthew Leeds <mwleeds@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_EPIPHANY (gs_plugin_epiphany_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginEpiphany, gs_plugin_epiphany, GS, PLUGIN_EPIPHANY, GsPlugin)

G_END_DECLS
