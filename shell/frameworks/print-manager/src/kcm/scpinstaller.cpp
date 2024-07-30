/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
*/

#include "scpinstaller.h"
#include "printermanager.h"
#include "pmkcm_log.h"

#include <KLocalizedString>

void SCPInstaller::install()
{
    setInstalling(true);
    PackageKit::Daemon::setHints(PackageKit::Daemon::hints() + QStringList{QStringLiteral("interactive=true")});

    const QString package = QStringLiteral(SCP_PACKAGE_NAME);
    QStringList distroSCPPackages = package.split(QLatin1Char(','));

    PackageKit::Transaction *resolveTransaction = PackageKit::Daemon::resolve(distroSCPPackages, PackageKit::Transaction::FilterNone);

    QSharedPointer<QStringList> pkgids(new QStringList);

    connect(resolveTransaction, &PackageKit::Transaction::errorCode, this, [this](PackageKit::Transaction::Error err, const QString &details) {
        Q_EMIT error(details);
        setFailed(true);
    });

    connect(resolveTransaction, &PackageKit::Transaction::package, this, [pkgids](PackageKit::Transaction::Info info, const QString &packageId) {
        qCDebug(PMKCM) << info << packageId;
        pkgids->append(packageId);
    });

    connect(resolveTransaction, &PackageKit::Transaction::finished, this, [this, pkgids](PackageKit::Transaction::Exit exit) {
        if (exit != PackageKit::Transaction::ExitSuccess) {
            setFailed(true);
            return;
        }
        auto installTransaction = PackageKit::Daemon::installPackages(*pkgids);
        connect(installTransaction, &PackageKit::Transaction::finished, this, &SCPInstaller::packageFinished);
    });
}

bool SCPInstaller::isInstalling() const
{
    return m_installing;
}

bool SCPInstaller::hasFailed() const
{
    return m_failed;
}

bool SCPInstaller::isInstalled()
{
    return PrinterManager::isSCPAvailable();
}

void SCPInstaller::packageFinished(PackageKit::Transaction::Exit status)
{
    qCDebug(PMKCM) << "INSTALLER FINISHED" << status;
    setInstalling(false);
    switch (status) {
    case PackageKit::Transaction::ExitSuccess:
        Q_EMIT installedChanged();
        return;
    default:
        // a bit generic but we don't need much more, in this case
        Q_EMIT error(i18nc("@info", "Install Error (package: %1)\nCheck authentication credentials and make sure PackageKit is installed properly.", QStringLiteral(SCP_PACKAGE_NAME)));
        break;
    }
    setFailed(true);
}

void SCPInstaller::setFailed(bool failed)
{
    if (m_failed == failed) {
        return;
    }

    if (m_installing && failed) {
        setInstalling(false);
    }

    m_failed = true;
    Q_EMIT failedChanged();
}

void SCPInstaller::setInstalling(bool installing)
{
    if (m_installing == installing) {
        return;
    }

    m_installing = installing;
    if (m_installing) {
        m_failed = false;
        Q_EMIT failedChanged();
    }
    Q_EMIT installingChanged();
}
