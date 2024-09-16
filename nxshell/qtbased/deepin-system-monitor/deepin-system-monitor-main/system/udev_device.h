// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UDEV_DEVICE_H
#define UDEV_DEVICE_H

// qt
#include <QByteArray>

// c++11
#include <memory>

// libudev
#include <libudev.h>

namespace core {
namespace system {

class UDev;

class UDevDevice
{
public:
    using HANDLE = struct udev_device *;

    explicit UDevDevice(const UDev *udev, const char *path);
    explicit UDevDevice(HANDLE udevice);
    virtual ~UDevDevice();

    HANDLE handle() const;

    QByteArray devPath() const;
    QByteArray subSystem() const;
    QByteArray sysPath() const;
    QByteArray sysName() const;
    QByteArray devType() const;
    QByteArray attrValue(const QByteArray &attr) const;
    QByteArray propValue(const QByteArray &key) const;
    std::unique_ptr<UDevDevice> parent();
    std::unique_ptr<UDevDevice> parent(const QByteArray &subSystem, const QByteArray &devType);

private:
    HANDLE m_device;
};

inline UDevDevice::HANDLE UDevDevice::handle() const
{
    return m_device;
}

inline QByteArray UDevDevice::devPath() const
{
    if (m_device)
        return {udev_device_get_devpath(m_device)};
    else
        return {};
}

inline QByteArray UDevDevice::subSystem() const
{
    if (m_device)
        return {udev_device_get_subsystem(m_device)};
    else
        return {};
}

inline QByteArray UDevDevice::sysPath() const
{
    if (m_device)
        return {udev_device_get_syspath(m_device)};
    else
        return {};
}

inline QByteArray UDevDevice::sysName() const
{
    if (m_device)
        return {udev_device_get_sysname(m_device)};
    else
        return {};
}

inline QByteArray UDevDevice::devType() const
{
    if (m_device)
        return {udev_device_get_devtype(m_device)};
    else
        return {};
}

inline QByteArray UDevDevice::attrValue(const QByteArray &attr) const
{
    if (m_device)
        return {udev_device_get_sysattr_value(m_device, attr.data())};
    else
        return {};
}

inline QByteArray UDevDevice::propValue(const QByteArray &key) const
{
    if (m_device)
        return {udev_device_get_property_value(m_device, key.data())};
    else
        return {};
}

inline std::unique_ptr<UDevDevice> UDevDevice::parent()
{
    auto *p = udev_device_get_parent(m_device);
    if (p) {
        p = udev_device_ref(p);
        return std::unique_ptr<UDevDevice>(new UDevDevice(p));
    }
    return nullptr;
}

inline std::unique_ptr<UDevDevice> UDevDevice::parent(const QByteArray &subSystem, const QByteArray &devType)
{
    auto *p = udev_device_get_parent_with_subsystem_devtype(m_device, subSystem.data(), devType.data());
    if (p) {
        p = udev_device_ref(p);
        return std::unique_ptr<UDevDevice>(new UDevDevice(p));
    } else
        return nullptr;
}

} // namespace system
} // namespace core

#endif // UDEV_DEVICE_H
