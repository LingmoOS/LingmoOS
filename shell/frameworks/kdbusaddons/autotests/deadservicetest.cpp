/*
   This file is part of libkdbus

    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QProcess>
#include <QTest>

#include <signal.h>
#include <unistd.h>

static const QString s_serviceName = QStringLiteral("org.kde.kdbussimpleservice");

class TestObject : public QObject
{
    Q_OBJECT

    QList<int> m_danglingPids;
private Q_SLOTS:
    void cleanupTestCase()
    {
        // Make sure we don't leave dangling processes even when we had an
        // error and the process is stopped.
        for (int pid : m_danglingPids) {
            kill(pid, SIGKILL);
        }
    }

    void testDeadService()
    {
        QVERIFY(!QDBusConnection::sessionBus().interface()->isServiceRegistered(s_serviceName).value());

        QProcess proc1;
        proc1.setProgram(QFINDTESTDATA("kdbussimpleservice"));
        proc1.setProcessChannelMode(QProcess::ForwardedChannels);
        proc1.start();
        QVERIFY(proc1.waitForStarted());
        m_danglingPids << proc1.processId();

        // Spy isn't very suitable here because we'd be racing with proc1 or
        // signal blocking since we'd need to unblock before spying but then
        // there is an ɛ between unblock and spy.
        qint64 pid1 = proc1.processId(); // store local, in case the proc disappears
        QVERIFY(pid1 >= 0);
        bool proc1Registered = QTest::qWaitFor(
            [&]() {
                QTest::qSleep(1000);
                return QDBusConnection::sessionBus().interface()->servicePid(s_serviceName).value() == pid1;
            },
            8000);
        QVERIFY(proc1Registered);

        // suspend proc1, we don't want it responding on dbus anymore, but still
        // be running so it holds the name.
        QCOMPARE(kill(proc1.processId(), SIGSTOP), 0);

        // start second instance
        QProcess proc2;
        proc2.setProgram(QFINDTESTDATA("kdbussimpleservice"));
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("KCRASH_AUTO_RESTARTED", "1");
        proc2.setProcessEnvironment(env);
        proc2.setProcessChannelMode(QProcess::ForwardedChannels);
        proc2.start();
        QVERIFY(proc2.waitForStarted());
        m_danglingPids << proc2.processId();

        // sleep a bit. fairly awkward. we need proc2 to be waiting on the name
        // but we can't easily determine when it started waiting. in lieu of
        // better instrumentation let's just sleep a bit.
        qDebug() << "sleeping";
        QTest::qSleep(4000);

        // Let proc1 go up in flames so that dbus-daemon reclaims the name and
        // gives it to proc2.
        qDebug() << "murder on the orient express";
        QCOMPARE(0, kill(proc1.processId(), SIGUSR1));
        QCOMPARE(0, kill(proc1.processId(), SIGCONT));

        qint64 pid2 = proc2.processId(); // store local, in case the proc disappears
        QVERIFY(pid2 >= 0);
        // Wait for service to be owned by proc2.
        bool proc2Registered = QTest::qWaitFor(
            [&]() {
                QTest::qSleep(1000);
                return QDBusConnection::sessionBus().interface()->servicePid(s_serviceName).value() == pid2;
            },
            8000);
        QVERIFY(proc2Registered);

        proc1.kill();
        m_danglingPids.removeAll(pid1);
        proc2.kill();
        m_danglingPids.removeAll(pid2);
    }
};

QTEST_MAIN(TestObject)

#include "deadservicetest.moc"
