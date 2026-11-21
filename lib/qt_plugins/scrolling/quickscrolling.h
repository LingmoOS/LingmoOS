// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_QUICKSCROLLING_H
#define LINGMO_QUICKSCROLLING_H

#include <QHash>
#include <QObject>
#include <QPointer>

class QQuickFlickable;
class QQuickItem;
class QQuickWindow;
class QWheelEvent;

namespace Lingmo {

class FlickableScroller;

// Keeps a touchpad gesture in a Qt Quick window with the view that took it.
// Automatic, it also scrolls every interactive Flickable that nothing has
// claimed with the shared touchpad behaviour.
class QuickScrolling : public QObject
{
    Q_OBJECT

public:
    static QuickScrolling *self();

    void setAutomatic(bool automatic);
    bool isAutomatic() const;
    // The item scrolls itself; automatic scrolling leaves it alone.
    void claim(QQuickItem *item);
    void unclaim(QQuickItem *item);
    // The rest of the gesture under way in the item's window goes to the item.
    void setGestureTarget(QQuickItem *item);
    // The automatic scroller of the Flickable, once it has scrolled.
    FlickableScroller *scroller(QQuickFlickable *flickable) const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QuickScrolling();
    bool routeGesture(QQuickWindow *window, QWheelEvent *event);
    bool scrollAutomatically(QQuickFlickable *flickable, QWheelEvent *event);
    bool isScrollable(QQuickFlickable *flickable) const;

    QHash<QQuickItem *, QMetaObject::Connection> m_claims;
    QHash<QQuickFlickable *, FlickableScroller *> m_scrollers;
    QHash<QQuickWindow *, QPointer<QQuickItem>> m_gestureTargets;
    bool m_automatic = false;

    friend class QuickScrollingSingleton;
};

} // namespace Lingmo

// For loading the library at run time: turns automatic scrolling on.
extern "C" Q_DECL_EXPORT void lingmo_scrolling_quick_set_automatic();

#endif // LINGMO_QUICKSCROLLING_H
