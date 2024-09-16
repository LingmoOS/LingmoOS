// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "monitornetflow_interface.h"
#include "csingleteon.h"
#include "usedata.h"
#include "common.h"
#include "window/modules/common/getprocinfointer.h"
#include "window/modules/common/defenderprocinfo.h"
#include "window/modules/common/defenderprocinfolist.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QObject>

class QThread;

#define Work_Debug_Flag 0

using Launcherd = com::deepin::dde::daemon::Launcher;
using MonitorNetFlow = com::deepin::defender::MonitorNetFlow;

class TrafficDetailsWorker : public QObject
{
    Q_OBJECT
public:
    TrafficDetailsWorker(QObject *parent = nullptr);
    ~TrafficDetailsWorker();

    //获取worker中的使用数据库类实例
    inline UseData *getUseData() { return m_useData; }
    //获取流量监控接口
    inline MonitorNetFlow *getNetFlowMonitorInter() { return m_monitornetflow; }

    // 获取当前进程流量信息列表
    QList<DefenderProcInfo> getFlowInfoList();

    void notifyGetYesterdayFlowData();
    void notifyGetTodayFlowData();
    void notifyGetLastMonthFlowData();
    void notifyGetThisMonthFlowData();
    // 通知停止获取所有应用流量数据
    void NotifyDontGetFlowData();

    void notifyGetAppYesterdayFlowData(QString sPkgName);
    void notifyGetAppTodayFlowData(QString sPkgName);
    void notifyGetAppLastMonthFlowData(QString sPkgName);
    void notifyGetAppThisMonthFlowData(QString sPkgName);

Q_SIGNALS:
    void sendAllAppFlowData(QList<DefenderProcInfo> allAppFlowList, TimeRangeType range);
    void sendAppFlowData(QList<DefenderProcInfo> allAppFlowList);

public Q_SLOTS:
    // 接收到进程信息列表
    void onRecProcInfos(const DefenderProcInfoList &list);
    //存储应用流量数据
    void saveFlowData();
    //当从UseData中接收到流量数据
    void onReceiveAllAppFlowDate(QList<AppFlowDateParameter> appLst, TimeRangeType range);
    //当从UseData中接收到流量数据
    void onReceiveAppFlowDate(QList<AppFlowDateParameter> appLst);

private:
    MonitorNetFlow *m_monitornetflow;
    //当前显示的时间段
    TimeRangeType m_flowRangeStyle;
    //当前显示时间段下的应用流量统计列表
    QList<DefenderProcInfo> m_onShowFlowList;
    //使用数据库
    UseData *m_useData;
    QThread *m_useDataThread;
    //上次存储流量数据的时间
    QDateTime m_lastSavedTime;
    // 进程信息列表
    DefenderProcInfoList m_procInfos;
};
