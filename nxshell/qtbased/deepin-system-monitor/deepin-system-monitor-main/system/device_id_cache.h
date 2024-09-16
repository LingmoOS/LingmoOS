// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICE_ID_CACHE_H
#define DEVICE_ID_CACHE_H

#include <QByteArray>
#include <QHash>

struct udev_hwdb;

namespace core {
namespace system {

class UDev;

class DeviceIDCache
{
public:
    explicit DeviceIDCache(const UDev *udev);
    virtual ~DeviceIDCache();

    QByteArray deviceName(const QByteArray &modalias) const;

private:
    struct udev_hwdb *m_hwdb;
    QHash<QByteArray, QByteArray> m_cache; // modalias : product_name
};

} // namespace system
} // namespace core

#endif // DEVICE_ID_CACHE_H
