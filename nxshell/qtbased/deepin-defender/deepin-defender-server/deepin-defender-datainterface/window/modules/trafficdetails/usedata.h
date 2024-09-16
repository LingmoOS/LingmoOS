// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QWidget>
#include <QDateTime>
#include <QDebug>
#include <QQueue>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

#define UseDataDebugFlag 0

//消息队列
enum SignalQueueType {
    DeleteFlowCacheData = 0,
    SaveFlowCacheDataToLst,
    SaveFlowData,
    GetYesterdayFlowData,
    GetTodayFlowData,
    GetLastMonthFlowData,
    GetThisMonthFlowData,
    GetAppYesterdayFlowData,
    GetAppTodayFlowData,
    GetAppLastMonthFlowData,
    GetAppThisMonthFlowData,
    SignalCount
};

// 应用流量参数
struct AppFlowDateParameter {
    QString sPkgName;
    QString sExecPath;
    QString sTimeyMdh;
    double dDownloaded = 0;
    double dUploaded = 0;
    double dTotal = 0;//单个APP总流量
};

Q_DECLARE_METATYPE(AppFlowDateParameter);

class UseData : public QObject
{
    Q_OBJECT

public:
    explicit UseData(QObject *parent = nullptr);
    //清空数据缓冲列表
    bool deleteFlowCacheData();
    //向数据缓冲列表中添加数据
    bool saveFlowCacheDataToLst(QString sPkgName, QString sExecPath, double dDownloaded, double dUploaded);
    //储存缓冲列表中的数据
    bool saveFlowData();
    //获取所有应用某天的流量数据
    bool getDayFlowData(QList<AppFlowDateParameter> &appLst, QDateTime time);
    bool getYesterdayFlowData();
    bool getTodayFlowData();
    //获取所有应用某个月的数据
    bool getMonthFlowData(QList<AppFlowDateParameter> &appLst, QDateTime time);
    bool getLastMonthFlowData();
    bool getThisMonthFlowData();
    //获取某个应用某天的流量数据
    QList<AppFlowDateParameter> getAppDayFlowData(QString sPkgName, QDateTime time);
    bool getAppYesterdayFlowData(QString sPkgName);
    bool getAppTodayFlowData(QString sPkgName);
    //获取某个应用某月的流量数据
    QList<AppFlowDateParameter> getAppMonthFlowData(QString sPkgName, QDateTime time);
    bool getAppLastMonthFlowData(QString sPkgName);
    bool getAppThisMonthFlowData(QString sPkgName);
    //向操作信号序列中添加相应操作信号
    void apendDeleteFlowCacheDataSignal();
    void apendSaveFlowCacheDataToLstSignal(QString sPkgName, QString sExecPath, double dDownloaded, double dUploaded);
    void apendSaveFlowDataSignal();
    //删除旧的数据
    void deleteOldData();


Q_SIGNALS:
    void notifyDeleteFlowCacheData();
    // 通知 保存缓存数据到数据库
    void notifySaveFlowCacheDataToDB(QList<AppFlowDateParameter> flowList);
    void notifySaveFlowData();
    //获取所有app流量数据
    void notifyGetYesterdayFlowData();
    void sendYesterdayFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetTodayFlowData();
    void sendTodayFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetLastMonthFlowData();
    void sendLastMonthFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetThisMonthFlowData();
    void sendThisMonthFlowData(QList<AppFlowDateParameter> appLst);
    //获取某个app流量数据
    void notifyGetAppYesterdayFlowData(QString appname);
    void sendAppYesterdayFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetAppTodayFlowData(QString appname);
    void sendAppTodayFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetAppLastMonthFlowData(QString appname);
    void sendAppLastMonthFlowData(QList<AppFlowDateParameter> appLst);
    void notifyGetAppThisMonthFlowData(QString appname);
    void sendAppThisMonthFlowData(QList<AppFlowDateParameter> appLst);
    //请求相应消息信号
    void reqSendSignal();

public Q_SLOTS:
    //响应消息信号，操作信号处理中心
    void onReqSendSignal();

private:
    void initDataBase();

private:
    //数据库对象
    QSqlDatabase m_sqlDb;
    //数据库文件路径
    QString m_dbPath;
    //查询数据库工具对象
    QSqlQuery m_sqlQuery;
    //待储存的数据列表
    QList<AppFlowDateParameter> m_appFlowCacheLst;
    //操作信号队列
    QQueue<QPair<SignalQueueType, AppFlowDateParameter>> m_signalList;
};
