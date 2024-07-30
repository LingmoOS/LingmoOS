/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexmanagertest.h"
#include "autotests.h"
#include "initobexmanagerjob.h"
#include "obexmanager.h"

#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

void ObexManagerTest::initTestCase()
{
    bluezqt_initFakeBluezTestRun();
    FakeBluez::start(); // to check that it works
}

void ObexManagerTest::cleanup()
{
    FakeBluez::stop();
}

void ObexManagerTest::obexNotRunningTest()
{
    // org.bluez.obex is not running at all
    // expected: init successful
    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(!manager->isOperational());

    delete manager;
}

void ObexManagerTest::obexNotExportingInterfacesTest()
{
    // org.bluez.obex is running, but it does not export any interfaces
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("obex-not-exporting-interfaces"));

    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());

    delete manager;
}

void ObexManagerTest::obexEmptyManagedObjectsTest()
{
    // org.bluez.obex exports ObjectManager, but there is no AgentManager1 nor Client1
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("obex-empty-managed-objects"));

    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());

    delete manager;
}

void ObexManagerTest::obexNoClientTest()
{
    // org.bluez.obex exports ObjectManager and AgentManager1, but there is no Client1
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("obex-no-client"));

    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());

    delete manager;
}

void ObexManagerTest::obexNoAgentManagerTest()
{
    // org.bluez.obex exports ObjectManager and Client1, but there is no AgentManager1
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("obex-no-agentmanager"));

    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());

    delete manager;
}

void ObexManagerTest::obexServiceOkTest()
{
    // org.bluez.obex is running and exports all used interfaces
    // expected: init successful
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("obex-standard"));

    ObexManager *manager = new ObexManager;
    InitObexManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(manager->isOperational());

    delete manager;
}

QTEST_MAIN(ObexManagerTest)

#include "moc_obexmanagertest.cpp"
