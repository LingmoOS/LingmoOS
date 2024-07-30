/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattserviceremotetest.h"
#include "adapter.h"
#include "autotests.h"
#include "device.h"
#include "initmanagerjob.h"
#include "pendingcall.h"

#include <QSignalSpy>
#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

GattServiceRemoteTest::GattServiceRemoteTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();
}

void GattServiceRemoteTest::initTestCase()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.kde.bluezqt.fakebluez");

    bluezqt_initFakeBluezTestRun();

    FakeBluez::start();
    FakeBluez::runTest(QStringLiteral("bluez-standard"));

    // Create adapters
    QDBusObjectPath adapter1 = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("1C:E5:C3:BC:94:7E");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    QDBusObjectPath adapter2 = QDBusObjectPath(QStringLiteral("/org/bluez/hci1"));
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter2);
    adapterProps[QStringLiteral("Address")] = QStringLiteral("2E:3A:C3:BC:85:7C");
    adapterProps[QStringLiteral("Name")] = QStringLiteral("TestAdapter2");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-adapter"), adapterProps);

    // Create devices
    QVariantMap deviceProps;
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0"));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    deviceProps[QStringLiteral("Alias")] = QStringLiteral("TestAlias");
    deviceProps[QStringLiteral("Icon")] = QStringLiteral("phone");
    deviceProps[QStringLiteral("Class")] = QVariant::fromValue(quint32(101));
    deviceProps[QStringLiteral("Appearance")] = QVariant::fromValue(quint16(25));
    deviceProps[QStringLiteral("UUIDs")] = QStringList();
    deviceProps[QStringLiteral("Paired")] = false;
    deviceProps[QStringLiteral("Connected")] = false;
    deviceProps[QStringLiteral("Trusted")] = false;
    deviceProps[QStringLiteral("Blocked")] = false;
    deviceProps[QStringLiteral("LegacyPairing")] = false;
    deviceProps[QStringLiteral("RSSI")] = QVariant::fromValue(qint16(20));
    deviceProps[QStringLiteral("Modalias")] = QStringLiteral("bluetooth:v001Dp1200d1236");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75"));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1"));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    deviceProps[QStringLiteral("Alias")] = QStringLiteral("TestAlias2");
    deviceProps[QStringLiteral("Icon")] = QStringLiteral("joypad");
    deviceProps[QStringLiteral("Class")] = QVariant::fromValue(quint32(201));
    deviceProps[QStringLiteral("Appearance")] = QVariant::fromValue(quint16(32));
    deviceProps[QStringLiteral("UUIDs")] = QStringList();
    deviceProps[QStringLiteral("Paired")] = true;
    deviceProps[QStringLiteral("Connected")] = false;
    deviceProps[QStringLiteral("Trusted")] = true;
    deviceProps[QStringLiteral("Blocked")] = false;
    deviceProps[QStringLiteral("LegacyPairing")] = false;
    deviceProps[QStringLiteral("RSSI")] = QVariant::fromValue(qint16(-15));
    deviceProps[QStringLiteral("Modalias")] = QStringLiteral("bluetooth:v001Dp1100d1236");
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    // Create services
    QVariantMap serviceProps;
    serviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75/service0"));
    serviceProps[QStringLiteral("UUID")] = QStringLiteral("04FA28C0-2D0C-11EC-8D3D-0242AC130003");
    serviceProps[QStringLiteral("Primary")] = true;
    serviceProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75"));
    serviceProps[QStringLiteral("Includes")] = QVariant::fromValue(QList<QDBusObjectPath>());
    serviceProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(1));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-service"), serviceProps);

    serviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75/service0"));
    serviceProps[QStringLiteral("UUID")] = QStringLiteral("0663A394-9A76-4361-9DE6-82577B82AC9A");
    serviceProps[QStringLiteral("Primary")] = true;
    serviceProps[QStringLiteral("Device")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75"));
    serviceProps[QStringLiteral("Includes")] = QVariant::fromValue(QList<QDBusObjectPath>());
    serviceProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(2));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-service"), serviceProps);

    m_manager = new Manager();
    InitManagerJob *initJob = m_manager->init();
    initJob->exec();
    QVERIFY(!initJob->error());

    for (const AdapterPtr &adapter : m_manager->adapters()) {
        QVERIFY(!adapter->ubi().isEmpty());

        for (const DevicePtr &device : adapter->devices()) {
            QVERIFY(!device->ubi().isEmpty());

            for (const GattServiceRemotePtr &gattService : device->gattServices()) {
                QVERIFY(!gattService->ubi().isEmpty());

                GattServiceRemoteUnit u;
                u.service = gattService;
                u.dbusService = new org::bluez::GattService1(service, gattService->ubi(), connection, this);
                u.dbusProperties = new org::freedesktop::DBus::Properties(service, gattService->ubi(), connection, this);
                m_units.append(u);
            }

            QCOMPARE(device->gattServices().count(), 1);
        }
    }

    QCOMPARE(m_manager->adapters().count(), 2);
    QCOMPARE(m_manager->devices().count(), 2);
}

void GattServiceRemoteTest::cleanupTestCase()
{
    for (const GattServiceRemoteUnit &unit : std::as_const(m_units)) {
        delete unit.dbusService;
        delete unit.dbusProperties;
    }

    delete m_manager;

    FakeBluez::stop();
}

void GattServiceRemoteTest::getPropertiesTest()
{
    for (const GattServiceRemoteUnit &unit : std::as_const(m_units)) {
        QCOMPARE(unit.service->ubi(), unit.dbusService->path());
        QCOMPARE(unit.service->uuid(), unit.dbusService->uUID());
        QCOMPARE(unit.service->isPrimary(), unit.dbusService->primary());
        QCOMPARE(unit.service->device()->ubi(), unit.dbusService->device().path());
        QCOMPARE(unit.service->includes(), unit.dbusService->includes());
        QCOMPARE(unit.service->handle(), unit.dbusService->handle());
    }
}

void GattServiceRemoteTest::setHandleTest()
{
    for (const GattServiceRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy serviceSpy(unit.service.data(), SIGNAL(handleChanged(quint16)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));

        quint16 value = unit.service->handle() + 3;

        unit.service->setHandle(value);
        QTRY_COMPARE(serviceSpy.count(), 1);

        QList<QVariant> arguments = serviceSpy.takeFirst();
        QCOMPARE(arguments.at(0).toUInt(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Handle"), value);

        QCOMPARE(unit.service->handle(), value);
        QCOMPARE(unit.dbusService->handle(), value);
    }
}

void GattServiceRemoteTest::serviceRemovedTest()
{
    for (const GattServiceRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy deviceSpy(unit.service->device().data(), SIGNAL(gattServiceRemoved(GattServiceRemotePtr)));

        QVariantMap properties;
        properties[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(unit.service->ubi()));
        FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("remove-gatt-service"), properties);

        QTRY_COMPARE(deviceSpy.count(), 1);

        QCOMPARE(deviceSpy.at(0).at(0).value<GattServiceRemotePtr>(), unit.service);
    }
}

QTEST_MAIN(GattServiceRemoteTest)

#include "moc_gattserviceremotetest.cpp"
