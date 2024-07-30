/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGEJOBTHREAD_P_H
#define KPACKAGE_PACKAGEJOBTHREAD_P_H

#include "package.h"
#include "packagejob.h"
#include <QRunnable>

namespace KPackage
{
class PackageJobThreadPrivate;

bool indexDirectory(const QString &dir, const QString &dest);

class PackageJobThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit PackageJobThread(PackageJob::OperationType type, const QString &src, const QString &dest, const KPackage::Package &package);
    ~PackageJobThread() override;

    void run() override;

    bool install(const QString &src, const QString &dest, const Package &package);
    bool update(const QString &src, const QString &dest, const Package &package);
    bool uninstall(const QString &packagePath);

    PackageJob::JobError errorCode() const;

Q_SIGNALS:
    void jobThreadFinished(bool success, PackageJob::JobError errorCode, const QString &errorMessage = QString());
    void percentChanged(int percent);
    void error(const QString &errorMessage);
    void installPathChanged(const QString &installPath);

private:
    // OperationType says whether we want to install, update or any
    // new similar operation it will be expanded
    bool installDependency(const QUrl &src);
    bool installPackage(const QString &src, const QString &dest, const Package &package, PackageJob::OperationType operation);
    bool uninstallPackage(const QString &packagePath);
    PackageJobThreadPrivate *d;
};

}

#endif
