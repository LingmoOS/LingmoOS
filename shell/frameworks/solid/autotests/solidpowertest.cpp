/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QSignalSpy>
#include <QTest>
#include <Solid/AcPluggedJob>
#include <Solid/Inhibition>
#include <Solid/InhibitionJob>
#include <Solid/Power>
#include <Solid/RequestStateJob>
#include <Solid/StatesJob>

using namespace Solid;
class solidPowerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testAcPluggedJob();
    void testAcPluggedChanged();
    void testAddInhibition();
    void testSupportedStates();
    void testRequestState();
};

void solidPowerTest::initTestCase()
{
    qputenv("SOLID_POWER_BACKEND", "DUMMY");
}

void solidPowerTest::testAcPluggedJob()
{
    AcPluggedJob *job = new AcPluggedJob();
    QVERIFY(job->exec());
    QVERIFY(job->isPlugged());

    job = Power::isAcPlugged();
    QVERIFY(job->exec());
    QVERIFY(job->isPlugged());
}

void solidPowerTest::testAcPluggedChanged()
{
    Power *power = Power::self();
    QSignalSpy spy(power, SIGNAL(acPluggedChanged(bool)));

    QVERIFY(spy.wait());
    QVERIFY(spy.takeFirst().first().toBool());
}

void solidPowerTest::testAddInhibition()
{
    InhibitionJob *job = new InhibitionJob();
    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)InhibitionJob::InvalidInhibitions);
    delete job;

    job = new InhibitionJob();
    job->setInhibitions(Power::Sleep);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), (int)InhibitionJob::EmptyDescription);
    delete job;

    job = Power::inhibit(Power::Sleep, QLatin1String("Running a test, we don't want to suspend now"));
    QVERIFY(job->exec());

    auto inhibition = job->inhibition();
    QSignalSpy spy(inhibition, SIGNAL(stateChanged(Inhibition::State)));
    inhibition->deleteLater();

    QVERIFY(spy.wait());
    QCOMPARE(spy.takeFirst().first().toInt(), (int)Inhibition::Stopped);
}

void solidPowerTest::testSupportedStates()
{
    auto job = new StatesJob();
    QVERIFY(job->exec());

    QCOMPARE(job->states(), Power::Shutdown | Power::Sleep);

    job = Power::supportedStates();
    QVERIFY(job->exec());

    QCOMPARE(job->states(), Power::Shutdown | Power::Sleep);
}

void solidPowerTest::testRequestState()
{
    auto job = new RequestStateJob();
    job->setState(Power::Sleep);
    QVERIFY(job->exec());

    job = Power::requestState(Power::Sleep);
    QVERIFY(job->exec());

    job = Power::requestState(Power::Shutdown);
    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)RequestStateJob::Unsupported);
    QCOMPARE(job->errorText(), QLatin1String(QLatin1String("State Brightness is unsupported")));
}

QTEST_MAIN(solidPowerTest)

#include "solidpowertest.moc"
