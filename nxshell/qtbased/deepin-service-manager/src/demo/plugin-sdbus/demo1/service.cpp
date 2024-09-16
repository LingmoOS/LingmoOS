// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

#include <iostream>
#include <string>

int method_hello(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    (void)userdata;
    (void)ret_error;
    const char *string = "World";
    return sd_bus_reply_method_return(m, "s", string);
}

int method_multiply(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    (void)userdata;
    (void)ret_error;
    int64_t x, y;
    int r;

    /* Read the parameters */
    r = sd_bus_message_read(m, "xx", &x, &y);
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }

    /* Reply with the response */
    return sd_bus_reply_method_return(m, "x", x * y);
}

int method_divide(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    (void)userdata;
    std::cout << "method_divide called." << std::endl;
    int64_t x, y;
    int r;

    /* Read the parameters */
    r = sd_bus_message_read(m, "xx", &x, &y);
    if (r < 0) {
        fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
        return r;
    }

    /* Return an error on division by zero */
    if (y == 0) {
        sd_bus_error_set_const(ret_error,
                               "org.deepin.demo.sdbus.test.error.DivisionByZero",
                               "Sorry, can't allow division by zero.");
        return -EINVAL;
    }

    return sd_bus_reply_method_return(m, "x", x / y);
}
