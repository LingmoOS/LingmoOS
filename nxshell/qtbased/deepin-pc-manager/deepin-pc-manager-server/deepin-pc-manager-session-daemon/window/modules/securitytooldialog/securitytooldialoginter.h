// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../deepin-pc-manager/src/window/modules/securitytools/defsecuritytoolinfo.h"
#include "securitytoolbackend/defsecuritytoolsbuslinemgr.h"
#include "window/modules/securitytooldialog/startupdialog/startupwidget.h"
#include "window/modules/securitytooldialog/netcheckdialog/netcheckwidget.h"
#include "securitytooldialog_adaptor.h"

#include <DGuiApplicationHelper>

#include <QDBusAbstractAdaptor>
#include <QObject>

class QGSettings;

class SecurityToolDialogInter : public QObject
{
    Q_OBJECT
public:
    explicit SecurityToolDialogInter(QObject *parent = nullptr);
    virtual ~SecurityToolDialogInter() override;

Q_SIGNALS:
    // 所有的安全工具状态更新信号
    void notifyToolsInfoUpdate(const DEFSECURITYTOOLINFOLIST &infos);
    // 某一个安全工具的状态发生变化的信号
    void notifyAppStatusChanged(const QString &strPackageKey, int status);
    // 安全工具状态检测完成信号，在安全工具状态更新过程中可能会耗时较长，此时不应让使用者操作会禁用界面，最后使用此信号通知可以让用户进行操作了。
    // 此方法暂未使用，需要在实现依据具体情况判断是否需要使用。
    void notifyStatusCheckFinished();

public Q_SLOTS:
    // 显示自启动管理弹框
    void ShowStartup();
    // 显示网络检测弹框
    void ShowNetCheck();

    // 响应更新工具信息请求
    void OnRequestUpdateToolsInfo();
    // 响应工具包安装请求
    void OnInstallPackage(const QString &strPackageKey);
    // 响应包安装请求
    void OnUnInstallPackage(const QString &strPackageKey);
    // 响应包更新请求
    void OnUpdatePackage(const QString &strPackageKey);
    // 响应显示工具请求
    void OnShowTool(const QString &strPackageKey, const QStringList &strParams);

    // 得到正在安装/更新/卸载的app
    QStringList GetInstallingApps() { return m_appInstalling; }

    QStringList GetUpdatingApps() { return m_appUpdating; }

    QStringList GetUninstallingApps() { return m_appUninstalling; }

    void SetDefenderPaletteType(int type);

private:
    void initConnections();
    void getDefenderPaletteType();
    // 接收app状态改变槽
    void acceptAppStatusChanged(const QString &strPackageKey, DEFSECURITYTOOLSTATUS status);

private:
    SecuritytooldialogAdaptor *m_securityToolsAdaptor;
    DGuiApplicationHelper *m_guiHelper; // 方便得到系统主题
    QGSettings *m_gsetting; // gsetting配置对象

    StartupMainWindow *m_autoStartupMwnd;
    NetCheckMainWindow *m_netCheckMwnd;
    DefSecurityToolsBusLineMgr *m_pToolBusLineMgr;

    QStringList m_appInstalling;
    QStringList m_appUpdating;
    QStringList m_appUninstalling;
};
