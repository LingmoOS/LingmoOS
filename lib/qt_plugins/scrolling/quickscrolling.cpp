// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickscrolling.h"
#include "flickablescroller.h"

#include <QCoreApplication>
#include <QQuickWindow>
#include <QWheelEvent>
#include <private/qquickflickable_p.h>

#include <utility>

namespace Lingmo {

class QuickScrollingSingleton
{
public:
    QuickScrolling self;
};

Q_GLOBAL_STATIC(QuickScrollingSingleton, quickScrollingSelf)

QuickScrolling::QuickScrolling()
{
    if (auto *application = QCoreApplication::instance())
        application->installEventFilter(this);
}

QuickScrolling *QuickScrolling::self()
{
    return &quickScrollingSelf()->self;
}

void QuickScrolling::setAutomatic(bool automatic)
{
    if (m_automatic == automatic)
        return;

    m_automatic = automatic;
    if (!automatic) {
        const auto scrollers = std::exchange(m_scrollers, {});
        for (auto *scroller : scrollers)
            delete scroller;
    }
}

bool QuickScrolling::isAutomatic() const
{
    return m_automatic;
}

void QuickScrolling::claim(QQuickItem *item)
{
    if (!item || m_claims.contains(item))
        return;

    m_claims.insert(item, connect(item, &QObject::destroyed, this, [this, item] {
        m_claims.remove(item);
    }));
    if (auto *flickable = qobject_cast<QQuickFlickable *>(item))
        delete m_scrollers.take(flickable);
}

void QuickScrolling::unclaim(QQuickItem *item)
{
    disconnect(m_claims.take(item));
}

void QuickScrolling::setGestureTarget(QQuickItem *item)
{
    if (item && item->window())
        m_gestureTargets.insert(item->window(), item);
}

FlickableScroller *QuickScrolling::scroller(QQuickFlickable *flickable) const
{
    return m_scrollers.value(flickable);
}

bool QuickScrolling::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::Wheel)
        return QObject::eventFilter(watched, event);

    auto *wheel = static_cast<QWheelEvent *>(event);
    if (auto *window = qobject_cast<QQuickWindow *>(watched))
        return routeGesture(window, wheel);
    if (m_automatic) {
        if (auto *flickable = qobject_cast<QQuickFlickable *>(watched))
            return scrollAutomatically(flickable, wheel);
    }
    return QObject::eventFilter(watched, event);
}

bool QuickScrolling::routeGesture(QQuickWindow *window, QWheelEvent *event)
{
    if (event->phase() == Qt::ScrollBegin || event->phase() == Qt::NoScrollPhase) {
        m_gestureTargets.remove(window);
        return false;
    }

    QQuickItem *target = m_gestureTargets.value(window).data();
    if (!target)
        return false;
    auto *flickable = qobject_cast<QQuickFlickable *>(target);
    if (target->window() != window || !target->isVisible() || !target->isEnabled()
        || !(m_claims.contains(target) || m_scrollers.contains(flickable))) {
        m_gestureTargets.remove(window);
        return false;
    }

    QWheelEvent forwarded(target->mapFromScene(event->position()), event->globalPosition(),
                          event->pixelDelta(), event->angleDelta(), event->buttons(),
                          event->modifiers(), event->phase(), event->inverted(),
                          event->source(), event->pointingDevice());
    forwarded.setTimestamp(event->timestamp());
    QCoreApplication::sendEvent(target, &forwarded);
    if (event->phase() == Qt::ScrollEnd)
        m_gestureTargets.remove(window);
    event->accept();
    return true;
}

bool QuickScrolling::isScrollable(QQuickFlickable *flickable) const
{
    // A view that snaps or keeps its current item in a range positions itself.
    const QVariant snapMode = flickable->property("snapMode");
    const QVariant highlightRangeMode = flickable->property("highlightRangeMode");
    if ((snapMode.isValid() && snapMode.toInt() != 0)
        || (highlightRangeMode.isValid() && highlightRangeMode.toInt() == 2))
        return false;

    // Without an exposed window, as in a QQuickWidget, no frames would drive a coast.
    return flickable->isInteractive() && flickable->isEnabled() && flickable->window()
        && flickable->window()->isExposed();
}

bool QuickScrolling::scrollAutomatically(QQuickFlickable *flickable, QWheelEvent *event)
{
    // Wheels keep the view's own scrolling.
    if (event->phase() == Qt::NoScrollPhase || m_claims.contains(flickable))
        return false;

    FlickableScroller *scroller = m_scrollers.value(flickable);
    if (!isScrollable(flickable)) {
        if (scroller)
            scroller->cancel();
        return false;
    }
    if (!scroller) {
        scroller = new FlickableScroller(flickable);
        m_scrollers.insert(flickable, scroller);
        connect(scroller, &QObject::destroyed, this, [this, flickable] {
            m_scrollers.remove(flickable);
        });
    }

    const bool elastic = flickable->boundsBehavior() != QQuickFlickable::StopAtBounds;
    const bool accepted = scroller->handleWheel(event, elastic);
    // Unaccepted, the event goes on to the parents instead of the view's own handling.
    event->setAccepted(accepted);
    if (accepted && event->phase() != Qt::ScrollEnd)
        setGestureTarget(flickable);
    return true;
}

} // namespace Lingmo

void lingmo_scrolling_quick_set_automatic()
{
    Lingmo::QuickScrolling::self()->setAutomatic(true);
}
