/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_MENU_SETTINGS_H
#define LINGMO_MENU_SETTINGS_H

#include <QObject>
#include <QGSettings>
#include <QVariant>
#include <QMutex>
#include <QMutexLocker>

#define LINGMO_STYLE_VALUE_DARK    "lingmo-dark"
#define LINGMO_STYLE_VALUE_LIGHT   "lingmo-light"
#define LINGMO_STYLE_VALUE_DEFAULT "lingmo-default"

// lingmo-menu settings
#define LINGMO_MENU_ANIMATION_DURATION "animationDuration"
#define FOLLOW_LINGMO_PANEL            "followLingmoUIPanel"
#define MENU_WIDTH                   "width"
#define MENU_HEIGHT                  "height"
#define MENU_MARGIN                  "margin"

namespace LingmoUIMenu {

class SettingModule
{
public:
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
};

class GlobalSetting : public QObject
{
    Q_OBJECT
public:
    enum Key {
        UnKnowKey = 0,
        StyleName,
        ThemeColor,
        IconThemeName,
        Transparency,
        EffectEnabled,
        SystemFontSize,
        IsLiteMode
    };
    Q_ENUM(Key)

    static GlobalSetting *instance();
    GlobalSetting() = delete;
    GlobalSetting(const GlobalSetting& obj) = delete;
    GlobalSetting(const GlobalSetting&& obj) = delete;

    QVariant get(const GlobalSetting::Key& key);

    bool isSystemApp(const QString &appid) const;
    QStringList systemApps() const;

    bool isDefaultFavoriteApp(const QString &appid) const;
    QStringList defaultFavoriteApps() const;

Q_SIGNALS:
    void styleChanged(const GlobalSetting::Key& key);

private Q_SLOTS:
    void onSysModeChanged(QMap<QString, QVariant> map);

private:
    explicit GlobalSetting(QObject *parent = nullptr);
    void initStyleSetting();
    void initGlobalSettings();
    void initUSDSetting();
    void updateData(const GlobalSetting::Key& key, const QVariant &value);

private:
    QMutex mutex;
    QMap<GlobalSetting::Key, QVariant> m_cache;
    QStringList m_systemApps;
    QStringList m_defaultFavoriteApps;
};

class MenuSetting : public QObject
{
    Q_OBJECT
public:
    static MenuSetting *instance();
    MenuSetting() = delete;
    MenuSetting(const MenuSetting& obj) = delete;
    MenuSetting(const MenuSetting&& obj) = delete;

    Q_INVOKABLE QVariant get(const QString& key);
    Q_INVOKABLE bool set(const QString& key, const QVariant& value);

Q_SIGNALS:
    void changed(const QString& key);

private Q_SLOTS:
    void updateValue(const QString& key);

private:
    explicit MenuSetting(QObject *parent = nullptr);
    void initSettings();

private:
    QMutex mutex;
    QMap<QString, QVariant> m_cache;
    QGSettings *m_gSettings{nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_SETTINGS_H
