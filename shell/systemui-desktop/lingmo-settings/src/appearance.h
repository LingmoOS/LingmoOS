/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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
#include <QDBusInterface>

class Appearance : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY dockIconSizeChanged)
    Q_PROPERTY(int dockDirection READ dockDirection WRITE setDockDirection NOTIFY dockDirectionChanged)
    Q_PROPERTY(int dockVisibility READ dockVisibility WRITE setDockVisibility NOTIFY dockVisibilityChanged)
    Q_PROPERTY(int dockStyle READ dockStyle WRITE setDockStyle NOTIFY dockStyleChanged)
    Q_PROPERTY(int fontPointSize READ fontPointSize WRITE setFontPointSize NOTIFY fontPointSizeChanged)
    Q_PROPERTY(int DesktopSettings READ DesktopSettings WRITE setDesktopIcons NOTIFY desktopIconsChanged)
    Q_PROPERTY(bool dimsWallpaper READ dimsWallpaper WRITE setDimsWallpaper NOTIFY dimsWallpaperChanged)
    Q_PROPERTY(double devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY devicePixelRatioChanged)
    Q_PROPERTY(bool dockRoundedWindow READ dockRoundedWindow WRITE setDockRoundedWindow NOTIFY dockRoundedWindowChanged)
    Q_PROPERTY(bool systemEffects READ systemEffects WRITE setSystemEffects NOTIFY systemEffectsChanged)
    Q_PROPERTY(int minimiumAnimation READ minimiumAnimation WRITE setMinimiumAnimation NOTIFY minimiumAnimationChanged)

public:
    explicit Appearance(QObject *parent = nullptr);

    Q_INVOKABLE void switchDarkMode(bool darkMode);

    bool dimsWallpaper() const;
    Q_INVOKABLE void setDimsWallpaper(bool value);

    int dockIconSize() const;
    Q_INVOKABLE void setDockIconSize(int dockIconSize);

    int dockDirection() const;
    Q_INVOKABLE void setDockDirection(int dockDirection);

    int dockVisibility() const;
    Q_INVOKABLE void setDockVisibility(int visibility);

    int dockRoundedWindow() const;
    Q_INVOKABLE void setDockRoundedWindow(bool enable);

    int dockStyle() const;
    Q_INVOKABLE void setDockStyle(int style);

    Q_INVOKABLE void setGenericFontFamily(const QString &name);
    Q_INVOKABLE void setFixedFontFamily(const QString &name);

    int fontPointSize() const;
    Q_INVOKABLE void setFontPointSize(int fontPointSize);

    Q_INVOKABLE void setAccentColor(int defaultColor);

    int DesktopSettings() const;
    Q_INVOKABLE void setDesktopIcons(int DesktopSettings);

    double devicePixelRatio() const;
    Q_INVOKABLE void setDevicePixelRatio(double value);

    bool systemEffects() const;
    void setSystemEffects(bool systemEffects);

    int minimiumAnimation() const;
    void setMinimiumAnimation(int minimiumAnimation);

signals:
    void dockIconSizeChanged();
    void dockDirectionChanged();
    void dockVisibilityChanged();
    void dockStyleChanged();
    void fontPointSizeChanged();
    void desktopIconsChanged();
    void dimsWallpaperChanged();
    void devicePixelRatioChanged();
    void dockRoundedWindowChanged();
    void systemEffectsChanged();
    void minimiumAnimationChanged();

private:
    QDBusInterface m_interface;
    QSettings *m_dockSettings;
    QSettings *m_kwinSettings;
    bool m_dockRoundedWindow;

    int m_dockIconSize;
    int m_dockDirection;
    int m_dockVisibility;
    int m_dockStyle;
    int m_fontPointSize;

    int m_DesktopSettings;

    bool m_systemEffects;
    int m_minimiumAnimation;
};

#endif // APPEARANCE_H
