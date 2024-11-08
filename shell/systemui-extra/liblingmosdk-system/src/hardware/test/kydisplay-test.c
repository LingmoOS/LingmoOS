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

#include "../libkydisplay.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char *vendor = kdk_display_get_vendor();
    printf("生产厂商：%s\n", vendor);
    free(vendor);
    char *product = kdk_display_get_product();
    printf("型号：%s\n", product);
    free(product);
    char *description = kdk_display_get_description();
    printf("设备类型：%s\n", description);
    free(description);
    char *id = kdk_display_get_physical_id();
    printf("物理id：%s\n", id);
    free(id);
    char *bus = kdk_display_get_bus_info();
    printf("总线地址：%s\n", bus);
    free(bus);
    char *version = kdk_display_get_version();
    printf("设备版本：%s\n", version);
    free(version);
    char *width = kdk_display_get_width();
    printf("数据宽度：%s\n", width);
    free(width);
    char *clock = kdk_display_get_clock();
    printf("频率：%s\n", clock);
    free(clock);
    char *capabilities = kdk_display_get_capabilities();
    printf("功能：%s\n", capabilities);
    free(capabilities);
    char *configuration = kdk_display_get_configuration();
    printf("配置：%s\n", configuration);
    free(configuration);
    char *resources = kdk_display_get_resources();
    printf("资源：%s\n", resources);
    free(resources);
    return 0;
} 
