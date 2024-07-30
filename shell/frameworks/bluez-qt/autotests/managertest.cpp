/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "managertest.h"
#include "adapter.h"
#include "autotests.h"
#include "device.h"
#include "initmanagerjob.h"
#include "manager.h"

#include <QDBusObjectPath>
#include <QSignalSpy>
#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

void ManagerTest::initTestCase()
{
    bluezqt_initFakeBluezTestRun();
    FakeBluez::start(); // to check that it works
    Autotests::registerMetatypes();
}

void ManagerTest::cleanup()
{
    FakeBluez::stop();
}

void ManagerTest::bluezNotRunningTest()
{
    // org.bluez is not running at all
    // expected: init successful
    auto manager = std::make_unique<Manager>();
    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(!manager->isOperational());
    QVERIFY(!manager->isBluetoothOperational());
}

void ManagerTest::bluezNotExportingInterfacesTest()
{
    // org.bluez is running, but it does not export any interfaces
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-not-exporting-interfaces"));

    auto manager = std::make_unique<Manager>();
    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());
    QVERIFY(!manager->isBluetoothOperational());
}

void ManagerTest::bluezEmptyManagedObjectsTest()
{
    // org.bluez exports ObjectManager, but there is no AgentManager1
    // expected: init error
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-empty-managed-objects"));

    auto manager = std::make_unique<Manager>();
    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(job->error());
    QVERIFY(!manager->isInitialized());
    QVERIFY(!manager->isOperational());
    QVERIFY(!manager->isBluetoothOperational());
}

void ManagerTest::bluezNoAdaptersTest()
{
    // org.bluez exports ObjectManager with AgentManager1, but there are no adapters
    // expected: init successful
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-no-adapters"));

    auto manager = std::make_unique<Manager>();
    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(manager->isOperational());
    QVERIFY(!manager->isBluetoothOperational());
}

void ManagerTest::bluezShutdownTest()
{
    // tests whether the adapter/device removed signals work correctly

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapters
    QDBusObjectPath adapter1path = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    adapterProps[QStringLiteral("Powered")] = true;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    QDBusObjectPath adapter2path = QDBusObjectPath(QStringLiteral("/org/bluez/hci1"));
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter2path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("2E:3A:C3:BC:85:7C");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter2");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create devices
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter1path);
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter2path);
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    auto manager = std::make_unique<Manager>();

    QSignalSpy btOperationalChangedSpy(manager.get(), SIGNAL(bluetoothOperationalChanged(bool)));

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(manager->isOperational());
    QVERIFY(manager->isBluetoothOperational());
    QCOMPARE(manager->adapters().count(), 2);
    QCOMPARE(manager->devices().count(), 2);

    QCOMPARE(btOperationalChangedSpy.count(), 1);
    QCOMPARE(btOperationalChangedSpy.first().first().toBool(), true);

    AdapterPtr adapter1 = manager->adapterForAddress(QStringLiteral("1C:E5:C3:BC:94:7E"));
    AdapterPtr adapter2 = manager->adapterForAddress(QStringLiteral("2E:3A:C3:BC:85:7C"));
    DevicePtr device1 = manager->deviceForAddress(QStringLiteral("40:79:6A:0C:39:75"));
    DevicePtr device2 = manager->deviceForAddress(QStringLiteral("50:79:6A:0C:39:75"));

    QVERIFY(adapter1);
    QVERIFY(adapter2);
    QVERIFY(device1);
    QVERIFY(device2);

    QSignalSpy allAdaptersRemovedSpy(manager.get(), SIGNAL(allAdaptersRemoved()));
    QSignalSpy adapterRemovedSpy(manager.get(), SIGNAL(adapterRemoved(AdapterPtr)));
    QSignalSpy device1RemovedSpy(adapter1.data(), SIGNAL(deviceRemoved(DevicePtr)));
    QSignalSpy device2RemovedSpy(adapter2.data(), SIGNAL(deviceRemoved(DevicePtr)));

    btOperationalChangedSpy.clear();
    FakeBluez::stop();

    // allAdaptersRemoved will be last signal
    QTRY_COMPARE(allAdaptersRemovedSpy.count(), 1);

    QCOMPARE(adapterRemovedSpy.count(), 2);
    QCOMPARE(device1RemovedSpy.count(), 1);
    QCOMPARE(device2RemovedSpy.count(), 1);

    QCOMPARE(btOperationalChangedSpy.count(), 1);
    QCOMPARE(btOperationalChangedSpy.first().first().toBool(), false);
}

void ManagerTest::usableAdapterTest()
{
    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapters
    QDBusObjectPath adapter1path = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    adapterProps[QStringLiteral("Powered")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    QDBusObjectPath adapter2path = QDBusObjectPath(QStringLiteral("/org/bluez/hci1"));
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter2path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("2E:3A:C3:BC:85:7C");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter2");
    adapterProps[QStringLiteral("Powered")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    auto manager = std::make_unique<Manager>();

    QSignalSpy usableAdapterChangedSpy(manager.get(), SIGNAL(usableAdapterChanged(AdapterPtr)));

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());
    QVERIFY(manager->isInitialized());
    QVERIFY(manager->isOperational());
    QVERIFY(!manager->isBluetoothOperational());
    QCOMPARE(usableAdapterChangedSpy.count(), 0);
    QCOMPARE(manager->adapters().count(), 2);
    QCOMPARE(manager->devices().count(), 0);

    QVariantMap properties;
    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    properties[QStringLiteral("Name")] = QStringLiteral("Powered");
    properties[QStringLiteral("Value")] = true;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    QTRY_COMPARE(usableAdapterChangedSpy.count(), 1);
    QCOMPARE(manager->usableAdapter()->ubi(), adapter1path.path());

    usableAdapterChangedSpy.clear();

    properties[QStringLiteral("Value")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    QTRY_COMPARE(usableAdapterChangedSpy.count(), 1);
    QVERIFY(manager->usableAdapter().isNull());

    usableAdapterChangedSpy.clear();

    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter2path);
    properties[QStringLiteral("Value")] = true;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    QTRY_COMPARE(usableAdapterChangedSpy.count(), 1);
    QCOMPARE(manager->usableAdapter()->ubi(), adapter2path.path());
}

void ManagerTest::deviceForAddressTest()
{
    // tests whether the deviceForAddress correctly prefer powered adapters

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapters
    QDBusObjectPath adapter1path = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    adapterProps[QStringLiteral("Powered")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    QDBusObjectPath adapter2path = QDBusObjectPath(QStringLiteral("/org/bluez/hci1"));
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter2path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("2E:3A:C3:BC:85:7C");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter2");
    adapterProps[QStringLiteral("Powered")] = true;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create devices
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter1path);
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter2path);
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    auto manager = std::make_unique<Manager>();

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());

    QString address(QStringLiteral("40:79:6A:0C:39:75"));
    AdapterPtr adapter1 = manager->adapterForUbi(QStringLiteral("/org/bluez/hci0"));
    AdapterPtr adapter2 = manager->adapterForUbi(QStringLiteral("/org/bluez/hci1"));

    QVERIFY(adapter1);
    QVERIFY(adapter2);

    QSignalSpy adapter1Spy(adapter1.data(), SIGNAL(poweredChanged(bool)));
    QSignalSpy adapter2Spy(adapter2.data(), SIGNAL(poweredChanged(bool)));

    QCOMPARE(manager->deviceForAddress(address)->adapter(), adapter2);

    QVariantMap properties;
    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    properties[QStringLiteral("Name")] = QStringLiteral("Powered");
    properties[QStringLiteral("Value")] = true;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter2path);
    properties[QStringLiteral("Value")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    QTRY_COMPARE(adapter1Spy.count(), 1);
    QTRY_COMPARE(adapter2Spy.count(), 1);

    QCOMPARE(manager->deviceForAddress(address)->adapter(), adapter1);

    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    properties[QStringLiteral("Value")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("change-adapter-property"), properties);

    QTRY_COMPARE(adapter1Spy.count(), 2);
    QVERIFY(manager->deviceForAddress(address));
}

void ManagerTest::adapterWithDevicesRemovedTest()
{
    // tests whether the devices are always removed from the adapter before removing adapter

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapters
    QDBusObjectPath adapter1path = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    adapterProps[QStringLiteral("Powered")] = false;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create devices
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter1path);
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_50_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(adapter1path);
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    auto manager = std::make_unique<Manager>();

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());

    AdapterPtr adapter = manager->adapterForAddress(QStringLiteral("1C:E5:C3:BC:94:7E"));
    DevicePtr device1 = manager->deviceForAddress(QStringLiteral("40:79:6A:0C:39:75"));
    DevicePtr device2 = manager->deviceForAddress(QStringLiteral("50:79:6A:0C:39:75"));

    QVERIFY(adapter);
    QVERIFY(device1);
    QVERIFY(device2);

    QSignalSpy adapterRemovedSpy(manager.get(), SIGNAL(adapterRemoved(AdapterPtr)));
    QSignalSpy deviceRemovedSpy(manager.get(), SIGNAL(deviceRemoved(DevicePtr)));

    QVariantMap properties;
    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("remove-adapter"), properties);

    QTRY_COMPARE(adapterRemovedSpy.count(), 1);
    QTRY_COMPARE(deviceRemovedSpy.count(), 2);
    QCOMPARE(manager->adapters().count(), 0);
    QCOMPARE(manager->devices().count(), 0);
    QCOMPARE(adapter->devices().count(), 0);
}

void ManagerTest::bug364416()
{
    // Bug 364416: Crash when device is added with adapter that is unknown to Manager

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    auto manager = std::make_unique<Manager>();

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());

    // Add device to invalid adapter
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/bluez/hci0")));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    // Wait for Manager to receive the interfacesAdded signal
    QTest::qWait(100);
}

void ManagerTest::bug377405()
{
    // Bug 377405: Property changes immediately after adapter is added are lost

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    auto manager = std::make_unique<Manager>();

    InitManagerJob *job = manager->init();
    job->exec();

    QVERIFY(!job->error());

    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("bug377405"));

    // Adapter property Powered is changed to true immediately after being added
    QTRY_COMPARE(manager->isBluetoothOperational(), true);
}

QTEST_MAIN(ManagerTest)

#include "moc_managertest.cpp"
