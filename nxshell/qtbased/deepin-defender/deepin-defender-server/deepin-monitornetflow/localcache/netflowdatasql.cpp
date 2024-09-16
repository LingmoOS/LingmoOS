// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netflowdatasql.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

#define TABLE_NAME_NET_FLOW_LOG "NetFlowLog"

NetFlowDataSql::NetFlowDataSql(QObject *parent)
    : QObject(parent)
    , m_inited(false)
    , m_sqlQuery(nullptr)
{
}

NetFlowDataSql::~NetFlowDataSql()
{
}

void NetFlowDataSql::init()
{
    //初始化数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE", TABLE_NAME_NET_FLOW_LOG);
    m_localCacheDbPath = QString("%1%2").arg(DEFENDER_DATA_DIR_PATH).arg(LOCAL_CACHE_DB_NAME);
    m_db.setDatabaseName(m_localCacheDbPath);

    if (!m_db.open()) {
        qDebug() << "local db open error!";
        return;
    }

    // 如果 localcache.db数据库已经存在，则服务启动时不需要刷新数据
    QStringList tableStrList = m_db.tables();
    m_sqlQuery = QSqlQuery(m_db);
    if (!tableStrList.contains(TABLE_NAME_NET_FLOW_LOG)) {
        // 服务第一次启动启动时，创建流量日志表
        QString cmd = QString("CREATE TABLE %1 (id Long primary key, pkgName text, exePath text, dateymdh text,"
                              " total real, downloaded real, uploaded real);")
                          .arg(TABLE_NAME_NET_FLOW_LOG);
        bool bRet = m_sqlQuery.exec(cmd);
        if (!bRet) {
            qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
            return;
        }
    }

    // 定存储数据
    m_dataSaveTimer = new QTimer(this);
    connect(m_dataSaveTimer, &QTimer::timeout, this, &NetFlowDataSql::saveNetFlowCache);
    m_dataSaveTimer->start(1000);

    m_inited = true;
}

void NetFlowDataSql::saveNetFlowFrameToCache(const QString &sPkgName, const QString &sExecPath, double dDownloaded, double dUploaded)
{
    checkInited();

    NetFlowData flow;
    flow.sPkgName = sPkgName;
    flow.sExecPath = sExecPath;
    flow.sTimeyMdh = QDateTime::currentDateTime().toString("yyyy/MM/dd/hh");
    flow.dTotal = dDownloaded + dUploaded;
    flow.dDownloaded = dDownloaded;
    flow.dUploaded = dUploaded;

    // 整合每个进程同一个小时内的流量数据
    bool needMerge = false;
    for (int i = 0; i < m_flowCacheList.size(); ++i) {
        NetFlowData flowTmp = m_flowCacheList.at(i);
        if (flowTmp.sExecPath == flow.sExecPath && flowTmp.sTimeyMdh == flow.sTimeyMdh) {
            needMerge = true;
            flowTmp.dDownloaded += flow.dDownloaded;
            flowTmp.dUploaded += flow.dUploaded;
            m_flowCacheList.replace(i, flowTmp);
            break;
        }
    }

    if (!needMerge) {
        m_flowCacheList.append(flow);
    }
}

bool NetFlowDataSql::saveNetFlowCache()
{
    checkInited();

    if (m_flowCacheList.isEmpty()) {
        return true;
    }
    //数据库操作命令
    QString command;

    //判断是否到了新的月份
    QFileInfo dbInf(m_localCacheDbPath);
    if (dbInf.exists()) {
        QDateTime lastSavedTime = dbInf.lastModified();
        QDateTime currentTime = QDateTime::currentDateTime();
        if (currentTime.date().month() != lastSavedTime.date().month()) {
            //如果到了新的月份，就删除两个月之前的数据
            deleteOldData();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "local db not exists!";
        return false;
    }

    //储存数据
    for (NetFlowData flow : m_flowCacheList) {
        // 若进程路径为空,则标记为[unkown-process]
        if (flow.sExecPath.isEmpty()) {
            flow.sExecPath = "[unkown-process]";
        }

        // 1.从数据库中取出与该流量数据的进程和时间相同的数据
        NetFlowData sameHourFlow;
        if (!getExeHourFlowData(sameHourFlow, flow.sExecPath, flow.sTimeyMdh)) {
            return false;
        }

        // 2. 更新或新加数据
        if (!sameHourFlow.sExecPath.isEmpty()) {
            // 整合
            flow.dTotal += sameHourFlow.dTotal;
            flow.dDownloaded += sameHourFlow.dDownloaded;
            flow.dUploaded += sameHourFlow.dUploaded;
            command = QString("UPDATE %1 SET pkgName='%2',total=%3,downloaded=%4,uploaded=%5 WHERE exePath='%6' AND dateymdh='%7';")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(flow.sPkgName)
                          .arg(flow.dTotal)
                          .arg(flow.dDownloaded)
                          .arg(flow.dUploaded)
                          .arg(flow.sExecPath)
                          .arg(flow.sTimeyMdh);

        } else {
            command = QString("INSERT INTO %1 (pkgName, exePath, dateymdh, "
                              "total, downloaded, uploaded) "
                              "VALUES ('%2','%3','%4',%5,%6,%7);")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(flow.sPkgName)
                          .arg(flow.sExecPath)
                          .arg(flow.sTimeyMdh)
                          .arg(flow.dDownloaded + flow.dUploaded)
                          .arg(flow.dDownloaded)
                          .arg(flow.dUploaded);
        }

        if (!m_sqlQuery.exec(command)) {
            qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
            return false;
        }
    }

    // 3.存储后，清缓存
    m_flowCacheList.clear();
    return true;
}

bool NetFlowDataSql::queryAppFlowList(const QString &sPkgName, const int type)
{
    switch (TimeRangeType(type)) {
    case Today:
        return queryAppTodayFlowListByHour(sPkgName);
    case Yesterday:
        return queryAppYesterdayFlowListByHour(sPkgName);
    case ThisMonth:
        return queryAppThisMonthFlowListByDay(sPkgName);
    case LastMonth:
        return queryAppLastMonthFlowListByDay(sPkgName);
    default:
        return false;
    }
}

bool NetFlowDataSql::queryAllAppFlowList(const int type)
{
    checkInited();

    switch (TimeRangeType(type)) {
    case Today:
        return queryAllAppTodayFlowList();
    case Yesterday:
        return queryAllAppYesterdayFlowList();
    case ThisMonth:
        return queryAllAppThisMonthFlowList();
    case LastMonth:
        return queryAllAppLastMonthFlowList();
    default:
        break;
    }
    return true;
}

void NetFlowDataSql::checkInited()
{
    if (!m_inited) {
        init();
    }
}

void NetFlowDataSql::deleteOldData()
{
    checkInited();

    //找到需要删除的时间节点deleteDateTime
    QDateTime deleteDateTime = QDateTime::currentDateTime().addMonths(-2);

    QString command = QString("DELETE FROM %1 WHERE dateymdh<'%2';")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(deleteDateTime.toString("yyyy/MM/dd/hh"));

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
    }
}

bool NetFlowDataSql::getExeHourFlowData(NetFlowData &flow, const QString &sExecPath, const QString &time)
{
    checkInited();

    QString cmd;
    cmd = QString("SELECT * FROM %1 WHERE exePath='%2' AND dateymdh='%3';")
              .arg(TABLE_NAME_NET_FLOW_LOG)
              .arg(sExecPath)
              .arg(time);

    if (!m_sqlQuery.exec(cmd)) {
        qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return false;
    }

    int count = 0;
    while (m_sqlQuery.next()) {
        // 第一个数值为id
        flow.sPkgName = m_sqlQuery.value(1).toString();
        flow.sExecPath = m_sqlQuery.value(2).toString();
        flow.sTimeyMdh = m_sqlQuery.value(3).toString();
        flow.dTotal += m_sqlQuery.value(4).toDouble();
        flow.dDownloaded += m_sqlQuery.value(5).toDouble();
        flow.dUploaded += m_sqlQuery.value(6).toDouble();
        count++;
    }

    if (1 < count) {
        // 正常情况下一个进程同一小时，数据库中只存在一个数据
        qDebug() << Q_FUNC_INFO << "queried more than 1 result!";
    }
    return true;
}

bool NetFlowDataSql::getAppDayFlowListByHour(QList<NetFlowData> &flowList, const QString &sPkgName, const QDateTime &time)
{
    checkInited();

    ////获取流量数据
    QDateTime startTime = time.addDays(-1); //从前一天23:59时开始读取
    startTime.setTime(QTime(23, 59, 0));
    QDateTime endTime = time; //到这天23:59时读取结束
    endTime.setTime(QTime(23, 59, 0));
    QString command = QString("SELECT * FROM %1 WHERE dateymdh>'%2' AND"
                              " dateymdh<'%3' AND pkgName='%4';")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(startTime.toString("yyyy/MM/dd/hh"))
                          .arg(endTime.toString("yyyy/MM/dd/hh"))
                          .arg(sPkgName);

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
        return false;
    }

    //把读取的数据存入列表
    QList<NetFlowData> appListTmp;
    while (m_sqlQuery.next()) {
        NetFlowData appTmp;
        appTmp.sPkgName = m_sqlQuery.value(1).toString(); // 数据库中第一栏为id，第二栏为包名
        appTmp.sExecPath = m_sqlQuery.value(2).toString();
        appTmp.sTimeyMdh = m_sqlQuery.value(3).toString();
        appTmp.dTotal = m_sqlQuery.value(4).toDouble();
        appTmp.dDownloaded = m_sqlQuery.value(5).toDouble();
        appTmp.dUploaded = m_sqlQuery.value(6).toDouble();
        // 添加扫临时列表
        appListTmp.append(appTmp);
    }

    // 按24小时整理数据
    QDateTime timeIndex = time;
    timeIndex.setTime(QTime(0, 0, 0));
    for (int i = 0; i < 24; i++) {
        timeIndex.setTime(QTime(i, 0, 0));
        QString timeIndexStr = timeIndex.toString("yyyy/MM/dd/hh");
        NetFlowData appFlowByHourTmp; //每小时流量数据临时
        int sameTimeIndex = -1;
        for (int j = 0; j < appListTmp.size(); j++) {
            NetFlowData appTmp = appListTmp.at(j);
            if (timeIndexStr == appTmp.sTimeyMdh) { //如果数据库中此时刻存在数据
                sameTimeIndex = j;
                appFlowByHourTmp.sPkgName = sPkgName;
                appFlowByHourTmp.sExecPath = appTmp.sExecPath;
                appFlowByHourTmp.sTimeyMdh = timeIndexStr;
                appFlowByHourTmp.dTotal = appTmp.dTotal;
                appFlowByHourTmp.dDownloaded = appTmp.dDownloaded;
                appFlowByHourTmp.dUploaded = appTmp.dUploaded;
                appListTmp.removeAt(sameTimeIndex);
                break;
            }
        }

        if (-1 == sameTimeIndex) {
            appFlowByHourTmp.sPkgName = sPkgName;
            appFlowByHourTmp.sTimeyMdh = timeIndexStr;
            appFlowByHourTmp.dTotal = 0;
            appFlowByHourTmp.dDownloaded = 0;
            appFlowByHourTmp.dUploaded = 0;
        }
        // 添加到需要返回的列表
        flowList.append(appFlowByHourTmp);
    } // end - 按24小时整理数据 结束
    return true;
}

bool NetFlowDataSql::getAppMonthFlowListByDay(QList<NetFlowData> &flowList, const QString &sPkgName, const QDateTime &time)
{
    checkInited();

    ////获取流量数据
    int year = time.date().year();
    int month = time.date().month();
    int daysInThisMonth = time.date().daysInMonth();
    // 设置读取开始时间
    const QDateTime firstDayTime = QDateTime(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime startTime = firstDayTime.addDays(-1); // 从月首的前一天23:59时开始读取
    startTime.setTime(QTime(23, 59, 0));
    // 设置读取结束时间
    QDateTime endTime = firstDayTime.addDays(daysInThisMonth); // 到月底的后一天00:00时读取结束
    QString command = QString("SELECT * FROM %1 WHERE dateymdh>'%2' AND"
                              " dateymdh<'%3' AND pkgName='%4';")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(startTime.toString("yyyy/MM/dd/hh"))
                          .arg(endTime.toString("yyyy/MM/dd/hh"))
                          .arg(sPkgName);
    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
        return false;
    }

    //把读取的数据存入列表
    QList<NetFlowData> appListTmp;
    while (m_sqlQuery.next()) {
        NetFlowData appTmp;
        appTmp.sPkgName = m_sqlQuery.value(1).toString(); // 数据库中第一栏为id，第二栏为包名
        appTmp.sExecPath = m_sqlQuery.value(2).toString();
        appTmp.sTimeyMdh = m_sqlQuery.value(3).toString();
        appTmp.dTotal = m_sqlQuery.value(4).toDouble();
        appTmp.dDownloaded = m_sqlQuery.value(5).toDouble();
        appTmp.dUploaded = m_sqlQuery.value(6).toDouble();

        // 合并同一天数据
        int sameDayFlowIndex = -1;
        // 去掉时间字符串中的小时，用于判断是否为同一天
        QString appTmpTimeyMd = appTmp.sTimeyMdh.left(11);

        appTmp.sTimeyMdh = appTmpTimeyMd + "00"; // 时间字符串中小时置0，代表一天的数据
        for (int i = 0; i < appListTmp.size(); i++) {
            NetFlowData appDayFlowTmp = appListTmp.at(i);
            // 去掉时间字符串中的小时，用于判断是否为同一天
            QString appDayFlowTmpTimeyMd = appDayFlowTmp.sTimeyMdh.left(11);
            if (appTmpTimeyMd == appDayFlowTmpTimeyMd) {
                sameDayFlowIndex = i;
                appTmp.dTotal += appDayFlowTmp.dTotal;
                appTmp.dDownloaded += appDayFlowTmp.dDownloaded;
                appTmp.dUploaded += appDayFlowTmp.dUploaded;
                // 替换
                appListTmp.replace(i, appTmp);
                break;
            }
        }
        if (-1 == sameDayFlowIndex) {
            // 添加扫临时列表
            appListTmp.append(appTmp);
        }
    }

    // 按当月天数整理数据
    QDateTime timeIndex = firstDayTime;
    timeIndex.setTime(QTime(0, 0, 0));
    for (int i = 0; i < daysInThisMonth; i++) {
        // 增加时间检索值
        timeIndex = firstDayTime.addDays(i);
        // 用于比较的时间检索字符串
        QString timeIndexStr = timeIndex.toString("yyyy/MM/dd/hh");

        NetFlowData appFlowByHourTmp; //每小时流量数据临时
        int sameTimeIndex = -1;
        for (int j = 0; j < appListTmp.size(); j++) {
            NetFlowData appTmp = appListTmp.at(j);
            if (timeIndexStr == appTmp.sTimeyMdh) { // 如果数据库中此时刻存在数据
                sameTimeIndex = j;
                appFlowByHourTmp.sPkgName = sPkgName;
                appFlowByHourTmp.sExecPath = appTmp.sExecPath;
                appFlowByHourTmp.sTimeyMdh = timeIndexStr;
                appFlowByHourTmp.dTotal = appTmp.dTotal;
                appFlowByHourTmp.dDownloaded = appTmp.dDownloaded;
                appFlowByHourTmp.dUploaded = appTmp.dUploaded;
                appListTmp.removeAt(sameTimeIndex);
                break;
            }
        }

        if (-1 == sameTimeIndex) {
            appFlowByHourTmp.sPkgName = sPkgName;
            appFlowByHourTmp.sTimeyMdh = timeIndexStr;
            appFlowByHourTmp.dTotal = 0;
            appFlowByHourTmp.dDownloaded = 0;
            appFlowByHourTmp.dUploaded = 0;
        }
        // 添加到需要返回的列表
        flowList.append(appFlowByHourTmp);
    } // end - 按当月天数整理数据 结束
    return true;
}

bool NetFlowDataSql::queryAppTodayFlowListByHour(const QString &sPkgName)
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<NetFlowData> flowLst;
    ret = getAppDayFlowListByHour(flowLst, sPkgName, currentTime); //获取今天24小时流量数据
    if (!ret) {
        return false;
    }
    Q_EMIT sendAppFlowList(sPkgName, TimeRangeType::Today, flowLst);
    return true;
}

bool NetFlowDataSql::queryAppYesterdayFlowListByHour(const QString &sPkgName)
{
    checkInited();

    bool ret;
    QDateTime yesterdayTime = QDateTime::currentDateTime().addDays(-1);
    QList<NetFlowData> flowLst;
    ret = getAppDayFlowListByHour(flowLst, sPkgName, yesterdayTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAppFlowList(sPkgName, TimeRangeType::Yesterday, flowLst);
    return true;
}

bool NetFlowDataSql::queryAppThisMonthFlowListByDay(const QString &sPkgName)
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<NetFlowData> flowLst;
    ret = getAppMonthFlowListByDay(flowLst, sPkgName, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAppFlowList(sPkgName, TimeRangeType::ThisMonth, flowLst);
    return true;
}

bool NetFlowDataSql::queryAppLastMonthFlowListByDay(const QString &sPkgName)
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime().addMonths(-1);
    QList<NetFlowData> flowLst;
    ret = getAppMonthFlowListByDay(flowLst, sPkgName, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAppFlowList(sPkgName, TimeRangeType::LastMonth, flowLst);
    return true;
}

bool NetFlowDataSql::getAllAppTimeRangeFlowList(QList<NetFlowData> &flowList, const QDateTime &startTime, const QDateTime &endTime)
{
    checkInited();

    //获取流量数据
    QString command = QString("SELECT * FROM %1 WHERE dateymdh>'%2' AND"
                              " dateymdh<'%3';")
                          .arg(TABLE_NAME_NET_FLOW_LOG)
                          .arg(startTime.toString("yyyy/MM/dd/hh"))
                          .arg(endTime.toString("yyyy/MM/dd/hh"));

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
        return false;
    }

    // 把读取的数据存入列表
    while (m_sqlQuery.next()) {
        NetFlowData appTmp;
        appTmp.sPkgName = m_sqlQuery.value(1).toString(); // 数据库中第一栏为id，第二栏为包名
        appTmp.sExecPath = m_sqlQuery.value(2).toString();
        appTmp.sTimeyMdh = m_sqlQuery.value(3).toString();
        appTmp.dTotal = m_sqlQuery.value(4).toDouble();
        appTmp.dDownloaded = m_sqlQuery.value(5).toDouble();
        appTmp.dUploaded = m_sqlQuery.value(6).toDouble();
        // 找到相同项累加数据并替换
        int sameAppIndex = -1;
        for (int i = 0; i < flowList.size(); i++) {
            NetFlowData appFlowInList = flowList.at(i);
            if (appTmp.sPkgName == appFlowInList.sPkgName) {
                sameAppIndex = i;
                appTmp.dTotal += appFlowInList.dTotal;
                appTmp.dDownloaded += appFlowInList.dDownloaded;
                appTmp.dUploaded += appFlowInList.dUploaded;
                flowList.replace(i, appTmp);
            }
        }
        if (-1 == sameAppIndex) {
            flowList.append(appTmp);
        }
    }
    return true;
}

bool NetFlowDataSql::getAllAppDayFlowList(QList<NetFlowData> &flowList, const QDateTime &time)
{
    checkInited();

    // 计算时间跨度
    QDateTime startTime = time.addDays(-1); //从前一天23:59:00时开始读取
    startTime.setTime(QTime(23, 59, 0));
    QDateTime endTime = time; //到这天23:59:00时读取结束
    endTime.setTime(QTime(23, 59, 0));
    // 获取流量数据
    return getAllAppTimeRangeFlowList(flowList, startTime, endTime);
}

bool NetFlowDataSql::getAllAppMonthFlowList(QList<NetFlowData> &flowList, const QDateTime &time)
{
    checkInited();

    int year = time.date().year();
    int month = time.date().month();
    int daysInThisMonth = time.date().daysInMonth();
    // 设置读取开始时间
    const QDateTime firstDayTime = QDateTime(QDate(year, month, 1), QTime(0, 0, 0));
    QDateTime startTime = firstDayTime.addDays(-1); //从月首的前一天23:59时开始读取
    startTime.setTime(QTime(23, 59, 0));
    // 设置读取结束时间
    QDateTime endTime = startTime.addDays(daysInThisMonth); //到月底23:59时读取结束

    // 获取流量数据
    return getAllAppTimeRangeFlowList(flowList, startTime, endTime);
}

bool NetFlowDataSql::queryAllAppTodayFlowList()
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<NetFlowData> flowLst;
    ret = getAllAppDayFlowList(flowLst, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAllAppFlowList(TimeRangeType::Today, flowLst);
    return true;
}

bool NetFlowDataSql::queryAllAppYesterdayFlowList()
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime().addDays(-1);
    QList<NetFlowData> flowLst;
    ret = getAllAppDayFlowList(flowLst, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAllAppFlowList(TimeRangeType::Yesterday, flowLst);
    return true;
}

bool NetFlowDataSql::queryAllAppThisMonthFlowList()
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<NetFlowData> flowLst;
    ret = getAllAppMonthFlowList(flowLst, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAllAppFlowList(TimeRangeType::ThisMonth, flowLst);
    return true;
}

bool NetFlowDataSql::queryAllAppLastMonthFlowList()
{
    checkInited();

    bool ret;
    QDateTime currentTime = QDateTime::currentDateTime().addMonths(-1);
    QList<NetFlowData> flowLst;
    ret = getAllAppMonthFlowList(flowLst, currentTime);
    if (!ret) {
        return false;
    }
    Q_EMIT sendAllAppFlowList(TimeRangeType::LastMonth, flowLst);
    return true;
}
