/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <libinput.h>

#ifndef SUPPORT_GESTURE_HOLD

enum libinput_event_adaptor_type {
    LIBINPUT_EVENT_POINTER_SCROLL_WHEEL = libinput_event_type::LIBINPUT_EVENT_POINTER_AXIS + 1,
    LIBINPUT_EVENT_POINTER_SCROLL_FINGER,
    LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS,
    LIBINPUT_EVENT_GESTURE_HOLD_BEGIN = libinput_event_type::LIBINPUT_EVENT_GESTURE_TAP_END + 1,
    LIBINPUT_EVENT_GESTURE_HOLD_END,
};

enum libinput_tablet_tool_adaptor_type {
    LIBINPUT_TABLET_TOOL_TYPE_TOTEM = libinput_tablet_tool_type::LIBINPUT_TABLET_TOOL_TYPE_LENS + 1,
};

#endif

#ifdef __cplusplus
}
#endif