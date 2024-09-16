// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_monitor.h"

#include "device_db.h"
#include "process/process_db.h"
#include "process/desktop_entry_cache_updater.h"
#include "wm/wm_window_list.h"
#include "sys_info.h"

#include <QTimerEvent>

using namespace common::core;

namespace core {
namespace system {

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
    , m_sysInfo(new SysInfo())
    , m_deviceDB(new DeviceDB())
    , m_processDB(new ProcessDB(this))
{
    m_sysInfo->readSysInfoStatic();
}

SystemMonitor::~SystemMonitor()
{
    m_basictimer.stop();
    if (m_sysInfo) {
        delete m_sysInfo;
        m_sysInfo = nullptr;
    }
    if (m_deviceDB) {
        delete m_deviceDB;
        m_deviceDB = nullptr;
    }
    if (m_processDB) {
        delete m_processDB;
        m_processDB = nullptr;
    }
}

SystemMonitor *SystemMonitor::instance()
{
    return ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread)->systemMonitorInstance();
}

DeviceDB *SystemMonitor::SystemMonitor::deviceDB()
{
    return m_deviceDB;
}

ProcessDB *SystemMonitor::SystemMonitor::processDB()
{
    return m_processDB;
}

SysInfo *SystemMonitor::SystemMonitor::sysInfo()
{
    return m_sysInfo;
}

void SystemMonitor::startMonitorJob()
{
    common::init::global_init();

    m_basictimer.stop();
    m_basictimer.start(1000, Qt::VeryCoarseTimer, this);
    updateSystemMonitorInfo();
}
static uint cnt = 0 ;
void SystemMonitor::timerEvent(QTimerEvent *event)
{

    QObject::timerEvent(event);
    if (event->timerId() == m_basictimer.timerId()) {
        if(cnt & 0x0001){
            m_sysInfo->readSysInfo();
            m_deviceDB->update();
            m_processDB->update();
        } else {
            emit statInfoUpdated();
            recountAppAndProcess();
        }
        if(cnt++ >250)
            cnt = 0;
    }
}

void SystemMonitor::updateSystemMonitorInfo()
{
    m_sysInfo->readSysInfo();
    m_deviceDB->update();
    m_processDB->update();

    emit statInfoUpdated();
    recountAppAndProcess();
}

/**
   @brief Count current apps and processes on SystemMonitor child thread.
 */
void SystemMonitor::recountAppAndProcess()
{
    ProcessSet *processSet = m_processDB->processSet();
    // count all app
    const QList<pid_t> &newpidlst = processSet->getPIDList();
    int appCount = 0;
    for (const auto &pid : newpidlst) {
        auto process = processSet->getProcessById(pid);
        if (process.appType() == kFilterApps)
            appCount++;
    }

    emit appAndProcCountUpdate(appCount, newpidlst.size());
}

} // namespace system
} // namespace core
