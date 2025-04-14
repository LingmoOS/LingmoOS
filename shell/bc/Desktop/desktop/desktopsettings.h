/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QDBusInterface>

class DesktopSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString wallpaper READ wallpaper NOTIFY wallpaperChanged)
    Q_PROPERTY(bool dimsWallpaper READ dimsWallpaper NOTIFY dimsWallpaperChanged)
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible NOTIFY backgroundVisibleChanged)
    Q_PROPERTY(int backgroundType READ backgroundType NOTIFY backgroundTypeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)

public:
    explicit DesktopSettings(QObject *parent = nullptr);

    QString wallpaper() const;
    bool dimsWallpaper() const;
    bool backgroundVisible() const;
    int backgroundType() const;
    QString backgroundColor() const;
    QString version();

    Q_INVOKABLE void launch(const QString &command, const QStringList &args);

signals:
    void wallpaperChanged();
    void dimsWallpaperChanged();
    void backgroundColorChanged();
    void backgroundTypeChanged();
    void backgroundVisibleChanged();
    
private slots:
    void init();
    void onWallpaperChanged(QString);

private:
    QDBusInterface m_interface;
    QString m_wallpaper;
    QString m_currentVersion;
};

#endif // SETTINGS_H
