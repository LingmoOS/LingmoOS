/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristicremotetest.h"

#include "adapter.h"
#include "autotests.h"
#include "device.h"
#include "initmanagerjob.h"
#include "pendingcall.h"

#include <QSignalSpy>
#include <QTest>
#include <QDebug>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

GattCharacteristicRemoteTest::GattCharacteristicRemoteTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();
}

void GattCharacteristicRemoteTest::initTestCase()
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

    // Create characteristics
    QVariantMap charProps;
    charProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75/service0/char0"));
    charProps[QStringLiteral("UUID")] = QStringLiteral("04FA28C0-2D0C-11EC-8D3D-0242AC130004");
    charProps[QStringLiteral("Service")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75/service0"));
    charProps[QStringLiteral("Value")] = QVariant::fromValue(QByteArray());
    charProps[QStringLiteral("Notifying")] = false;
    charProps[QStringLiteral("Flags")] = QStringList({QStringLiteral("read"), QStringLiteral("write")});
    charProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(3));
    charProps[QStringLiteral("MTU")] = QVariant::fromValue(qint16(512));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-characteristic"), charProps);

    charProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75/service0/char0"));
    charProps[QStringLiteral("UUID")] = QStringLiteral("0663A394-9A76-4361-9DE6-82577B82AC9B");
    charProps[QStringLiteral("Service")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75/service0"));
    charProps[QStringLiteral("Value")] = QVariant::fromValue(QByteArray());
    charProps[QStringLiteral("Notifying")] = false;
    charProps[QStringLiteral("Flags")] = QStringList({QStringLiteral("read"), QStringLiteral("write")});
    charProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(4));
    charProps[QStringLiteral("MTU")] = QVariant::fromValue(qint16(512));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-characteristic"), charProps);


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

                for (const GattCharacteristicRemotePtr &gattCharacteristic : gattService->characteristics()) {
                    QVERIFY(!gattCharacteristic->ubi().isEmpty());

                    GattCharacteristicRemoteUnit u;
                    u.characteristic = gattCharacteristic;
                    u.dbusCharacteristic = new org::bluez::GattCharacteristic1(service, gattCharacteristic->ubi(), connection, this);
                    u.dbusProperties = new org::freedesktop::DBus::Properties(service, gattCharacteristic->ubi(), connection, this);
                    m_units.append(u);
                }

                QCOMPARE(gattService->characteristics().count(), 1);
            }

            QCOMPARE(device->gattServices().count(), 1);
        }
    }

    QCOMPARE(m_manager->adapters().count(), 2);
    QCOMPARE(m_manager->devices().count(), 2);
}

void GattCharacteristicRemoteTest::cleanupTestCase()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        delete unit.dbusCharacteristic;
        delete unit.dbusProperties;
    }

    delete m_manager;

    FakeBluez::stop();
}

void GattCharacteristicRemoteTest::getPropertiesTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QCOMPARE(unit.characteristic->ubi(), unit.dbusCharacteristic->path());
        QCOMPARE(unit.characteristic->uuid(), unit.dbusCharacteristic->uUID());
        QCOMPARE(unit.characteristic->service()->ubi(), unit.dbusCharacteristic->service().path());
        QCOMPARE(unit.characteristic->value(), unit.dbusCharacteristic->value());
        QCOMPARE(unit.characteristic->isNotifying(), unit.dbusCharacteristic->notifying());
        QCOMPARE(unit.characteristic->flags(), unit.dbusCharacteristic->flags());
        QCOMPARE(unit.characteristic->handle(), unit.dbusCharacteristic->handle());
        QCOMPARE(unit.characteristic->MTU(), unit.dbusCharacteristic->mTU());
    }
}

void GattCharacteristicRemoteTest::setHandleTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.characteristic.data(), SIGNAL(handleChanged(quint16)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));

        quint16 value = unit.characteristic->handle() + 3;

        unit.characteristic->setHandle(value);
        QTRY_COMPARE(characteristicSpy.count(), 1);

        QList<QVariant> arguments = characteristicSpy.takeFirst();
        QCOMPARE(arguments.at(0).toUInt(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Handle"), value);

        QCOMPARE(unit.characteristic->handle(), value);
        QCOMPARE(unit.dbusCharacteristic->handle(), value);
    }
}

void GattCharacteristicRemoteTest::readValueTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.characteristic.data(), SIGNAL(valueChanged(const QByteArray)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));


        // Test initial value
        QByteArray value = QByteArray();
        QCOMPARE(unit.characteristic->value(), value);
        QCOMPARE(unit.dbusCharacteristic->value(), value);


        // Test read
        value = QByteArray("TEST");
        unit.characteristic->readValue({});
        QTRY_COMPARE(characteristicSpy.count(), 1);

        QList<QVariant> arguments = characteristicSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<QByteArray>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Value"), value);

        QCOMPARE(unit.characteristic->value(), value);
        QCOMPARE(unit.dbusCharacteristic->value(), value);
    }
}

void GattCharacteristicRemoteTest::writeValueTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.characteristic.data(), SIGNAL(valueChanged(const QByteArray)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));

        // Test write
        QByteArray value = QByteArray("WRITE");
        unit.characteristic->writeValue(value, {});
        QTRY_COMPARE(characteristicSpy.count(), 1);

        QList<QVariant> arguments = characteristicSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<QByteArray>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Value"), value);

        QCOMPARE(unit.characteristic->value(), value);
        QCOMPARE(unit.dbusCharacteristic->value(), value);
    }
}

void GattCharacteristicRemoteTest::startNotifyTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.characteristic.data(), SIGNAL(notifyingChanged(bool)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));


        // Test initial value
        bool value = false;
        QCOMPARE(unit.characteristic->isNotifying(), value);
        QCOMPARE(unit.dbusCharacteristic->notifying(), value);


        // Test start notifying
        value = true;
        unit.characteristic->startNotify();
        QTRY_COMPARE(characteristicSpy.count(), 1);

        QList<QVariant> arguments = characteristicSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<bool>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Notifying"), value);

        QCOMPARE(unit.characteristic->isNotifying(), value);
        QCOMPARE(unit.dbusCharacteristic->notifying(), value);
    }
}

void GattCharacteristicRemoteTest::stopNotifyTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.characteristic.data(), SIGNAL(notifyingChanged(bool)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));


        // Test initial value
        bool value = true;
        QCOMPARE(unit.characteristic->isNotifying(), value);
        QCOMPARE(unit.dbusCharacteristic->notifying(), value);


        // Test start notifying
        value = false;
        unit.characteristic->stopNotify();
        QTRY_COMPARE(characteristicSpy.count(), 1);

        QList<QVariant> arguments = characteristicSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<bool>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Notifying"), value);

        QCOMPARE(unit.characteristic->isNotifying(), value);
        QCOMPARE(unit.dbusCharacteristic->notifying(), value);
    }
}

void GattCharacteristicRemoteTest::characteristicRemovedTest()
{
    for (const GattCharacteristicRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy serviceSpy(unit.characteristic->service().data(), SIGNAL(gattCharacteristicRemoved(GattCharacteristicRemotePtr)));

        QVariantMap properties;
        properties[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(unit.characteristic->ubi()));
        FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("remove-gatt-characteristic"), properties);

        QTRY_COMPARE(serviceSpy.count(), 1);

        QCOMPARE(serviceSpy.at(0).at(0).value<GattCharacteristicRemotePtr>(), unit.characteristic);
    }
}

QTEST_MAIN(GattCharacteristicRemoteTest)

#include "moc_gattcharacteristicremotetest.cpp"
