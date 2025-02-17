/*
SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "appearancesettings.h"

#include <KConfigLoader>
#include <KPackage/PackageLoader>

#include "kscreensaversettings.h"
#include "lnf_integration.h"
#include "wallpaper_integration.h"

AppearanceSettings::AppearanceSettings(QObject *parent)
    : QObject(parent)
{
}

QUrl AppearanceSettings::lnfConfigFile() const
{
    return m_lnfConfigFile;
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

KConfigPropertyMap *AppearanceSettings::lnfConfiguration() const
{
    if (!m_lnfIntegration) {
        return nullptr;
    }
    return m_lnfIntegration->configuration();
}

ScreenLocker::WallpaperIntegration *AppearanceSettings::wallpaperIntegration() const
{
    return m_wallpaperIntegration;
}

void AppearanceSettings::load()
{
    loadWallpaperConfig();
    loadLnfConfig();

    if (m_lnfSettings) {
        m_lnfSettings->load();
        Q_EMIT m_lnfSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->load();
        Q_EMIT m_wallpaperSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }
}

void AppearanceSettings::save()
{
    if (m_lnfSettings) {
        m_lnfSettings->save();
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->save();
    }
}

void AppearanceSettings::defaults()
{
    if (m_lnfSettings) {
        m_lnfSettings->setDefaults();
        Q_EMIT m_lnfSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }

    if (m_wallpaperSettings) {
        m_wallpaperSettings->setDefaults();
        Q_EMIT m_wallpaperSettings->configChanged(); // To force the ConfigPropertyMap to reevaluate
    }
}

bool AppearanceSettings::isDefaults() const
{
    bool defaults = true;

    if (m_lnfSettings) {
        defaults &= m_lnfSettings->isDefaults();
    }

    if (m_wallpaperSettings) {
        defaults &= m_wallpaperSettings->isDefaults();
    }
    return defaults;
}

bool AppearanceSettings::isSaveNeeded() const
{
    bool saveNeeded = false;

    if (m_lnfSettings) {
        saveNeeded |= m_lnfSettings->isSaveNeeded();
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

    m_wallpaperIntegration = new ScreenLocker::WallpaperIntegration(this);
    m_wallpaperIntegration->setConfig(KScreenSaverSettings::getInstance().sharedConfig());
    m_wallpaperIntegration->setPluginName(KScreenSaverSettings::getInstance().wallpaperPluginId());
    m_wallpaperIntegration->init();
    m_wallpaperSettings = m_wallpaperIntegration->configScheme();
    m_wallpaperConfigFile = m_wallpaperIntegration->package().fileUrl(QByteArrayLiteral("ui"), QStringLiteral("config.qml"));
    Q_EMIT currentWallpaperChanged();
}

void AppearanceSettings::loadLnfConfig()
{
    if (m_package.isValid() && m_lnfIntegration) {
        return;
    }

    Q_ASSERT(!m_package.isValid() && !m_lnfIntegration);

    m_package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), "KDE");
    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    if (!packageName.isEmpty()) {
        m_package.setPath(packageName);
    }

    m_lnfIntegration = new ScreenLocker::LnFIntegration(this);
    m_lnfIntegration->setPackage(m_package);
    m_lnfIntegration->setConfig(KScreenSaverSettings::getInstance().sharedConfig());
    m_lnfIntegration->init();
    m_lnfSettings = m_lnfIntegration->configScheme();

    auto sourceFile = m_package.fileUrl(QByteArrayLiteral("lockscreen"), QStringLiteral("config.qml"));
    m_lnfConfigFile = sourceFile;
}
