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


#ifndef __LINGMOSESSIONINTERFACE_H__
#define __LINGMOSESSIONINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusSession(void);

//Delete the DBus Communication
bool DeInitDBusSession(void);

/*
 * Time before session is considered idle
 */
void lingmo_hardware_power_idledelay(const int in_arg);

#ifdef __cplusplus
}
#endif

#endif













