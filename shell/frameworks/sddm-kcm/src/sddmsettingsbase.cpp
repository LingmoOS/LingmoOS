/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sddmsettingsbase.h"

#include "config.h"

#include <QDebug>
#include <QDir>

#include <algorithm>

SddmSettingsBase::SddmSettingsBase(KSharedConfigPtr config, QObject *parent)
    : KConfigSkeleton(config, parent)
{
    auto defaultFiles = QDir(QStringLiteral(SDDM_SYSTEM_CONFIG_DIR)).entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::LocaleAware);
    std::transform(defaultFiles.begin(), defaultFiles.end(), defaultFiles.begin(), [](const QString &filename) -> QString {
        return QStringLiteral(SDDM_SYSTEM_CONFIG_DIR "/") + filename;
    });
    // If no filename is set, KConfig will not parse any file
    if (!defaultFiles.isEmpty()) {
        m_defaultConfig = KSharedConfig::openConfig(defaultFiles.takeLast(), KConfig::CascadeConfig);
    } else {
        m_defaultConfig = KSharedConfig::openConfig(QString(), KConfig::CascadeConfig);
    }
    m_defaultConfig->addConfigSources(defaultFiles);
}

QString SddmSettingsBase::defaultCurrent() const
{
    return m_defaultConfig->group(QStringLiteral("Theme")).readEntry("Current");
}

unsigned int SddmSettingsBase::defaultMinimumUid() const
{
    return m_defaultConfig->group(QStringLiteral("Users")).readEntry("MinimumUid", 1000);
}

unsigned int SddmSettingsBase::defaultMaximumUid() const
{
    return m_defaultConfig->group(QStringLiteral("Users")).readEntry("MaximumUid", 60000);
}

QString SddmSettingsBase::defaultUser() const
{
    return m_defaultConfig->group(QStringLiteral("AutoLogin")).readEntry("User");
}

QString SddmSettingsBase::defaultSession() const
{
    return m_defaultConfig->group(QStringLiteral("AutoLogin")).readEntry("Session");
}

bool SddmSettingsBase::defaultRelogin() const
{
    return m_defaultConfig->group(QStringLiteral("AutoLogin")).readEntry("Relogin", false);
}

QString SddmSettingsBase::defaultHaltCommand() const
{
    return m_defaultConfig->group(QStringLiteral("General")).readEntry("HaltCommand");
}

QString SddmSettingsBase::defaultRebootCommand() const
{
    return m_defaultConfig->group(QStringLiteral("General")).readEntry("RebootCommand");
}
