/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "kcm.h"
#include "appearancesettings.h"
#include "kscreenlockerdata.h"
#include "screenlocker_interface.h"
#include "shell_integration.h"

#include <KConfigLoader>
#include <KConfigPropertyMap>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QList>

K_PLUGIN_FACTORY_WITH_JSON(ScreenLockerKcmFactory, "kcm_screenlocker.json", registerPlugin<ScreenLockerKcm>(); registerPlugin<KScreenLockerData>();)

ScreenLockerKcm::ScreenLockerKcm(QObject *parent, const KPluginMetaData &data)
    : KQuickManagedConfigModule(parent, data)
    , m_appearanceSettings(new AppearanceSettings(this))
{
    registerSettings(&KScreenSaverSettings::getInstance());

    constexpr const char *url = "org.kde.private.kcms.screenlocker";
    qRegisterMetaType<QList<WallpaperInfo>>("QList<WallpaperInfo>");
    qmlRegisterAnonymousType<KScreenSaverSettings>(url, 1);
    qmlRegisterAnonymousType<WallpaperInfo>(url, 1);
    qmlRegisterAnonymousType<ScreenLocker::WallpaperIntegration>(url, 1);
    qmlRegisterAnonymousType<KConfigPropertyMap>(url, 1);
    qmlProtectModule(url, 1);

    // Our modules will be checking the Plasmoid attached object when running from Plasma, let it load the module
    constexpr const char *uri = "org.kde.plasma.plasmoid";
    qmlRegisterUncreatableType<QObject>(uri, 2, 0, "PlasmoidPlaceholder", QStringLiteral("Do not create objects of type Plasmoid"));

    connect(&KScreenSaverSettings::getInstance(),
            &KScreenSaverSettings::wallpaperPluginIdChanged,
            m_appearanceSettings,
            &AppearanceSettings::loadWallpaperConfig);
    connect(m_appearanceSettings, &AppearanceSettings::currentWallpaperChanged, this, &ScreenLockerKcm::currentWallpaperChanged);
}

void ScreenLockerKcm::load()
{
    KQuickManagedConfigModule::load();
    m_appearanceSettings->load();

    updateState();
    Q_EMIT loadCalled();
}

void ScreenLockerKcm::save()
{
    KQuickManagedConfigModule::save();
    m_appearanceSettings->save();

    // reconfigure through DBus
    OrgKdeScreensaverInterface interface(QStringLiteral("org.kde.screensaver"), QStringLiteral("/ScreenSaver"), QDBusConnection::sessionBus());
    if (interface.isValid()) {
        interface.configure();
    }
    updateState();
}

void ScreenLockerKcm::defaults()
{
    KQuickManagedConfigModule::defaults();
    m_appearanceSettings->defaults();

    updateState();
    Q_EMIT defaultsCalled();
}

void ScreenLockerKcm::updateState()
{
    m_forceUpdateState = false;
    settingsChanged();
    Q_EMIT isDefaultsAppearanceChanged();
}

void ScreenLockerKcm::forceUpdateState()
{
    m_forceUpdateState = true;
    settingsChanged();
    Q_EMIT isDefaultsAppearanceChanged();
}

bool ScreenLockerKcm::isSaveNeeded() const
{
    return m_forceUpdateState || m_appearanceSettings->isSaveNeeded();
}

bool ScreenLockerKcm::isDefaults() const
{
    return m_appearanceSettings->isDefaults();
}

KConfigPropertyMap *ScreenLockerKcm::wallpaperConfiguration() const
{
    return m_appearanceSettings->wallpaperConfiguration();
}

KConfigPropertyMap *ScreenLockerKcm::shellConfiguration() const
{
    return m_appearanceSettings->shellConfiguration();
}

KScreenSaverSettings *ScreenLockerKcm::settings() const
{
    return &KScreenSaverSettings::getInstance();
}

QString ScreenLockerKcm::currentWallpaper() const
{
    return KScreenSaverSettings::getInstance().wallpaperPluginId();
}

bool ScreenLockerKcm::isDefaultsAppearance() const
{
    return m_appearanceSettings->isDefaults();
}

QUrl ScreenLockerKcm::shellConfigFile() const
{
    return m_appearanceSettings->shellConfigFile();
}

QUrl ScreenLockerKcm::wallpaperConfigFile() const
{
    return m_appearanceSettings->wallpaperConfigFile();
}

ScreenLocker::WallpaperIntegration *ScreenLockerKcm::wallpaperIntegration() const
{
    return m_appearanceSettings->wallpaperIntegration();
}

#include "kcm.moc"

#include "moc_kcm.cpp"
