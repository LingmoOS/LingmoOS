// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICE_DB_H
#define DEVICE_DB_H

namespace core {
namespace system {

class MemInfo;
class CPUSet;
class NetInfo;
class DiskIOInfo;
class BlockDeviceInfoDB;

/**
 * @brief The DeviceDB class
 */
class DeviceDB
{
public:
    explicit DeviceDB();
    ~DeviceDB();

    static DeviceDB *instance();

    CPUSet *cpuSet();
    MemInfo *memInfo();
    DiskIOInfo *diskIoInfo();
    BlockDeviceInfoDB *blockDeviceInfoDB();
    NetInfo *netInfo();

    void update();

private:
    CPUSet *m_cpuSet;
    MemInfo *m_memInfo;
    NetInfo *m_netInfo;
    BlockDeviceInfoDB *m_blkDevInfoDB;
    DiskIOInfo *m_diskIoInfo;
};

} // namespace system
} // namespace core

#endif // DEVICE_DB_H
