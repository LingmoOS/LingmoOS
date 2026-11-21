// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "flickablescroller.h"

#include <QQuickWindow>
#include <private/qquickflickable_p.h>
#include <private/qquickflickable_p_p.h>
#include <private/qquickitem_p.h>

namespace Lingmo {

namespace {

// Reaches the Flickable's protected movement notifications.
class FlickableMovement : public QQuickFlickable
{
public:
    static void start(QQuickFlickable *flickable)
    {
        (flickable->*(&FlickableMovement::movementStarting))();
    }

    static void end(QQuickFlickable *flickable)
    {
        const auto ending = static_cast<void (QQuickFlickable::*)(bool, bool)>(&FlickableMovement::movementEnding);
        (flickable->*ending)(true, true);
    }
};

} // namespace

FlickableScroller::FlickableScroller(QQuickFlickable *flickable)
    : Scroller(flickable)
    , m_flickable(flickable)
{
    setWindow(flickable->window());
    connect(flickable, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        setWindow(window);
    });
    connect(flickable, &QQuickItem::visibleChanged, this, [this] {
        if (!m_flickable->isVisible())
            cancel();
    });
    connect(flickable, &QQuickItem::enabledChanged, this, [this] {
        if (!m_flickable->isEnabled())
            cancel();
    });
    connect(flickable, &QQuickFlickable::contentXChanged, this, &FlickableScroller::viewMoved);
    connect(flickable, &QQuickFlickable::contentYChanged, this, &FlickableScroller::viewMoved);
}

QQuickFlickable *FlickableScroller::flickable() const
{
    return m_flickable;
}

Scroller::Axis FlickableScroller::horizontalAxis() const
{
    const qreal minimum = m_flickable->originX() - m_flickable->leftMargin();
    const qreal maximum = qMax(minimum, m_flickable->originX() + m_flickable->contentWidth()
                              + m_flickable->rightMargin() - m_flickable->width());
    return {minimum, maximum, m_flickable->width(),
            maximum > minimum && m_flickable->flickableDirection() != QQuickFlickable::VerticalFlick};
}

Scroller::Axis FlickableScroller::verticalAxis() const
{
    const qreal minimum = m_flickable->originY() - m_flickable->topMargin();
    const qreal maximum = qMax(minimum, m_flickable->originY() + m_flickable->contentHeight()
                              + m_flickable->bottomMargin() - m_flickable->height());
    return {minimum, maximum, m_flickable->height(),
            maximum > minimum && m_flickable->flickableDirection() != QQuickFlickable::HorizontalFlick};
}

QPointF FlickableScroller::position() const
{
    return {m_flickable->contentX(), m_flickable->contentY()};
}

void FlickableScroller::applyPosition(const QPointF &position)
{
    // setContentX/Y would end the movement that reportMoving() reports.
    auto *d = QQuickFlickablePrivate::get(m_flickable.data());
    const auto move = [d](QQuickFlickablePrivate::AxisData &data, qreal value) {
        d->resetTimeline(data);
        data.explicitValue = true;
        if (data.move.value() != -value)
            data.move.setValue(-value);
    };
    move(d->hData, position.x());
    move(d->vData, position.y());
}

void FlickableScroller::reportMoving(bool moving, Qt::Orientations axes)
{
    // A Flickable that is not moving rounds or clamps its position whenever its
    // content size changes, as a ListView's does while it creates delegates.
    if (!moving) {
        FlickableMovement::end(m_flickable);
        return;
    }

    auto *d = QQuickFlickablePrivate::get(m_flickable.data());
    d->hMoved = axes.testFlag(Qt::Horizontal);
    d->vMoved = axes.testFlag(Qt::Vertical);
    FlickableMovement::start(m_flickable);
}

void FlickableScroller::stopViewMotion()
{
    m_flickable->cancelFlick();
}

bool FlickableScroller::isViewAlive() const
{
    // A Flickable in ~QQuickItem has lost its Flickable part and cannot be scrolled.
    return m_flickable && !QQuickItemPrivate::get(m_flickable.data())->inDestructor;
}

} // namespace Lingmo
