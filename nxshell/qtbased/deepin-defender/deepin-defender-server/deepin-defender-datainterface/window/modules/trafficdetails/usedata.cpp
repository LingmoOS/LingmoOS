// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usedata.h"
#include "csingleteon.h"

#include <QThread>
#include <QDir>

UseData::UseData(QObject *parent)
    : QObject(parent)
{
    //注册信号类型；
    qRegisterMetaType<QList<AppFlowDateParameter>>("QList<AppFlowDateParameter>");
    qRegisterMetaType<QPair<SignalQueueType, AppFlowDateParameter>>("QPair<SignalQueueType, AppFlowDateParameter>");

    //初始化数据库
    initDataBase();

    //connect
    //链接响应消息信号执行函数
    connect(this, &UseData::reqSendSignal, this, &UseData::onReqSendSignal);
    //向操作消息队列中添加 清空数据缓冲列表 的操作信号，并响应消息信号执行函数
    connect(this, &UseData::notifyDeleteFlowCacheData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(DeleteFlowCacheData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    //向操作消息队列中添加 "保存缓存数据到数据库" 的操作信号，并响应消息信号执行函数
    connect(this, &UseData::notifySaveFlowCacheDataToDB, this, [this](QList<AppFlowDateParameter> flowList) {
        Q_EMIT this->notifyDeleteFlowCacheData();
        for (AppFlowDateParameter app : flowList) {
            AppFlowDateParameter appTmp = {};
            app.sPkgName = app.sPkgName;
            app.sExecPath = app.sExecPath;
            app.dDownloaded = app.dDownloaded;
            app.dUploaded = app.dUploaded;
            QPair<SignalQueueType, AppFlowDateParameter> data(SaveFlowCacheDataToLst, app);
            m_signalList.append(data);
        }
        Q_EMIT this->notifySaveFlowData();
        Q_EMIT reqSendSignal();
    });
    //向操作消息队列中添加 储存缓冲列表中的数据 的操作信号，并响应消息信号执行函数
    connect(this, &UseData::notifySaveFlowData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(SaveFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    //向操作消息队列中添加 获取所有应用某天/某月的流量数据 的操作信号，并响应消息信号执行函数
    connect(this, &UseData::notifyGetYesterdayFlowData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(GetYesterdayFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetTodayFlowData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(GetTodayFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetLastMonthFlowData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(GetLastMonthFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetThisMonthFlowData, this, [this] {
        AppFlowDateParameter app = {};
        QPair<SignalQueueType, AppFlowDateParameter> data(GetThisMonthFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });

    //向操作消息队列中添加 获取某个应用某天/某月的流量数据 的操作信号，并响应消息信号执行函数
    connect(this, &UseData::notifyGetAppYesterdayFlowData, this, [this](QString sPkgName) {
        AppFlowDateParameter app = {};
        app.sPkgName = sPkgName;
        QPair<SignalQueueType, AppFlowDateParameter> data(GetAppYesterdayFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetAppTodayFlowData, this, [this](QString sPkgName) {
        AppFlowDateParameter app = {};
        app.sPkgName = sPkgName;
        QPair<SignalQueueType, AppFlowDateParameter> data(GetAppTodayFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetAppLastMonthFlowData, this, [this](QString sPkgName) {
        AppFlowDateParameter app = {};
        app.sPkgName = sPkgName;
        QPair<SignalQueueType, AppFlowDateParameter> data(GetAppLastMonthFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
    connect(this, &UseData::notifyGetAppThisMonthFlowData, this, [this](QString sPkgName) {
        AppFlowDateParameter app = {};
        app.sPkgName = sPkgName;
        QPair<SignalQueueType, AppFlowDateParameter> data(GetAppThisMonthFlowData, app);
        m_signalList.append(data);
        Q_EMIT reqSendSignal();
    });
}

//初始化数据库
void UseData::initDataBase()
{
    m_sqlDb = QSqlDatabase::addDatabase("QSQLITE");
    QDir dir;
    QString dir_str = QString("%1/.deepin-defender/").arg(QDir::homePath());
    //目录不存在，就创建
    if (!dir.exists(dir_str)) {
        dir.mkpath(dir_str);
    }
    m_dbPath = dir_str + QString("flowdata.db");
    m_sqlDb.setDatabaseName(m_dbPath);
    // 打开数据库
    if (!m_sqlDb.open()) {
        qDebug() << "open database error !";
    };

    //读取数据
    m_sqlQuery = QSqlQuery(m_sqlDb);

    //创建流量统计存储表
    if (!m_sqlDb.tables().contains("flowDetail")) {
        bool suc = m_sqlQuery.exec("CREATE TABLE flowDetail (sPkgName text, sExecPath text, sDateymdh text,"
                                   " dTotalFlow real, dDownloadedFlow real, dUploadedFlow real);");
        if (!suc) {
            qDebug() << m_sqlQuery.lastError();
        }
    }
}

//删除流量数据存储缓冲
bool UseData::deleteFlowCacheData()
{
    m_appFlowCacheLst.clear();

    return true;
}
//向数据缓冲列表中添加数据
bool UseData::saveFlowCacheDataToLst(QString sPkgName, QString sExecPath, double dDownloaded, double dUploaded)
{
    AppFlowDateParameter app;
    app.sPkgName = sPkgName;
    app.sExecPath = sExecPath;
    app.sTimeyMdh = QDateTime::currentDateTime().toString("yyyy/MM/dd/hh");
    app.dTotal = dDownloaded + dUploaded;
    app.dDownloaded = dDownloaded;
    app.dUploaded = dUploaded;
    m_appFlowCacheLst.append(app);

    return true;
}

//储存缓冲列表中的数据
bool UseData::saveFlowData()
{
    bool ret = true;
    if (m_appFlowCacheLst.count() == 0)
        return ret;

    //数据库操作命令
    QString command;

    //判断是否到了新的月份
    QFileInfo dbInf(m_dbPath);
    if (dbInf.exists()) {
        QDateTime lastSavedTime = dbInf.lastModified();
        QDateTime currentTime = QDateTime::currentDateTime();
        if (currentTime.date().month() != lastSavedTime.date().month()) {
            //如果到了新的月份，就删除两个月之前的数据
            deleteOldData();
        }
    } else {
        initDataBase();
    }

    //储存数据
    for (const AppFlowDateParameter &app : m_appFlowCacheLst) {
        command = QString("INSERT INTO flowDetail VALUES ('%1','%2','%3',%4,%5,%6);").arg(app.sPkgName).arg(app.sExecPath).arg(app.sTimeyMdh).arg(app.dDownloaded + app.dUploaded).arg(app.dDownloaded).arg(app.dUploaded);

        if (!m_sqlQuery.exec(command)) {
            qDebug() << m_sqlQuery.lastError();
            ret = false;
            return ret;
        }
    }

    ret = true;
    return ret;
}

//获取所有应用某天的流量数据
bool UseData::getDayFlowData(QList<AppFlowDateParameter> &appLst, QDateTime time)
{
    QString command;

    //获取流量数据
    QDateTime startTime = time.addDays(-1); //从前一天23:59:00时开始读取
    startTime.setTime(QTime(23, 59, 0));
    QDateTime endTime = time; //到这天23:59:00时读取结束
    endTime.setTime(QTime(23, 59, 0));
    command = QString("SELECT * FROM flowDetail WHERE sDateymdh>'%1' AND"
                      " sDateymdh<'%2';")
                  .arg(startTime.toString("yyyy/MM/dd/hh"))
                  .arg(endTime.toString("yyyy/MM/dd/hh"));

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
        return false;
    }

    //把读取的数据存入列表
    while (m_sqlQuery.next()) {
        AppFlowDateParameter appTmp;
        appTmp.sPkgName = m_sqlQuery.value(0).toString();
        appTmp.sExecPath = m_sqlQuery.value(1).toString();
        appTmp.sTimeyMdh = m_sqlQuery.value(2).toString();
        appTmp.dTotal = m_sqlQuery.value(3).toDouble();
        appTmp.dDownloaded = m_sqlQuery.value(4).toDouble();
        appTmp.dUploaded = m_sqlQuery.value(5).toDouble();
        //找到相同项累加数据并替换
        int sameAppIndex = -1;
        for (int i = 0; i < appLst.size(); i++) {
            if (appTmp.sExecPath == appLst.at(i).sExecPath) {
                sameAppIndex = i;
                appTmp.dTotal += appLst.at(i).dTotal;
                appTmp.dDownloaded += appLst.at(i).dDownloaded;
                appTmp.dUploaded += appLst.at(i).dUploaded;
                appLst.replace(i, appTmp);
            }
        }
        if (-1 == sameAppIndex)
            appLst.append(appTmp);
    }
    return true;
}

bool UseData::getYesterdayFlowData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime yesterdayTime = currentTime.addDays(-1); //获取所有应用昨天24小时总流量数据
    QList<AppFlowDateParameter> appLst;
    bool ret = getDayFlowData(appLst, yesterdayTime);
    Q_EMIT sendYesterdayFlowData(appLst);
    return ret;
}

bool UseData::getTodayFlowData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<AppFlowDateParameter> appLst;
    bool ret = getDayFlowData(appLst, currentTime); //获取所有应用今天24小时总流量数据
    Q_EMIT sendTodayFlowData(appLst);
    return ret;
}

//获取所有应用某个月的数据
bool UseData::getMonthFlowData(QList<AppFlowDateParameter> &appLst, QDateTime time)
{
    //获取流量数据
    QDate startDate(time.date().year(), time.date().month(), 1); //此时间所处月份第一天
    QDateTime startDateTime;
    startDateTime.setDate(startDate);

    for (int i = 0; i < time.date().daysInMonth(); i++) {
        QDateTime indexTime = startDateTime.addDays(i);
        QList<AppFlowDateParameter> appDayFlowLst;
        bool ret = getDayFlowData(appDayFlowLst, indexTime);
        if (!ret)
            return false;
        for (int j = 0; j < appDayFlowLst.size(); j++) {
            AppFlowDateParameter appTmp = appDayFlowLst.at(j);
            int sameAppIndex = -1;
            for (int k = 0; k < appLst.size(); k++) {
                if (appTmp.sExecPath == appLst.at(k).sExecPath) {
                    sameAppIndex = k;
                    appTmp.dTotal += appLst.at(k).dTotal;
                    appTmp.dDownloaded += appLst.at(k).dDownloaded;
                    appTmp.dUploaded += appLst.at(k).dUploaded;
                    appLst.replace(k, appTmp);
                }
            }
            if (-1 == sameAppIndex)
                appLst.append(appTmp);
        }
    }

    return true;
}

bool UseData::getLastMonthFlowData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime lastMonthTime = currentTime.addMonths(-1); //获取所有应用上月总流量数据
    QList<AppFlowDateParameter> appLst;
    bool ret = getMonthFlowData(appLst, lastMonthTime);
    Q_EMIT sendLastMonthFlowData(appLst);
    return ret;
}

bool UseData::getThisMonthFlowData()
{
    QDateTime currentTime = QDateTime::currentDateTime(); //获取所有应用本月总流量数据
    QList<AppFlowDateParameter> appLst;
    bool ret = getMonthFlowData(appLst, currentTime);
    Q_EMIT sendThisMonthFlowData(appLst);
    return ret;
}

//获取某个应用某天的流量数据
QList<AppFlowDateParameter> UseData::getAppDayFlowData(QString sPkgName, QDateTime time)
{
    QList<AppFlowDateParameter> appLst;
    QString command;

    ////获取流量数据
    QDateTime startTime = time.addDays(-1); //从前一天23:59时开始读取
    startTime.setTime(QTime(23, 59, 0));
    QDateTime endTime = time; //到这天23:59时读取结束
    endTime.setTime(QTime(23, 59, 0));
    command = QString("SELECT * FROM flowDetail WHERE sDateymdh>'%1' AND"
                      " sDateymdh<'%2' AND sPkgName='%3';")
                  .arg(startTime.toString("yyyy/MM/dd/hh"))
                  .arg(endTime.toString("yyyy/MM/dd/hh"))
                  .arg(sPkgName);

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
    }

    //把读取的数据存入列表
    while (m_sqlQuery.next()) {
        AppFlowDateParameter appTmp;
        appTmp.sPkgName = m_sqlQuery.value(0).toString();
        appTmp.sExecPath = m_sqlQuery.value(1).toString();
        appTmp.sTimeyMdh = m_sqlQuery.value(2).toString();
        appTmp.dTotal = m_sqlQuery.value(3).toDouble();
        appTmp.dDownloaded = m_sqlQuery.value(4).toDouble();
        appTmp.dUploaded = m_sqlQuery.value(5).toDouble();
        int sameAppIndex = -1;
        for (int i = 0; i < appLst.size(); i++) {
            if (appTmp.sTimeyMdh == appLst.at(i).sTimeyMdh) {
                sameAppIndex = i;
                appTmp.dTotal += appLst.at(i).dTotal;
                appTmp.dDownloaded += appLst.at(i).dDownloaded;
                appTmp.dUploaded += appLst.at(i).dUploaded;
                appLst.replace(i, appTmp);
            }
        }
        if (-1 == sameAppIndex)
            appLst.append(appTmp);
    }

    //整理每小时显示的列表，同一天的数据进行累加
    QDateTime timeIndex = time;
    QList<AppFlowDateParameter> hourFlowLst;
    timeIndex.setTime(QTime(0, 0, 0));
    for (int i = 0; i < 24; i++) {
        timeIndex.setTime(QTime(i, 0, 0));
        QString timeIndexStr = timeIndex.toString("yyyy/MM/dd/hh");
        AppFlowDateParameter appTmp;
        int sameTimeIndex = -1;
        for (int j = 0; j < appLst.size(); j++) {
            if (timeIndexStr == appLst.at(j).sTimeyMdh) { //如果数据库中此时刻存在数据
                sameTimeIndex = j;
                appTmp.sPkgName = sPkgName;
                appTmp.sExecPath = appLst.at(sameTimeIndex).sExecPath;
                appTmp.sTimeyMdh = timeIndex.toString("hh:mm"); //24小时制
                appTmp.dTotal = appLst.at(sameTimeIndex).dTotal;
                appTmp.dDownloaded = appLst.at(sameTimeIndex).dDownloaded;
                appTmp.dUploaded = appLst.at(sameTimeIndex).dUploaded;
                appLst.removeAt(sameTimeIndex);
                break;
            }
        }

        if (-1 == sameTimeIndex) {
            appTmp.sPkgName = sPkgName;
            appTmp.sTimeyMdh = timeIndex.toString("hh:mm"); //24小时制
            appTmp.dTotal = 0;
            appTmp.dDownloaded = 0;
            appTmp.dUploaded = 0;
        }

        hourFlowLst.append(appTmp);
    }

    return hourFlowLst;
}

bool UseData::getAppYesterdayFlowData(QString sPkgName)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime yesterdayTime = currentTime.addDays(-1); //获取昨天24小时流量数据
    QList<AppFlowDateParameter> appLst = getAppDayFlowData(sPkgName, yesterdayTime);
    Q_EMIT sendAppYesterdayFlowData(appLst);
    return true;
}

bool UseData::getAppTodayFlowData(QString sPkgName)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<AppFlowDateParameter> appTodayFlowLst = getAppDayFlowData(sPkgName, currentTime); //获取今天24小时流量数据
    Q_EMIT sendAppTodayFlowData(appTodayFlowLst);
    return true;
}

//获取某个应用某月的流量数据
QList<AppFlowDateParameter> UseData::getAppMonthFlowData(QString sPkgName, QDateTime time)
{
    //获取此时间所处月份的流量数据
    QList<AppFlowDateParameter> appMonthFlowLst;
    QDate startDate(time.date().year(), time.date().month(), 1); //此时间所处月份第一天
    QDateTime dateTime;
    dateTime.setDate(startDate);

    int days = time.date().daysInMonth(); //获取此时间所处月份的天数
    for (int i = 0; i < days; i++) {
        AppFlowDateParameter appDayFlow;
        appDayFlow.sPkgName = sPkgName;
        appDayFlow.sExecPath = sPkgName;
        appDayFlow.sTimeyMdh = dateTime.toString("MM-dd");
        appDayFlow.dDownloaded = 0;
        appDayFlow.dUploaded = 0;
        appDayFlow.dTotal = 0;
        QList<AppFlowDateParameter> appDayFlowLst = getAppDayFlowData(sPkgName, dateTime);
        //计算一天的流量
        for (AppFlowDateParameter app : appDayFlowLst) {
            appDayFlow.sPkgName = app.sPkgName;
            appDayFlow.dDownloaded += app.dDownloaded;
            appDayFlow.dUploaded += app.dUploaded;
        }
        appDayFlow.dTotal = appDayFlow.dDownloaded + appDayFlow.dUploaded;
        appMonthFlowLst.append(appDayFlow);

        dateTime = dateTime.addDays(1); //切换到下一天
    }

    return appMonthFlowLst;
}

bool UseData::getAppLastMonthFlowData(QString sPkgName)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime lastMonthTime = currentTime.addMonths(-1); //获取上月流量数据
    QList<AppFlowDateParameter> appLastMonthFlowLst = getAppMonthFlowData(sPkgName, lastMonthTime);
    Q_EMIT sendAppLastMonthFlowData(appLastMonthFlowLst);
    return true;
}

bool UseData::getAppThisMonthFlowData(QString sPkgName)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<AppFlowDateParameter> appThisMonthFlowLst = getAppMonthFlowData(sPkgName, currentTime); //获取本月流量数据
    Q_EMIT sendAppThisMonthFlowData(appThisMonthFlowLst);
    return true;
}

//向操作信号序列中添加相应操作信号
void UseData::apendDeleteFlowCacheDataSignal()
{
    AppFlowDateParameter app = {};
    QPair<SignalQueueType, AppFlowDateParameter> data(DeleteFlowCacheData, app);
    m_signalList.append(data);
    Q_EMIT reqSendSignal();
}
void UseData::apendSaveFlowCacheDataToLstSignal(QString sPkgName, QString sExecPath, double dDownloaded, double dUploaded)
{
    AppFlowDateParameter app = {};
    app.sPkgName = sPkgName;
    app.sExecPath = sExecPath;
    app.dDownloaded = dDownloaded;
    app.dUploaded = dUploaded;
    QPair<SignalQueueType, AppFlowDateParameter> data(SaveFlowCacheDataToLst, app);
    m_signalList.append(data);
    Q_EMIT reqSendSignal();
}

void UseData::apendSaveFlowDataSignal()
{
    AppFlowDateParameter app = {};
    QPair<SignalQueueType, AppFlowDateParameter> data(SaveFlowData, app);
    m_signalList.append(data);
    Q_EMIT reqSendSignal();
}

//删除旧的数据
void UseData::deleteOldData()
{
    //找到需要删除的时间节点deleteDateTime
    QDateTime deleteDateTime = QDateTime::currentDateTime().addMonths(-2);

    QString command = QString("DELETE FROM flowDetail WHERE sDateymdh<'%1';")
                          .arg(deleteDateTime.toString("yyyy/MM/dd/hh"));

    if (!m_sqlQuery.exec(command)) {
        qDebug() << m_sqlQuery.lastError();
    }
}

//消息队列响应
void UseData::onReqSendSignal()
{
    if (m_signalList.count() == 0)
        return;

    QPair<SignalQueueType, AppFlowDateParameter> data = m_signalList.head();

    bool ret = false;
    switch (data.first) {
    case DeleteFlowCacheData:
        ret = deleteFlowCacheData();
        break;
    case SaveFlowCacheDataToLst:
        ret = saveFlowCacheDataToLst(data.second.sPkgName, data.second.sExecPath, data.second.dDownloaded, data.second.dUploaded);
        break;
    case SaveFlowData:
        ret = saveFlowData();
        break;
    case GetYesterdayFlowData:
        ret = getYesterdayFlowData();
        break;
    case GetTodayFlowData:
        ret = getTodayFlowData();
        break;
    case GetLastMonthFlowData:
        ret = getLastMonthFlowData();
        break;
    case GetThisMonthFlowData:
        ret = getThisMonthFlowData();
        break;
    case GetAppYesterdayFlowData:
        ret = getAppYesterdayFlowData(data.second.sPkgName);
        break;
    case GetAppTodayFlowData:
        ret = getAppTodayFlowData(data.second.sPkgName);
        break;
    case GetAppLastMonthFlowData:
        ret = getAppLastMonthFlowData(data.second.sPkgName);
        break;
    case GetAppThisMonthFlowData:
        ret = getAppThisMonthFlowData(data.second.sPkgName);
        break;
    default:
        break;
    }

    //操作数据库成功以后，移除消息队列的头
    if (ret) {
        if (m_signalList.count() > 0) {
            AppFlowDateParameter popData = m_signalList.head().second;
            m_signalList.pop_front();
        }
    }

    //然后当消息队列还存在数据时，需要继续发送信号调用onReqSendSignal
    if (m_signalList.count() > 0) {
        Q_EMIT reqSendSignal();
    }
}
