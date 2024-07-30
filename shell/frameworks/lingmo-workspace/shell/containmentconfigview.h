/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <LingmoQuick/ConfigModel>
#include <LingmoQuick/ConfigView>
#include <QQmlPropertyMap>

namespace Lingmo
{
class Containment;
}

class QAbstractItemModel;
class CurrentContainmentActionsModel;

// TODO: out of the library?
class ContainmentConfigView : public LingmoQuick::ConfigView
{
    Q_OBJECT
    Q_PROPERTY(LingmoQuick::ConfigModel *containmentActionConfigModel READ containmentActionConfigModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *currentContainmentActionsModel READ currentContainmentActionsModel CONSTANT)
    Q_PROPERTY(LingmoQuick::ConfigModel *wallpaperConfigModel READ wallpaperConfigModel CONSTANT)
    Q_PROPERTY(LingmoQuick::ConfigModel *containmentPluginsConfigModel READ containmentPluginsConfigModel CONSTANT)
    Q_PROPERTY(QQmlPropertyMap *wallpaperConfiguration READ wallpaperConfiguration NOTIFY wallpaperConfigurationChanged)
    Q_PROPERTY(QString currentWallpaper READ currentWallpaper WRITE setCurrentWallpaper NOTIFY currentWallpaperChanged)
    Q_PROPERTY(QString containmentPlugin READ containmentPlugin WRITE setContainmentPlugin NOTIFY containmentPluginChanged)

public:
    explicit ContainmentConfigView(Lingmo::Containment *interface, QWindow *parent = nullptr);
    ~ContainmentConfigView() override;

    void init() override;

    LingmoQuick::ConfigModel *containmentActionConfigModel();
    QAbstractItemModel *currentContainmentActionsModel();
    LingmoQuick::ConfigModel *wallpaperConfigModel();
    LingmoQuick::ConfigModel *containmentPluginsConfigModel();
    QString currentWallpaper() const;
    void setCurrentWallpaper(const QString &wallpaper);
    QQmlPropertyMap *wallpaperConfiguration() const;
    QString containmentPlugin() const;
    void setContainmentPlugin(const QString &plugin);

    Q_INVOKABLE void applyWallpaper();

public Q_SLOTS:
    void onWallpaperChanged(uint screenIdx);

Q_SIGNALS:
    void currentWallpaperChanged();
    void wallpaperConfigurationChanged();
    void containmentPluginChanged();

protected:
    void syncWallpaperObjects();

private:
    Lingmo::Containment *m_containment = nullptr;
    LingmoQuick::ConfigModel *m_wallpaperConfigModel = nullptr;
    LingmoQuick::ConfigModel *m_containmentActionConfigModel = nullptr;
    LingmoQuick::ConfigModel *m_containmentPluginsConfigModel = nullptr;
    CurrentContainmentActionsModel *m_currentContainmentActionsModel = nullptr;
    QString m_currentWallpaperPlugin;
    QQmlPropertyMap *m_currentWallpaperConfig = nullptr;
    QQmlPropertyMap *m_ownWallpaperConfig = nullptr;
};
