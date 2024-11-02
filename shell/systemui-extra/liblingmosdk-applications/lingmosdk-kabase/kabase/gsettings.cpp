/*
 * liblingmosdk-base's Library
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

#include "gsettings.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{
namespace
{
constexpr char lingmoThemeSchemasId[] = "org.lingmo.style";
constexpr char lingmoControlCenterPersonaliseSchemasId[] = "org.lingmo.control-center.personalise";
constexpr char themeFlag[] = "_ThemeFlag";
constexpr char controlCenterPersonaliseFlag[] = "_ControlCenterPersonaliseFlag";
constexpr char systemFontSizeKey[] = "systemFontSize";
constexpr char systemTesnsparencyKey[] = "transparency";
constexpr char systemThemeKey[] = "styleName";

QHash<QString, QGSettings *> g_gsettings;
Gsettings _gsettings;
} // namespace

Gsettings::Gsettings()
{
    if (Gsettings::registerObject(themeFlag, lingmoThemeSchemasId)) {
        error << "kabase : register org.lingmo.style gsettings fail";
    }

    if (Gsettings::registerObject(controlCenterPersonaliseFlag, lingmoControlCenterPersonaliseSchemasId)) {
        error << "kabase : register org.lingmo.control-center.personalise gsettings fail";
    }

    this->conn();
}

Gsettings::~Gsettings()
{
    for (QHash<QString, QGSettings *>::iterator i = g_gsettings.begin(); i != g_gsettings.end(); i++) {
        QGSettings *p = i.value();
        if (p != nullptr) {
            delete p;
            p = nullptr;
        }
    }

    g_gsettings.clear();
}

Gsettings *Gsettings::getPoint(void)
{
    return &_gsettings;
}

int Gsettings::registerObject(QString flag, QByteArray schemasId)
{
    QGSettings *p = nullptr;

    if (g_gsettings.contains(flag)) {
        error << "kabase : flag repeat !";
        return -1;
    }

    if (QGSettings::isSchemaInstalled(schemasId)) {
        p = new QGSettings(schemasId);
    } else {
        error << "kabase : schemasId not fount !";
        return -1;
    }

    g_gsettings.insert(flag, p);

    return 0;
}

int Gsettings::cancelRegister(QString flag)
{
    if (g_gsettings.contains(flag)) {
        if (g_gsettings.value(flag) != nullptr) {
            delete g_gsettings.value(flag);
            g_gsettings.remove(flag);
        }
    } else {
        error << "kabase : flag not fount !";
        return -1;
    }

    return 0;
}

QVariant Gsettings::get(QString flag, QString key)
{
    QGSettings *p = nullptr;
    QVariant ret;
    ret.clear();

    if (!g_gsettings.contains(flag)) {
        error << "kabase : flag not fount !";
        return ret;
    }

    p = g_gsettings.value(flag);

    if (p->keys().contains(key)) {
        ret = p->get(key);
    } else {
        error << "kabase : key not fount !";
        return ret;
    }

    return ret;
}

int Gsettings::set(QString flag, QString key, QVariant value)
{
    QGSettings *p = nullptr;

    if (!g_gsettings.contains(flag)) {
        error << "kabase : flag not fount !";
        return -1;
    }

    p = g_gsettings.value(flag);

    if (p->keys().contains(key)) {
        p->set(key, value);
    } else {
        error << "kabase : key not fount !";
        return -1;
    }

    return 0;
}

int Gsettings::reset(QString flag, QString key)
{
    QGSettings *p = nullptr;

    if (!g_gsettings.contains(flag)) {
        error << "kabase : flag not fount !";
        return -1;
    }

    p = g_gsettings.value(flag);

    if (p->keys().contains(key)) {
        p->reset(key);
    } else {
        error << "kabase : key not fount !";
        return -1;
    }

    return 0;
}

QStringList Gsettings::keys(QString flag)
{
    QGSettings *p = nullptr;
    QStringList ret;
    ret.clear();

    if (!g_gsettings.contains(flag)) {
        error << "kabase : flag not fount !";
        return ret;
    }

    p = g_gsettings.value(flag);

    ret = p->keys();

    return ret;
}

QVariant Gsettings::getSystemFontSize(void)
{
    QGSettings *p = nullptr;
    QVariant ret;
    ret.clear();

    if (g_gsettings.contains(QString(themeFlag))) {
        p = g_gsettings.value(QString(themeFlag));
    }

    if (p->keys().contains(QString(systemFontSizeKey))) {
        ret = p->get(QString(systemFontSizeKey));
    }

    return ret;
}

QVariant Gsettings::getSystemTransparency(void)
{
    QGSettings *p = nullptr;
    QVariant ret;
    ret.clear();

    if (g_gsettings.contains(QString(controlCenterPersonaliseFlag))) {
        p = g_gsettings.value(QString(controlCenterPersonaliseFlag));
    }

    if (p->keys().contains(QString(systemTesnsparencyKey))) {
        ret = p->get(QString(systemTesnsparencyKey));
    }

    return ret;
}

QVariant Gsettings::getSystemTheme(void)
{
    QGSettings *p = nullptr;
    QVariant ret;
    ret.clear();

    if (g_gsettings.contains(QString(themeFlag))) {
        p = g_gsettings.value(QString(themeFlag));
    }

    if (p->keys().contains(QString(systemThemeKey))) {
        ret = p->get(QString(systemThemeKey));
    }

    return ret;
}

void Gsettings::conn(void)
{
    QGSettings *theme = nullptr;
    QGSettings *controlCentePersonaliser = nullptr;

    if (g_gsettings.contains(themeFlag)) {
        theme = g_gsettings.value(themeFlag);
    }

    if (g_gsettings.contains(controlCenterPersonaliseFlag)) {
        controlCentePersonaliser = g_gsettings.value(controlCenterPersonaliseFlag);
    }

    if (theme != nullptr) {
        QObject::connect(theme, &QGSettings::changed, this, &Gsettings::slotThemeChange);
    }

    if (controlCentePersonaliser != nullptr) {
        QObject::connect(controlCentePersonaliser, &QGSettings::changed, this,
                         &Gsettings::slotControlCenterPersonaliseChange);
    }

    return;
}

void Gsettings::slotThemeChange(QString key)
{
    if (key == QString(systemThemeKey)) {
        Q_EMIT this->systemThemeChange();
    } else if (key == QString(systemFontSizeKey)) {
        Q_EMIT this->systemFontSizeChange();
    }

    return;
}

void Gsettings::slotControlCenterPersonaliseChange(QString key)
{
    if (key == QString(systemTesnsparencyKey)) {
        Q_EMIT this->systemTransparencyChange();
    }

    return;
}


} /* namespace kabase */
} /* namespace kdk */
