/*
 * Copyright (C) 2023-2024 LingmoOS Team.
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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QSettings>

class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDarkMode READ isDarkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(bool darkModeDimsWallpaer READ darkModeDimsWallpaer WRITE setDarkModeDimsWallpaer NOTIFY darkModeDimsWallpaerChanged)
    Q_PROPERTY(bool backgroundVisible READ backgroundVisible WRITE setBackgroundVisible NOTIFY backgroundVisibleChanged)
    Q_PROPERTY(QString systemFont READ systemFont WRITE setSystemFont NOTIFY systemFontChanged)
    Q_PROPERTY(QString systemFixedFont READ systemFixedFont WRITE setSystemFixedFont)
    Q_PROPERTY(qreal systemFontPointSize READ systemFontPointSize WRITE setSystemFontPointSize NOTIFY systemFontPointSizeChanged)
    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio)
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY wallpaperChanged)
    Q_PROPERTY(int accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int backgroundType READ backgroundType WRITE setBackgroundType NOTIFY backgroundTypeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString color0 READ color0 WRITE setColor0 NOTIFY colorChanged)
    Q_PROPERTY(QString color1 READ color1 WRITE setColor1 NOTIFY colorChanged)
    Q_PROPERTY(QString color2 READ color2 WRITE setColor2 NOTIFY colorChanged)
    Q_PROPERTY(QString color3 READ color3 WRITE setColor3 NOTIFY colorChanged)
    Q_PROPERTY(QString color4 READ color4 WRITE setColor4 NOTIFY colorChanged)
    Q_PROPERTY(QString color5 READ color5 WRITE setColor5 NOTIFY colorChanged)
    Q_PROPERTY(QString color6 READ color6 WRITE setColor6 NOTIFY colorChanged)
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setCursorTheme NOTIFY cursorThemeChanged)
    Q_PROPERTY(int cursorSize READ cursorSize WRITE setCursorSize NOTIFY cursorSizeChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(QString darkIconTheme READ darkIconTheme WRITE setDarkIconTheme NOTIFY iconThemeChanged)

public:
    static ThemeManager *self();

    ThemeManager(QObject *parent = nullptr);

    bool isDarkMode();
    void setDarkMode(bool darkMode);

    bool darkModeDimsWallpaer() const;
    void setDarkModeDimsWallpaer(bool value);

    bool backgroundVisible() const;
    void setBackgroundVisible(bool value);

    QString color0() const;
    void setColor0(const QString &color);
    QString color1() const;
    void setColor1(const QString &color);
    QString color2() const;
    void setColor2(const QString &color);
    QString color3() const;
    void setColor3(const QString &color);
    QString color4() const;
    void setColor4(const QString &color);
    QString color5() const;
    void setColor5(const QString &color);
    QString color6() const;
    void setColor6(const QString &color);
    QString systemFont();
    void setSystemFont(const QString &fontFamily);

    QString systemFixedFont();
    void setSystemFixedFont(const QString &fontFamily);

    qreal systemFontPointSize();
    void setSystemFontPointSize(qreal fontSize);

    qreal devicePixelRatio();
    void setDevicePixelRatio(qreal ratio);

    QString wallpaper();
    void setWallpaper(const QString &path);

    int backgroundType();
    void setBackgroundType(int type);

    QString backgroundColor();
    void setBackgroundColor(QString color);

    int accentColor();
    void setAccentColor(int accentColor);

    QString cursorTheme() const;
    void setCursorTheme(const QString &theme);

    int cursorSize() const;
    void setCursorSize(int size);

    void updateGtk2Config();
    void updateGtk3Config();
    void applyXResources();
    void applyCursor();

    QString iconTheme() const;
    void setIconTheme(const QString &iconTheme);

    QString darkIconTheme() const;
    void setDarkIconTheme(const QString &iconTheme);

    void updateFontConfig();

signals:
    void darkModeChanged(bool darkMode);
    void wallpaperChanged(QString path);
    void colorChanged();
    void darkModeDimsWallpaerChanged();
    void backgroundVisibleChanged();
    void accentColorChanged(int accentColor);
    void backgroundTypeChanged();
    void backgroundColorChanged();
    void cursorThemeChanged();
    void cursorSizeChanged();
    void iconThemeChanged();
    void systemFontPointSizeChanged();
    void systemFontChanged();

private:
    QSettings *m_settings;

    bool m_isDarkMode;
    bool m_darkModeDimsWallpaer;
    bool m_backgroundVisible;
    QString m_wallpaperPath;
    int m_backgroundType;
    QString m_Color0;
    QString m_Color1;
    QString m_Color2;
    QString m_Color3;
    QString m_Color4;
    QString m_Color5;
    QString m_Color6;
    QString m_backgroundColor;
    int m_accentColor;

    QString m_cursorTheme;
    int m_cursorSize;

    QString m_iconTheme = "Crule";

    QString m_darkIconTheme = "Crule-dark";
};

#endif
