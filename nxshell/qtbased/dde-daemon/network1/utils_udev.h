// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <libudev.h>

int is_device_has_property(struct udev_device *device, const char *property);
char *get_device_vendor(const char *syspath);
char *get_device_product(const char *syspath);
int is_usb_device(const char *syspath);
