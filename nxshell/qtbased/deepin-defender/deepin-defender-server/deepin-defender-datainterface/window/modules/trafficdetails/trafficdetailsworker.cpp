// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trafficdetailsworker.h"

#include <QThread>

TrafficDetailsWorker::TrafficDetailsWorker(QObject *parent)
    : QObject(parent)
    , m_monitornetflow(new MonitorNetFlow("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_useData(new UseData)
    , m_useDataThread(new QThread)
{
    connect(GetProcInfoInter::instance(), &GetProcInfoInter::sendProcInfos, this, &TrafficDetailsWorker::onRecProcInfos);

    m_monitornetflow->StartApp();

    //初始化默认显示的时间跨度
    m_flowRangeStyle = TimeRangeType::None;

    //save flow date
    m_useData->moveToThread(m_useDataThread);
    m_useDataThread->start();

    connect(m_useData, &UseData::sendTodayFlowData, this, [=](QList<AppFlowDateParameter> list) {
        onReceiveAllAppFlowDate(list, TimeRangeType::Today);
    });
    connect(m_useData, &UseData::sendYesterdayFlowData, this, [=](QList<AppFlowDateParameter> list) {
        onReceiveAllAppFlowDate(list, TimeRangeType::Yesterday);
    });
    connect(m_useData, &UseData::sendThisMonthFlowData, this, [=](QList<AppFlowDateParameter> list) {
        onReceiveAllAppFlowDate(list, TimeRangeType::ThisMonth);
    });
    connect(m_useData, &UseData::sendLastMonthFlowData, this, [=](QList<AppFlowDateParameter> list) {
        onReceiveAllAppFlowDate(list, TimeRangeType::LastMonth);
    });

    connect(m_useData, &UseData::sendAppTodayFlowData, this, &TrafficDetailsWorker::onReceiveAppFlowDate);
    connect(m_useData, &UseData::sendAppYesterdayFlowData, this, &TrafficDetailsWorker::onReceiveAppFlowDate);
    connect(m_useData, &UseData::sendAppThisMonthFlowData, this, &TrafficDetailsWorker::onReceiveAppFlowDate);
    connect(m_useData, &UseData::sendAppLastMonthFlowData, this, &TrafficDetailsWorker::onReceiveAppFlowDate);
}

TrafficDetailsWorker::~TrafficDetailsWorker()
{
    m_useDataThread->quit();
    m_useDataThread->wait();
    m_useDataThread->deleteLater();
    m_useData->deleteLater();
}

QList<DefenderProcInfo> TrafficDetailsWorker::getFlowInfoList()
{
    QList<DefenderProcInfo> retProcInfos;
    // 合并无包名的进程信息为 "其它应用"
    DefenderProcInfo otherAppInfo;
    otherAppInfo.nPid = 0;
    otherAppInfo.sExecPath = "";
    otherAppInfo.isbSysApp = true;
    otherAppInfo.sProcName = "other apps";
    otherAppInfo.sDesktopPath = "";
    otherAppInfo.sPkgName = "";
    otherAppInfo.sThemeIcon = "";
    otherAppInfo.dDownloadSpeed = 0.0;
    otherAppInfo.dUploadSpeed = 0.0;
    otherAppInfo.dDownloads = 0.0;
    otherAppInfo.dUploads = 0.0;
    // 网速信息以上一段时间内统计的为准
    for (DefenderProcInfo procInfo : m_procInfos) {
        if (procInfo.sPkgName.isEmpty()) {
            otherAppInfo.dDownloadSpeed += procInfo.dDownloadSpeed;
            otherAppInfo.dUploadSpeed += procInfo.dUploadSpeed;
            otherAppInfo.dDownloads += procInfo.dDownloads;
            otherAppInfo.dUploads += procInfo.dUploads;
        } else {
            // 合并相同包名项
            bool isbSamePkgInfo = false;
            for (int i = 0; i < retProcInfos.size(); ++i) {
                DefenderProcInfo sameProcInfo = retProcInfos.at(i);
                if (procInfo.sPkgName == sameProcInfo.sPkgName) {
                    sameProcInfo.dDownloadSpeed += procInfo.dDownloadSpeed;
                    sameProcInfo.dUploadSpeed += procInfo.dUploadSpeed;
                    sameProcInfo.dDownloads += procInfo.dDownloads;
                    sameProcInfo.dUploads += procInfo.dUploads;

                    isbSamePkgInfo = true;
                    retProcInfos.replace(i, sameProcInfo);
                    break;
                }
            }
            if (!isbSamePkgInfo) {
                retProcInfos.append(procInfo);
            }
        }
    }

    retProcInfos.append(otherAppInfo);
    return retProcInfos;
}

void TrafficDetailsWorker::onRecProcInfos(const DefenderProcInfoList &list)
{
    m_procInfos = list;
    saveFlowData();
}

void TrafficDetailsWorker::notifyGetYesterdayFlowData()
{
    m_flowRangeStyle = TimeRangeType::Yesterday;
    Q_EMIT m_useData->notifyGetYesterdayFlowData();
}

void TrafficDetailsWorker::notifyGetTodayFlowData()
{
    m_flowRangeStyle = TimeRangeType::Today;
    Q_EMIT m_useData->notifyGetTodayFlowData();
}

void TrafficDetailsWorker::notifyGetLastMonthFlowData()
{
    m_flowRangeStyle = TimeRangeType::LastMonth;
    Q_EMIT m_useData->notifyGetLastMonthFlowData();
}

void TrafficDetailsWorker::notifyGetThisMonthFlowData()
{
    m_flowRangeStyle = TimeRangeType::ThisMonth;
    Q_EMIT m_useData->notifyGetThisMonthFlowData();
}

// 通知停止获取所有应用流量数据
void TrafficDetailsWorker::NotifyDontGetFlowData()
{
    m_flowRangeStyle = TimeRangeType::None;
}

void TrafficDetailsWorker::notifyGetAppYesterdayFlowData(QString sPkgName)
{
    Q_EMIT m_useData->notifyGetAppYesterdayFlowData(sPkgName);
}

void TrafficDetailsWorker::notifyGetAppTodayFlowData(QString sPkgName)
{
    Q_EMIT m_useData->notifyGetAppTodayFlowData(sPkgName);
}

void TrafficDetailsWorker::notifyGetAppLastMonthFlowData(QString sPkgName)
{
    Q_EMIT m_useData->notifyGetAppLastMonthFlowData(sPkgName);
}

void TrafficDetailsWorker::notifyGetAppThisMonthFlowData(QString sPkgName)
{
    Q_EMIT m_useData->notifyGetAppThisMonthFlowData(sPkgName);
}

void TrafficDetailsWorker::saveFlowData()
{
    QList<AppFlowDateParameter> appFlowCacheList = {};
    for (DefenderProcInfo procInfo : m_procInfos) {
        AppFlowDateParameter appFlowCache = {};
        // 当前进程有流量数据时，向数据库中储存，单位为byte
        if (0 < procInfo.dDownloads || 0 < procInfo.dUploads) {
            appFlowCache.sPkgName = procInfo.sPkgName;
            appFlowCache.sExecPath = procInfo.sExecPath;

            // 优先使用desktop路径代替执行路径
            if (!procInfo.sDesktopPath.isEmpty()) {
                appFlowCache.sExecPath = procInfo.sDesktopPath;
            }

            // 合并相同进程流量数据
            bool isbSameProc = false;
            for (int i = 0; i < appFlowCacheList.size(); ++i) {
                AppFlowDateParameter sameProcFlow = appFlowCacheList.at(i);
                // 若存在执行路径相同的项,则进行流量叠加并替换
                if (sameProcFlow.sExecPath == appFlowCache.sExecPath) {
                    sameProcFlow.dDownloaded += appFlowCache.dUploaded;
                    sameProcFlow.dUploaded += appFlowCache.dUploaded;
                    // 替换
                    appFlowCacheList.replace(i, sameProcFlow);
                    isbSameProc = true;
                    break;
                }
            }
            // 若应用进程缓存列表中没有相同应用,则新添加
            if (!isbSameProc) {
                appFlowCache.dDownloaded = procInfo.dDownloads;
                appFlowCache.dUploaded = procInfo.dUploads;
                appFlowCacheList.append(appFlowCache);
            }
        }
    }
    Q_EMIT m_useData->notifySaveFlowCacheDataToDB(appFlowCacheList);

    //根据此时刻更新详情列表显示的数据
    switch (m_flowRangeStyle) {
    case TimeRangeType::Today:
        notifyGetTodayFlowData();
        break;
    case TimeRangeType::Yesterday:
        notifyGetYesterdayFlowData();
        break;
    case TimeRangeType::ThisMonth:
        notifyGetThisMonthFlowData();
        break;
    case TimeRangeType::LastMonth:
        notifyGetLastMonthFlowData();
        break;
    default:
        break;
    }

    m_lastSavedTime = QDateTime::currentDateTime();
}

void TrafficDetailsWorker::onReceiveAllAppFlowDate(QList<AppFlowDateParameter> appLst, TimeRangeType range)
{
    m_onShowFlowList.clear();
    for (AppFlowDateParameter app : appLst) {
        DefenderProcInfo procInfoTmp = {};
        procInfoTmp.sExecPath = app.sExecPath;

        // 从进程信息解析接口中得到 应用名称、图标、包名、是否是系统应用
        QString sPkgName = GetProcInfoInter::instance()->getPkgNameByExecPath(procInfoTmp.sExecPath);
        DefenderProcInfo installedAppInfo = GetProcInfoInter::instance()->getProcInfoByPkgName(sPkgName);

        procInfoTmp.sProcName = procInfoTmp.sExecPath.split("/").last();
        procInfoTmp.isbSysApp = installedAppInfo.isbSysApp;
        procInfoTmp.sDesktopPath = installedAppInfo.sDesktopPath;
        procInfoTmp.sPkgName = installedAppInfo.sPkgName;
        // 如果包名为空，则将进程名和执行路径设为 "other apps"
        if (procInfoTmp.sPkgName.isEmpty()) {
            procInfoTmp.sProcName = "other apps";
            procInfoTmp.sExecPath = "other apps";
            procInfoTmp.isbSysApp = true;
        }

        procInfoTmp.sAppName = installedAppInfo.sAppName;
        procInfoTmp.sThemeIcon = installedAppInfo.sThemeIcon;
        // 更新下载量
        procInfoTmp.dDownloads = app.dDownloaded;
        // 更新上传量
        procInfoTmp.dUploads = app.dUploaded;

        // 在显示列表中找到相同应用项
        int iSameIndex = -1;
        for (int i = 0; i < m_onShowFlowList.size(); ++i) {
            DefenderProcInfo sameInfoTmp = m_onShowFlowList.at(i);
            if (procInfoTmp.sPkgName == sameInfoTmp.sPkgName) {
                iSameIndex = i;
                break;
            }
        }
        // 合并相同应用的数据
        if (-1 != iSameIndex) {
            DefenderProcInfo sameInfo = m_onShowFlowList.at(iSameIndex);
            sameInfo.dDownloads += procInfoTmp.dDownloads;
            sameInfo.dUploads += procInfoTmp.dUploads;
            m_onShowFlowList.replace(iSameIndex, sameInfo);
        } else {
            m_onShowFlowList.append(procInfoTmp);
        }
    }
    Q_EMIT sendAllAppFlowData(m_onShowFlowList, range);
}

void TrafficDetailsWorker::onReceiveAppFlowDate(QList<AppFlowDateParameter> appLst)
{
    DefenderProcInfoList onShowAppFlowList;
    for (AppFlowDateParameter app : appLst) {
        DefenderProcInfo procInfoTmp;
        procInfoTmp.sExecPath = app.sExecPath;

        // 初始化 应用名称、图标、包名、是否是系统应用
        procInfoTmp.sProcName = "";
        procInfoTmp.isbSysApp = true;
        procInfoTmp.sDesktopPath = "";
        procInfoTmp.sPkgName = app.sPkgName;
        procInfoTmp.sAppName = "";
        procInfoTmp.sThemeIcon = "";
        // 更新日期时间
        procInfoTmp.sTimeyMdh = app.sTimeyMdh;
        // 更新下载量
        procInfoTmp.dDownloads = app.dDownloaded;
        // 更新上传量
        procInfoTmp.dUploads = app.dUploaded;

        onShowAppFlowList.append(procInfoTmp);
    }
    Q_EMIT sendAppFlowData(onShowAppFlowList);
}
