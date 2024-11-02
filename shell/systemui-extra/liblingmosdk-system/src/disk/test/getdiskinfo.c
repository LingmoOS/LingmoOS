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

#include "../libkydiskinfo.h"
#include <stdio.h>
#include <stdlib.h>

static void display_disk_info(kdk_diskinfo *di)
{
    printf("---------------%s------------------\n", di->name);
    printf("Disk Sectors:\t%llu\n", di->sectors_num);
    printf("Disk Sector Size:\t%u\n", di->sector_size);
    printf("Disk Size:\t%f MiB\n", di->total_size_MiB);
    printf("Disk Model:\t%s\n", di->model ? di->model : "None");
    printf("Disk Serial:\t%s\n", di->serial ? di->serial : "None");
    printf("Disk Partition Number:\t%u\n", di->partition_nums);
    printf("Disk Type:\t%d\n", di->disk_type);
    printf("Disk fwrev:\t%s\n", di->fwrev);
}

static void display_disk_list(char **disklist)
{
    int count = 0;
    while (disklist[count])
    {
        printf("No. %d\t %s\n", count + 1, disklist[count]);
        count ++;
    }
}

int main()
{
    char **disklist = kdk_get_disklist();
    // display_disk_list(disklist);
    for (int i = 0; disklist[i]; i++)
    {
        kdk_diskinfo *sdainfo = kdk_get_diskinfo(disklist[i]);
        if (!sdainfo)
        {
            printf("[%s]Get disk info error.\n", disklist[i]);
            continue;
        }

        display_disk_info(sdainfo);

        kdk_free_diskinfo(sdainfo);

        int time = kdk_get_hard_running_time(disklist[i]);
        printf("运行时长 = %d\n", time);
    }
    kdk_free_disklist(disklist);


    char **di = kdk_get_hard_disk();
    size_t num = 0;
    while (di[num])
    {
        printf("No. %ld\t %s\n", num + 1, di[num]);
        char *res = kdk_get_hard_disk_size(di[num]);
        printf("容量 = %s\n", res);
        free(res);

        res = kdk_get_hard_fwrev(di[num]);
        printf("固态版本 = %s\n", res);
        free(res);

        res = kdk_get_hard_vendor(di[num]);
        printf("厂商 = %s\n", res);
        free(res);

        res = kdk_get_hard_type(di[num]);
        printf("类型 = %s\n", res);
        free(res);

        res = kdk_get_hard_model(di[num]);
        printf("型号 = %s\n", res);
        free(res);

        res =kdk_get_hard_serial(di[num]);
        printf("序列号 = %s\n", res);
        free(res);

        num ++;
    }
    kdk_free_disklist(di);

    return 0;
}
