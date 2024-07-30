/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <KConfig>
#include <KIO/CommandLauncherJob>
#include <KNotificationJobUiDelegate>
#include <KPluginFactory>
#include <QString>

#include "../plasma-welcome-version.h"
#include "daemon.h"

K_PLUGIN_CLASS_WITH_JSON(PlasmaWelcomeDaemon, "kded_plasma-welcome.json")

PlasmaWelcomeDaemon::PlasmaWelcomeDaemon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_config(new KConfig(QStringLiteral("plasma-welcomerc")), "General")
    , m_currentVersion(QVersionNumber::fromString(QString::fromLatin1(PLASMA_WELCOME_VERSION_STRING)))
    , m_previousVersion(QVersionNumber::fromString(m_config.readEntry("LastSeenVersion", QString::fromLatin1(PLASMA_WELCOME_VERSION_STRING))))
{
    if (m_config.readEntry("LiveEnvironment", false)) {
        // Live installer, always launch
        launch(QStringList{QStringLiteral("--live-environment")});
    } else {
        // Normal, check if launched before or updated since
        if (m_config.readEntry("LastSeenVersion", QStringLiteral("")).isEmpty()) {
            launch(QStringList{});
        } else if (m_config.readEntry("ShowUpdatePage", true) && isSignificantUpgrade()) {
            const bool isBeta = (m_currentVersion.microVersion() >= 90 || m_currentVersion.minorVersion() >= 80);
            launch(QStringList{isBeta ? QStringLiteral("--post-update-beta") : QStringLiteral("--post-update")});
        }

        m_config.writeEntry("LastSeenVersion", m_currentVersion.toString());
        m_config.config()->sync();
    }
}

bool PlasmaWelcomeDaemon::isSignificantUpgrade() const
{
    if (m_currentVersion > m_previousVersion) {
        // We should get atleast two version numbers, X.X(.X)
        Q_ASSERT(m_currentVersion.segmentCount() > 1 && m_previousVersion.segmentCount() > 1);

        // Ignore development versions
        if (m_currentVersion.microVersion() == 80)
            return false;

        // 5(.X.X) -> 6(.X.X)
        if (m_currentVersion.majorVersion() > m_previousVersion.majorVersion())
            return true;

        // 5.1(.X) -> 5.2(.X)
        if ((m_currentVersion.majorVersion() == m_previousVersion.majorVersion()) && (m_currentVersion.minorVersion() > m_previousVersion.minorVersion()))
            return true;

        // X.X.X -> X.X.90 (upgrade to beta version)
        if (m_currentVersion.microVersion() >= 90 && m_currentVersion.microVersion() > m_previousVersion.microVersion())
            return true;
    }

    return false;
}

void PlasmaWelcomeDaemon::launch(const QStringList &args)
{
    auto *job = new KIO::CommandLauncherJob(QStringLiteral("plasma-welcome"), args);
    job->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoErrorHandlingEnabled));
    job->setDesktopName(QStringLiteral("org.kde.plasma-welcome"));
    job->start();
}

#include "daemon.moc"

#include "moc_daemon.cpp"
