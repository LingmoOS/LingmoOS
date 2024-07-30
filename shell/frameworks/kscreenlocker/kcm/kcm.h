/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KConfigPropertyMap>
#include <KQuickManagedConfigModule>

#include "kscreensaversettings.h"
#include "wallpaper_integration.h"

class ScreenLockerKcmForm;
class AppearanceSettings;
namespace ScreenLocker
{
class ShellIntegration;
}

class ScreenLockerKcm : public KQuickManagedConfigModule
{
    Q_OBJECT
public:
    explicit ScreenLockerKcm(QObject *parent, const KPluginMetaData &data);

    Q_PROPERTY(KScreenSaverSettings *settings READ settings CONSTANT)
    Q_PROPERTY(KConfigPropertyMap *wallpaperConfiguration READ wallpaperConfiguration NOTIFY currentWallpaperChanged)
    Q_PROPERTY(KConfigPropertyMap *shellConfiguration READ shellConfiguration CONSTANT)
    Q_PROPERTY(QUrl shellConfigFile READ shellConfigFile CONSTANT)
    Q_PROPERTY(QUrl wallpaperConfigFile READ wallpaperConfigFile NOTIFY currentWallpaperChanged)
    Q_PROPERTY(ScreenLocker::WallpaperIntegration *wallpaperIntegration READ wallpaperIntegration NOTIFY currentWallpaperChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper NOTIFY currentWallpaperChanged)
    Q_PROPERTY(bool isDefaultsAppearance READ isDefaultsAppearance NOTIFY isDefaultsAppearanceChanged)

    Q_INVOKABLE QList<WallpaperInfo> availableWallpaperPlugins()
    {
        return KScreenSaverSettings::getInstance().availableWallpaperPlugins();
    }

    KScreenSaverSettings *settings() const;
    QUrl shellConfigFile() const;
    QUrl wallpaperConfigFile() const;
    ScreenLocker::WallpaperIntegration *wallpaperIntegration() const;
    QString currentWallpaper() const;
    bool isDefaultsAppearance() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    void updateState();
    void forceUpdateState();

Q_SIGNALS:
    void currentWallpaperChanged();

    /**
     * Emitted when the defaults function is called.
     */
    void defaultsCalled();

    void isDefaultsAppearanceChanged();

    /**
     * Emitted when the load function is called.
     */
    void loadCalled();

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    KConfigPropertyMap *wallpaperConfiguration() const;
    KConfigPropertyMap *shellConfiguration() const;

    AppearanceSettings *m_appearanceSettings;
    QString m_currentWallpaper;
    bool m_forceUpdateState = false;
};
