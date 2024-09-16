// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytoolsmodel.h"

#include "defsecuritytoolinfo.h"
#include "window/modules/common/common.h"
#include "window/modules/common/invokers/invokerfactory.h"

#include <QDebug>
#include <QProcess>
#include <QVersionNumber>

#define IMA_NAME "com.deepin.daemon.ImaDaemon"
#define IMA_PATH "/com/deepin/daemon/ImaDaemon"
#define IMA_INTERFACE "com.deepin.daemon.ImaDaemon"

SecurityToolsModel::SecurityToolsModel(QObject *parent)
    : QObject(parent)
    , m_pSecurityToolDBusInter(nullptr)
    , m_gsetting(nullptr)
{
    // 初始化数据
    initData();
    initConnectoin();
}

SecurityToolsModel::~SecurityToolsModel() { }

void SecurityToolsModel::initData()
{
    registerDefSecurityToolInfo();
    registerDefSecurityToolInfoList();

    m_pSecurityToolDBusInter =
        InvokerFactory::GetInstance().CreateInvoker("com.deepin.pc.manager.securitytooldialog",
                                                    "/com/deepin/pc/manager/securitytooldialog",
                                                    "com.deepin.pc.manager.securitytooldialog",
                                                    ConnectType::SESSION,
                                                    this);
}

void SecurityToolsModel::initConnectoin()
{
    m_pSecurityToolDBusInter->Connect("notifyToolsInfoUpdate",
                                      this,
                                      SLOT(onToolsInfoUpdate(const DEFSECURITYTOOLINFOLIST &)));
    m_pSecurityToolDBusInter->Connect("notifyAppStatusChanged",
                                      this,
                                      SLOT(onAppStatusChanged(const QString &, int)));
}

void SecurityToolsModel::onToolsInfoUpdate(const DEFSECURITYTOOLINFOLIST &infos)
{
    m_toolinfos.clear();
    m_toolinfos = infos;
    Q_EMIT sUpdateToolsInfo();
}

void SecurityToolsModel::onAppStatusChanged(const QString &strPackageKey, int status)
{
    DEFSECURITYTOOLSTATUS curStatus = DEFSECURITYTOOLSTATUS(status);

    if (curStatus == DEFSECURITYTOOLSTATUS::INSTALLSUCCESS
        || curStatus == DEFSECURITYTOOLSTATUS::INSTALLFAILED) {
        bool bInstalled = false;

        if (curStatus == DEFSECURITYTOOLSTATUS::INSTALLSUCCESS) {
            bInstalled = true;
        } else if (curStatus == DEFSECURITYTOOLSTATUS::INSTALLFAILED) {
            bInstalled = false;
        }

        for (auto iter = m_toolinfos.begin(); iter != m_toolinfos.end(); iter++) {
            if (iter->getPackageKey() == strPackageKey) {
                iter->bHasInstalled = bInstalled;
            }
        }
    }

    Q_EMIT notityAppStatusChanged(strPackageKey, status);
}

void SecurityToolsModel::onStatusCheckFinished() { }

void SecurityToolsModel::updateToolsInfo()
{
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "OnRequestUpdateToolsInfo");
}

QList<DEFSECURITYTOOLINFO> SecurityToolsModel::getToolsInfo(TOOLSSTATUS status)
{
    QList<DEFSECURITYTOOLINFO> infos;

    foreach (auto info, m_toolinfos) {
        if (TOOLSSTATUS::ALL == status) {
            infos.push_back(info);
        } else if (TOOLSSTATUS::INSTALLEDTOOL == status) {
            if (info.bHasInstalled) {
                infos.push_back(info);
            }
        } else {
            if (!info.bHasInstalled) {
                infos.push_back(info);
            }
        }
    }

    return infos;
}

void SecurityToolsModel::openTool(const QString &strPackageKey, const QStringList &strParams)
{
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "OnShowTool", strPackageKey, strParams);
}

void SecurityToolsModel::updateTool(const QString &strPackageKey)
{
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "OnUpdatePackage", strPackageKey);
}

void SecurityToolsModel::installTool(const QString &strPackageKey)
{
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "OnInstallPackage", strPackageKey);
}

void SecurityToolsModel::unInstallTool(const QString &strPackageKey)
{
    DBUS_NOBLOCK_INVOKE(m_pSecurityToolDBusInter, "OnUnInstallPackage", strPackageKey);
}

// 得到正在安装/更新/卸载的app
QStringList SecurityToolsModel::getInstallingApps()
{
    // 获取等保开启状态
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_pSecurityToolDBusInter, "GetInstallingApps");
    GET_MESSAGE_VALUE(QStringList, applist, msg);
    return applist;
}

QStringList SecurityToolsModel::getUpdatingApps()
{
    // 获取等保开启状态
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_pSecurityToolDBusInter, "GetUpdatingApps");
    GET_MESSAGE_VALUE(QStringList, applist, msg);
    return applist;
}

QStringList SecurityToolsModel::getUninstallingApps()
{
    // 获取等保开启状态
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_pSecurityToolDBusInter, "GetUninstallingApps");
    GET_MESSAGE_VALUE(QStringList, applist, msg);
    return applist;
}
