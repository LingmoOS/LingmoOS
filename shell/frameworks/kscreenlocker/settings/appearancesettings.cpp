/*
SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appearancesettings.h"

#include <KConfigLoader>
#include <KPackage/PackageLoader>

#include "kscreensaversettings.h"
#include "shell_integration.h"
#include "wallpaper_integration.h"

AppearanceSettings::AppearanceSettings(QObject *parent)
    : QObject(parent)
{
}

QUrl AppearanceSettings::shellConfigFile() const
{
    return m_shellConfigFile;
}

QUrl AppearanceSettings::wallpaperConfigFile() const
{
    return m_wallpaperConfigFile;
}

KConfigPropertyMap *AppearanceSettings::wallpaperConfiguration() const
{
    if (!m_wallpaperIntegration) {
        return nullptr;
    }
    return m_wallpaperIntegration->configuration();
}

KConfigPropertyMap *AppearanceSettings::shellConfiguration() const
{
    if (!m_shellIntegration) {
        return nullptr;
    }
    return m_shellIntegration->configuration();
}

ScreenLocker::WallpaperIntegration *AppearanceSettings::wallpaperIntegration() const
{
    return m_wallpaperIntegration;
}

void AppearanceSettings::load()
{
    loadWallpaperConfig();
    loadShellConfig();

    if (m_shellSettings) {
        m_shellSettings->load();
        Q_EMIT m_shellSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->load();
        Q_EMIT m_wallpaperSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }
}

void AppearanceSettings::save()
{
    if (m_shellSettings) {
        m_shellSettings->save();
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->save();
    }
}

void AppearanceSettings::defaults()
{
    if (m_shellSettings) {
        m_shellSettings->setDefaults();
        Q_EMIT m_shellSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->setDefaults();
        Q_EMIT m_wallpaperSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }
}

bool AppearanceSettings::isDefaults() const
{
    bool defaults = true;

    if (m_shellSettings) {
        defaults &= m_shellSettings->isDefaults();
    }

    if (m_wallpaperSettings) {
        defaults &= m_wallpaperSettings->isDefaults();
    }
    return defaults;
}

bool AppearanceSettings::isSaveNeeded() const
{
    bool saveNeeded = false;

    if (m_shellSettings) {
        saveNeeded |= m_shellSettings->isSaveNeeded();
    }

    if (m_wallpaperSettings) {
        saveNeeded |= m_wallpaperSettings->isSaveNeeded();
    }

    return saveNeeded;
}

void AppearanceSettings::loadWallpaperConfig()
{
    if (m_wallpaperIntegration) {
        if (m_wallpaperIntegration->pluginName() == KScreenSaverSettings::getInstance().wallpaperPluginId()) {
            // nothing changed
            return;
        }
        delete m_wallpaperIntegration;
    }

    m_wallpaperIntegration = new ScreenLocker::WallpaperIntegration();
    m_wallpaperIntegration->setConfig(KScreenSaverSettings::getInstance().sharedConfig());
    m_wallpaperIntegration->setPluginName(KScreenSaverSettings::getInstance().wallpaperPluginId());
    m_wallpaperIntegration->init();
    m_wallpaperSettings = m_wallpaperIntegration->configScheme();
    m_wallpaperConfigFile = m_wallpaperIntegration->package().fileUrl(QByteArrayLiteral("ui"), QStringLiteral("config.qml"));
    Q_EMIT currentWallpaperChanged();
}

void AppearanceSettings::loadShellConfig()
{
    if (m_package.isValid() && m_shellIntegration) {
        return;
    }

    Q_ASSERT(!m_package.isValid() && !m_shellIntegration);

    m_package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Shell"));
    m_shellIntegration = new ScreenLocker::ShellIntegration(this);
    m_package.setPath(m_shellIntegration->defaultShell());
    m_shellIntegration->setPackage(m_package);
    m_shellIntegration->setConfig(KScreenSaverSettings::getInstance().sharedConfig());
    m_shellIntegration->init();
    m_shellSettings = m_shellIntegration->configScheme();

    auto sourceFile = m_package.fileUrl(QByteArrayLiteral("lockscreen"), QStringLiteral("config.qml"));
    m_shellConfigFile = sourceFile;
}

#include "moc_appearancesettings.cpp"
