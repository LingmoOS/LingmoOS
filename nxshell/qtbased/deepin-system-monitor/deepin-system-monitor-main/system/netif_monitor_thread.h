// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_MONITOR_THREAD_H
#define NETIF_MONITOR_THREAD_H

#include "common/base_thread.h"

using namespace common::core;

namespace core {
namespace system {

class NetifMonitor;
class NetifMonitorThread : public BaseThread
{
public:
    explicit NetifMonitorThread(QObject *parent = nullptr);
    ~NetifMonitorThread();

    int threadKey() const final;
    NetifMonitor *netifJobInstance() const;

protected:
    void run();

private:
    NetifMonitor *m_netifMonitor;
    QThread m_netIfmoniterThread;
};

inline int NetifMonitorThread::threadKey() const
{
    return kNetifMonitorThread;
}

} // namespace system
} // namespace core

#endif // NETIF_MONITOR_THREAD_H
