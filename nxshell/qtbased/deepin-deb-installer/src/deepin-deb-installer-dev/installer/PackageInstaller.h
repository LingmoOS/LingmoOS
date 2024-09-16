// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>

#include <QApt/Backend>
#include <QApt/DebFile>

class PackagesManager;
class Package;

class PackageInstaller : public QObject
{
    Q_OBJECT
public:
    explicit PackageInstaller(QApt::Backend *b);

    virtual ~PackageInstaller();

    void appendPackage(Package *packages);

    void installPackage();

    void uninstallPackage();

signals:
    void signal_startInstall();

    void signal_installError(int, QString);

    void signal_installProgress(int progress);

    void signal_installDetailStatus(QString);

    void signal_installFinished(QApt::ExitStatus);

    void signal_uninstallFinished(QApt::ExitStatus);

    void signal_packageNotInstall();

private:
    bool isDpkgRunning();

    void dealBreakPackage();

    void dealAvailablePackage();

    void dealInstallablePackage();

    void installAvailableDepends();

private:
    Package *m_packages = nullptr;

    QApt::Backend *m_backend = nullptr;
    QApt::Transaction *m_pTrans = nullptr;
};

#endif  // INSTALLER_H
