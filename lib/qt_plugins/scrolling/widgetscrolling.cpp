// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetscrolling.h"
#include "scrollareascroller.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QGraphicsView>
#include <QMdiArea>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QWheelEvent>

namespace Lingmo {

static WidgetScrolling *instance = nullptr;

WidgetScrolling::WidgetScrolling()
    : QObject(qApp)
{
    qApp->installEventFilter(this);
}

void WidgetScrolling::install()
{
    if (!instance && qobject_cast<QApplication *>(QCoreApplication::instance()))
        instance = new WidgetScrolling;
}

WidgetScrolling *WidgetScrolling::self()
{
    return instance;
}

ScrollAreaScroller *WidgetScrolling::scroller(QAbstractScrollArea *area) const
{
    return m_scrollers.value(area);
}

QAbstractScrollArea *WidgetScrolling::areaOf(QScrollBar *bar)
{
    for (QWidget *widget = bar->parentWidget(); widget; widget = widget->parentWidget()) {
        if (auto *area = qobject_cast<QAbstractScrollArea *>(widget)) {
            const bool own = area->horizontalScrollBar() == bar || area->verticalScrollBar() == bar;
            return own ? area : nullptr;
        }
    }
    return nullptr;
}

bool WidgetScrolling::isSupported(QAbstractScrollArea *area)
{
    // The bars of other areas may count lines or items, as QPlainTextEdit's do.
    return qobject_cast<QAbstractItemView *>(area) || qobject_cast<QScrollArea *>(area)
        || qobject_cast<QTextEdit *>(area) || qobject_cast<QGraphicsView *>(area)
        || qobject_cast<QMdiArea *>(area);
}

bool WidgetScrolling::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::Wheel)
        return QObject::eventFilter(watched, event);

    auto *wheel = static_cast<QWheelEvent *>(event);
    auto *bar = qobject_cast<QScrollBar *>(watched);
    // Wheels keep the area's own scrolling.
    if (!bar || wheel->phase() == Qt::NoScrollPhase)
        return false;
    QAbstractScrollArea *area = areaOf(bar);
    if (!area || !isSupported(area) || !area->isEnabled() || !area->window()->windowHandle())
        return false;

    ScrollAreaScroller *scroller = m_scrollers.value(area);
    if (!scroller) {
        if (auto *view = qobject_cast<QAbstractItemView *>(area)) {
            view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }
        scroller = new ScrollAreaScroller(area);
        m_scrollers.insert(area, scroller);
        connect(scroller, &QObject::destroyed, this, [this, area] {
            m_scrollers.remove(area);
        });
    }

    // Unaccepted, the event goes on to the parents instead of the bar.
    wheel->setAccepted(scroller->scroll(wheel));
    return true;
}

} // namespace Lingmo
