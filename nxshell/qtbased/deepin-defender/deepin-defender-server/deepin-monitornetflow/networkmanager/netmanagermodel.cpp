// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netmanagermodel.h"
#include "getprocinfointer.h"
#include "localcache/settingssql.h"
#include "../../deepin-defender/src/window/modules/common/gsettingkey.h"

#include <QDebug>

NetManagerModel::NetManagerModel(QObject *parent)
    : QObject(parent)
    , m_settingsSql(nullptr)
    , m_getProcFlow(nullptr)
    , m_getProcFlowThread(nullptr)
    , m_netFlowDataSqlThread(nullptr)
    , m_netFlowDataSql(nullptr)
    , m_config(nullptr)
    , m_getProcInfoInter(nullptr)
{
    // 注册DBus传参类型
    registerDefenderProcInfoMetaType();
    registerDefenderProcInfoListMetaType();

    qRegisterMetaType<QList<NetFlowData>>("QList<NetFlowData>");

    // 本地配置
    m_settingsSql = new SettingsSql(this->metaObject()->className(), this);

    // 流量监控服务
    m_getProcFlow = new GetProcNetFlow;
    m_getProcFlowThread = new QThread;
    m_getProcFlow->moveToThread(m_getProcFlowThread);
    connect(m_getProcFlowThread, &QThread::started, m_getProcFlow, &GetProcNetFlow::init, Qt::QueuedConnection);
    m_getProcFlowThread->start();

    // 流量统计数据查询线程
    m_netFlowDataSqlThread = new QThread;
    m_netFlowDataSql = new NetFlowDataSql;
    m_netFlowDataSql->moveToThread(m_netFlowDataSqlThread);
    // connect
    connect(m_netFlowDataSqlThread, &QThread::started, m_netFlowDataSql, &NetFlowDataSql::init);
    m_netFlowDataSqlThread->start();

    // get proc info list
    m_getProcInfoInter = new GetProcInfoInter;
    m_getProcInfoInter->setParent(this);

    // 读取是否开启流量监控
    bool enable = m_settingsSql->getValue(DATA_USAGE_ON_OFF).toBool();
    enableNetFlowMonitor(enable);

    // connection
    connect(m_getProcFlow, &GetProcNetFlow::sendPocNetFlowInfos, m_getProcInfoInter, &GetProcInfoInter::sendPocNetFlowInfos);

    connect(this, &NetManagerModel::requestSaveNetFlowFrameToCache, m_netFlowDataSql, &NetFlowDataSql::saveNetFlowFrameToCache);
    connect(this, &NetManagerModel::installedAppListChanged, m_getProcInfoInter, &GetProcInfoInter::installedAppListChanged);

    connect(m_getProcInfoInter, &GetProcInfoInter::sendProcInfos, this, &NetManagerModel::onRecProcInfos);
    connect(m_getProcInfoInter, &GetProcInfoInter::sendProcInfos, this, &NetManagerModel::sendProcInfoList);

    // 获取流量统计
    connect(this, &NetManagerModel::requestQueryAppFlowList, m_netFlowDataSql, &NetFlowDataSql::queryAppFlowList);
    connect(m_netFlowDataSql, &NetFlowDataSql::sendAppFlowList, this, &NetManagerModel::onRecAppFlowListFromSql);
    connect(this, &NetManagerModel::requestQueryAllAppFlowList, m_netFlowDataSql, &NetFlowDataSql::queryAllAppFlowList);
    connect(m_netFlowDataSql, &NetFlowDataSql::sendAllAppFlowList, this, &NetManagerModel::onRecAllAppFlowListFromSql);
}

NetManagerModel::~NetManagerModel()
{
    // 停止并释放流量监控线程
    QMetaObject::invokeMethod(m_getProcFlow, "enableNetFlowMonitor", Qt::QueuedConnection, Q_ARG(bool, false));
    m_getProcFlowThread->quit();
    m_getProcFlowThread->wait();
    m_getProcFlowThread->deleteLater();
    m_getProcFlow->deleteLater();

    // 停止并释放流量数据库查询线程
    m_netFlowDataSqlThread->quit();
    m_netFlowDataSqlThread->wait();
    m_netFlowDataSqlThread->deleteLater();
    m_netFlowDataSql->deleteLater();
}

void NetManagerModel::enableNetFlowMonitor(bool enable)
{
    m_settingsSql->setValue(DATA_USAGE_ON_OFF, enable);
    QMetaObject::invokeMethod(m_getProcFlow, "enableNetFlowMonitor", Qt::QueuedConnection, Q_ARG(bool, enable));
}

void NetManagerModel::onRecProcInfos(const DefenderProcInfoList &list)
{
    m_procInfoList = list;
    saveFlowData();
}

void NetManagerModel::onRecAppFlowListFromSql(const QString &sPkgName, const int type, const QList<NetFlowData> &flowList)
{
    DefenderProcInfoList onShowAppFlowList;
    for (const NetFlowData &app : flowList) {
        DefenderProcInfo procInfoTmp;
        // 从进程信息解析接口中得到 应用名称、图标、是否是系统应用
        procInfoTmp = m_getProcInfoInter->getProcInfoByPkgName(app.sPkgName);

        // 更新日期时间
        procInfoTmp.sTimeyMdh = app.sTimeyMdh;
        // 更新下载量
        procInfoTmp.dDownloads = app.dDownloaded;
        // 更新上传量
        procInfoTmp.dUploads = app.dUploaded;

        onShowAppFlowList.append(procInfoTmp);
    }
    Q_EMIT sendAppFlowList(sPkgName, type, onShowAppFlowList);
}

void NetManagerModel::onRecAllAppFlowListFromSql(const int type, const QList<NetFlowData> &flowList)
{
    DefenderProcInfoList onShowFlowList;
    for (const NetFlowData &app : flowList) {
        DefenderProcInfo procInfoTmp = {};
        procInfoTmp.sPkgName = app.sPkgName;
        // 如果包名为空，则将进程名和执行路径设为 "other apps"
        if (procInfoTmp.sPkgName.isEmpty()) {
            procInfoTmp = {};
            procInfoTmp.isbSysApp = true;
        } else {
            // 从进程信息解析接口中得到 应用名称、图标、是否是系统应用
            procInfoTmp = m_getProcInfoInter->getProcInfoByPkgName(procInfoTmp.sPkgName);
        }

        // 更新下载量
        procInfoTmp.dDownloads = app.dDownloaded;
        // 更新上传量
        procInfoTmp.dUploads = app.dUploaded;

        // 在显示列表中找到相同应用项
        int iSameIndex = -1;
        for (int i = 0; i < onShowFlowList.size(); ++i) {
            DefenderProcInfo sameInfoTmp = onShowFlowList.at(i);
            if (procInfoTmp.sPkgName == sameInfoTmp.sPkgName) {
                iSameIndex = i;
                break;
            }
        }
        // 合并相同应用的数据
        if (-1 != iSameIndex) {
            DefenderProcInfo sameInfo = onShowFlowList.at(iSameIndex);
            sameInfo.dDownloads += procInfoTmp.dDownloads;
            sameInfo.dUploads += procInfoTmp.dUploads;
            onShowFlowList.replace(iSameIndex, sameInfo);
        } else {
            onShowFlowList.append(procInfoTmp);
        }
    }
    Q_EMIT sendAllAppFlowList(type, onShowFlowList);
}

void NetManagerModel::saveFlowData()
{
    DefenderProcInfoList appFlowCacheList = {};
    for (DefenderProcInfo procInfo : m_procInfoList) {
        DefenderProcInfo appFlowCache = {};
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
                DefenderProcInfo sameProcFlow = appFlowCacheList.at(i);
                // 若存在执行路径相同的项,则进行流量叠加并替换
                if (sameProcFlow.sExecPath == appFlowCache.sExecPath) {
                    sameProcFlow.dDownloads += appFlowCache.dUploads;
                    sameProcFlow.dUploads += appFlowCache.dUploads;
                    // 替换
                    appFlowCacheList.replace(i, sameProcFlow);
                    isbSameProc = true;
                    break;
                }
            }
            // 若应用进程缓存列表中没有相同应用,则新添加
            if (!isbSameProc) {
                appFlowCache.dDownloads = procInfo.dDownloads;
                appFlowCache.dUploads = procInfo.dUploads;
                appFlowCacheList.append(appFlowCache);
            }
        }
    }
    // 存储流量数据
    for (DefenderProcInfo info : appFlowCacheList) {
        Q_EMIT requestSaveNetFlowFrameToCache(info.sPkgName, info.sExecPath, info.dDownloads, info.dUploads);
    }
}
