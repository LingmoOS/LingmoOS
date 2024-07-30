/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeUpower.h"
#include "fakelogind.h"
#include "qtest_dbus.h"

#include <QDBusConnection>
#include <QSignalSpy>
#include <QTest>
#include <Solid/AcPluggedJob>
#include <Solid/Inhibition>
#include <Solid/InhibitionJob>
#include <Solid/Power>
#include <Solid/RequestStateJob>
#include <Solid/StatesJob>

using namespace Solid;
class solidFreedesktopTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testAcPluggedJob();
    void testAcPluggedChanged();
    void testAddInhibition();
    void testSupportedStates();
    void testRequestState();

private:
    FakeUpower *m_fakeUPower;
    FakeLogind *m_fakeLogind;
};

void solidFreedesktopTest::initTestCase()
{
    qputenv("SOLID_POWER_BACKEND", "FREE_DESKTOP");

    m_fakeUPower = new FakeUpower(this);
    QDBusConnection::systemBus().registerService(QStringLiteral("org.freedesktop.UPower"));
    QDBusConnection::systemBus().registerObject(QStringLiteral("/org/freedesktop/UPower"), m_fakeUPower, QDBusConnection::ExportAllContents);

    m_fakeLogind = new FakeLogind(this);
    QDBusConnection::systemBus().registerService(QStringLiteral("org.freedesktop.login1"));
    QDBusConnection::systemBus().registerObject(QStringLiteral("/org/freedesktop/login1"), m_fakeLogind, QDBusConnection::ExportAllContents);
}

void solidFreedesktopTest::testAcPluggedJob()
{
    m_fakeUPower->m_onBattery = true;
    auto job = new AcPluggedJob(this);
    QVERIFY(job->exec());
    QCOMPARE(job->isPlugged(), false);

    m_fakeUPower->m_onBattery = false;
    job = Solid::Power::isAcPlugged();
    QVERIFY(job->exec());
    QCOMPARE(job->isPlugged(), true);
}

void solidFreedesktopTest::testAcPluggedChanged()
{
    auto power = Solid::Power::self();
    QSignalSpy spy(power, SIGNAL(acPluggedChanged(bool)));

    m_fakeUPower->setOnBattery(true);
    m_fakeUPower->setOnBattery(false);
    spy.wait(10000);

    QCOMPARE(spy.count(), 2);

    QCOMPARE(spy.first().first().toBool(), false);
    QCOMPARE(spy.at(1).first().toBool(), true);
}

void solidFreedesktopTest::testAddInhibition()
{
    QSignalSpy spy(m_fakeLogind, SIGNAL(newInhibition(QString, QString, QString, QString)));
    auto job = new InhibitionJob(this);
    job->setDescription(QStringLiteral("Foo! I am inhibing!"));
    job->setInhibitions(Power::Shutdown | Power::Sleep);

    job->exec();

    QCOMPARE(spy.count(), 1);

    QSignalSpy spyRemoved(m_fakeLogind, SIGNAL(inhibitionRemoved()));
    auto inhibition = job->inhibition();
    inhibition->stop();

    spyRemoved.wait(100);
    QCOMPARE(spy.count(), 1);

    QSignalSpy spyStatusChanged(inhibition, SIGNAL(stateChanged(Inhibition::State)));
    inhibition->start();
    spyStatusChanged.wait(100);

    QCOMPARE(spyStatusChanged.count(), 1);

    delete inhibition;
    spyRemoved.wait(100);
    QCOMPARE(spyRemoved.count(), 2);
}

void solidFreedesktopTest::testSupportedStates()
{
}

void solidFreedesktopTest::testRequestState()
{
}

QTEST_GUILESS_MAIN_SYSTEM_DBUS(solidFreedesktopTest)

#include "solidfreedesktoptest.moc"
