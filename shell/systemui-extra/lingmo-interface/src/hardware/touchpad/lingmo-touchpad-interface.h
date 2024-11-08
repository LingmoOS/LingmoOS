/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#ifndef __LINGMOTOUCHPADINTERFACE_H__
#define __LINGMOTOUCHPADINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusTouchpad(void);

//Delete the DBus Communication
bool DeInitDBusTouchpad(void);

/*
 * Set this to TRUE to enable all touchpads.
 *
 */
void lingmo_hardware_touchpad_touchpadenabled(const bool in_arg);

/*
 * Enable mouse clicks with touchpad.
 *
 */
void lingmo_hardware_touchpad_taptoclick(const bool in_arg);

/*
 * Select the touchpad scroll method.
 * Supported values are:
 * @in_arg is equal to 0: disabled;
 * @in_arg is equal to 1: edge scrolling;
 * @in_arg is equal to 2: two-finger scrolling.
 *
 */
void lingmo_hardware_touchpad_scrollmethod(const int in_arg);

/*
 * Set this to true to enable natural (reverse) scrolling for touchpads.
 *
 */
void lingmo_hardware_touchpad_naturalscroll(const bool in_arg);

#ifdef __cplusplus
}
#endif

#endif













