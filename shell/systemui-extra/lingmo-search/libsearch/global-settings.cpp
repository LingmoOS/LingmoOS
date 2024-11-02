/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangzihao <zhangzihao@kylinos.cn>
 * Modified by: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */

#include "global-settings.h"
#include "global-settings-private.h"
#include <QPalette>
#include <QApplication>
#include <QDBusReply>
#include <QDebug>

#define CONTROL_CENTER_PERSONALISE_GSETTINGS_ID "org.lingmo.control-center.personalise"
#define THEME_GSETTINGS_ID "org.lingmo.style"

using namespace LingmoUISearch;

GlobalSettingsPrivate::GlobalSettingsPrivate(QObject *parent) : QObject(parent)
{
    //全局页面透明度
    //the default number of transparency for mainwindow is 0.7
    setValue(TRANSPARENCY_KEY, 0.7);
    if(QGSettings::isSchemaInstalled(CONTROL_CENTER_PERSONALISE_GSETTINGS_ID)) {
        m_transGsettings = new QGSettings(CONTROL_CENTER_PERSONALISE_GSETTINGS_ID, QByteArray(), this);
        connect(m_transGsettings, &QGSettings::changed, this, [ = ](const QString & key) {
            if(key == TRANSPARENCY_KEY) {
                double transparency = m_transGsettings->get(TRANSPARENCY_KEY).toDouble();
                setValue(TRANSPARENCY_KEY, transparency);
                qApp->paletteChanged(qApp->palette());
                Q_EMIT this->valueChanged(TRANSPARENCY_KEY, transparency);
            }
        });
        if(m_transGsettings->keys().contains(TRANSPARENCY_KEY)) {
            setValue(TRANSPARENCY_KEY, m_transGsettings->get(TRANSPARENCY_KEY).toDouble());
        }
    }

    //主题，字体大小
    setValue(STYLE_NAME_KEY, "lingmo-light");
    setValue(FONT_SIZE_KEY, 11);
    setValue(WINDOW_RADIUS_KEY, 12);
    if(QGSettings::isSchemaInstalled(THEME_GSETTINGS_ID)) {
        m_themeGsettings = new QGSettings(THEME_GSETTINGS_ID, QByteArray(), this);
        connect(m_themeGsettings, &QGSettings::changed, this, [ = ](const QString & key) {
            if(key == STYLE_NAME_KEY) {
                //当前主题改变时也发出paletteChanged信号，通知主界面刷新
                setValue(STYLE_NAME_KEY, m_themeGsettings->get(STYLE_NAME_KEY).toString());
                qApp->paletteChanged(qApp->palette());
                Q_EMIT this->valueChanged(STYLE_NAME_KEY, m_themeGsettings->get(STYLE_NAME_KEY).toString());
            } else if(key == FONT_SIZE_KEY) {
                setValue(FONT_SIZE_KEY, m_themeGsettings->get(FONT_SIZE_KEY).toDouble());
                qApp->paletteChanged(qApp->palette());
            } else if (key == ICON_THEME_KEY) {
                qApp->paletteChanged(qApp->palette());
            } else if (key == WINDOW_RADIUS_KEY) {
                qDebug() << WINDOW_RADIUS_KEY << m_themeGsettings->get(WINDOW_RADIUS_KEY).toInt();
                setValue(WINDOW_RADIUS_KEY, m_themeGsettings->get(WINDOW_RADIUS_KEY).toInt());
                qApp->paletteChanged(qApp->palette());
                Q_EMIT this->valueChanged(WINDOW_RADIUS_KEY, m_themeGsettings->get(WINDOW_RADIUS_KEY).toInt());
            }
        });
        if(m_themeGsettings->keys().contains(STYLE_NAME_KEY)) {
            setValue(STYLE_NAME_KEY, m_themeGsettings->get(STYLE_NAME_KEY).toString());
        }
        if(m_themeGsettings->keys().contains(FONT_SIZE_KEY)) {
            setValue(FONT_SIZE_KEY, m_themeGsettings->get(FONT_SIZE_KEY).toDouble());
        }
        if(m_themeGsettings->keys().contains(WINDOW_RADIUS_KEY)) {
            setValue(WINDOW_RADIUS_KEY, m_themeGsettings->get(WINDOW_RADIUS_KEY).toInt());
        }
    }
}

QVariant GlobalSettingsPrivate::getValue(const QString &key) {

    m_mutex.lock();
    QVariant value =  m_cache.value(key);
    m_mutex.unlock();
    return value;
}

//this method is designed for main process settings only!!
void GlobalSettingsPrivate::setValue(const QString &key, const QVariant &value) {
    m_mutex.lock();
    m_cache.insert(key, value);
    m_mutex.unlock();
}

GlobalSettings &GlobalSettings::getInstance() {
    static GlobalSettings instance;
    return instance;
}

const QVariant GlobalSettings::getValue(const QString& key)
{
    return d->getValue(key);
}

GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent), d(new GlobalSettingsPrivate(this))
{
    connect(d, &GlobalSettingsPrivate::valueChanged, this, &GlobalSettings::valueChanged);
}
