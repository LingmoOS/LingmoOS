/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "process_monitor.h"
#include "process_list.h"

#include <QReadLocker>
#include <QWriteLocker>
#include <QDebug>
#include <QTimer>
#include <QTimerEvent>

namespace sysmonitor {
namespace process {

#define TIME_SCANPROCESS    3000

Q_GLOBAL_STATIC(ProcessMonitorThread, theInstance)
ProcessMonitorThread *ProcessMonitorThread::instance()
{
    return theInstance();
}

ProcessMonitorThread::ProcessMonitorThread(QObject *parent)
    : QThread(parent)
    , m_monitor(new ProcessMonitor())
{
    //m_monitor->moveToThread(this);
    connect(this, &QThread::finished, this, &QObject::deleteLater);
    connect(this, &QThread::started, m_monitor, &ProcessMonitor::startMonitorJob);
    connect(this,SIGNAL(requestMonitorInterrupt()),m_monitor,SLOT(onStopScanProcess()));
}

ProcessMonitorThread::~ProcessMonitorThread()
{
    if (m_monitor) {
        delete m_monitor;
        m_monitor = nullptr;
    }
    quit();
    wait();
}

ProcessMonitor *ProcessMonitorThread::procMonitorInstance() const
{
    return m_monitor;
}

void ProcessMonitorThread::stop()
{
    emit requestMonitorInterrupt();
}


ProcessMonitor::ProcessMonitor(QObject *parent)
    : QObject(parent)
    , m_processList(new ProcessList())
{
    connect(this,SIGNAL(clearProcessList()),m_processList,SLOT(onClearAllProcess()));
}

ProcessMonitor::~ProcessMonitor()
{
    requestInterrupt();
    if (m_processList) {
        delete m_processList;
        m_processList = nullptr;
    }
}

ProcessMonitor *ProcessMonitor::instance()
{
    return ProcessMonitorThread::instance()->procMonitorInstance();
}

ProcessList *ProcessMonitor::processList()
{
    return m_processList;
}

void ProcessMonitor::startMonitorJob()
{
    processList()->stopScanProcess();
    m_basicTimer.stop();
    processList()->startScanProcess();
    QTimer::singleShot(100, this, [&,this](){
        processList()->forceQuickUpdate();
        this->updateProcMonitorInfo();
        m_basicTimer.start(TIME_SCANPROCESS, this);
    });
}

void ProcessMonitor::timerEvent(QTimerEvent *event)
{
    QObject::timerEvent(event);
    if (event->timerId() == m_basicTimer.timerId()) {
        updateProcMonitorInfo();
    }
}

void ProcessMonitor::requestInterrupt()
{
    processList()->stopScanProcess();
    m_basicTimer.stop();
}

void ProcessMonitor::onChangeRefreshFilter(QString strFilter)
{
    requestInterrupt();
    m_basicTimer.stop();
    //emit clearProcessList();
    // 切换过滤方式，保留之前的进程信息
    //processList()->onClearAllProcess();
    processList()->setScanFilter(strFilter);
    processList()->startScanProcess();
    updateProcMonitorInfo();
    m_basicTimer.start(TIME_SCANPROCESS, this);
}

void ProcessMonitor::onStartScanProcess()
{
    processList()->connectNetStateRefresh();
    startMonitorJob();
}

void ProcessMonitor::onStopScanProcess()
{
    requestInterrupt();
    processList()->disconnectNetStateRefresh();
}

void ProcessMonitor::updateProcMonitorInfo()
{
    processList()->refresh();

    emit procInfoUpdated();
}

} // namespace sysmonitor
} // namespace process
