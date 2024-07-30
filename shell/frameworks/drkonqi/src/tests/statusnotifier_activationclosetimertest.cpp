/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <QSignalSpy>
#include <QTest>

#include <config-drkonqi.h>
#include <statusnotifier_activationclosetimer.h>

class MockDBusServiceWatcher : public DBusServiceWatcher
{
    Q_OBJECT
public:
    using DBusServiceWatcher::DBusServiceWatcher;
    void start() override
    {
    }
};

class MockIdleWatcher : public IdleWatcher
{
    Q_OBJECT
public:
    using IdleWatcher::IdleWatcher;
    void start() override
    {
    }
};

class ActivationCloseTimerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void serviceLostTest()
    {
        ActivationCloseTimer timer;
        timer.setActivationTimeout(1s);
        timer.setCloseTimeout(2s);

        MockDBusServiceWatcher dbus;
        MockIdleWatcher idle;
        QSignalSpy activateSpy(&timer, &ActivationCloseTimer::autoActivate);
        QSignalSpy closeSpy(&timer, &ActivationCloseTimer::autoClose);
        timer.start(&dbus, &idle);
        Q_EMIT dbus.serviceUnregistered();
        QVERIFY(activateSpy.wait());
        QVERIFY(closeSpy.empty());
    }

    void serviceLostThenBackTest()
    {
        ActivationCloseTimer timer;
        timer.setActivationTimeout(1s);
        timer.setCloseTimeout(2s);

        MockDBusServiceWatcher dbus;
        MockIdleWatcher idle;
        QSignalSpy activateSpy(&timer, &ActivationCloseTimer::autoActivate);
        QSignalSpy closeSpy(&timer, &ActivationCloseTimer::autoClose);
        timer.start(&dbus, &idle);
        Q_EMIT dbus.serviceUnregistered();
        Q_EMIT dbus.serviceRegistered();
        QVERIFY(!activateSpy.wait());
        QVERIFY(!closeSpy.empty()); // seeing as the service returned we should receive an auto close
    }

    void idleSystemLostAServiceTest()
    {
        ActivationCloseTimer timer;
        timer.setActivationTimeout(1s);
        timer.setCloseTimeout(2s);

        MockDBusServiceWatcher dbus;
        MockIdleWatcher idle;
        QSignalSpy activateSpy(&timer, &ActivationCloseTimer::autoActivate);
        QSignalSpy closeSpy(&timer, &ActivationCloseTimer::autoClose);
        timer.start(&dbus, &idle);
        Q_EMIT idle.idle();
        Q_EMIT dbus.serviceUnregistered();
        QVERIFY(activateSpy.wait());
        QVERIFY(closeSpy.empty());
    }

    void idleSystemHasAllServices()
    {
        ActivationCloseTimer timer;
        timer.setActivationTimeout(1s);
        timer.setCloseTimeout(2s);

        MockDBusServiceWatcher dbus;
        MockIdleWatcher idle;
        QSignalSpy activateSpy(&timer, &ActivationCloseTimer::autoActivate);
        QSignalSpy closeSpy(&timer, &ActivationCloseTimer::autoClose);
        timer.start(&dbus, &idle);
        Q_EMIT idle.idle();
        QVERIFY(!closeSpy.wait());
        QVERIFY(activateSpy.empty());
    }

    void justAutoCloseTest()
    {
        ActivationCloseTimer timer;
        timer.setActivationTimeout(1s);
        timer.setCloseTimeout(2s);

        MockDBusServiceWatcher dbus;
        MockIdleWatcher idle;
        QSignalSpy activateSpy(&timer, &ActivationCloseTimer::autoActivate);
        QSignalSpy closeSpy(&timer, &ActivationCloseTimer::autoClose);
        timer.start(&dbus, &idle);
        QVERIFY(closeSpy.wait());
        QVERIFY(activateSpy.empty());
    }
};

QTEST_GUILESS_MAIN(ActivationCloseTimerTest)

#include "statusnotifier_activationclosetimertest.moc"
