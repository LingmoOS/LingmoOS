// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_WIDGETSCROLLING_H
#define LINGMO_WIDGETSCROLLING_H

#include <QHash>
#include <QObject>

class QAbstractScrollArea;
class QScrollBar;

namespace Lingmo {

class ScrollAreaScroller;

// Scrolls the standard pixel scroll areas of a widget application with the
// shared touchpad behaviour. It takes the touchpad events that a scroll area
// passes to its scroll bars, so views that handle the wheel themselves keep it.
class WidgetScrolling : public QObject
{
    Q_OBJECT

public:
    static void install();

    // The scroller of the area, once it has scrolled.
    ScrollAreaScroller *scroller(QAbstractScrollArea *area) const;
    static WidgetScrolling *self();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    WidgetScrolling();
    static QAbstractScrollArea *areaOf(QScrollBar *bar);
    static bool isSupported(QAbstractScrollArea *area);

    QHash<QAbstractScrollArea *, ScrollAreaScroller *> m_scrollers;
};

} // namespace Lingmo

#endif // LINGMO_WIDGETSCROLLING_H
