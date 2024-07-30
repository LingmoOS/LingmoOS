/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <ktwofingertap.h>

#include <QTest>
#include <QSignalSpy>
#include <QWidget>
#include <QMainWindow>

class KTwoFingerSwipeTest : public QObject
{
    Q_OBJECT

    QPointingDevice *mDev = nullptr;

    QMainWindow mMainWindow;
    QWidget mWidget;
    KTwoFingerTapRecognizer *mTwoFingerRec = nullptr;
    Qt::GestureType mKTwoFingerTapGesture;
    QPoint mTouchPointPos = QPoint(100, 100);
    QSignalSpy *mSpyTapFinished = nullptr;
    QSignalSpy *mSpyTapStarted = nullptr;
    QSignalSpy *mSpyTapUpdated = nullptr;
    QSignalSpy *mSpyTapCanceled = nullptr;

Q_SIGNALS:
    void tapStarted(KTwoFingerTap *);
    void tapFinished(KTwoFingerTap *);
    void tapUpdated(KTwoFingerTap *);
    void tapCanceled(KTwoFingerTap *);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override
    {
        if (e->type() == QEvent::Gesture) {
            QGestureEvent *gEvent = static_cast<QGestureEvent *>(e);
            KTwoFingerTap *tap = static_cast<KTwoFingerTap *>(gEvent->gesture(mKTwoFingerTapGesture));
            if (tap) {
                const Qt::GestureState state = tap->state();
                if (state == Qt::GestureFinished) {
                    Q_EMIT tapFinished(tap);
                } else if (state == Qt::GestureStarted) {
                    Q_EMIT tapStarted(tap);
                } else if (state == Qt::GestureUpdated) {
                    Q_EMIT tapUpdated(tap);
                } else if (state == Qt::GestureCanceled) {
                    Q_EMIT tapCanceled(tap);
                }
            }
            e->accept();
            return true;
        }
        return QObject::eventFilter(watched, e);
    }

protected Q_SLOTS:
    void slotTapFinished (KTwoFingerTap *tap)
    {
        compareGesturePositions(tap);
    }
    void slotTapStarted (KTwoFingerTap *tap)
    {
        compareGesturePositions(tap);
    }
    void slotTapUpdated (KTwoFingerTap *tap)
    {
        compareGesturePositions(tap);
    }
    void slotTapCanceled (KTwoFingerTap *tap)
    {
        compareGesturePositions(tap);
    }
private:
    void compareGesturePositions(KTwoFingerTap *tap)
    {
        QCOMPARE(tap->pos(), mTouchPointPos);
        QCOMPARE(tap->screenPos(), mWidget.mapToGlobal(mTouchPointPos));
        QCOMPARE(tap->hotSpot(), mWidget.mapToGlobal(mTouchPointPos));
        QCOMPARE(tap->scenePos(), mWidget.mapToGlobal(mTouchPointPos));
        QVERIFY(tap->hasHotSpot());
    }

    void clearSignalSpys()
    {
        mSpyTapFinished->clear();
        mSpyTapCanceled->clear();
        mSpyTapUpdated->clear();
        mSpyTapStarted->clear();
    }

private Q_SLOTS:
    void cleanupTestCase()
    {
        delete mSpyTapFinished;
        delete mSpyTapCanceled;
        delete mSpyTapUpdated;
        delete mSpyTapStarted;
    }

    void initTestCase()
    {
        mDev = QTest::createTouchDevice();

        mMainWindow.setGeometry(0, 0, 500, 500);
        mMainWindow.setCentralWidget(&mWidget);
        mMainWindow.show();

        mWidget.installEventFilter(this);
        mWidget.setAttribute(Qt::WA_AcceptTouchEvents);

        QVERIFY(QTest::qWaitForWindowActive(&mMainWindow));
        QVERIFY(QTest::qWaitForWindowActive(&mWidget));

        mTwoFingerRec = new KTwoFingerTapRecognizer();
        QVERIFY(mTwoFingerRec);

        mKTwoFingerTapGesture = QGestureRecognizer::registerRecognizer(mTwoFingerRec);
        QVERIFY(mKTwoFingerTapGesture & Qt::CustomGesture);

        mWidget.grabGesture(mKTwoFingerTapGesture);

        connect(this, &KTwoFingerSwipeTest::tapFinished, this, &KTwoFingerSwipeTest::slotTapFinished);
        connect(this, &KTwoFingerSwipeTest::tapStarted, this, &KTwoFingerSwipeTest::slotTapStarted);
        connect(this, &KTwoFingerSwipeTest::tapUpdated, this, &KTwoFingerSwipeTest::slotTapUpdated);
        connect(this, &KTwoFingerSwipeTest::tapCanceled, this, &KTwoFingerSwipeTest::slotTapCanceled);

        mSpyTapFinished = new QSignalSpy(this, &KTwoFingerSwipeTest::tapFinished);
        QVERIFY(mSpyTapFinished->isValid());

        mSpyTapStarted = new QSignalSpy(this, &KTwoFingerSwipeTest::tapStarted);
        QVERIFY(mSpyTapStarted->isValid());

        mSpyTapUpdated = new QSignalSpy(this, &KTwoFingerSwipeTest::tapUpdated);
        QVERIFY(mSpyTapUpdated->isValid());

        mSpyTapCanceled = new QSignalSpy(this, &KTwoFingerSwipeTest::tapCanceled);
        QVERIFY(mSpyTapCanceled->isValid());

        QTest::qWait(1000);
    }

    void testChangeTapRadius()
    {
        QVERIFY(mTwoFingerRec->tapRadius() >= 0);

        mTwoFingerRec->setTapRadius(10);
        QCOMPARE(mTwoFingerRec->tapRadius(), 10);

        mTwoFingerRec->setTapRadius(-100);
        QCOMPARE(mTwoFingerRec->tapRadius(), 0);

        mTwoFingerRec->setTapRadius(40);
        QCOMPARE(mTwoFingerRec->tapRadius(), 40);
    }

    void testSuccessfulGesture()
    {
        // Test a successful gesture with two fingers, where the touch points wiggle only a little bit.

        clearSignalSpys();

        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius / 2, tapRadius / 2);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + wiggleRoom, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos + wiggleRoom, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        if (mSpyTapStarted->count() == 0) {
            QVERIFY(mSpyTapStarted->wait(1000));
        }
        if (mSpyTapFinished->count() == 0) {
            QVERIFY(mSpyTapFinished->wait(1000));
        }

        QCOMPARE(mSpyTapStarted->count(), 1);
        QCOMPARE(mSpyTapFinished->count(), 1);
        QCOMPARE(mSpyTapCanceled->count(), 0);
        QVERIFY(mSpyTapUpdated->count() > 0);
    }

    void testFailingGesture()
    {
        // Test a failing gesture with two fingers, where the touch points wiggle too much at beginning.

        clearSignalSpys();

        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius, tapRadius);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + wiggleRoom, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos + wiggleRoom, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        QCOMPARE(mSpyTapStarted->count(), 0);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 0);
        QCOMPARE(mSpyTapUpdated->count(), 0);
    }

    void testFailingGesture2()
    {
        // Test a failing gesture with two fingers, where the touch points wiggle too much in the middle.
        clearSignalSpys();
        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius, tapRadius);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos + wiggleRoom, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        if (mSpyTapStarted->count() == 0) {
            QVERIFY(mSpyTapStarted->wait(1000));
        }
        QCOMPARE(mSpyTapStarted->count(), 1);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 1);
        QVERIFY(mSpyTapUpdated->count() >= 0);
    }

    void testFailingGesture3()
    {
        // Test a failing gesture with two fingers, where the touch points wiggle too much in the middle,
        // and the second finger press and release successively.

        clearSignalSpys();
        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius, tapRadius);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos + wiggleRoom, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        if (mSpyTapStarted->count() == 0) {
            QVERIFY(mSpyTapStarted->wait(1000));
        }
        QCOMPARE(mSpyTapStarted->count(), 1);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 1);
        QVERIFY(mSpyTapUpdated->count() >= 0);
    }

    void testFailingGesture_threeFingers()
    {
        // the test a gesture with three fingers should be end with gesture canceled.
        clearSignalSpys();
        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius / 2, tapRadius / 2);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos + wiggleRoom, (QWidget *) nullptr)
            .press(2, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        if (mSpyTapStarted->count() == 0) {
            QVERIFY(mSpyTapStarted->wait(1000));
        }
        QCOMPARE(mSpyTapStarted->count(), 1);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 1);
        QVERIFY(mSpyTapUpdated->count() >= 0);
    }

    void testFailingGesture_threeFingers2()
    {

        clearSignalSpys();

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos, (QWidget *) nullptr)
            .press(2, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .move(1, mTouchPointPos, (QWidget *) nullptr)
            .release(3, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .press(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(1, mTouchPointPos, (QWidget *) nullptr);

        if (mSpyTapStarted->count() == 0) {
            QVERIFY(mSpyTapStarted->wait(1000));
        }
        QCOMPARE(mSpyTapStarted->count(), 1);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 1);
        QVERIFY(mSpyTapUpdated->count() >= 0);
    }

    void testFailingGesture_oneFinger()
    {
        // Test a failing gesture where we use only one finger.
        clearSignalSpys();
        int tapRadius = mTwoFingerRec->tapRadius();
        QPoint wiggleRoom = QPoint (tapRadius / 2, tapRadius / 2);

        QTest::touchEvent(&mWidget, mDev)
            .press(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos + wiggleRoom, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .move(0, mTouchPointPos, (QWidget *) nullptr);

        QTest::touchEvent(&mWidget, mDev)
            .release(0, mTouchPointPos, (QWidget *) nullptr);

        QCOMPARE(mSpyTapStarted->count(), 0);
        QCOMPARE(mSpyTapFinished->count(), 0);
        QCOMPARE(mSpyTapCanceled->count(), 0);
        QCOMPARE(mSpyTapUpdated->count(), 0);
    }
    void testSomeRandomTaps()
    {
        for (int i = 0; i < 50; ++i) {
            int v1 = rand() % 450;
            int v2 = rand() % 450;
            mTouchPointPos = QPoint(v1, v2);
            testSuccessfulGesture();
        }
    }
};

QTEST_MAIN(KTwoFingerSwipeTest)

#include "ktwofingertaptest.moc"
