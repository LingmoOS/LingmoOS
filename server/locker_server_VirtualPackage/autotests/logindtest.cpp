/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../logind.h"
#include "fakelogind.h"
// Qt
#include <QtTest>

class LogindTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLockUnlock();
    void testLogindPresent();
    void testRegisterUnregister();
};

void LogindTest::testLockUnlock()
{
    QScopedPointer<LogindIntegration> logindIntegration(new LogindIntegration(QDBusConnection::sessionBus(), this));
    QSignalSpy lockSpy(logindIntegration.data(), SIGNAL(requestLock()));
    QSignalSpy unlockSpy(logindIntegration.data(), SIGNAL(requestUnlock()));
    QSignalSpy connectedSpy(logindIntegration.data(), SIGNAL(connectedChanged()));

    FakeLogind fakeLogind;

    // need to wait till we got the pending reply
    QVERIFY(connectedSpy.wait());
    QVERIFY(logindIntegration->isConnected());

    fakeLogind.lock();
    QVERIFY(lockSpy.wait());
    fakeLogind.lock();

    QVERIFY(lockSpy.wait());
    QCOMPARE(lockSpy.count(), 2);

    fakeLogind.unlock();
    QVERIFY(unlockSpy.wait());
    QCOMPARE(unlockSpy.count(), 1);
}

void LogindTest::testLogindPresent()
{
    QTest::qWait(100);
    FakeLogind fakeLogind;
    QScopedPointer<LogindIntegration> logindIntegration(new LogindIntegration(QDBusConnection::sessionBus(), this));

    QSignalSpy connectedSpy(logindIntegration.data(), SIGNAL(connectedChanged()));
    QVERIFY(connectedSpy.wait());
    QVERIFY(logindIntegration->isConnected());

    QSignalSpy lockSpy(logindIntegration.data(), SIGNAL(requestLock()));
    fakeLogind.lock();
    QVERIFY(lockSpy.wait());
    QCOMPARE(lockSpy.count(), 1);
}

void LogindTest::testRegisterUnregister()
{
    QTest::qWait(100);
    QScopedPointer<FakeLogind> fakeLogind(new FakeLogind(this));
    QScopedPointer<LogindIntegration> logindIntegration(new LogindIntegration(QDBusConnection::sessionBus(), this));

    // should get connected
    QSignalSpy connectedSpy(logindIntegration.data(), SIGNAL(connectedChanged()));
    QVERIFY(connectedSpy.wait());
    QVERIFY(logindIntegration->isConnected());
    connectedSpy.clear();

    fakeLogind.reset();
    // should no longer be connected
    QVERIFY(connectedSpy.wait());
    QVERIFY(!logindIntegration->isConnected());
    connectedSpy.clear();

    fakeLogind.reset(new FakeLogind(this));
    // should be connected again
    QVERIFY(connectedSpy.wait());
    QVERIFY(logindIntegration->isConnected());
}

QTEST_MAIN(LogindTest)
#include "logindtest.moc"
