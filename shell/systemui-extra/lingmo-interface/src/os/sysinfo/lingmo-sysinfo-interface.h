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


#ifndef __LINGMOSYSINFOINTERFACE_H__
#define __LINGMOSYSINFOINTERFACE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
 * Get the name of the implementation of the operating system.
 * @sysname is used to store the name of the operating system.
 */
void lingmo_os_sysinfo_sysname(char *sysname);

/*
 * Get the name of this node on the network.
 * @nodename is used to store the name of network node.
 */
void lingmo_os_sysinfo_nodename(char *nodename);

/*
 * Get current release level of this implementation.
 * @release is used to store the release level.
 */
void lingmo_os_sysinfo_release(char *release);

/*
 * Get the name of the hardware type the system is running on.
 * @machine is used to store the name of the hardware type.
 */
void lingmo_os_sysinfo_machine(char *machine);

/*
 * Get current version level of this release.
 * @version is used to store the current version level.
 */
void lingmo_os_sysinfo_version(char *version);

#ifdef __cplusplus
}
#endif

#endif
