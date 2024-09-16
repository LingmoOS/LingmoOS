// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytooldialoginter.h"

#include "../../deepin-pc-manager/src/window/modules/common/gsettingkey.h"

#include <QDBusConnection>
#include <QGSettings>

const QString service = "com.deepin.pc.manager.securitytooldialog";
const QString path = "/com/deepin/pc/manager/securitytooldialog";

// 自动化标签
#define SET_ACCESS_NAME_T(control, className, name) \
    control->setAccessibleName(ACCNAMESTR(className##_##name));
#define SET_ACC_NAME(control, name) SET_ACCESS_NAME_T(control, securityToolDialogInter, name)

SecurityToolDialogInter::SecurityToolDialogInter(QObject *parent)
    : QObject(parent)
    , m_securityToolsAdaptor(nullptr)
    , m_guiHelper(nullptr)
    , m_gsetting(new QGSettings(DEEPIN_PC_MANAGER_GSETTING_PATH, QByteArray(), this))
    , m_autoStartupMwnd(nullptr)
    , m_netCheckMwnd(nullptr)
{
    m_securityToolsAdaptor = new SecuritytooldialogAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(service)
        || !QDBusConnection::sessionBus().registerObject(path, this)) {
        exit(0);
    }

    m_pToolBusLineMgr = new DefSecurityToolsBusLineMgr;
    DefToolAuthorityCheckObj *pCheckObj = new DefToolAuthorityCheckObj(m_pToolBusLineMgr);
    m_pToolBusLineMgr->setAutorityCheckObj(pCheckObj);

    // 主题
    m_guiHelper = DGuiApplicationHelper::instance();
    getDefenderPaletteType();

    // 安全工具信息
    registerDefSecurityToolInfo();
    registerDefSecurityToolInfoList();
    initConnections();
}

SecurityToolDialogInter::~SecurityToolDialogInter()
{
    if (m_autoStartupMwnd) {
        m_autoStartupMwnd->deleteLater();
        m_autoStartupMwnd = nullptr;
    }
    if (m_netCheckMwnd) {
        m_netCheckMwnd->deleteLater();
        m_netCheckMwnd = nullptr;
    }

    if (m_pToolBusLineMgr) {
        m_pToolBusLineMgr->deleteLater();
        m_pToolBusLineMgr = nullptr;
    }
}

void SecurityToolDialogInter::initConnections()
{
    QObject::connect(m_pToolBusLineMgr,
                     &DefSecurityToolsBusLineMgr::notifyToolsInfoUpdate,
                     this,
                     &SecurityToolDialogInter::notifyToolsInfoUpdate);
    QObject::connect(m_pToolBusLineMgr,
                     &DefSecurityToolsBusLineMgr::notifyAppStatusChanged,
                     this,
                     &SecurityToolDialogInter::acceptAppStatusChanged);
}

void SecurityToolDialogInter::getDefenderPaletteType()
{
    int type = m_gsetting->get(DEFENDER_PALETTE_TYPE).toInt();
    m_guiHelper->setPaletteType(DGuiApplicationHelper::ColorType(type));
}

void SecurityToolDialogInter::SetDefenderPaletteType(int type)
{
    m_gsetting->set(DEFENDER_PALETTE_TYPE, type);
    m_guiHelper->setPaletteType(DGuiApplicationHelper::ColorType(type));
}

// 显示自启动管理弹框
void SecurityToolDialogInter::ShowStartup()
{
    if (!m_autoStartupMwnd) {
        m_autoStartupMwnd = new StartupMainWindow;
        connect(m_autoStartupMwnd, &StartupMainWindow::sendWindowClose, this, [=] {
            m_autoStartupMwnd->deleteLater();
            m_autoStartupMwnd = nullptr;
        });
    }

    m_autoStartupMwnd->show();
    m_autoStartupMwnd->activateWindow();
}

// 显示网络检测弹框
void SecurityToolDialogInter::ShowNetCheck()
{
    if (!m_netCheckMwnd) {
        m_netCheckMwnd = new NetCheckMainWindow;
        connect(m_netCheckMwnd, &NetCheckMainWindow::sendWindowClose, this, [=] {
            m_netCheckMwnd->deleteLater();
            m_netCheckMwnd = nullptr;
        });
    }

    m_netCheckMwnd->show();
    m_netCheckMwnd->activateWindow();
}

void SecurityToolDialogInter::OnRequestUpdateToolsInfo()
{
    m_pToolBusLineMgr->init();
}

void SecurityToolDialogInter::OnInstallPackage(const QString &strPackageKey)
{
    m_pToolBusLineMgr->installPackage(strPackageKey);
}

void SecurityToolDialogInter::OnUnInstallPackage(const QString &strPackageKey)
{
    m_pToolBusLineMgr->unInstallPackage(strPackageKey);
}

void SecurityToolDialogInter::OnUpdatePackage(const QString &strPackageKey)
{
    m_pToolBusLineMgr->updatePackage(strPackageKey);
}

void SecurityToolDialogInter::OnShowTool(const QString &strPackageKey, const QStringList &strParams)
{
    m_pToolBusLineMgr->showTool(strPackageKey, strParams);
}

// 接收app状态改变槽
void SecurityToolDialogInter::acceptAppStatusChanged(const QString &strPackageKey,
                                                     DEFSECURITYTOOLSTATUS status)
{
    Q_EMIT notifyAppStatusChanged(strPackageKey, status);

    if (DEFSECURITYTOOLSTATUS::INSTALLING == status) {
        if (!m_appInstalling.contains(strPackageKey)) {
            m_appInstalling.append(strPackageKey);
        }
    } else if (DEFSECURITYTOOLSTATUS::UPDATING == status) {
        if (!m_appUpdating.contains(strPackageKey)) {
            m_appUpdating.append(strPackageKey);
        }
    } else if (DEFSECURITYTOOLSTATUS::UNINSTALLING == status) {
        if (!m_appUninstalling.contains(strPackageKey)) {
            m_appUninstalling.append(strPackageKey);
        }
    } else {
        if (m_appInstalling.contains(strPackageKey)) {
            m_appInstalling.removeOne(strPackageKey);
        }
        if (m_appUpdating.contains(strPackageKey)) {
            m_appUpdating.removeOne(strPackageKey);
        }
        if (m_appUninstalling.contains(strPackageKey)) {
            m_appUninstalling.removeOne(strPackageKey);
        }
    }
}
