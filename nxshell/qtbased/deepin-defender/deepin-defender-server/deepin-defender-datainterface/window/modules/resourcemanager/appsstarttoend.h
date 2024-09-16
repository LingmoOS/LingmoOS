// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "monitornetflow_interface.h"

#include <QObject>

using MonitorNetFlow = com::deepin::defender::MonitorNetFlow;

class AppsStartToEnd : public QObject
{
    Q_OBJECT

public:
    explicit AppsStartToEnd(QObject *parent = nullptr);
    ~AppsStartToEnd();

private:
    // 初始化进程信息map
    void refreshProcInfos();
    // 判断字符是否为纯数字
    bool is_number_t(const char *string);

    // 通过pid获取进程的包名
    QString getPkgNameByPid(int pid);
    // 通过pid获取进程执行路径
    QString getProcExecPath(pid_t pid) const;
    // 从进程环境变量中获取环境信息
    QString getProcEnvProperties(pid_t pid, QString sEvnKey);
    // 通过执行路径获取包名
    QString getPkgNameByDesktopPath(QString sExecPath);

public Q_SLOTS:
    void initAppsStartEndThread();
    void startTimer();
    void stopTimer();

Q_SIGNALS:
    void sendAppsStart(QStringList);
    void sendAppsEnd(QStringList);

private:
    MonitorNetFlow *m_monitornetflow;

    QStringList m_oldAppsInfoList;
    QStringList m_newAppsInfoList;
    QTimer *m_timer;
};
