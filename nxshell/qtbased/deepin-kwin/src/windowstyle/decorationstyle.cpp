/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "decorationstyle.h"
#include "window.h"
#include "workspace.h"
#include "windowstylemanager.h"
#include "utils.h"

#define _DEEPIN_NET_EFFECT   "_DEEPIN_NET_EFFECT"
#define _DEEPIN_NET_STARTUP  "_DEEPIN_NET_STARTUP"
#define _DEEPIN_NET_RADIUS   "_DEEPIN_NET_RADIUS"
#define _DEEPIN_NET_SHADOW   "_DEEPIN_NET_SHADOW"

namespace KWin
{
DecorationStyle::DecorationStyle(Window *window)
{
    static QFunctionPointer build_function = qApp->platformFunction("_d_buildNativeSettings");
    if (build_function) {
        reinterpret_cast<bool(*)(QObject*, quint32)>(build_function)(this, window->window());
    }
}

QPointF DecorationStyle::variant2Point(QVariant value, QPointF defaultV)
{
    QStringList l = value.toString().split(",");

    if (l.count() < 2) {
        return defaultV;
    }

    QPoint p;
    p.setX(l.first().toDouble());
    p.setY(l.at(1).toDouble());

    return p;
}

QMarginsF DecorationStyle::variant2Margins(QVariant value, QMarginsF defaultV)
{
    if (!value.isValid()) {
        return defaultV;
    }

    QStringList l = value.toStringList();

    if (l.isEmpty()) {
        l = value.toString().split(",");
    }

    if (l.count() < 4) {
        return defaultV;
    }

    return QMarginsF(l.at(0).toDouble(), l.at(1).toDouble(),
                     l.at(2).toDouble(), l.at(3).toDouble());
}

void DecorationStyle::setValidProperties(qint64 validProperties)
{
    if (m_validProperties == validProperties)
        return;

    PropertyFlags p = PropertyFlag(validProperties);

    m_validProperties = p;

    Q_EMIT validPropertiesChanged(m_validProperties);
}

/******************************************************/

X11DecorationStyle::X11DecorationStyle(Window *window)
    : DecorationStyle(window)
    , m_window(window)
{
}

X11DecorationStyle::~X11DecorationStyle()
{
}

X11DecorationStyle::PropertyFlags X11DecorationStyle::validProperties()
{
    return m_validProperties;
}

bool X11DecorationStyle::propertyIsValid(PropertyFlag p)
{
    return m_validProperties.testFlag(p);
}

QString X11DecorationStyle::theme()
{
    return property("theme").toString();
}

QPointF X11DecorationStyle::windowRadius()
{
    return variant2Point(property("windowRadius"));
}

void X11DecorationStyle::setWindowRadius(const QPointF value)
{
    QString point = QString::number(value.x()) + "," + QString::number(value.y());
    setProperty("windowRadius", point);
}

qreal X11DecorationStyle::borderWidth()
{
    return property("borderWidth").toDouble();
}

void X11DecorationStyle::setBorderWidth(qreal width)
{
    setProperty("borderWidth", width);
}

QColor X11DecorationStyle::borderColor()
{
    return qvariant_cast<QColor>(property("borderColor"));
}

void X11DecorationStyle::setBorderColor(QColor color)
{
    setProperty("borderColor", QVariant::fromValue(color));
}

qreal X11DecorationStyle::shadowRadius()
{
    return property("shadowRadius").toDouble();
}

QPointF X11DecorationStyle::shadowOffset()
{
    return variant2Point(property("shadowOffset"));
}

QColor X11DecorationStyle::shadowColor()
{
    return qvariant_cast<QColor>(property("shadowColor"));
}

void X11DecorationStyle::setShadowColor(QColor color)
{
    setProperty("shadowColor", QVariant::fromValue(color));
}

QMarginsF X11DecorationStyle::mouseInputAreaMargins()
{
    return variant2Margins(property("mouseInputAreaMargins"));
}

qreal X11DecorationStyle::windowPixelRatio()
{
    return m_validProperties.testFlag(WindowPixelRatioProperty) ? property("windowPixelRatio").toDouble() : 1.0;
}

X11DecorationStyle::effectScenes X11DecorationStyle::windowEffect()
{
    return effectScene(property("windowEffect").toDouble());
}

qreal X11DecorationStyle::windowStartUpEffect()
{
    return property("windowStartUpEffect").toDouble();
}

X11DecorationStyle::effectScenes X11DecorationStyle::getWindowEffectScene()
{
    effectScenes validSce;
    if (m_validProperties.testFlag(WindowEffectProperty)) {
        validSce = windowEffect();
    } else {
        auto scenAtom = Utils::internAtom(_DEEPIN_NET_EFFECT);
        const QByteArray property_data = Utils::readWindowProperty(m_window->window(), scenAtom, XCB_ATOM_CARDINAL);
        if (!property_data.isEmpty()) {
            const char *cdata = property_data.constData();
            validSce = effectScene(*(reinterpret_cast<const quint32 *>(cdata)));
        }
    }

    return validSce;
}

void X11DecorationStyle::parseWinCustomRadius()
{
    auto atom = Utils::internAtom(_DEEPIN_NET_RADIUS);
    const QByteArray data = Utils::readWindowProperty(m_window->window(), atom, XCB_ATOM_STRING);
    if (!data.isEmpty()) {
        const char *cdata = data.constData();
        setValidProperties(validProperties() | DecorationStyle::WindowRadiusProperty);
        setWindowRadius(variant2Point(cdata));
    }
}

void X11DecorationStyle::parseWinCustomShadow()
{
    auto atom = Utils::internAtom(_DEEPIN_NET_SHADOW);
    const QByteArray data = Utils::readWindowProperty(m_window->window(), atom, XCB_ATOM_STRING);
    if (!data.isEmpty()) {
        QString cdata = data.constData();
        setValidProperties(validProperties() | DecorationStyle::ShadowColorProperty);
        setShadowColor(cdata);
    }
}

void X11DecorationStyle::parseWinStartUpEffect()
{
    if (m_validProperties.testFlag(WindowStartUpEffectProperty)) {
        m_window->setStartUpEffectType(windowStartUpEffect());
    } else {
        auto atom = Utils::internAtom(_DEEPIN_NET_STARTUP);
        const QByteArray data = Utils::readWindowProperty(m_window->window(), atom, XCB_ATOM_CARDINAL);
        if (!data.isEmpty()) {
            const char *cdata = data.constData();
            m_window->setStartUpEffectType(*(reinterpret_cast<const quint32 *>(cdata)));
        }
    }
}

/***********************************************************/

WaylandDecorationStyle::WaylandDecorationStyle(Window *window)
    : DecorationStyle(window)
    , m_window(window)
{
    connect(window, &Window::waylandWindowRadiusChanged, this, &WaylandDecorationStyle::onUpdateWindowRadiusByWayland);
    connect(window, &Window::waylandShadowColorChanged, this, &WaylandDecorationStyle::onUpdateShadowColorByWayland);
    connect(window, &Window::waylandBorderWidthChanged, this, &WaylandDecorationStyle::onUpdateBorderWidthByWayland);
    connect(window, &Window::waylandBorderColorChanged, this, &WaylandDecorationStyle::onUpdateBorderColorByWayland);
}

WaylandDecorationStyle::PropertyFlags WaylandDecorationStyle::validProperties()
{
    return m_validProperties;
}

bool WaylandDecorationStyle::propertyIsValid(PropertyFlag p)
{
    return m_validProperties.testFlag(p);
}

QPointF WaylandDecorationStyle::windowRadius()
{
    return m_radius;
}

void WaylandDecorationStyle::setWindowRadius(const QPointF value)
{
    m_radius = value;
}

qreal WaylandDecorationStyle::borderWidth()
{
    return m_borderWidth;
}

void WaylandDecorationStyle::setBorderWidth(qreal width)
{
    m_borderWidth = width;
}

QColor WaylandDecorationStyle::borderColor()
{
    return m_borderColor;
}

void WaylandDecorationStyle::setBorderColor(QColor color)
{
    m_borderColor = color;
}


QColor WaylandDecorationStyle::shadowColor()
{
    return m_shadowColor;
}

void WaylandDecorationStyle::setShadowColor(QColor color)
{
    m_shadowColor = color;
}

void WaylandDecorationStyle::onUpdateWindowRadiusByWayland(QPointF radius)
{
    setValidProperties(validProperties() | DecorationStyle::WindowRadiusProperty);
    m_radius = radius * Workspace::self()->getWindowStyleMgr()->getOsScale();
    Q_EMIT windowRadiusChanged();
}

void WaylandDecorationStyle::onUpdateShadowColorByWayland(QString color)
{
    setValidProperties(validProperties() | DecorationStyle::ShadowColorProperty);
    setShadowColor(QColor(color));
    Q_EMIT shadowColorChanged();
}

void WaylandDecorationStyle::onUpdateBorderWidthByWayland(qint32 width)
{
    setValidProperties(validProperties() | DecorationStyle::BorderWidthProperty);
    setBorderWidth(width);
    Q_EMIT borderWidthChanged();
}

void WaylandDecorationStyle::onUpdateBorderColorByWayland(QString color)
{
    setValidProperties(validProperties() | DecorationStyle::BorderColorProperty);
    setBorderColor(QColor(color));
    Q_EMIT borderColorChanged();
}

}
