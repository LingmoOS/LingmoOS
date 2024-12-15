// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <QObject>
#include <QBasicTimer>

namespace core {
namespace process {
class ProcessDB;
} // namespace process
} // namespace core

using namespace core::process;

namespace core {
namespace system {

class DeviceDB;
class SysInfo;

class SystemMonitor : public QObject
{
    Q_OBJECT
    friend class CPUSet;

signals:
    void statInfoUpdated();
    void appAndProcCountUpdate(int appCount, int procCount);

public:
    explicit SystemMonitor(QObject *parent = nullptr);
    virtual ~SystemMonitor();

    static SystemMonitor *instance();

    SysInfo *sysInfo();
    DeviceDB *deviceDB();
    ProcessDB *processDB();

    void startMonitorJob();

protected:
    void timerEvent(QTimerEvent *event);

private:
    void updateSystemMonitorInfo();
    void recountAppAndProcess();

private:
    SysInfo      *m_sysInfo;
    DeviceDB     *m_deviceDB;
    ProcessDB    *m_processDB;

    QBasicTimer m_basictimer;
};

} // namespace system
} // namespace core

#endif // SYSTEM_MONITOR_H
