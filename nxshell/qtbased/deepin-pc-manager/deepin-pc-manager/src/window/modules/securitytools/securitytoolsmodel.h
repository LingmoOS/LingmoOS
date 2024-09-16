// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defsecuritytoolinfo.h"

#include <QObject>

class DBusInvokerInterface;
class SettingsInvokerInterface;

class SecurityToolsModel : public QObject
{
    Q_OBJECT
public:
    explicit SecurityToolsModel(QObject *parent = nullptr);
    ~SecurityToolsModel();

    void updateToolsInfo();

    // 获取工具信息
    QList<DEFSECURITYTOOLINFO> getToolsInfo(TOOLSSTATUS status);
    void openTool(const QString &strPackageKey, const QStringList &strParams = QStringList());
    void updateTool(const QString &strPackageKey);
    void installTool(const QString &strPackageKey);
    void unInstallTool(const QString &strPackageKey);

    // 得到正在安装/更新/卸载的app
    QStringList getInstallingApps();
    QStringList getUpdatingApps();
    QStringList getUninstallingApps();

private:
    // 初始化数据
    void initData();
    void initConnectoin();

public Q_SLOTS:
    void onToolsInfoUpdate(const DEFSECURITYTOOLINFOLIST &infos);
    void onAppStatusChanged(const QString &strPackageKey, int status);
    void onStatusCheckFinished();

Q_SIGNALS:
    void sUpdateToolsInfo();
    void notityAppStatusChanged(const QString &strPackageKey, int status);

private:
    DBusInvokerInterface *m_pSecurityToolDBusInter; // 安全工具
    SettingsInvokerInterface *m_gsetting;           // 安全中心gsetting配置
    QList<DEFSECURITYTOOLINFO> m_toolinfos;
};
