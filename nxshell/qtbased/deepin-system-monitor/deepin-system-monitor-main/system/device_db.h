// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICE_DB_H
#define DEVICE_DB_H

namespace core {
namespace system {

class BlockDeviceInfoDB;
class NetifInfoDB;
class MemInfo;
class CPUSet;
class SystemMonitor;
class DiskIOInfo;
class NetInfo;

/**
 * @brief The DeviceDB class
 */
class DeviceDB
{
public:
    explicit DeviceDB();
    virtual ~DeviceDB();

    static DeviceDB *instance();

    CPUSet *cpuSet();
    MemInfo *memInfo();
    NetifInfoDB *netifInfoDB();
    BlockDeviceInfoDB *blockDeviceInfoDB();
    DiskIOInfo *diskIoInfo();
    NetInfo *netInfo();

    void update();

private:
    CPUSet *m_cpuSet;
    MemInfo *m_memInfo;
    NetifInfoDB *m_netifInfoDB;
    BlockDeviceInfoDB *m_blkDevInfoDB;
    DiskIOInfo *m_diskIoInfo;
    NetInfo *m_netInfo;
};

} // namespace system
} // namespace core

#endif // DEVICE_DB_H
