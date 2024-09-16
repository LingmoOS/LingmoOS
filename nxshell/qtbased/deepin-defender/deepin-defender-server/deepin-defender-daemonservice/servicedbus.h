// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "daemonservice_adaptor.h"
#include "daemonservice_interface.h"
#include "monitornetflow_interface.h"
#include "hmiscreen_interface.h"

// 病毒查杀服务
#include "datainterface_interface.h"

#include "common/common.h"
#include "../../deepin-defender/src/window/modules/common/gsettingkey.h"
#include "../../deepin-defender/src/window/modules/common/comdata.h"

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QGSettings>

using HmiscreenServer = com::deepin::defender::hmiscreen;
using MonitorNetFlow = com::deepin::defender::MonitorNetFlow;

// 病毒查杀服务接口
using DaemonService = com::deepin::defender::daemonservice;

using DataInterFaceServer = com::deepin::defender::datainterface;

class ServiceDBus : public QObject
{
    Q_OBJECT
public:
    explicit ServiceDBus(QObject *parent = nullptr);

public Q_SLOTS:
    void ExitApp();
    void StartApp();
    void StartAllService();
    void ExitAllService();
    void ExitMonitorNetFloService();

    // 获取密码修改错误提示文字
    QString GetPwdError() const;
    // 获取密码安全等级
    int GetPwdLimitLevel() const;

private:
    // 判断主窗口是否启动
    bool isbDefenderMainWindowRunning();

private:
    DaemonserviceAdaptor *m_adaptor; // 守护进城适配器
    MonitorNetFlow *m_monitornetflow; // 流量检测
    HmiscreenServer *m_hmiscreenServer; // 界面服务
    DataInterFaceServer *m_DataInterFaceServer; // 数据接口
    QGSettings *m_defenderSetting; // 页面gsettings
    bool m_restartHmiscreenFlag;
    QTimer *m_timer;
};
