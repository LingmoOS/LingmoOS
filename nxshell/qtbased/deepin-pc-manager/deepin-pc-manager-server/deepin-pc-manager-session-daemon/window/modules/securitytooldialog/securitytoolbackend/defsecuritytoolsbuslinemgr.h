// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSBUSLINEMGR_H
#define DEFSECURITYTOOLSBUSLINEMGR_H

#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerinterface.h"
#include "../../deepin-pc-manager/src/window/modules/securitytools/defsecuritytoolinfo.h"
#include "installer/defsecuritytoolsbaseinstaller.h"

#include <QMap>
#include <QObject>

class DefToolAuthorityCheckObj : public QObject
{
    Q_OBJECT
public:
    explicit DefToolAuthorityCheckObj(QObject *pParent = nullptr);
    ~DefToolAuthorityCheckObj() override;

private:
};

class DefSecurityToolsBusLineMgr : public QObject
{
    Q_OBJECT
public:
    explicit DefSecurityToolsBusLineMgr(QObject *pParent = nullptr);
    ~DefSecurityToolsBusLineMgr() override;

public:
    void setAutorityCheckObj(DefToolAuthorityCheckObj *pObj);
    void installPackage(const QString &strPackageKey);
    void unInstallPackage(const QString &strPackageKey);
    void updatePackage(const QString &strPackageKey);
    void showTool(const QString &strPackageKey, const QStringList &strParams);
    bool isInit() const;
    void init();

private:
    bool hasAutority();
    void registerInstaller(const QString &strPackageKey, DefSecurityToolsBaseInstaller *pInstall);
    void unregistInstaller(const QString &strPackageKey);
    bool hasInstaller(const QString &strPackageKey);
    DEFSECURITYTOOLINFOLIST defaultSecurityInfos();
    DEFSECURITYTOOLINFOLIST readFrom(const QString &strFileName);
    void writeTo(const DEFSECURITYTOOLINFOLIST &infolist, const QString &strFileName);
    DefSecurityToolsBaseInstaller *getInstaller(const QString strPackageKey) const;
    void loadAlltools();
    // 请注意，此函数必需在loadAlltools之前调用
    void initLatestLocalPackages();
    // 隐藏工具 -- 例如：超级防护
    bool isHideTools(const DEFSECURITYTOOLINFO &info,
                     const QString &strAppName,
                     const QString &strGsetting);
    // 判断工具是否可用
    bool isExecPreventValid();
    bool isSuperiorProtectionValid();
    bool isKernelModulesProtectionValid();
    bool isResourceManagerValid();

Q_SIGNALS:
    // 所有的安全工具状态更新信号
    void notifyToolsInfoUpdate(const DEFSECURITYTOOLINFOLIST &infos);
    // 某一个安全工具的状态发生变化的信号
    void notifyAppStatusChanged(const QString &strPackageKey, DEFSECURITYTOOLSTATUS status);
    // 安全工具状态检测完成信号
    void notifyStatusCheckFinished();

private Q_SLOTS:
    void onAppStatusChanged(const QString &strPackageKey, DEFSECURITYTOOLSTATUS status);

private:
    DefToolAuthorityCheckObj *m_pAuthorityCheckObj;
    QMap<QString, DefSecurityToolsBaseInstaller *> m_pToolInstallers;
    QMap<QString, PACKAGEMSG> m_latestLocalPackages;
    DBusInvokerInterface *m_pManagerInter;
    bool m_bLoad;
    SettingsInvokerInterface *m_gSettingsInvokerInter;
    bool m_isSecEnhanceSrvValid;
    bool m_isResourceManagerValid;
};

#endif // DEFSECURITYTOOLSBUSLINEMGR_H
