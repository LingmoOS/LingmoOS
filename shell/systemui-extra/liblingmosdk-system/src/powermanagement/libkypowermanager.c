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

#include "libkypowermanager.h"
#include <stdio.h>
#include <lingmosdk/lingmosdk-base/libkygsetting.h>
#include <string.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>

int kdk_power_get_screenidle_timeout()
{
    int value = kdk_settings_get_int("org.lingmo.power-manager", "sleep-display-ac");
    return value;
}

int kdk_power_is_active()
{
    char line[512] = {0};
    int active = -1;
    FILE *fp = popen("xset q", "r");
    if (!fp)
    {
        return -1;
    }

    while (fgets(line, 512, fp))
    {
        if (strstr(line, "timeout"))
        {
            sscanf(line, "%*s %d", &active);
            break;
        }
    }
    return active;
}

int kdk_power_get_mode()
{
    int value = kdk_settings_get_int("org.lingmo.power-manager", "power-policy-ac");
    return value;
}

char *kdk_power_is_hibernate()
{
    char line[128] = {0};
    char *hibernate = NULL;
    FILE *fp = fopen("/sys/power/state", "r");
    if (!fp)
    {
        return NULL;
    }

    fgets(line, 128, fp);
    if(strlen(line) != 0)
    {
        hibernate = (char *)malloc(128 * sizeof(char));
        if(!hibernate)
            return NULL;
        strstripspace(line);
        strcpy(hibernate, line);
    }
    return hibernate;
}

char* kdk_power_get_control_disk_status()
{
    char line[128] = {0};
    char *status = NULL;
    FILE *fp = fopen("/sys/power/disk", "r");
    if (!fp)
    {
        return NULL;
    }

    fgets(line, 128, fp);
    if(strlen(line) != 0)
    {
        status = (char *)malloc(128 * sizeof(char));
        if(!status)
            return NULL;
        strstripspace(line);
        strcpy(status, line);
    }
    return status;
}