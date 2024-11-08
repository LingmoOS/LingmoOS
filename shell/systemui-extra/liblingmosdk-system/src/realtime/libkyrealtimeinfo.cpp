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

#include "libkyrealtimeinfo.h"

float (*net_speed)(const char *) = kdk_real_get_net_speed;
double (*cpu_temperature)() = kdk_real_get_cpu_temperature;
int (*disk_temperature)(const char *name) = kdk_real_get_disk_temperature;
int (*disk_rate)(const char *name) = kdk_real_get_disk_rate;
float (*if_speed)(const char *nc) = kdk_real_get_if_speed;

RealTimeInfo::RealTimeInfo()
{
}

float RealTimeInfo::kdk_real_get_io_speed()
{
    return 0;
}

float RealTimeInfo::kdk_real_get_net_speed(const char *nc)
{
    return net_speed(nc);
}

float RealTimeInfo::kdk_real_get_if_speed(const char *nc)
{
    return if_speed(nc);
}

float RealTimeInfo::kdk_real_get_cpu_temperature()
{
    return cpu_temperature();
}

float RealTimeInfo::kdk_real_get_disk_temperature(const char *name)
{
    return disk_temperature(name);
}

int RealTimeInfo::kdk_real_get_disk_rate(const char *name)
{
    return disk_rate(name);
}