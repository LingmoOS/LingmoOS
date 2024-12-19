/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_FEDORA_LANGPACKS (gs_plugin_fedora_langpacks_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginFedoraLangpacks, gs_plugin_fedora_langpacks, GS, PLUGIN_FEDORA_LANGPACKS, GsPlugin)

G_END_DECLS
