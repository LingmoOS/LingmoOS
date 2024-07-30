/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "launchapp.h"

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>
#include <KNotificationJobUiDelegate>

LaunchApp::LaunchApp(QObject *parent)
    : QObject(parent)
{
}

void LaunchApp::launchWizard()
{
    auto *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(QStringLiteral("org.kde.bluedevilwizard")));
    auto *delegate = new KNotificationJobUiDelegate(KNotificationJobUiDelegate::AutoErrorHandlingEnabled);
    job->setUiDelegate(delegate);
    job->start();
}

void LaunchApp::launchSendFile(const QString &ubi)
{
    auto *job = new KIO::CommandLauncherJob(QStringLiteral("bluedevil-sendfile"), {QStringLiteral("-u"), ubi});
    job->setDesktopName(QStringLiteral("org.kde.bluedevilsendfile"));
    auto *delegate = new KNotificationJobUiDelegate(KNotificationJobUiDelegate::AutoErrorHandlingEnabled);
    job->setUiDelegate(delegate);
    job->start();
}

#include "moc_launchapp.cpp"
