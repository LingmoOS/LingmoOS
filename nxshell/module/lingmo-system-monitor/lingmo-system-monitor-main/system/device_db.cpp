// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device_db.h"

#include "cpu_set.h"
#include "mem.h"
#include "block_device_info_db.h"
#include "netif_info_db.h"
#include "diskio_info.h"
#include "net_info.h"
#include "common/thread_manager.h"
#include "system/system_monitor.h"
#include "system/system_monitor_thread.h"

#include <QSharedData>
#include <QReadLocker>
#include <QWriteLocker>

namespace core {
namespace system {

DeviceDB::DeviceDB()
{
    m_cpuSet = new CPUSet();
    m_memInfo = new MemInfo();
    m_netifInfoDB = new NetifInfoDB();
    m_blkDevInfoDB = new BlockDeviceInfoDB();
    m_diskIoInfo = new DiskIOInfo();
    m_netInfo = new NetInfo();
}

DeviceDB::~DeviceDB()
{
    if (m_diskIoInfo) {
        delete m_diskIoInfo;
        m_diskIoInfo  = nullptr;
    }
    if (m_memInfo) {
        delete m_memInfo;
        m_memInfo  = nullptr;
    }
    if (m_cpuSet) {
        delete m_cpuSet;
        m_cpuSet  = nullptr;
    }
    if (m_blkDevInfoDB) {
        delete m_blkDevInfoDB;
        m_blkDevInfoDB  = nullptr;
    }
    if (m_netifInfoDB) {
        delete m_netifInfoDB;
        m_netifInfoDB  = nullptr;
    }
    if (m_netInfo) {
        delete m_netInfo;
        m_netInfo  = nullptr;
    }
}

void DeviceDB::update()
{
    m_cpuSet->update();
    m_memInfo->readMemInfo();
    m_netifInfoDB->update();
    m_blkDevInfoDB->update();
    m_diskIoInfo->update();
    m_netInfo->resdNetInfo();
}

DeviceDB *DeviceDB::instance()
{
    auto *monitor = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
    return monitor->deviceDB();
}

BlockDeviceInfoDB *DeviceDB::blockDeviceInfoDB()
{
    return m_blkDevInfoDB;
}

NetifInfoDB *DeviceDB::netifInfoDB()
{
    return m_netifInfoDB;
}

MemInfo *DeviceDB::memInfo()
{
    return m_memInfo;
}

CPUSet *DeviceDB::cpuSet()
{
    return m_cpuSet;
}

DiskIOInfo *DeviceDB::diskIoInfo()
{
    return m_diskIoInfo;
}

NetInfo *DeviceDB::netInfo()
{
    return m_netInfo;
}

} // namespace system
} // namespace core
