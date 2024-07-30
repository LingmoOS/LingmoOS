/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisingmanagertest.h"
#include "autotests.h"
#include "initmanagerjob.h"
#include "leadvertisingmanager.h"
#include "manager.h"
#include "pendingcall.h"

#include <QDBusObjectPath>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

void TestAdvertisement::release()
{
    m_releaseCalled = true;
}

void LEAdvertisingManagerTest::initTestCase()
{
    bluezqt_initFakeBluezTestRun();
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapter
    QString adapter = QStringLiteral("/org/bluez/hci0");
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(adapter));
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    Manager *manager = new Manager(this);
    InitManagerJob *initJob = manager->init();
    initJob->exec();
    QVERIFY(!initJob->error());
    QCOMPARE(manager->adapters().count(), 1);

    m_adapter = manager->adapters().at(0);
    QVERIFY(m_adapter->leAdvertisingManager());

    m_advertisement = new TestAdvertisement({QStringLiteral("ad100000-d901-11e8-9f8b-f2801f1b9fd1")}, this);
    auto call = m_adapter->leAdvertisingManager()->registerAdvertisement(m_advertisement);
    call->waitForFinished();
    QVERIFY(!call->error());
}

void LEAdvertisingManagerTest::cleanupTestCase()
{
    FakeBluez::stop();
}

void LEAdvertisingManagerTest::releaseTest()
{
    QCOMPARE(m_advertisement->m_releaseCalled, false);

    QVariantMap params;
    params.insert(QStringLiteral("AdapterPath"), QVariant::fromValue(QDBusObjectPath(m_adapter->ubi())));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("adapter-leadvertisingmanager:release"), params);

    QTRY_COMPARE(m_advertisement->m_releaseCalled, true);
}

QTEST_MAIN(LEAdvertisingManagerTest)

#include "moc_leadvertisingmanagertest.cpp"
