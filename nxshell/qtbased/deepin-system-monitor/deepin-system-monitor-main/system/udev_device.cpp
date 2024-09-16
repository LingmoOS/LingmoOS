// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "udev_device.h"
#include "udev.h"

#include <libudev.h>

namespace core {
namespace system {

UDevDevice::UDevDevice(const UDev *udev, const char *path)
{
    m_device = udev_device_new_from_syspath(udev->handle(), path);
}

UDevDevice::UDevDevice(HANDLE udevice)
{
    m_device = udevice;
}

UDevDevice::~UDevDevice()
{
    udev_device_unref(m_device);
}

} // namespace system
} // namespace core
