// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_monitor_thread.h"

#include "system/system_monitor.h"
#include "process/process_db.h"

#include <QReadLocker>
#include <QWriteLocker>
#include <QDebug>
#include <QTimer>

namespace core {
namespace system {

SystemMonitorThread::SystemMonitorThread(QObject *parent)
    : BaseThread(parent)
    , m_monitor(new SystemMonitor())
{
    m_monitor->moveToThread(this);
    connect(this, &QThread::finished, this, &QObject::deleteLater);
    connect(this, &QThread::started, m_monitor, &SystemMonitor::startMonitorJob);
}

SystemMonitorThread::~SystemMonitorThread()
{
    m_monitor->deleteLater();
    quit();
    wait();
}

SystemMonitor *SystemMonitorThread::systemMonitorInstance() const
{
    return m_monitor;
}

} // namespace system
} // namespace core
