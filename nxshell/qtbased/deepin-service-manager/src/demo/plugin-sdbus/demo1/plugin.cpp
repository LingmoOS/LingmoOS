// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)name;
    if (!data) {
        return -1;
    }
    sd_bus *bus = (sd_bus *)data;
    sd_bus_slot *slot = NULL;
    if (sd_bus_add_object_vtable(bus,
                                 &slot,
                                 "/org/deepin/service/sdbus/demo1",
                                 "org.deepin.service.sdbus.demo1",
                                 calculator_vtable,
                                 NULL)
        < 0) {
        return -1;
    }
    return 0;
}

// 该函数用于资源释放
// 非常驻插件必须实现该函数，以防内存泄漏
extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    return 0;
}
