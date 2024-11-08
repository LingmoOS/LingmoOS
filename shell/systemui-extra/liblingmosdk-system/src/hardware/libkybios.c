/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkybios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *kdk_bios_get_vendor()
{
    char *vendor = (char *)calloc(32,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_vendor","r");
    if(fd)
    {
        fgets(vendor,32,fd);
        fclose(fd);
    }
    return vendor;
}

const char *kdk_bios_get_version()
{
    char *version = (char *)calloc(32,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_version","r");
    if(fd)
    {
        fgets(version,32,fd);
        fclose(fd);
    }
    return version;
}

void kdk_bios_free(char* info)
{
    free(info);
}
