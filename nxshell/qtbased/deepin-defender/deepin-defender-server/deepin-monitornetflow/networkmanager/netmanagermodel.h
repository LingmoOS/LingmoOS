// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "getprocnetflow/getprocnetflow.h"
#include "localcache/netflowdatasql.h"

#include <QObject>
#include <QThread>

class GetProcInfoInter;
class SettingsSql;

class NetManagerModel : public QObject
{
    Q_OBJECT
public:
    explicit NetManagerModel(QObject *parent = nullptr);
    ~NetManagerModel();

    void enableNetFlowMonitor(bool enable);

Q_SIGNALS:
    // 发送进程信息列表
    void sendProcInfoList(const DefenderProcInfoList &list);
    void requestSaveNetFlowFrameToCache(const QString &pkgName, const QString &execPath, const double downloads, const double uploads);
    void requestQueryAppFlowList(const QString &sPkgName, const int type);
    void sendAppFlowList(const QString &sPkgName, const int type, const DefenderProcInfoList &flowList);
    // 获取所有应用流量列表
    void requestQueryAllAppFlowList(const int type);
    // 所有应用流量信息
    void sendAllAppFlowList(const int type, const DefenderProcInfoList &flowList);
    // 已安装应用列表改变信号
    void installedAppListChanged();

public Q_SLOTS:
    void onRecProcInfos(const DefenderProcInfoList &list);
    void onRecAppFlowListFromSql(const QString &sPkgName, const int type, const QList<NetFlowData> &flowList);
    void onRecAllAppFlowListFromSql(const int type, const QList<NetFlowData> &flowList);

private:
    void saveFlowData();

private:
    // 本地配置
    SettingsSql *m_settingsSql;
    // 流量监控服务
    GetProcNetFlow *m_getProcFlow;
    QThread *m_getProcFlowThread;
    // 流量统计数据查询线程
    QThread *m_netFlowDataSqlThread;
    NetFlowDataSql *m_netFlowDataSql;

    QSettings *m_config;
    GetProcInfoInter *m_getProcInfoInter;

    DefenderProcInfoList m_procInfoList;
};
