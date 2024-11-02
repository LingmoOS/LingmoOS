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


#ifndef __LINGMOSCREENSAVERINTERFACE_H__
#define __LINGMOSCREENSAVERINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusScreensaver(void);

//Delete the DBus Communication
bool DeInitDBusScreensaver(void);

/*
 * Set this to TRUE to activate the screensaver when the session is idle.
 *
 */
void lingmo_hardware_power_idleactivationenabled(const bool in_arg);

/*
 * Set this to TRUE to lock the screen when the screensaver goes active.
 */
void lingmo_hardware_power_lockenabled(const bool in_arg);

#ifdef __cplusplus
}
#endif

#endif













