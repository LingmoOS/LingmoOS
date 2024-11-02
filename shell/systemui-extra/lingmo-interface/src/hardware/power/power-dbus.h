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


#ifndef __POWERDBUS_H__
#define __POWERDBUS_H__

#include <gio/gio.h> /* Bus define */

#ifdef  __cplusplus
extern "C" {
#endif

#define POWER_BUS         G_BUS_TYPE_SESSION
#define POWER_BUS_NAME    "cn.lingmoos.power"
#define POWER_OBJECT_PATH "/cn/lingmoos/power"

#ifdef  __cplusplus
}
#endif

#endif
