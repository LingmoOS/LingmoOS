// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockpositioner.h"
#include <panel.h>

#include <QLoggingCategory>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

DS_USE_NAMESPACE

namespace dock {

static DPanel *isInDockPanel(QObject *object)
{
    auto dockPanel = qobject_cast<DPanel *>(DPanel::qmlAttachedProperties(object));
    if (!dockPanel || dockPanel->pluginId() != "org.lingmo.ds.dock") {
        qWarning() << "only used in DockPanel.";
        return nullptr;
    }
    return dockPanel;
}

DockPositioner::DockPositioner(DPanel *panel, QObject *parent)
    : QObject(parent)
    , m_panel(panel)
    , m_positionTimer(new QTimer(this))
{
    m_positionTimer->setSingleShot(true);
    m_positionTimer->setInterval(0);
    connect(m_positionTimer, &QTimer::timeout, this, &DockPositioner::updatePosition);

    Q_ASSERT(m_panel);
    connect(m_panel, SIGNAL(positionChanged(Position)), this, SLOT(update()));
    connect(m_panel, SIGNAL(geometryChanged(QRect)), this, SLOT(update()));
    connect(this, &DockPositioner::boundingChanged, this, &DockPositioner::update);
}

DockPositioner::~DockPositioner()
{

}

DockPositioner *DockPositioner::qmlAttachedProperties(QObject *object)
{
    if (auto dockPanel = isInDockPanel(object))
        return new DockPositioner(dockPanel, object);

    return nullptr;
}

QRect DockPositioner::bounding() const
{
    return m_bounding;
}

void DockPositioner::setBounding(const QRect &newBounding)
{
    if (m_bounding == newBounding)
        return;
    m_bounding = newBounding;
    emit boundingChanged();
}

int DockPositioner::x() const
{
    return m_x;
}

int DockPositioner::y() const
{
    return m_y;
}

QRect DockPositioner::dockGeometry() const
{
    return m_panel ? m_panel->property("geometry").toRect() : QRect();
}

Position DockPositioner::dockPosition() const
{
    return m_panel ? static_cast<Position>(m_panel->property("position").toInt()) : Position::Top;
}

void DockPositioner::setX(int x)
{
    if (m_x == x)
        return;
    m_x = x;
    emit xChanged();
}

void DockPositioner::setY(int y)
{
    if (m_y == y)
        return;
    m_y = y;
    emit yChanged();
}

void DockPositioner::update()
{
    m_positionTimer->start();
}

void DockPositioner::updatePosition()
{
    int xPosition = 0;
    int yPosition = 0;
    switch (dockPosition()) {
    case dock::Top: {
        xPosition = m_bounding.x();
        yPosition = m_bounding.y();
        break;
    }
    case dock::Right: {
        xPosition = m_bounding.x() - m_bounding.width();
        yPosition = m_bounding.y();
        break;
    }
    case dock::Bottom: {
        xPosition = m_bounding.x();
        yPosition = m_bounding.y() - m_bounding.height();
        break;
    }
    case dock::Left: {
        xPosition = m_bounding.x();
        yPosition = m_bounding.y();
        break;
    }
    default:
        break;
    }

    setX(xPosition);
    setY(yPosition);
}

DockPanelPositioner::DockPanelPositioner(DPanel *panel, QObject *parent)
    : DockPositioner(panel, parent)
{
    connect(this, &DockPanelPositioner::horizontalOffsetChanged, this, &DockPanelPositioner::update);
    connect(this, &DockPanelPositioner::vertialOffsetChanged, this, &DockPanelPositioner::update);
}

DockPanelPositioner::~DockPanelPositioner()
{

}

DockPanelPositioner *DockPanelPositioner::qmlAttachedProperties(QObject *object)
{
    if (auto dockPanel = isInDockPanel(object))
        return new DockPanelPositioner(dockPanel, object);
    return nullptr;
}

int DockPanelPositioner::horizontalOffset() const
{
    return m_horizontalOffset;
}

void DockPanelPositioner::setHorizontalOffset(int newHorizontalOffset)
{
    if (m_horizontalOffset == newHorizontalOffset)
        return;
    m_horizontalOffset = newHorizontalOffset;
    emit horizontalOffsetChanged();
}

int DockPanelPositioner::vertialOffset() const
{
    return m_vertialOffset;
}

void DockPanelPositioner::setVertialOffset(int newVertialOffset)
{
    if (m_vertialOffset == newVertialOffset)
        return;
    m_vertialOffset = newVertialOffset;
    emit vertialOffsetChanged();
}

void DockPanelPositioner::resetHorizontalOffset()
{
    setHorizontalOffset(-1);
}

void DockPanelPositioner::resetVertialOffset()
{
    setVertialOffset(-1);
}

void DockPanelPositioner::updatePosition()
{
    const auto dockWindowRect = dockGeometry();
    int xPosition = 0;
    int yPosition = 0;
    int horizontalOffset = m_horizontalOffset == -1 ? m_bounding.width() / 2 : m_horizontalOffset;
    int vertialOffset = m_vertialOffset == -1 ? m_bounding.height() / 2 : m_vertialOffset;
    switch (dockPosition()) {
    case dock::Top: {
        xPosition = m_bounding.x() - horizontalOffset;
        yPosition = dockWindowRect.height() + 10;
        break;
    }
    case dock::Right: {
        xPosition = -m_bounding.width() - 10;
        yPosition = m_bounding.y() - vertialOffset;
        break;
    }
    case dock::Bottom: {
        xPosition = m_bounding.x() - horizontalOffset;
        yPosition = -m_bounding.height() - 10;
        break;
    }
    case dock::Left: {
        xPosition = dockWindowRect.width() + 10;
        yPosition = m_bounding.y() - vertialOffset;
        break;
    }
    default:
        break;
    }

    setX(xPosition);
    setY(yPosition);
}
}
