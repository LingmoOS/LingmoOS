// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSBASEINSTALLER_H
#define DEFSECURITYTOOLSBASEINSTALLER_H

#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerinterface.h"
#include "../../deepin-pc-manager/src/window/modules/securitytools/defsecuritytoolinfo.h"

#include <QDebug>
#include <QObject>

typedef struct PackageMsg
{
    explicit PackageMsg()
        : strPackageName("")
        , strVersion("")
    {
    }

    bool operator<(const PackageMsg &otherPackage)
    {
        if (this == &otherPackage) {
            return true;
        }

        if (strPackageName != otherPackage.strPackageName) {
            qWarning() << "can't been compared!";
            return false;
        }

        return strVersion < otherPackage.strVersion;
    }

    QString strPackageName;
    QString strVersion;
} PACKAGEMSG, *LPPACKAGEMSG;

class DefSecurityToolsBaseInstaller : public QObject
{
    Q_OBJECT
public:
    explicit DefSecurityToolsBaseInstaller(QObject *parent = nullptr);
    virtual ~DefSecurityToolsBaseInstaller();

public:
    void setInfo(const DEFSECURITYTOOLINFO &info);
    DEFSECURITYTOOLINFO toolInfo() const;
    void setLatestPackageInfo(const PACKAGEMSG &packagemsg);
    void installPackage();
    void unInstallPackage();
    void updatePackage();
    void setStoreManagerInter(DBusInvokerInterface *pInterface);
    bool isPackageExist();
    bool isUpdatable();
    bool isPackageInstallable();

    // virtual
public:
    virtual void showtool(const QStringList &strParams);

Q_SIGNALS:
    void sAppStatusChanged(const QString &strKey, DEFSECURITYTOOLSTATUS status);

public Q_SLOTS:
    void onInstallStart(const QString &strID, const QString &strPackage);
    void onSigIntallProgress(const QString &strID,
                             const QString &strPackage,
                             int iState,
                             int iTotal,
                             int iProcess,
                             const QString &strStatusMsg);
    void onSigRemoveProgress(const QString &strID, const QString &strPackage, bool bSuccess);

private:
    PACKAGEMSG getLocalPackageInfo(QString strMsg);

protected:
    DEFSECURITYTOOLINFO m_info;
    PACKAGEMSG m_LatestPackageMsg;

private:
    DBusInvokerInterface *m_pManagerInvoker;
    QString m_jobPath;
    QString m_strType;
    QString m_strStatus;
    bool m_bUpdate;
};

class DefSecurityToolsInstallerFactory : public QObject
{
    explicit DefSecurityToolsInstallerFactory(QObject *parent = nullptr);
    ~DefSecurityToolsInstallerFactory() override;

public:
    static DefSecurityToolsBaseInstaller *createInstaller(const DEFSECURITYTOOLINFO &info);
};

#endif // DEFSECURITYTOOLSBASEINSTALLER_H
