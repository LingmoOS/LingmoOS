/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
// own
#include "../ksldapp.h"
// KDE Frameworks
#include <KIdleTime>
// Qt
#include <QProcess>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <QtTest>
// xcb
#include <xcb/xcb.h>
#include <xcb/xtest.h>

class KSldTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testEstablishGrab();
    void testActivateOnTimeout();
    void testGraceTimeUnlocking();
};

void KSldTest::initTestCase()
{
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets);
    // change to the build bin dir
    QDir::setCurrent(QCoreApplication::applicationDirPath());
}

void KSldTest::testEstablishGrab()
{
    ScreenLocker::KSldApp ksld;
    ksld.initialize();
    QVERIFY(ksld.establishGrab());
    // grab is established, trying again should succeed as well
    QVERIFY(ksld.establishGrab());

    // let's ungrab
    ksld.doUnlock();

    // to get the grab to fail we need another X client
    // we start another process to perform our grab
    QProcess keyboardGrabber;
    keyboardGrabber.start(QStringLiteral("./keyboardGrabber"), QStringList());
    QVERIFY(keyboardGrabber.waitForStarted());

    // let's add some delay to be sure that keyboardGrabber has it's X stuff done
    QTest::qWait(100);

    // now grabbing should fail
    QVERIFY(!ksld.establishGrab());

    // let's terminate again
    keyboardGrabber.terminate();
    QVERIFY(keyboardGrabber.waitForFinished());

    // now grabbing should succeed again
    QVERIFY(ksld.establishGrab());

    ksld.doUnlock();

    // now the same with pointer
    QProcess pointerGrabber;
    pointerGrabber.start(QStringLiteral("./pointerGrabber"), QStringList());
    QVERIFY(pointerGrabber.waitForStarted());

    // let's add some delay to be sure that pointerGrabber has it's X stuff done
    QTest::qWait(100);

    // now grabbing should fail
    QVERIFY(!ksld.establishGrab());

    // let's terminate again
    pointerGrabber.terminate();
    QVERIFY(pointerGrabber.waitForFinished());

    // now grabbing should succeed again
    QVERIFY(ksld.establishGrab());
}

void KSldTest::testActivateOnTimeout()
{
    // this time verifies that the screen gets locked on idle timeout, requires the system to be idle
    ScreenLocker::KSldApp ksld;
    ksld.initialize();

    // we need to modify the idle timeout of KSLD, it's in minutes we cannot wait that long
    if (ksld.idleId() != 0) {
        // remove old Idle id
        KIdleTime::instance()->removeIdleTimeout(ksld.idleId());
    }
    ksld.setIdleId(KIdleTime::instance()->addIdleTimeout(5000));

    QSignalSpy lockStateChangedSpy(&ksld, &ScreenLocker::KSldApp::lockStateChanged);
    QVERIFY(lockStateChangedSpy.isValid());

    // let's wait the double of the idle timeout
    QVERIFY(lockStateChangedSpy.wait(10000));
    QCOMPARE(ksld.lockState(), ScreenLocker::KSldApp::AcquiringLock);

    // let's simulate unlock to get rid of started greeter process
    const auto children = ksld.children();
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (qstrcmp((*it)->metaObject()->className(), "LogindIntegration") != 0) {
            continue;
        }
        QMetaObject::invokeMethod(*it, "requestUnlock");
        break;
    }
    QVERIFY(lockStateChangedSpy.wait());

    KIdleTime::instance()->removeIdleTimeout(ksld.idleId());
}

void KSldTest::testGraceTimeUnlocking()
{
    // this time verifies that the screen gets unlocked during grace time by simulated user activity
    ScreenLocker::KSldApp ksld(this);
    ksld.initialize();

    // we need to modify the idle timeout of KSLD, it's in minutes we cannot wait that long
    if (ksld.idleId() != 0) {
        // remove old Idle id
        KIdleTime::instance()->removeIdleTimeout(ksld.idleId());
    }
    ksld.setIdleId(KIdleTime::instance()->addIdleTimeout(5000));
    // infinite
    ksld.setGraceTime(-1);

    QSignalSpy lockedSpy(&ksld, &ScreenLocker::KSldApp::locked);
    QVERIFY(lockedSpy.isValid());
    QSignalSpy unlockedSpy(&ksld, &ScreenLocker::KSldApp::unlocked);
    QVERIFY(unlockedSpy.isValid());

    // let's wait quite some time to give the greeter a chance to come up
    QVERIFY(lockedSpy.wait(30000));
    QCOMPARE(ksld.lockState(), ScreenLocker::KSldApp::Locked);

    // let's simulate unlock by faking input
    const QPoint cursorPos = QCursor::pos();
    xcb_test_fake_input(QX11Info::connection(), XCB_MOTION_NOTIFY, 0, XCB_TIME_CURRENT_TIME, XCB_WINDOW_NONE, cursorPos.x() + 1, cursorPos.y() + 1, 0);
    xcb_flush(QX11Info::connection());
    QVERIFY(unlockedSpy.wait());

    // now let's test again without grace time
    ksld.setGraceTime(0);
    QVERIFY(lockedSpy.wait(30000));
    QCOMPARE(ksld.lockState(), ScreenLocker::KSldApp::Locked);

    xcb_test_fake_input(QX11Info::connection(), XCB_MOTION_NOTIFY, 0, XCB_TIME_CURRENT_TIME, XCB_WINDOW_NONE, cursorPos.x(), cursorPos.y(), 0);
    xcb_flush(QX11Info::connection());
    QVERIFY(!unlockedSpy.wait());

    // and unlock
    const auto children = ksld.children();
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (qstrcmp((*it)->metaObject()->className(), "LogindIntegration") != 0) {
            continue;
        }
        QMetaObject::invokeMethod(*it, "requestUnlock");
        break;
    }
    QVERIFY(unlockedSpy.wait());
}

QTEST_MAIN(KSldTest)
#include "ksldtest.moc"
