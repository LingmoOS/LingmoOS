// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PackagesManager_H
#define PackagesManager_H
#include "result.h"
#include "status/PackageStatus.h"

#include <QObject>
#include <QFuture>

class PackageSigntureStatus;
class Package;
class PackageInstaller;

typedef Result<QString> ConflictResult;

class GetStatusThread;
class PackagesManager : public QObject
{
    friend class installer;
    Q_OBJECT
public:
    PackagesManager();

    virtual ~PackagesManager();

public:
    bool checkPackageValid(int index = 0);

    bool checkPackageSignture(int index = 0);

    bool checkPackageDependsStatus(int index = 0);

    int checkInstallStatus(int index = 0);

public:
    void appendPackages(QStringList packages);

    void removePackage(int index);

signals:
    void signal_backendError();

    void signal_packageInvalid(int);

    void signal_signatureError(int, int);

    void signal_dependStatusError(int, int);

    void signal_addPackageSuccess(int);

    void signal_removePackageSuccess(int);

    void signal_packageAlreadyExits(int);

    void signal_invalidIndex(int);

    void signal_packageNotInstalled(int);

signals:
    void signal_startInstallPackages();

    void signal_installProgress(int progress);

    void signal_installDetailStatus(QString);

    void signal_installFinished();

    void signal_uninstallFinished();

    void signal_installErrorOccured(int, QString);

public:
    void install();

    void uninstall(int index);

private slots:
    void slot_installFinished(QApt::ExitStatus);

    void slot_uninstallFinished(QApt::ExitStatus);

    void slot_getDependsStatus(int, DependsStatus);

    void slot_getInstallStatus(int, InstallStatus);

private:
    QList<Package *> m_packages = {};

    QSet<QByteArray> m_packagesMd5 = {};

    PackageStatus *m_pPackageStatus = nullptr;

    PackageInstaller *m_pPackageInstaller = nullptr;

    GetStatusThread *m_pGetStatusThread = nullptr;

    bool m_appendFinished = false;

private:
    Package *searchByIndex(int index = 0);
    void getPackageInfo(QString pkg, int index = 0);

    bool checkPackageSuffix(QString packagePath);

    void initConnection();
};

#endif  // PackagesManager_H
