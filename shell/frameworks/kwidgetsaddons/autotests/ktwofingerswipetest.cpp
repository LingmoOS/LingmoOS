/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <ktwofingerswipe.h>

#include <QTest>
#include <QSignalSpy>
#include <QWidget>
#include <QMainWindow>

#include "windowscheck.h"

class KTwoFingerSwipeTest : public QObject
{
    Q_OBJECT

    QPointingDevice *mDev = nullptr;

    QMainWindow mMainWindow;
    QWidget mWidget;
    KTwoFingerSwipeRecognizer *mTwoFingerRec = nullptr;
    Qt::GestureType mKTwoFingerSwipeGesture;
    QPoint mTouchPointPos = QPoint(200, 200);
    QSignalSpy *mSpySwipeFinished = nullptr;
    QSignalSpy *mSpySwipeStarted = nullptr;
    QSignalSpy *mSpySwipeUpdated = nullptr;
    QSignalSpy *mSpySwipeCanceled = nullptr;
    qreal mAngel = 0.0;

Q_SIGNALS:
    void swipeStarted(KTwoFingerSwipe *);
    void swipeFinished(KTwoFingerSwipe *);
    void swipeUpdated(KTwoFingerSwipe *);
    void swipeCanceled(KTwoFingerSwipe *);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override
    {
        if (e->type() == QEvent::Gesture) {
            QGestureEvent *gEvent = static_cast<QGestureEvent *>(e);
            KTwoFingerSwipe *swipe = static_cast<KTwoFingerSwipe *>(gEvent->gesture(mKTwoFingerSwipeGesture));
            if (swipe) {
                const Qt::GestureState state = swipe->state();
                if (state == Qt::GestureFinished) {
                    Q_EMIT swipeFinished(swipe);
                } else if (state == Qt::GestureStarted) {
                    Q_EMIT swipeStarted(swipe);
                } else if (state == Qt::GestureUpdated) {
                    Q_EMIT swipeUpdated(swipe);
                } else if (state == Qt::GestureCanceled) {
                    Q_EMIT swipeCanceled(swipe);
                }
            }
            e->accept();
            return true;
        }
        return QObject::eventFilter(watched, e);
    }

protected Q_SLOTS:
    void slotSwipeFinished (KTwoFingerSwipe *swipe)
    {
        compareGesturePositions(swipe);
    }
    void slotSwipeStarted (KTwoFingerSwipe *swipe)
    {
        compareGesturePositions(swipe);
    }
    void slotSwipeUpdated (KTwoFingerSwipe *swipe)
    {
        compareGesturePositions(swipe);
    }
    void slotSwipeCanceled (KTwoFingerSwipe *swipe)
    {
        compareGesturePositions(swipe);
    }
private:
    void compareGesturePositions(KTwoFingerSwipe *swipe)
    {
        QCOMPARE(swipe->pos(), mTouchPointPos);
        QCOMPARE(swipe->screenPos(), mWidget.mapToGlobal(mTouchPointPos));
        QCOMPARE(swipe->hotSpot(), mWidget.mapToGlobal(mTouchPointPos));
        QCOMPARE(swipe->scenePos(), mWidget.mapToGlobal(mTouchPointPos));
        QVERIFY(swipe->hasHotSpot());
        mAngel = swipe->swipeAngle();
    }

    void clearSignalSpys()
    {
        mSpySwipeFinished->clear();
        mSpySwipeCanceled->clear();
        mSpySwipeUpdated->clear();
        mSpySwipeStarted->clear();
    }

private Q_SLOTS:
    void cleanupTestCase()
    {
        delete mSpySwipeFinished;
        delete mSpySwipeCanceled;
        delete mSpySwipeUpdated;
        delete mSpySwipeStarted;
    }
    void initTestCase()
    {
        if (isWindowsCI()) {
            QSKIP("GUI Tests on Windows CI are not supported");
        }

        mDev = QTest::createTouchDevice();

        mMainWindow.setGeometry(0, 0, 500, 500);
        mMainWindow.setCentralWidget(&mWidget);
        mMainWindow.show();

        mWidget.installEventFilter(this);
        mWidget.setAttribute(Qt::WA_AcceptTouchEvents);

        QVERIFY(QTest::qWaitForWindowActive(&mMainWindow));
        QVERIFY(QTest::qWaitForWindowActive(&mWidget));

        mTwoFingerRec = new KTwoFingerSwipeRecognizer();
        QVERIFY(mTwoFingerRec);

        mKTwoFingerSwipeGesture = QGestureRecognizer::registerRecognizer(mTwoFingerRec);
        QVERIFY(mKTwoFingerSwipeGesture & Qt::CustomGesture);

        mWidget.grabGesture(mKTwoFingerSwipeGesture);

        connect(this, &KTwoFingerSwipeTest::swipeFinished, this, &KTwoFingerSwipeTest::slotSwipeFinished);
        connect(this, &KTwoFingerSwipeTest::swipeStarted, this, &KTwoFingerSwipeTest::slotSwipeStarted);
        connect(this, &KTwoFingerSwipeTest::swipeUpdated, this, &KTwoFingerSwipeTest::slotSwipeUpdated);
        connect(this, &KTwoFingerSwipeTest::swipeCanceled, this, &KTwoFingerSwipeTest::slotSwipeCanceled);

        mSpySwipeFinished = new QSignalSpy(this, &KTwoFingerSwipeTest::swipeFinished);
        QVERIFY(mSpySwipeFinished->isValid());

        mSpySwipeStarted = new QSignalSpy(this, &KTwoFingerSwipeTest::swipeStarted);
        QVERIFY(mSpySwipeStarted->isValid());

        mSpySwipeUpdated = new QSignalSpy(this, &KTwoFingerSwipeTest::swipeUpdated);
        QVERIFY(mSpySwipeUpdated->isValid());

        mSpySwipeCanceled = new QSignalSpy(this, &KTwoFingerSwipeTest::swipeCanceled);
        QVERIFY(mSpySwipeCanceled->isValid());

        QTest::qWait(1000);
    }

    void testChangeTimeAndDistance()
    {
        QVERIFY(mTwoFingerRec->maxSwipeTime() >= 0);
        mTwoFingerRec->setMaxSwipeTime(10);
        QCOMPARE(mTwoFingerRec->maxSwipeTime(), 10);
        mTwoFingerRec->setMaxSwipeTime(-10);
        QCOMPARE(mTwoFingerRec->maxSwipeTime(), 0);
        mTwoFingerRec->setMaxSwipeTime(90);
        QCOMPARE(mTwoFingerRec->maxSwipeTime(), 90);

        QVERIFY(mTwoFingerRec->minSswipeDistance() >= 0);
        mTwoFingerRec->setSwipeDistance(10);
        QCOMPARE(mTwoFingerRec->minSswipeDistance(), 10);
        mTwoFingerRec->setSwipeDistance(-10);
        QCOMPARE(mTwoFingerRec->minSswipeDistance(), 0);
        mTwoFingerRec->setSwipeDistance(30);
        QCOMPARE(mTwoFingerRec->minSswipeDistance(), 30);
    }

    void testSuccessfulGesture_right()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(length, 0);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);


        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeStarted->count() == 0) {
            QVERIFY(mSpySwipeStarted->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 1);
        QCOMPARE(mSpySwipeCanceled->count(), 0);
        QVERIFY(mSpySwipeUpdated->count() > 0);
        QCOMPARE(mAngel, 0);
    }

    void testSuccessfulGesture_left()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(-length, 0);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);


        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeStarted->count() == 0) {
            QVERIFY(mSpySwipeStarted->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 1);
        QCOMPARE(mSpySwipeCanceled->count(), 0);
        QVERIFY(mSpySwipeUpdated->count() > 0);
        QCOMPARE(mAngel, 180);
    }

    void testSuccessfulGesture_up()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(0, -length);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);


        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeStarted->count() == 0) {
            QVERIFY(mSpySwipeStarted->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 1);
        QCOMPARE(mSpySwipeCanceled->count(), 0);
        QVERIFY(mSpySwipeUpdated->count() > 0);
        QCOMPARE(mAngel, 90);
    }

    void testSuccessfulGesture_down()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(0, length);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);


        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeStarted->count() == 0) {
            QVERIFY(mSpySwipeStarted->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 1);
        QCOMPARE(mSpySwipeCanceled->count(), 0);
        QVERIFY(mSpySwipeUpdated->count() > 0);
        QCOMPARE(mAngel, 270);
    }

    void testFailingGesture_toSlow()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance() / 3;
        const QPoint swipeDistance = QPoint(length, 0);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);


        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeStarted->count() == 0) {
            QVERIFY(mSpySwipeStarted->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 0);
        QCOMPARE(mSpySwipeCanceled->count(), 1);
        QVERIFY(mSpySwipeUpdated->count() > 0);
        QCOMPARE(mAngel, 0);
    }


    void testFailingGesture_threeFingers()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(length, 0);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr)
            .press(2, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(1, mTouchPointPos + swipeDistance, (QWidget *) nullptr)
            .move(2, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(1, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr)
            .release(2, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        if (mSpySwipeCanceled->count() == 0) {
            QVERIFY(mSpySwipeCanceled->wait(1000));
        }

        QCOMPARE(mSpySwipeStarted->count(), 1);
        QCOMPARE(mSpySwipeFinished->count(), 0);
        QCOMPARE(mSpySwipeCanceled->count(), 1);
        QVERIFY(mSpySwipeUpdated->count() >= 0);
        QCOMPARE(mAngel, 0);
    }

    void testFailingGesture_oneFinger()
    {
        clearSignalSpys();

        const int length = mTwoFingerRec->minSswipeDistance();
        const QPoint swipeDistance = QPoint(length, 0);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 3, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance / 2, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + swipeDistance, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos + swipeDistance * 1.5, (QWidget *) nullptr);

        QCOMPARE(mSpySwipeStarted->count(), 0);
        QCOMPARE(mSpySwipeFinished->count(), 0);
        QCOMPARE(mSpySwipeCanceled->count(), 0);
        QVERIFY(mSpySwipeUpdated->count() == 0);
    }

    void testSomeRandomSwipes()
    {
        for (int i = 0; i < 50; ++i) {
            const int x = rand() % 200 + 150;
            const int y = rand() % 200 + 150;
            mTouchPointPos = QPoint(x, y);
            testSuccessfulGesture_right();
            testSuccessfulGesture_up();
        }
    }
};

QTEST_MAIN(KTwoFingerSwipeTest)

#include "ktwofingerswipetest.moc"
