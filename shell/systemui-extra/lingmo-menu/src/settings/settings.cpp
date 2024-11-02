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

#include "settings.h"

#include <QVariant>
#include <QDebug>
#include <QtQml>
#include <QDBusInterface>
#include <QDBusReply>

#define LINGMO_MENU_SCHEMA                "org.lingmo.menu.settings"
#define CONTROL_CENTER_SETTING          "org.lingmo.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"
#define CONTROL_CENTER_EFFECT_KEY       "effect"

#define LINGMO_STYLE_SCHEMA               "org.lingmo.style"
#define LINGMO_STYLE_NAME_KEY             "styleName"
#define LINGMO_STYLE_THEME_COLOR_KEY      "themeColor"
#define LINGMO_STYLE_SYSTEM_FONT_SIZE     "systemFontSize"
#define LINGMO_STYLE_ICON_THEME_NAME_KEY  "iconThemeName"

namespace LingmoUIMenu {

void SettingModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qRegisterMetaType<GlobalSetting::Key>("GlobalSetting::Key");
    qmlRegisterUncreatableType<GlobalSetting>(uri, versionMajor, versionMinor, "GlobalSetting", "Use enums only.");
}

GlobalSetting *GlobalSetting::instance()
{
    static GlobalSetting setting(nullptr);
    return &setting;
}

GlobalSetting::GlobalSetting(QObject *parent) : QObject(parent)
{
    initStyleSetting();
    initGlobalSettings();
    initUSDSetting();
}

void GlobalSetting::initStyleSetting()
{
    m_cache.insert(StyleName, LINGMO_STYLE_VALUE_LIGHT);
    m_cache.insert(ThemeColor, LINGMO_STYLE_THEME_COLOR_KEY);
    m_cache.insert(SystemFontSize, LINGMO_STYLE_SYSTEM_FONT_SIZE);
    m_cache.insert(Transparency, 1);
    m_cache.insert(EffectEnabled, false);

    if (QGSettings::isSchemaInstalled(LINGMO_STYLE_SCHEMA)) {
        QGSettings *settings = new QGSettings(LINGMO_STYLE_SCHEMA, {}, this);

        QStringList keys = settings->keys();
        if (keys.contains(LINGMO_STYLE_NAME_KEY)) {
            m_cache.insert(StyleName, settings->get(LINGMO_STYLE_NAME_KEY));
        }
        if (keys.contains(LINGMO_STYLE_THEME_COLOR_KEY)) {
            m_cache.insert(ThemeColor,settings->get(LINGMO_STYLE_THEME_COLOR_KEY));
        }
        if (keys.contains(LINGMO_STYLE_SYSTEM_FONT_SIZE)) {
            m_cache.insert(SystemFontSize,settings->get(LINGMO_STYLE_SYSTEM_FONT_SIZE));
        }
        connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == LINGMO_STYLE_NAME_KEY) {
                updateData(StyleName, settings->get(key));
                Q_EMIT styleChanged(StyleName);
            } else if (key == LINGMO_STYLE_THEME_COLOR_KEY) {
                updateData(ThemeColor, settings->get(key));
                Q_EMIT styleChanged(ThemeColor);
            } else if (key == LINGMO_STYLE_SYSTEM_FONT_SIZE) {
                updateData(SystemFontSize, settings->get(key));
                Q_EMIT styleChanged(SystemFontSize);
            } else if (key == LINGMO_STYLE_ICON_THEME_NAME_KEY) {
                updateData(IconThemeName, settings->get(key));
                Q_EMIT styleChanged(IconThemeName);
            }
        });
    }

    if (QGSettings::isSchemaInstalled(CONTROL_CENTER_SETTING)) {
        QGSettings *settings = new QGSettings(CONTROL_CENTER_SETTING,{},this);

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            m_cache.insert(Transparency, settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal());
        }
        if (keys.contains(CONTROL_CENTER_EFFECT_KEY)) {
            m_cache.insert(EffectEnabled, settings->get(CONTROL_CENTER_EFFECT_KEY).toBool());
        }

        connect(settings, &QGSettings::changed, this, [this, settings] (const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY) {
                updateData(Transparency, settings->get(key).toReal());
                Q_EMIT styleChanged(Transparency);
            }
            if (key == CONTROL_CENTER_EFFECT_KEY) {
                updateData(EffectEnabled, settings->get(key).toReal());
                Q_EMIT styleChanged(EffectEnabled);
            }
        });
    }
}

QVariant GlobalSetting::get(const GlobalSetting::Key& key)
{
    QMutexLocker locker(&mutex);
    return m_cache.value(key);
}

void GlobalSetting::updateData(const GlobalSetting::Key &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    m_cache.insert(key, value);
}

bool GlobalSetting::isSystemApp(const QString &appid) const
{
    return std::any_of(m_systemApps.constBegin(), m_systemApps.constEnd(), [&appid](const QString &appName) {
        return appid.contains(appName);
    });
}

QStringList GlobalSetting::systemApps() const
{
    return m_systemApps;
}

bool GlobalSetting::isDefaultFavoriteApp(const QString &appid) const
{
    return std::any_of(m_defaultFavoriteApps.constBegin(), m_defaultFavoriteApps.constEnd(), [&appid](const QString &id) {
        return appid == id;
    });
}

QStringList GlobalSetting::defaultFavoriteApps() const
{
    return m_defaultFavoriteApps;
}

void GlobalSetting::initGlobalSettings()
{
    if (QFile::exists(LINGMO_MENU_GLOBAL_CONFIG_FILE)) {
        QSettings settings(LINGMO_MENU_GLOBAL_CONFIG_FILE, QSettings::IniFormat);
        settings.beginGroup("System Apps");
        m_systemApps.append(settings.allKeys());
        settings.endGroup();

        //=
        settings.beginGroup("Default Favorite Apps");
        QMap<int, QString> apps;
        for (const auto &key : settings.allKeys()) {
            apps.insert(key.toInt(), settings.value(key).toString());
        }

        for (const auto &app : apps) {
            m_defaultFavoriteApps.append(app);
        }
        settings.endGroup();
    }
}

void GlobalSetting::initUSDSetting()
{
    m_cache.insert(IsLiteMode, false);

    const QString service = QStringLiteral("org.lingmo.SettingsDaemon");
    const QString path = QStringLiteral("/GlobalSignal");
    const QString interface = QStringLiteral("org.lingmo.SettingsDaemon.GlobalSignal");

    QDBusInterface dBusInterface(service, path, interface);
    if (dBusInterface.isValid()) {
        QDBusReply<QString> reply = dBusInterface.call(QStringLiteral("getLINGMOLiteAnimation"));
        if (reply.isValid()) {
            QMap<QString, QVariant> m;
            m.insert("animation", reply.value());
            onSysModeChanged(m);
        }
    }

    QDBusConnection::sessionBus().connect(service, path, interface, "LINGMOLiteChanged", this, SLOT(onSysModeChanged));
}

void GlobalSetting::onSysModeChanged(QMap<QString, QVariant> map)
{
    if (map.contains("animation")) {
        m_cache.insert(IsLiteMode, (map.value("animation").toString() == QStringLiteral("lite")));
        Q_EMIT styleChanged(IsLiteMode);
    }
}

MenuSetting *MenuSetting::instance()
{
    static MenuSetting setting(nullptr);
    return &setting;
}

MenuSetting::MenuSetting(QObject *parent) : QObject(parent)
{
    initSettings();
}

void MenuSetting::initSettings()
{
    m_cache.insert(LINGMO_MENU_ANIMATION_DURATION, {300});
    m_cache.insert(FOLLOW_LINGMO_PANEL, {true});
    m_cache.insert(MENU_WIDTH, {652});
    m_cache.insert(MENU_HEIGHT, {540});
    m_cache.insert(MENU_MARGIN, {8});

    QByteArray id{LINGMO_MENU_SCHEMA};
    if (QGSettings::isSchemaInstalled(id)) {
        m_gSettings = new QGSettings(id, "/", this);

        if (!m_gSettings) {
            qWarning() << "Unable to initialize settings.";
            return;
        }

        for (const auto &key: m_gSettings->keys()) {
            m_cache.insert(key, m_gSettings->get(key));
        }

        connect(m_gSettings, &QGSettings::changed, this, &MenuSetting::updateValue);
    }
}

QVariant MenuSetting::get(const QString &key)
{
    QMutexLocker locker(&mutex);
    return m_cache.value(key, {});
}

bool MenuSetting::set(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    if (m_gSettings) {
        m_gSettings->set(key, value);
//        m_cache.insert(key,value);
//        Q_EMIT changed(key);
        return true;
    }

    return false;
}

void MenuSetting::updateValue(const QString &key)
{
    mutex.lock();
    m_cache.insert(key, m_gSettings->get(key));
    mutex.unlock();
    Q_EMIT changed(key);
}

} // LingmoUIMenu
