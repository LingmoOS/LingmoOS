#ifndef VERISONDEFINE_H
#define VERISONDEFINE_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <constants.h>

// `DOCK_API_VERSION` added after v2.0.0
#ifdef DOCK_API_VERSION
#    if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
#        define USE_DOCK_2_0
#    endif
#endif

#ifdef USE_DOCK_2_0
#define ITEM_ICON_SIZE Dock::DOCK_PLUGIN_ITEM_FIXED_WIDTH
#define PANEL_ICON_SIZE Dock::QUICK_PANEL_ICON_WIDTH
#else
#define ITEM_ICON_SIZE PLUGIN_ICON_MAX_SIZE
#define PANEL_ICON_SIZE  24
#endif


#endif // VERISONDEFINE_H
