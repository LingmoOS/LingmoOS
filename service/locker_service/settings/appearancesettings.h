/*
SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <KCoreConfigSkeleton>
#include <KPackage/Package>

namespace ScreenLocker
{
class WallpaperIntegration;
class LnFIntegration;
}

class KConfigPropertyMap;

class AppearanceSettings : public QObject
{
    Q_OBJECT

public:
    explicit AppearanceSettings(QObject *parent = nullptr);

    QUrl lnfConfigFile() const;
    QUrl wallpaperConfigFile() const;

    KConfigPropertyMap *wallpaperConfiguration() const;
    KConfigPropertyMap *lnfConfiguration() const;

    ScreenLocker::WallpaperIntegration *wallpaperIntegration() const;

    void load();
    void save();
    void defaults();

    bool isDefaults() const;
    bool isSaveNeeded() const;

    void loadWallpaperConfig();

Q_SIGNALS:
    void currentWallpaperChanged();

private:
    void loadLnfConfig();

    KPackage::Package m_package;
    ScreenLocker::WallpaperIntegration *m_wallpaperIntegration = nullptr;
    KCoreConfigSkeleton *m_wallpaperSettings = nullptr;
    QUrl m_wallpaperConfigFile;
    ScreenLocker::LnFIntegration *m_lnfIntegration = nullptr;
    KCoreConfigSkeleton *m_lnfSettings = nullptr;
    QUrl m_lnfConfigFile;
};
