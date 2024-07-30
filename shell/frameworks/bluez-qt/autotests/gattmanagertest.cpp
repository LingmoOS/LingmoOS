/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattmanagertest.h"
#include "autotests.h"
#include "gattmanager.h"
#include "gattservice.h"
#include "initmanagerjob.h"
#include "manager.h"
#include "pendingcall.h"

#include <QDBusObjectPath>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

void GattManagerTest::initTestCase()
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
    QVERIFY(m_adapter->gattManager());

    m_application = new GattApplication(QStringLiteral("/org/kde/bluezqt"), this);
    auto service = new GattService(QStringLiteral("ad100000-d901-11e8-9f8b-f2801f1b9fd1"), true, m_application);
    m_characteristic = new GattCharacteristic(QStringLiteral("ad10e100-d901-11e8-9f8b-f2801f1b9fd1"), service);
    m_adapter->gattManager()->registerApplication(m_application)->waitForFinished();

    // Let FakeBluez read local characteristic
    QVariantMap params;
    params.insert(QStringLiteral("AdapterPath"), QVariant::fromValue(QDBusObjectPath(m_adapter->ubi())));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("adapter-gattmanager:get-objects"), params);
    // Process events to let getManagedObjects call from FakeBluez be finished
    QTest::qWait(0);
}

void GattManagerTest::cleanupTestCase()
{
    FakeBluez::stop();
}

void GattManagerTest::readCharcTest()
{
    QCOMPARE(m_characteristic->readValue(), QByteArray());

    bool readCallbackCalled = false;
    m_characteristic->setReadCallback([&readCallbackCalled]() {
        readCallbackCalled = true;
        return QByteArray("1234");
    });
    QVariantMap params;
    params.insert(QStringLiteral("AdapterPath"), QVariant::fromValue(QDBusObjectPath(m_adapter->ubi())));
    params.insert(QStringLiteral("Options"), QVariant::fromValue(QVariantMap()));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("adapter-gattmanager:read-charc"), params);

    QTRY_COMPARE(readCallbackCalled, true);
    QTRY_COMPARE(m_characteristic->readValue(), QByteArray("1234"));
}

void GattManagerTest::writeCharcTest()
{
    m_characteristic->setReadCallback(nullptr);

    QVariantMap params;
    params.insert(QStringLiteral("AdapterPath"), QVariant::fromValue(QDBusObjectPath(m_adapter->ubi())));
    params.insert(QStringLiteral("Value"), QVariant::fromValue(QByteArray("4321")));
    params.insert(QStringLiteral("Options"), QVariant::fromValue(QVariantMap()));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("adapter-gattmanager:write-charc"), params);

    QTRY_COMPARE(m_characteristic->readValue(), QByteArray("4321"));
}

QTEST_MAIN(GattManagerTest)

#include "moc_gattmanagertest.cpp"
