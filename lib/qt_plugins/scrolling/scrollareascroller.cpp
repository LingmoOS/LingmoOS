// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollareascroller.h"

#include <QAbstractScrollArea>
#include <QCoreApplication>
#include <QScrollBar>
#include <QScrollEvent>
#include <QWheelEvent>
#include <QWindow>

#include <cmath>

namespace Lingmo {

ScrollAreaScroller::ScrollAreaScroller(QAbstractScrollArea *area)
    : Scroller(area)
    , m_area(area)
{
    area->installEventFilter(this);
    connect(area->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ScrollAreaScroller::viewMoved);
    connect(area->verticalScrollBar(), &QScrollBar::valueChanged, this, &ScrollAreaScroller::viewMoved);
}

QAbstractScrollArea *ScrollAreaScroller::area() const
{
    return m_area;
}

bool ScrollAreaScroller::scroll(QWheelEvent *event)
{
    // A widget window repaints in full on each update request, so the frames are ours.
    setWindow(m_area->window()->windowHandle(), true);
    return handleWheel(event, true);
}

Scroller::Axis ScrollAreaScroller::horizontalAxis() const
{
    const QScrollBar *bar = m_area->horizontalScrollBar();
    return {qreal(bar->minimum()), qreal(bar->maximum()), qreal(m_area->viewport()->width()),
            bar->maximum() > bar->minimum()};
}

Scroller::Axis ScrollAreaScroller::verticalAxis() const
{
    const QScrollBar *bar = m_area->verticalScrollBar();
    return {qreal(bar->minimum()), qreal(bar->maximum()), qreal(m_area->viewport()->height()),
            bar->maximum() > bar->minimum()};
}

QPoint ScrollAreaScroller::values() const
{
    return {m_area->horizontalScrollBar()->value(), m_area->verticalScrollBar()->value()};
}

qreal ScrollAreaScroller::horizontalPosition(int value) const
{
    const QScrollBar *bar = m_area->horizontalScrollBar();
    return m_area->isRightToLeft() ? bar->minimum() + bar->maximum() - value : value;
}

int ScrollAreaScroller::horizontalValue(qreal position) const
{
    const QScrollBar *bar = m_area->horizontalScrollBar();
    const int value = std::lround(position);
    return m_area->isRightToLeft() ? bar->minimum() + bar->maximum() - value : value;
}

QPointF ScrollAreaScroller::position() const
{
    // The bars hold whole pixels; the fraction and the overshoot are kept here
    // for as long as nothing else moves the bars.
    if (m_positioned && values() == m_values)
        return m_position;
    return {horizontalPosition(values().x()), qreal(values().y())};
}

void ScrollAreaScroller::applyPosition(const QPointF &position)
{
    const auto x = horizontalAxis();
    const auto y = verticalAxis();
    const QPointF inside(qBound(x.minimum, position.x(), x.maximum),
                         qBound(y.minimum, position.y(), y.maximum));
    const QPoint content(horizontalValue(inside.x()), std::lround(inside.y()));
    const QPointF overshoot = position - inside;
    QScrollEvent event(content, QPointF(std::round(overshoot.x()), std::round(overshoot.y())),
                       QScrollEvent::ScrollUpdated);
    QCoreApplication::sendEvent(m_area, &event);
    // Paint now: an update request comes at once while no frame is pending, so
    // the next frame would otherwise be asked for before this one is shown.
    QCoreApplication::sendPostedEvents(m_area->window(), QEvent::UpdateRequest);
    m_position = position;
    m_values = values();
    m_positioned = true;
}

bool ScrollAreaScroller::isViewAlive() const
{
    return m_area;
}

bool ScrollAreaScroller::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_area)
        return Scroller::eventFilter(watched, event);

    if (event->type() == QEvent::Hide
        || (event->type() == QEvent::EnabledChange && !m_area->isEnabled()))
        cancel();
    return false;
}

} // namespace Lingmo
