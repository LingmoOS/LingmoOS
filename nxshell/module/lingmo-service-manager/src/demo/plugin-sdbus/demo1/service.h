// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H

#include "systemd/sd-bus.h"

int method_hello(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
int method_multiply(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
int method_divide(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

const sd_bus_vtable calculator_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("Hello", "", "s", method_hello, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Multiply", "xx", "x", method_multiply, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("Divide", "xx", "x", method_divide, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END
};

#endif // SERVICE_H
