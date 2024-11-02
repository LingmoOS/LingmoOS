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

#include "../libkycpu.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("架构：%s\n", kdk_cpu_get_arch());
    printf("生产厂商：%s\n", kdk_cpu_get_vendor());
    printf("CPU 型号：%s\n", kdk_cpu_get_model());
    printf("CPU 主频：%s MHz\n", kdk_cpu_get_freq_MHz());
    printf("CPU 单核核心数量：%u\n", kdk_cpu_get_corenums());
    printf("CPU 虚拟化支持：%s\n", kdk_cpu_get_virt());
    printf("CPU 线程数：%u\n", kdk_cpu_get_process());

    printf("CPU 最大频率：%0.2f MHz\n", kdk_cpu_get_max_freq_MHz());
    printf("CPU 最小频率：%0.2f MHz\n", kdk_cpu_get_min_freq_MHz());
    char *run_time = kdk_cpu_get_running_time();
    printf("CPU 运行时间：%s\n", run_time);
    free(run_time);
    printf("CPU 插槽：%d\n", kdk_cpu_get_sockets());
    printf("CPU L1缓存（数据）：%d\n", kdk_cpu_get_L1d_cache());
    printf("CPU L1缓存（指令）：%d\n", kdk_cpu_get_L1i_cache());
    printf("CPU L2缓存：%d\n", kdk_cpu_get_L2_cache());
    printf("CPU L3缓存：%d\n", kdk_cpu_get_L3_cache());
    
    return 0;
}
