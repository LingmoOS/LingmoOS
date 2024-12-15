// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_MONITOR_THREAD_H
#define SYSTEM_MONITOR_THREAD_H

#include "common/base_thread.h"
#include "system/system_monitor.h"

#include <QReadWriteLock>

#include <memory>

using namespace common::core;

namespace core {
namespace system {

class SystemMonitor;

class SystemMonitorThread : public BaseThread
{
    Q_OBJECT

public:
    explicit SystemMonitorThread(QObject *parent = nullptr);
    virtual ~SystemMonitorThread();

    int threadKey() const final;

public:
    SystemMonitor *systemMonitorInstance() const;

private:
    SystemMonitor *m_monitor;
};

inline int SystemMonitorThread::threadKey() const
{
    return kSystemMonitorThread;
}

} // namespace system
} // namespace core

#endif // SYSTEM_MONITOR_THREAD_H
