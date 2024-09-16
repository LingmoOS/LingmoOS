// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTK_WMJACK_WAYLAND_CLIENT_H
#define DTK_WMJACK_WAYLAND_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Dtk_Window_State
{
    int32_t pid;
    int32_t windowId;
    char resourceName[256];
    struct Geometry
    {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
    } geometry;
    bool isMinimized;
    bool isFullScreen;
    bool isActive;
    int32_t splitable;
    char uuid[256];
} WindowState;

#ifdef __cplusplus
}
#endif

#endif
