// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "com_deepin_dde_daemon_launcherd.h"
#include "com_deepin_defender_monitornetflow.h"
#include "com_deepin_defender_trafficdetails.h"
#include "com_deepin_defender_analysisapp.h"
#include "trafficdetailsworker.h"
#include "csingleteon.h"

#include <QThread>
#include <QObject>

#define Work_Debug_Flag 0
#define Work_Launcher_Flag 0

struct allApp {
    QString Path;
    QString Name;
    QString ID;
    QString Icon;
};
Q_DECLARE_METATYPE(allApp);

using Launcherd = com::deepin::dde::daemon::Launcher;
using MonitorNetFlow = com::deepin::defender::MonitorNetFlow;

class DefenderHandleThread : public QThread
{
    Q_OBJECT
public:
    DefenderHandleThread(TrafficDetailsWorker *worker, QThread *parent = nullptr);
    void run();
    void getdisplaylist(QList<AppParameter> sppedlist, QList<AppParameter> dbflowlist);
    void setFlowRangeStyle(FlowRangeStyle style);
    void setFlowName();

Q_SIGNALS:
    void SendFlowData(DefenderDisplayList displayflowlist);
    void SendSpeedData(DefenderDisplayList displaylist);
    void setSaveFlowList(QList<AppParameter> saveflowlist);
    void UpdateSpeedLauncherList();
    void notifyUpdateAllProcPid();

public Q_SLOTS:
    void onUpdate();
    void onUpdateLauncherList();
    void UpdateAllProcPid();//有新应用打开

private:
    Launcherd *m_launcherInter;
    TrafficDetailsWorker *m_worker;
    QList<allApp> allapp;//系统所有应用列表
    QList<allApp> flowallapp;
    QList<AppParameter> m_speedlist;//打开应用的网速列表
    QList<AppParameter> m_flowlist;//系统使用过流量的流量列表
    QList<AppParameter> m_dbflowlist;//数据库储存的流量列表
    QList<AppParameter> m_flownamelist;
    DefenderDisplayList m_displaylist;
    DefenderDisplayList m_displayflowlist;
    DefenderDisplayList m_stabl_displaylist;//给其他类的list
    DefenderDisplay m_showspeed;
    DefenderDisplay m_showflow;
    FlowRangeStyle m_flowRangeStyle;
    bool m_updateflag = false;
    QTimer *m_time;
};
