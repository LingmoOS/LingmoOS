// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netif_monitor_thread.h"
#include "netif_monitor.h"

namespace core {
namespace system {

NetifMonitorThread::NetifMonitorThread(QObject *parent)
    : BaseThread(parent)
    , m_netifMonitor(new NetifMonitor)
{
    m_netifMonitor->moveToThread(&m_netIfmoniterThread);
    connect(&m_netIfmoniterThread, &QThread::finished, this, &QObject::deleteLater);
    connect(&m_netIfmoniterThread, &QThread::started, m_netifMonitor, &NetifMonitor::startNetmonitorJob);
    m_netIfmoniterThread.start();
}

NetifMonitorThread::~NetifMonitorThread()
{
    m_netifMonitor->requestQuit();
    m_netIfmoniterThread.quit();
    m_netIfmoniterThread.wait();

    m_netifMonitor->deleteLater();
    quit();
    wait();
}

NetifMonitor *NetifMonitorThread::netifJobInstance() const
{
    return m_netifMonitor;
}

void NetifMonitorThread::run()
{
    m_netifMonitor->handleNetData();
}

} // namespace system
} // namespace core
