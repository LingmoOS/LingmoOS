// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UDEV_H
#define UDEV_H

#include <QByteArray>

#include <memory>

struct udev;

namespace core {
namespace system {

class UDevDevice;

class UDev
{
public:
    using HANDLE = struct udev *;

    explicit UDev();
    virtual ~UDev();

    // subsystem phy80211 &

    // ifname => vendor & product name

    //subsystem: net => enumerate
    //udev_device_new_from_subsystem_sysname
    //parent device => subsystem (pci / usb)?
    // pci: => vendor & device
    // => udev_database
    // usb: => DEVTYPE=usb_interface => parent device
    // => idProduct & idVendor & subsystem(usb) & DEVTYPE=usb_device
    // => udev_database
    // ===> cache

    // disk => model name & size & sector size

    HANDLE handle() const;

private:
    HANDLE m_udev;
};

inline UDev::HANDLE UDev::handle() const
{
    return m_udev;
}

} // namespace system
} // namespace core

#endif // UDEV_H
