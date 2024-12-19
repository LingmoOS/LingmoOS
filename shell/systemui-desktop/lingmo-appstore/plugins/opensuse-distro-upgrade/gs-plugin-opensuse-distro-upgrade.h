/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2024 Jonathan Kang <jonathankang@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_OPENSUSE_DISTRO_UPGRADE (gs_plugin_opensuse_distro_upgrade_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginOpensuseDistroUpgrade, gs_plugin_opensuse_distro_upgrade, GS, PLUGIN_OPENSUSE_DISTRO_UPGRADE, GsPlugin)

G_END_DECLS
