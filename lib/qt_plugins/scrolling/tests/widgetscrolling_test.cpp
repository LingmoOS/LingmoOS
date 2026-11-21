// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollareascroller.h"
#include "widgetscrolling.h"

#include <QListWidget>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTest>
#include <QWheelEvent>

using Lingmo::Scroller;
using Lingmo::WidgetScrolling;

static qreal now = 0;

static qreal testClock()
{
    return now;
}

static void wheel(QAbstractScrollArea *area, QPoint pixels, Qt::ScrollPhase phase,
                  ulong timestamp = 1000, QPoint angles = {})
{
    now = timestamp;
    const QPointF position(50, 50);
    QWheelEvent event(position, area->viewport()->mapToGlobal(position), pixels, angles,
                      Qt::NoButton, Qt::NoModifier, phase, false);
    event.setTimestamp(timestamp);
    QCoreApplication::sendEvent(area->viewport(), &event);
}

static void swipe(QAbstractScrollArea *area, QPoint step, ulong start = 1000)
{
    wheel(area, {}, Qt::ScrollBegin, start);
    for (int i = 1; i <= 3; ++i)
        wheel(area, step, Qt::ScrollUpdate, start + 16 * i, step * 12);
}

// Shows the frame that samples the fingers at time.
static void frame(QAbstractScrollArea *area, qreal time)
{
    now = time + Scroller::resampleLatency;
    if (auto *scroller = WidgetScrolling::self()->scroller(area))
        scroller->advance(now);
}

class Area : public QScrollArea
{
public:
    Area()
    {
        auto *content = new QWidget;
        content->resize(2000, 2000);
        setWidget(content);
        resize(300, 300);
        show();
    }
};

class WidgetScrollingTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Scroller::setClock(testClock);
        WidgetScrolling::install();
        QVERIFY(WidgetScrolling::self());
    }

    void pixelsScrollOnePixelEach()
    {
        Area area;
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        swipe(&area, {0, -10});
        frame(&area, 1048);
        QCOMPARE(area.verticalScrollBar()->value(), 30);
        QCOMPARE(area.horizontalScrollBar()->value(), 0);
    }

    void releaseCoasts()
    {
        Area area;
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        swipe(&area, {0, -16});
        wheel(&area, {}, Qt::ScrollEnd, 1050);
        frame(&area, 1048);
        const int released = area.verticalScrollBar()->value();
        frame(&area, 1200);
        QVERIFY(area.verticalScrollBar()->value() > released);
    }

    void overshootMovesTheViewportAndReturns()
    {
        Area area;
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        const QPoint rest = area.viewport()->pos();
        swipe(&area, {0, 20});
        frame(&area, 1048);
        QCOMPARE(area.verticalScrollBar()->value(), 0);
        QVERIFY(area.viewport()->y() > rest.y());
        wheel(&area, {}, Qt::ScrollEnd, 1050);
        for (int time = 1064; time <= 2500; time += 16)
            frame(&area, time);
        QCOMPARE(area.viewport()->pos(), rest);
        QVERIFY(!WidgetScrolling::self()->scroller(&area)->isActive());
    }

    void wheelsKeepTheAreasOwnScrolling()
    {
        Area area;
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        wheel(&area, {}, Qt::NoScrollPhase, 1000, {0, -120});
        QVERIFY(!WidgetScrolling::self()->scroller(&area));
        QVERIFY(area.verticalScrollBar()->value() > 0);
    }

    void rightToLeftFollowsTheFingers()
    {
        Area area;
        area.setLayoutDirection(Qt::RightToLeft);
        QVERIFY(QTest::qWaitForWindowExposed(&area));
        const int start = area.horizontalScrollBar()->value();
        swipe(&area, {10, 0});
        frame(&area, 1048);
        QCOMPARE(area.horizontalScrollBar()->value(), start + 30);
    }

    void itemViewsScrollByPixel()
    {
        QListWidget list;
        for (int i = 0; i < 200; ++i)
            list.addItem(QString::number(i));
        list.resize(300, 300);
        list.show();
        QVERIFY(QTest::qWaitForWindowExposed(&list));
        swipe(&list, {0, -10});
        frame(&list, 1048);
        QCOMPARE(list.verticalScrollMode(), QAbstractItemView::ScrollPerPixel);
        QCOMPARE(list.verticalScrollBar()->value(), 30);
    }

    void lineCountingAreasAreLeftAlone()
    {
        QPlainTextEdit edit;
        edit.setPlainText(QString("line\n").repeated(500));
        edit.resize(300, 300);
        edit.show();
        QVERIFY(QTest::qWaitForWindowExposed(&edit));
        swipe(&edit, {0, -10});
        QVERIFY(!WidgetScrolling::self()->scroller(&edit));
    }
};

QTEST_MAIN(WidgetScrollingTest)
#include "widgetscrolling_test.moc"
