// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <KIO/CommandLauncherJob>
#include <KNotificationJobUiDelegate>
#include <KPluginFactory>

#include <QProcess>

#include "start.h"

K_PLUGIN_FACTORY_WITH_JSON(StartFactory, "kded_lingmo_mobile_start.json", registerPlugin<Start>();)

Start::Start(QObject *parent, const QList<QVariant> &)
    : KDEDModule{parent}
{
    auto *envmanagerJob = new KIO::CommandLauncherJob(QStringLiteral("lingmo-mobile-envmanager --apply-settings"), {});
    envmanagerJob->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled));
    envmanagerJob->setDesktopName(QStringLiteral("org.kde.lingmo-mobile-envmanager"));
    envmanagerJob->start();

    auto *initialstartJob = new KIO::CommandLauncherJob(QStringLiteral("lingmo-mobile-initial-start"), {});
    initialstartJob->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled));
    initialstartJob->setDesktopName(QStringLiteral("org.kde.lingmo-mobile-initial-start"));
    initialstartJob->start();
}

#include "start.moc"
