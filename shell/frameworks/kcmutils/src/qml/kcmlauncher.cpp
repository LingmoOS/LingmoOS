/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmlauncher_p.h"

#include <KIO/CommandLauncherJob>
#include <KService>

void KCMLauncher::open(const QStringList &names) const
{
    KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell6"), names);
    job->start();
}

void KCMLauncher::openSystemSettings(const QString &name, const QStringList &args) const
{
    // The desktop filename is the same as the binary and icon
    const QString systemSettings = QStringLiteral("systemsettings");
    KIO::CommandLauncherJob *job = nullptr;

    QStringList cmdline{name};
    if (!args.isEmpty()) {
        cmdline.append(QStringLiteral("--args"));
        cmdline.append(args.join(QLatin1Char(' ')));
    }

    // Open in System Settings if it's available
    if (KService::serviceByDesktopName(systemSettings)) {
        job = new KIO::CommandLauncherJob(systemSettings, cmdline);
        job->setDesktopName(systemSettings);
    } else {
        job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell6"), cmdline);
    }

    job->start();
}

void KCMLauncher::openInfoCenter(const QString &name) const
{
    const QString infoCenterDesktopFile = QStringLiteral("org.kde.kinfocenter");
    const QString infoCenterbinary = QStringLiteral("kinfocenter");

    KIO::CommandLauncherJob *job = nullptr;

    // Open in Info Center if it's available
    if (KService::serviceByDesktopName(infoCenterDesktopFile)) {
        job = new KIO::CommandLauncherJob(infoCenterbinary, QStringList(name));
        job->setDesktopName(infoCenterDesktopFile);
    } else {
        job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell6"), QStringList(name));
    }

    job->start();
}

#include "moc_kcmlauncher_p.cpp"
