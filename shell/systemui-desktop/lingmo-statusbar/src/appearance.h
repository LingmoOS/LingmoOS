/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
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

#ifndef APPEARANCE_H
#define APPEARANCE_H

#include <QObject>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QDBusInterface>

class Appearance : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY dockIconSizeChanged)
    Q_PROPERTY(int dockDirection READ dockDirection WRITE setDockDirection NOTIFY dockDirectionChanged)
    Q_PROPERTY(int fontPointSize READ fontPointSize WRITE setFontPointSize NOTIFY fontPointSizeChanged)
    Q_PROPERTY(bool dimsWallpaper READ dimsWallpaper WRITE setDimsWallpaper NOTIFY dimsWallpaperChanged)
    Q_PROPERTY(double devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY devicePixelRatioChanged)

public:
    explicit Appearance(QObject *parent = nullptr);

    Q_INVOKABLE void switchDarkMode(bool darkMode);

    bool dimsWallpaper() const;
    Q_INVOKABLE void setDimsWallpaper(bool value);

    int dockIconSize() const;
    Q_INVOKABLE void setDockIconSize(int dockIconSize);

    int dockDirection() const;
    Q_INVOKABLE void setDockDirection(int dockDirection);

    Q_INVOKABLE void setGenericFontFamily(const QString &name);
    Q_INVOKABLE void setFixedFontFamily(const QString &name);

    int fontPointSize() const;
    Q_INVOKABLE void setFontPointSize(int fontPointSize);

    Q_INVOKABLE void setAccentColor(int accentColor);

    double devicePixelRatio() const;
    Q_INVOKABLE void setDevicePixelRatio(double value);

signals:
    void dockIconSizeChanged();
    void dockDirectionChanged();
    void fontPointSizeChanged();
    void dimsWallpaperChanged();
    void devicePixelRatioChanged();

private:
    QDBusInterface m_interface;
    QSettings *m_dockSettings;
    QFileSystemWatcher *m_dockConfigWacher;

    int m_dockIconSize;
    int m_dockDirection;
    int m_fontPointSize;
};

#endif // APPEARANCE_H
