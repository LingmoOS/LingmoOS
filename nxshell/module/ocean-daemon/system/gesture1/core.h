// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GESTURE_CORE_H__
#define __GESTURE_CORE_H__

#include "touchscreen_core.h"

// only supported single finger
#define TOUCH_TYPE_RIGHT_BUTTON 50

#define BUTTON_TYPE_DOWN 501
#define BUTTON_TYPE_UP 502

#define GESTURE_TYPE_SWIPE 100
#define GESTURE_TYPE_PINCH 101
#define GESTURE_TYPE_TAP 102

// tap
#define GESTURE_DIRECTION_NONE 0
// swipe
#define GESTURE_DIRECTION_UP 10
#define GESTURE_DIRECTION_DOWN 11
#define GESTURE_DIRECTION_LEFT 12
#define GESTURE_DIRECTION_RIGHT 13
// pinch
#define GESTURE_DIRECTION_IN 14
#define GESTURE_DIRECTION_OUT 15

#ifndef LIBINPUT_EVENT_GESTURE_TAP_BEGIN
#define LIBINPUT_EVENT_GESTURE_TAP_BEGIN 806
#endif

#ifndef LIBINPUT_EVENT_GESTURE_TAP_UPDATE
#define LIBINPUT_EVENT_GESTURE_TAP_UPDATE 807
#endif

#ifndef LIBINPUT_EVENT_GESTURE_TAP_END
#define LIBINPUT_EVENT_GESTURE_TAP_END 808
#endif

int start_loop(int verbose, double distance);
void quit_loop(void);
void set_timer_duration(int duration);
void set_timer_short_duration(int duration);
void set_dblclick_duration(int duration);
void set_device_ignore(const char* node, bool ignore);

#endif
