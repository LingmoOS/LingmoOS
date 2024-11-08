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

#include "../libkyrtinfo.h"
#include <stdio.h>

int main()
{
    printf("内存总大小：%lu KiB\n", kdk_rti_get_mem_res_total_KiB());
    printf("当前已用内存：%lu KiB，%f\n", kdk_rti_get_mem_res_usage_KiB(), kdk_rti_get_mem_res_usage_percent());
    printf("当前空闲内存：%lu KiB\n", kdk_rti_get_mem_res_free_KiB());
    printf("可用内存大小：%lu KiB\n", kdk_rti_get_mem_res_available_KiB());
    printf("应用总申请虚拟内存大小：%lu KiB\n", kdk_rti_get_mem_virt_alloc_KiB());
    printf("交换分区总量：%lu KiB\n", kdk_rti_get_mem_swap_total_KiB());
    printf("交换分区已用量：%lu KiB, %f\n", kdk_rti_get_mem_swap_usage_KiB(), kdk_rti_get_mem_swap_usage_percent());
    printf("交换分区剩余大小：%lu KiB\n", kdk_rti_get_mem_swap_free_KiB());
    printf("当前CPU使用率：%f\n", kdk_rti_get_cpu_current_usage());
    printf("共享内存大小：%lu KiB\n", kdk_rti_get_mem_shared_KiB());
    printf("高速缓存大小：%lu KiB\n", kdk_rti_get_mem_cached_KiB());
    printf("数据缓存大小：%lu KiB\n", kdk_rti_get_mem_buffers_KiB());
    printf("交换缓存区大小：%lu KiB\n", kdk_rti_get_mem_swap_cached_KiB());
    printf("活跃的缓冲文件大小：%lu KiB\n", kdk_rti_get_mem_active_KiB());
    printf("不活跃的缓冲文件大小：%lu KiB\n", kdk_rti_get_mem_inactive_KiB());
    printf("脏页大小：%lu KiB\n", kdk_rti_get_mem_dirty_KiB());
    printf("映射大小：%lu KiB\n", kdk_rti_get_mem_map_KiB());
    printf("内核数据结构缓存大小：%lu KiB\n", kdk_rti_get_mem_slab_KiB());
    unsigned int day, hour, min, sec;
    kdk_rti_get_uptime(&day, &hour, &min, &sec);
    printf("开机时长：%u天%u小时%u分钟%u秒\n", day, hour, min, sec);
    return 0;
}
