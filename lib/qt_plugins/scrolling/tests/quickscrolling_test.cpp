// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "flickablescroller.h"
#include "quickscrolling.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTest>
#include <QWheelEvent>
#include <private/qquickflickable_p.h>

using Lingmo::QuickScrolling;
using Lingmo::Scroller;

static qreal now = 0;

static qreal testClock()
{
    return now;
}

static void wheel(QObject *target, QPoint pixels, Qt::ScrollPhase phase, ulong timestamp = 1000,
                  QPoint angles = {}, QPointF position = {50, 50})
{
    now = timestamp;
    QWheelEvent event(position, position, pixels, angles, Qt::NoButton, Qt::NoModifier, phase, false);
    event.setTimestamp(timestamp);
    QCoreApplication::sendEvent(target, &event);
}

// Shows the frame that samples the fingers at time.
static void frame(QQuickFlickable *view, qreal time)
{
    now = time + Scroller::resampleLatency;
    if (auto *scroller = QuickScrolling::self()->scroller(view))
        scroller->advance(now);
}

static void swipe(QQuickWindow *window, int step, ulong start = 1000, QPointF position = {50, 50})
{
    wheel(window, {}, Qt::ScrollBegin, start, {}, position);
    for (int i = 1; i <= 3; ++i)
        wheel(window, {0, step}, Qt::ScrollUpdate, start + 16 * i, {0, 12 * step}, position);
    wheel(window, {}, Qt::ScrollEnd, start + 50, {}, position);
}

class Scene
{
public:
    Scene()
    {
        window.resize(600, 300);
        left = view(0);
        right = view(300);
        window.show();
    }

    QQuickFlickable *view(qreal x)
    {
        auto *flickable = new QQuickFlickable(window.contentItem());
        flickable->setX(x);
        flickable->setSize({300, 300});
        flickable->setContentWidth(2000);
        flickable->setContentHeight(2000);
        flickable->setContentY(500);
        return flickable;
    }

    QQuickWindow window;
    QQuickFlickable *left;
    QQuickFlickable *right;
};

class QuickScrollingTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Scroller::setClock(testClock);
    }

    void init()
    {
        QuickScrolling::self()->setAutomatic(true);
    }

    void cleanup()
    {
        QuickScrolling::self()->setAutomatic(false);
    }

    void touchpadScrollsAndCoasts()
    {
        Scene scene;
        QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
        wheel(&scene.window, {}, Qt::ScrollBegin);
        wheel(&scene.window, {0, -10}, Qt::ScrollUpdate, 1016, {0, -120});
        frame(scene.left, 1016);
        QCOMPARE(scene.left->contentY(), 510);

        swipe(&scene.window, -16, 2000);
        frame(scene.left, 2048);
        const qreal released = scene.left->contentY();
        frame(scene.left, 2200);
        QVERIFY(scene.left->contentY() > released);
    }

    void wheelsKeepTheViewsOwnScrolling()
    {
        Scene scene;
        QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
        wheel(&scene.window, {}, Qt::NoScrollPhase, 1000, {0, -120});
        QVERIFY(!QuickScrolling::self()->scroller(scene.left));
        QTRY_VERIFY(scene.left->contentY() > 500);
    }

    void stopAtBoundsDoesNotOvershoot()
    {
        Scene scene;
        QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
        scene.left->setContentY(0);
        scene.right->setContentY(0);
        scene.left->setBoundsBehavior(QQuickFlickable::StopAtBounds);
        swipe(&scene.window, 10);
        frame(scene.left, 1040);
        QCOMPARE(scene.left->contentY(), 0);

        swipe(&scene.window, 10, 2000, {350, 50});
        frame(scene.right, 2040);
        QVERIFY(scene.right->contentY() < 0);
        for (int time = 2056; time <= 3000; time += 16)
            frame(scene.right, time);
        QCOMPARE(scene.right->contentY(), 0);
    }

    void claimedAndNonInteractiveViewsAreLeftAlone()
    {
        Scene scene;
        QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
        QuickScrolling::self()->claim(scene.left);
        scene.right->setInteractive(false);
        swipe(&scene.window, -10);
        swipe(&scene.window, -10, 2000, {350, 50});
        QVERIFY(!QuickScrolling::self()->scroller(scene.left));
        QVERIFY(!QuickScrolling::self()->scroller(scene.right));
        QCOMPARE(scene.right->contentY(), 500);
        QuickScrolling::self()->unclaim(scene.left);
    }

    void snappingViewsAreLeftAlone()
    {
        QQuickWindow window;
        window.resize(300, 300);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\n"
                          "ListView { width: 300; height: 300; snapMode: ListView.SnapToItem;"
                          " model: 100; delegate: Item { width: 300; height: 50 } }",
                          {});
        auto *list = qobject_cast<QQuickFlickable *>(component.create());
        QVERIFY2(list, qPrintable(component.errorString()));
        list->setParentItem(window.contentItem());
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        swipe(&window, -10);
        QVERIFY(!QuickScrolling::self()->scroller(list));
        delete list;
    }

    void gestureStaysWithItsFirstView()
    {
        Scene scene;
        QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
        wheel(&scene.window, {}, Qt::ScrollBegin);
        wheel(&scene.window, {0, -10}, Qt::ScrollUpdate, 1016, {0, -120});
        wheel(&scene.window, {0, -10}, Qt::ScrollUpdate, 1032, {0, -120}, {350, 50});
        frame(scene.left, 1032);
        QCOMPARE(scene.left->contentY(), 520);
        QCOMPARE(scene.right->contentY(), 500);
    }

    void perpendicularGestureReachesTheParent()
    {
        QQuickWindow window;
        window.resize(300, 300);
        auto *outer = new QQuickFlickable(window.contentItem());
        outer->setSize({300, 300});
        outer->setContentWidth(300);
        outer->setContentHeight(2000);
        auto *inner = new QQuickFlickable(outer->contentItem());
        inner->setSize({300, 300});
        inner->setContentWidth(2000);
        inner->setContentHeight(300);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        wheel(&window, {}, Qt::ScrollBegin);
        wheel(&window, {0, -10}, Qt::ScrollUpdate, 1016, {0, -120});
        frame(outer, 1016);
        QCOMPARE(inner->contentX(), 0);
        QCOMPARE(outer->contentY(), 10);
    }
};

QTEST_MAIN(QuickScrollingTest)
#include "quickscrolling_test.moc"
