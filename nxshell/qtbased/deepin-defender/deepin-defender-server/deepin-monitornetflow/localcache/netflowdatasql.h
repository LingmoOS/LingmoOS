// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-defender/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db" // 数据库路径

class NetFlowDataSql : public QObject
{
    Q_OBJECT

public:
    explicit NetFlowDataSql(QObject *parent = nullptr);
    ~NetFlowDataSql();

public Q_SLOTS:
    void init();
    void saveNetFlowFrameToCache(const QString &sPkgName, const QString &sExecPath, double dDownloaded, double dUploaded);
    bool saveNetFlowCache();

    // 获取某个应用流量列表
    bool queryAppFlowList(const QString &sPkgName, const int type);
    // 获取所有应用流量列表
    bool queryAllAppFlowList(const int type);

Q_SIGNALS:
    // 获取某个应用流量列表
    void sendAppFlowList(const QString &sPkgName, const int type, const QList<NetFlowData> &flowList);
    // 所有应用流量信息
    void sendAllAppFlowList(const int type, const QList<NetFlowData> &flowList);

private:
    void checkInited();
    void deleteOldData();
    bool getExeHourFlowData(NetFlowData &flow, const QString &sExecPath, const QString &time);
    // 获取某个应用流量列表
    bool getAppDayFlowListByHour(QList<NetFlowData> &flowList, const QString &sPkgName, const QDateTime &time);
    bool getAppMonthFlowListByDay(QList<NetFlowData> &flowList, const QString &sPkgName, const QDateTime &time);
    bool queryAppTodayFlowListByHour(const QString &sPkgName);
    bool queryAppYesterdayFlowListByHour(const QString &sPkgName);
    bool queryAppThisMonthFlowListByDay(const QString &sPkgName);
    bool queryAppLastMonthFlowListByDay(const QString &sPkgName);
    // 获取所有应用流量列表
    bool getAllAppTimeRangeFlowList(QList<NetFlowData> &flowList, const QDateTime &startTime, const QDateTime &endTime);
    bool getAllAppDayFlowList(QList<NetFlowData> &flowList, const QDateTime &time);
    bool getAllAppMonthFlowList(QList<NetFlowData> &flowList, const QDateTime &time);
    bool queryAllAppTodayFlowList();
    bool queryAllAppYesterdayFlowList();
    bool queryAllAppThisMonthFlowList();
    bool queryAllAppLastMonthFlowList();

private:
    bool m_inited;
    QString m_localCacheDbPath;
    QSqlDatabase m_db;
    QSqlQuery m_sqlQuery;
    QTimer *m_dataSaveTimer;
    QList<NetFlowData> m_flowCacheList;
};
