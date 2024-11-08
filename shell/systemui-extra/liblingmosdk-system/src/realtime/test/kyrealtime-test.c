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

#include "stdio.h"
#include "../libkyrealtimeinfo.h"

int main()
{
    // if(argc < 3)
    // {
    //     printf("./kyrealtime-test <网卡名> <磁盘名>\n");
    //     return 0;
    // }
    printf("netSpeed : %f\n", kdk_real_get_net_speed("enaftgm1i0"));
    printf("cpuTemp: %f\n", kdk_real_get_cpu_temperature());
    printf("diskTemp : %d\n", kdk_real_get_disk_temperature("/dev/nvme0n1"));
    printf("diskRate : %d\n", kdk_real_get_disk_rate("/dev/nvme0n1"));
    printf("ifnetSpeed : %f\n", kdk_real_get_if_speed("enaftgm1i0"));
    return 0;
}
