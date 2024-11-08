/*
 * liblingmosdk-ukenv's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QGSettings/QGSettings>
#include <QHash>
#include <QDebug>

#include "gsettingmonitor.h"

namespace kdk
{
namespace
{
constexpr char themeSchemasId[] = "org.lingmo.style";
constexpr char controlCenterPersonaliseSchemasId[] = "org.lingmo.control-center.personalise";

constexpr char systemFontSizeKey[] = "systemFontSize";
constexpr char systemTransparencyKey[] = "transparency";
constexpr char systemThemeKey[] = "styleName";

constexpr char themeFlag[] = "__themeFlag";
constexpr char controlCenterPersonaliseFlag[] = "__controlCenterPersonaliseFlag";

QHash<QString, QGSettings *> g_gsettings;
} // namespace

GsettingMonitor GsettingMonitor::m_gsettingMonitor;

GsettingMonitor::GsettingMonitor()
{
    if (!GsettingMonitor::registerGsetting(themeFlag, themeSchemasId)) {
        qCritical() << "kdk : register org.lingmo.style gsetting fail!";
    }

    if (!GsettingMonitor::registerGsetting(controlCenterPersonaliseFlag, controlCenterPersonaliseSchemasId)) {
        qCritical() << "kdk : register org.lingmo.control-center.personalise gsetting fail!";
    }

    conn();
}

GsettingMonitor::~GsettingMonitor()
{
    for (QHash<QString, QGSettings *>::iterator it = g_gsettings.begin(); it != g_gsettings.end(); it++) {
        QGSettings *s = it.value();
        if (s != nullptr) {
            delete s;
            s = nullptr;
        }
    }

    g_gsettings.clear();
}

GsettingMonitor *GsettingMonitor::getInstance(void)
{
    return &m_gsettingMonitor;
}

bool GsettingMonitor::registerGsetting(QString flag, QByteArray schemasId)
{
    QGSettings *s = nullptr;

    if (g_gsettings.contains(flag)) {
        qCritical() << "kdk : gsettings flag repeat!";
        return false;
    }

    if (QGSettings::isSchemaInstalled(schemasId)) {
        s = new QGSettings(schemasId);
    } else {
        qCritical() << "kdk : gsettings schemasId not fount!";
        return false;
    }

    g_gsettings.insert(flag, s);

    return true;
}

QVariant GsettingMonitor::getSystemFontSize(void)
{
    QGSettings *s = nullptr;

    if (g_gsettings.contains(themeFlag)) {
        s = g_gsettings.value(themeFlag);
    }

    if (s != nullptr && s->keys().contains(systemFontSizeKey)) {
        return s->get(systemFontSizeKey);
    }

    return QVariant();
}

QVariant GsettingMonitor::getSystemTransparency(void)
{
    QGSettings *s = nullptr;

    if (g_gsettings.contains(controlCenterPersonaliseFlag)) {
        s = g_gsettings.value(controlCenterPersonaliseFlag);
    }

    if (s != nullptr && s->keys().contains(systemTransparencyKey)) {
        return s->get(systemTransparencyKey);
    }

    return QVariant();
}

QVariant GsettingMonitor::getSystemTheme(void)
{
    QGSettings *s = nullptr;

    if (g_gsettings.contains(themeFlag)) {
        s = g_gsettings.value(themeFlag);
    }

    if (s != nullptr && s->keys().contains(systemThemeKey)) {
        return s->get(systemThemeKey);
    }

    return QVariant();
}

void GsettingMonitor::conn(void)
{
    QGSettings *theme = nullptr;
    QGSettings *control = nullptr;

    if (g_gsettings.contains(themeFlag)) {
        theme = g_gsettings.value(themeFlag);
    }

    if (g_gsettings.contains(controlCenterPersonaliseFlag)) {
        control = g_gsettings.value(controlCenterPersonaliseFlag);
    }

    if (theme != nullptr) {
        QObject::connect(theme, &QGSettings::changed, this, &GsettingMonitor::themeChange);
    }

    if (control != nullptr) {
        QObject::connect(control, &QGSettings::changed, this, &GsettingMonitor::controlCenterPersonaliseChange);
    }

    return;
}

void GsettingMonitor::themeChange(QString key)
{
    if (key == QString(systemThemeKey)) {
        Q_EMIT systemThemeChange();
    } else if (key == QString(systemFontSizeKey)) {
        Q_EMIT systemFontSizeChange();
    }

    return;
}

void GsettingMonitor::controlCenterPersonaliseChange(QString key)
{
    if (key == QString(systemTransparencyKey)) {
        Q_EMIT systemTransparencyChange();
    }

    return;
}

} // namespace kdk
