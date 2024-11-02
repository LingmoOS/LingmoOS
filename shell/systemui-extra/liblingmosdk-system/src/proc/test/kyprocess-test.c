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

#include "../libkyprocess.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    size_t index = 0;
    char *run_time = kdk_get_process_running_time(13366);
    char *cpu_time = kdk_get_process_cpu_time(13366);
    char *cmd = kdk_get_process_command(13366);
    char *start_time = kdk_get_process_start_time(13366);
    char *status = kdk_get_process_status(13366);
    char *user = kdk_get_process_user(13366);
    int *port = kdk_get_process_port_nums(13366);
    printf("获取某一进程的CPU利用率：%0.1f\n", kdk_get_process_cpu_usage_percent(13366));
    printf("获取某一进程的内存占用率：%0.1f\n", kdk_get_process_mem_usage_percent(13366));
    printf("获取某一进程的进程状态：%s\n", status);
    printf("获取某一进程的进程端口号：%d\n", kdk_get_process_port(3458));
    printf("获取某一进程的启动时间：%s\n", start_time);
    printf("获取某一进程的运行时间：%s\n", run_time);
    printf("获取某一进程的CPU时间：%s\n", cpu_time);
    printf("获取某一进程的Command：%s\n", cmd);
    printf("获取某一进程的属主：%s\n", user);
    free(status);
    free(run_time);
    free(cpu_time);
    free(cmd);
    free(start_time);
    free(user);

    if (port)
    {
        while (port[index])
        {
            printf("获取某一进程的进程端口号：%d\n", port[index]);
            index++;
        }
    }
    
    char** pid =  kdk_procname_get_process_infomation("systemd");
    if (NULL != pid)
    {
        index = 0;
        while (pid[index])
        {
            printf("pid %s\n", pid[index]);
            index++;
        }
        kdk_proc_freeall(pid);
    }

    char** info = kdk_get_process_all_information();
    size_t count = 0;
    while (info[count])
    {
        printf("No. %d\t %s\n", count + 1, info[count]);
        count ++;
    }
    kdk_proc_freeall(info);

    char *name = kdk_get_process_name(3458);
    printf("name = %s\n", name);
    free(name);

    int id =  kdk_get_process_id("systemd");
    printf("pid %d\n", id);
   
    return 0;
}
