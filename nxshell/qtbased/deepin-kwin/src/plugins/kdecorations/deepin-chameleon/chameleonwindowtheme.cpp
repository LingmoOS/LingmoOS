/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "chameleonwindowtheme.h"
#include "chameleontheme.h"

#include "kwinutils.h"

#include <QDebug>
#include <QGuiApplication>
#include <QX11Info>

ChameleonWindowTheme::ChameleonWindowTheme(QObject *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
{
    if (!window)
        return;

    updateScreen();

    bool ok = false;
    quint32 window_id = KWinUtils::instance()->getWindowId(window, &ok);

    if (!ok)
        return;

    // 将ChameleonWindowTheme对象的属性绑定到对应x11窗口的settings属性
    ok = KWinUtils::instance()->buildNativeSettings(this, window_id);
    Q_UNUSED(ok);
}

ChameleonWindowTheme::PropertyFlags ChameleonWindowTheme::validProperties() const
{
    return m_validProperties;
}

bool ChameleonWindowTheme::propertyIsValid(ChameleonWindowTheme::PropertyFlag p) const
{
    return m_validProperties.testFlag(p);
}

QString ChameleonWindowTheme::theme() const
{
    return property("theme").toString();
}

QPointF ChameleonWindowTheme::windowRadius() const
{
    if(!QX11Info::isPlatformX11()) {
        return m_windowRadius;
    }
    return ChameleonTheme::takePos(property("windowRadius"), QPointF(0.0, 0.0));
}

void ChameleonWindowTheme::setWindowRadius(const QPointF value) {
    this->m_windowRadius = value;
}

qreal ChameleonWindowTheme::borderWidth() const
{
    return property("borderWidth").toDouble();
}

QColor ChameleonWindowTheme::borderColor() const
{
    return qvariant_cast<QColor>(property("borderColor"));
}

qreal ChameleonWindowTheme::shadowRadius() const
{
    return property("shadowRadius").toDouble();
}

QPointF ChameleonWindowTheme::shadowOffset() const
{
    return ChameleonTheme::takePos(property("shadowOffect"), QPointF(0.0, 0.0));
}

QColor ChameleonWindowTheme::shadowColor() const
{
    return qvariant_cast<QColor>(property("shadowColor"));
}

QMarginsF ChameleonWindowTheme::mouseInputAreaMargins() const
{
    return ChameleonTheme::takeMargins(property("mouseInputAreaMargins"), QMarginsF(0, 0, 0, 0));
}

qreal ChameleonWindowTheme::windowPixelRatio() const
{
    return m_validProperties.testFlag(WindowPixelRatioProperty) ? property("windowPixelRatio").toDouble() : m_windowPixelRatio ;
}

void ChameleonWindowTheme::setValidProperties(qint64 validProperties)
{
    if (m_validProperties == validProperties)
        return;

    PropertyFlags p = PropertyFlag(validProperties);

    if (m_validProperties.testFlag(WindowPixelRatioProperty) && !p.testFlag(WindowPixelRatioProperty)) {
        Q_EMIT windowPixelRatioChanged();
    }

    m_validProperties = p;
    Q_EMIT validPropertiesChanged(m_validProperties);
}

void ChameleonWindowTheme::updateScreen()
{
    QScreen *screen = nullptr;

    if (m_window) {
        bool ok = false;
        int screen_index = m_window->property("screen").toInt(&ok);

        if (ok) {
            screen = qGuiApp->screens().value(screen_index);
        }
    }

    if (!screen) {
        screen = qGuiApp->primaryScreen();
    }

    if (m_screen == screen) {
        return;
    }

    if (m_screen) {
        disconnect(m_screen, &QScreen::logicalDotsPerInchChanged, this, &ChameleonWindowTheme::updateScreenScale);
        disconnect(m_screen, &QScreen::destroyed, this, &ChameleonWindowTheme::updateScreen);
    }

    m_screen = screen;

    connect(m_screen, &QScreen::logicalDotsPerInchChanged, this, &ChameleonWindowTheme::updateScreenScale);
    connect(m_screen, &QScreen::destroyed, this, &ChameleonWindowTheme::updateScreen);

    updateScreenScale();
}

void ChameleonWindowTheme::updateScreenScale()
{
    qreal scale = m_screen->logicalDotsPerInch() / 96.0f;

    if (qFuzzyCompare(scale, m_windowPixelRatio))
        return;

    m_windowPixelRatio = scale;

    if (!m_validProperties.testFlag(WindowPixelRatioProperty)) {
        Q_EMIT windowPixelRatioChanged();
    }
}
