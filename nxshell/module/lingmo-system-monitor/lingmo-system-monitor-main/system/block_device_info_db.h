// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEVICE_INFO_DB_H
#define BLOCK_DEVICE_INFO_DB_H

#include "block_device.h"

#include <QReadWriteLock>
#include <QList>

namespace core {
namespace system {


class DeviceDB;

/**
 * @brief The BlockDeviceInfoDB class
 */
class BlockDeviceInfoDB
{
public:
    explicit BlockDeviceInfoDB();
    virtual ~BlockDeviceInfoDB();

    QList<BlockDevice> deviceList();

    void update();

private:
    void readDiskInfo();

private:
    mutable QReadWriteLock m_rwlock;
    QList<BlockDevice> m_deviceList;
};

inline QList<BlockDevice> BlockDeviceInfoDB::deviceList()
{
    QReadLocker lock(&m_rwlock);
    return m_deviceList;
}

} // namespace system
} // namespace core

#endif // BLOCK_DEVICE_INFO_DB_H
