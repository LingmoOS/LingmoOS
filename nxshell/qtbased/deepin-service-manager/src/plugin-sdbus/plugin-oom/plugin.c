// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfig.h"
#include "oom.h"
#include "service.h"

#include <pthread.h>

int DSMRegister(const char *name, void *data)
{
    (void)name;
    if (!data) {
        return -1;
    }
#ifdef _DEBUG
    enable_debug = true;
#endif
    service_set_dbus(data);
    if (dbus_add_object())
        return -1;
    static poll_loop_args_t args = { .mem_term_percent = 10,
                                     .swap_term_percent = 10,
                                     .mem_kill_percent = 5,
                                     .swap_kill_percent = 5,
                                     .report_interval_ms = 1000,
                                     .kill_process_group = false,
                                     .ignore_root_user = false };

    parse_config(data, &args);
    pthread_t thread;
    pthread_create(&thread, NULL, (void *)&start_oom, &args);
    return 0;
}

// 该函数用于资源释放
// 非常驻插件必须实现该函数，以防内存泄漏
int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    return 0;
}
