/*
    SPDX-FileCopyrightText: 2016 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDBusConnection>
#include <QObject>
#include <QStandardPaths>
#include <qtest.h>

#include "../src/knotification.h"
#include "fake_notifications_server.h"
#include "qtest_dbus.h"

class KNotificationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void gettersTest();
    void idTest();
    void immediateCloseTest();
    void serverCallTest();
    void serverCloseTest();
    void serverActionsTest();
    void noActionsTest();

private:
    NotificationsServer *m_server;
};

void KNotificationTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    qDebug() << dataDir;

    QDir dir;
    bool pathOk = dir.mkpath(dataDir + QStringLiteral("/knotifications6/"));

    QVERIFY(pathOk);

    QFile testFile(QFINDTESTDATA(QStringLiteral("knotifications6/qttest.notifyrc")));
    bool fileOk = testFile.copy(dataDir + QStringLiteral("/knotifications6/qttest.notifyrc"));

    QVERIFY(fileOk);

    m_server = new NotificationsServer(this);

    bool rs = QDBusConnection::sessionBus().registerService(QStringLiteral("org.freedesktop.Notifications"));
    bool ro = QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/Notifications"), m_server, QDBusConnection::ExportAllContents);

    QVERIFY(rs);
    QVERIFY(ro);
}

void KNotificationTest::cleanupTestCase()
{
    // Cleanup
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QFile testFile(dataDir + QStringLiteral("/knotifications6/qttest.notifyrc"));
    bool fileRemoveOk = testFile.remove();

    QVERIFY(fileRemoveOk);

    QDir dir(dataDir + QStringLiteral("/knotifications6"));
    bool dirRemoveOk = dir.removeRecursively();

    QVERIFY(dirRemoveOk);
}

void KNotificationTest::gettersTest()
{
    const QString testEvent = QStringLiteral("testEvent");
    const QString testText = QStringLiteral("Test");

    KNotification *n = new KNotification(testEvent);
    n->setText(QStringLiteral("Test"));

    QCOMPARE(n->id(), -1);
    QCOMPARE(n->eventId(), testEvent);
    QCOMPARE(n->text(), testText);
    QCOMPARE(n->title(), QString());
    QCOMPARE(n->appName(), QCoreApplication::applicationName());

    // DefaultEvent is reserved for Workspace events
    n->setFlags(KNotification::DefaultEvent);
    QCOMPARE(n->appName(), QStringLiteral("plasma_workspace"));

    n->setFlags(KNotification::CloseOnTimeout);
    n->setComponentName(QStringLiteral("testtest"));
    QCOMPARE(n->appName(), QStringLiteral("testtest"));

    QSignalSpy nClosedSpy(n, SIGNAL(closed()));
    QSignalSpy nDestroyedSpy(n, SIGNAL(destroyed(QObject *)));

    // Calling ref and deref simulates a Notification plugin
    // starting and ending an action, after the action has
    // finished, the notification should close itself
    n->ref();
    n->deref();

    QCOMPARE(nClosedSpy.size(), 1);

    // ...and delete itself too
    nDestroyedSpy.wait(500);
    QCOMPARE(nDestroyedSpy.size(), 1);
}

void KNotificationTest::idTest()
{
    KNotification n(QStringLiteral("testEvent"));

    QCOMPARE(n.id(), -1);

    n.sendEvent();

    // The first id is 0; the previous test did not
    // call sendEvent() and therefore should not have
    // increased the id counter, which starts at 0
    QCOMPARE(n.id(), 0);

    // TODO
    // At this point there is no clean/sane way
    // to test the other id() changes throughout
    // the life of KNotification, ideally there
    // would be a signal sending out the changed
    // id values that would be caught by a signal spy
    // and then checked
}

void KNotificationTest::immediateCloseTest()
{
    KNotification *n = new KNotification(QStringLiteral("testEvent"));

    QSignalSpy nClosedSpy(n, SIGNAL(closed()));

    n->close();

    nClosedSpy.wait(100);

    QCOMPARE(nClosedSpy.size(), 1);
}

void KNotificationTest::serverCallTest()
{
    const QString testText = QStringLiteral("Test");

    QSignalSpy serverNewSpy(m_server, SIGNAL(newNotification()));
    QSignalSpy serverClosedSpy(m_server, SIGNAL(NotificationClosed(uint, uint)));

    KNotification n(QStringLiteral("testEvent"));
    n.setText(testText);
    n.setFlags(KNotification::Persistent);

    n.sendEvent();

    serverNewSpy.wait(500);

    QCOMPARE(m_server->notifications.last().body, testText);
    // timeout 0 is persistent notification
    QCOMPARE(m_server->notifications.last().timeout, 0);

    QCOMPARE(n.id(), 1);

    // Give the dbus communication some time to finish
    QTest::qWait(300);

    n.close();

    serverClosedSpy.wait(500);
    QCOMPARE(serverClosedSpy.size(), 1);
    QCOMPARE(m_server->notifications.size(), 0);
}

void KNotificationTest::serverCloseTest()
{
    KNotification n(QStringLiteral("testEvent"));
    n.setText(QStringLiteral("Test"));
    n.setFlags(KNotification::Persistent);
    n.sendEvent();

    QSignalSpy nClosedSpy(&n, SIGNAL(closed()));

    // Give the dbus some time
    QTest::qWait(300);

    uint id = m_server->notifications.last().id;

    m_server->NotificationClosed(id, 2);

    nClosedSpy.wait(100);

    QCOMPARE(nClosedSpy.size(), 1);
}

void KNotificationTest::serverActionsTest()
{
    KNotification n(QStringLiteral("testEvent"));
    n.setText(QStringLiteral("Test"));
    n.setActions(QStringList{QStringLiteral("a1"), QStringLiteral("a2")});
    n.sendEvent();

    QSignalSpy serverClosedSpy(m_server, SIGNAL(NotificationClosed(uint, uint)));
    QSignalSpy nClosedSpy(&n, SIGNAL(closed()));
    QSignalSpy nActivatedSpy(&n, SIGNAL(activated(uint)));
    QSignalSpy nActivated2Spy(&n, SIGNAL(action1Activated()));

    QTest::qWait(300);

    uint id = m_server->notifications.last().id;

    Q_EMIT m_server->ActionInvoked(id, QString::number(1));

    nActivatedSpy.wait(300);
    // After the notification action was invoked,
    // the notification should request closing
    serverClosedSpy.wait(300);
    nClosedSpy.wait(300);

    QCOMPARE(serverClosedSpy.size(), 1);
    QCOMPARE(nActivatedSpy.size(), 1);
    QCOMPARE(nActivated2Spy.size(), 1);
    // The argument must be 1, as was passed to the ActionInvoked
    QCOMPARE(nActivatedSpy.at(0).at(0).toInt(), 1);
    QCOMPARE(nClosedSpy.size(), 1);
}

void KNotificationTest::noActionsTest()
{
    // event doesn't exist in config, meaning it has no actions
    QPointer<KNotification> n(new KNotification(QStringLiteral("noActionsEvent")));
    QSignalSpy nClosedSpy(n, SIGNAL(closed()));
    n->sendEvent();

    nClosedSpy.wait(100);
    QCOMPARE(nClosedSpy.size(), 1);
    QVERIFY(n.isNull());
}

QTEST_GUILESS_MAIN_SYSTEM_DBUS(KNotificationTest)
#include "knotification_test.moc"
