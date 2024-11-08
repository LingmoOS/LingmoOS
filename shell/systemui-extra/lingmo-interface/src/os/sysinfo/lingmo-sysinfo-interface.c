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


#include <stdio.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <string.h>
#include "lingmo-sysinfo-interface.h"

void lingmo_os_sysinfo_sysname(char *sysname)
{
    struct utsname buf;
    if(uname(&buf))
    {
        perror("uname");
        return;
    }

    strcpy(sysname,buf.sysname);

}

void lingmo_os_sysinfo_nodename(char *nodename)
{
    struct utsname buf;
    if(uname(&buf))
    {
        perror("uname");
        return;
    }

    strcpy(nodename,buf.nodename);
}

void lingmo_os_sysinfo_release(char *release)
{
    struct utsname buf;
    if(uname(&buf))
    {
        perror("uname");
        return;
    }

    strcpy(release,buf.release);
}

void lingmo_os_sysinfo_machine(char *machine)
{
    struct utsname buf;
    if(uname(&buf))
    {
        perror("uname");
        return;
    }

    strcpy(machine,buf.machine);
}

void lingmo_os_sysinfo_version(char *version)
{
    struct utsname buf;
    if(uname(&buf))
    {
        perror("uname");
        return;
    }

    strcpy(version,buf.version);
}
