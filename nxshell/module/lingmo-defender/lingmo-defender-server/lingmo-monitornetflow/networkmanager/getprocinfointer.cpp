// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getprocinfointer.h"
#include "getprocinfojob.h"
#include "common.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>

GetProcInfoInter::GetProcInfoInter()
    : m_getProcInfoJob(nullptr)
    , m_getProcInfoJobThread(nullptr)
{
    m_getProcInfoJob = new GetProcInfoJob;
    m_getProcInfoJobThread = new QThread;
    m_getProcInfoJob->moveToThread(m_getProcInfoJobThread);
    m_getProcInfoJobThread->start();

    connect(this, &GetProcInfoInter::installedAppListChanged, m_getProcInfoJob, &GetProcInfoJob::updateLauncherItemInfoList);
    connect(this, &GetProcInfoInter::sendPocNetFlowInfos, m_getProcInfoJob, &GetProcInfoJob::updateProInfoList);
    connect(m_getProcInfoJob, &GetProcInfoJob::sendInstalledAppInfoList, this, &GetProcInfoInter::recInstalledAppInfos);
    connect(m_getProcInfoJob, &GetProcInfoJob::sendProcInfos, this, &GetProcInfoInter::recProcInfos);

    // 初始化GetProcInfoJob的应用列表
    Q_EMIT installedAppListChanged();
}

GetProcInfoInter::~GetProcInfoInter()
{
    m_getProcInfoJobThread->quit();
    m_getProcInfoJobThread->wait();
    m_getProcInfoJobThread->deleteLater();
    m_getProcInfoJobThread = nullptr;
    m_getProcInfoJob->deleteLater();
    m_getProcInfoJob = nullptr;
}

QString GetProcInfoInter::getPkgNameByExecPath(const QString &execPath)
{
    return Utils::getPkgNameByFilePathFromDb(execPath, "GetProcInfoInter");
}

// 通过包名，从已安装列表中获取进程信息
DefenderProcInfo GetProcInfoInter::getProcInfoByPkgName(const QString &pkgName)
{
    DefenderProcInfo retProcInfo;

    if (pkgName.isEmpty()) {
        return retProcInfo;
    }

    // 匹配包名
    for (const DefenderProcInfo &info : m_installedAppInfoList) {
        if (info.sPkgName.contains(pkgName)) {
            retProcInfo = info;
            break;
        }
    }

    return retProcInfo;
}

void GetProcInfoInter::recInstalledAppInfos(const DefenderProcInfoList &list)
{
    m_installedAppInfoList = list;
}

void GetProcInfoInter::recProcInfos(const DefenderProcInfoList &list)
{
    m_procInfoList = list;
    Q_EMIT sendProcInfos(list);
}
