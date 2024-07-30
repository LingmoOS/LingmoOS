/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptorremotetest.h"
#include "adapter.h"
#include "autotests.h"
#include "device.h"
#include "gattcharacteristicremote.h"
#include "gattserviceremote.h"
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

GattDescriptorRemoteTest::GattDescriptorRemoteTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();
    qRegisterMetaType<BluezQt::GattDescriptorRemotePtr>("GattDescriptorRemotePtr");
}

void GattDescriptorRemoteTest::initTestCase()
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

    // Create descriptors
    QVariantMap descrProps;
    descrProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75/service0/char0/descriptor0"));
    descrProps[QStringLiteral("UUID")] = QStringLiteral("04FA28C0-2D0C-11EC-8D3D-0242AC130005");
    descrProps[QStringLiteral("Characteristic")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci0/dev_40_79_6A_0C_39_75/service0/char0"));
    descrProps[QStringLiteral("Value")] = QVariant::fromValue(QByteArray());
    descrProps[QStringLiteral("Flags")] = QStringList({QStringLiteral("read"), QStringLiteral("write")});
    descrProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(5));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-descriptor"), descrProps);

    descrProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75/service0/char0/descriptor0"));
    descrProps[QStringLiteral("UUID")] = QStringLiteral("0663A394-9A76-4361-9DE6-82577B82AC9C");
    descrProps[QStringLiteral("Characteristic")] = QVariant::fromValue(QDBusObjectPath("/org/bluez/hci1/dev_50_79_6A_0C_39_75/service0/char0"));
    descrProps[QStringLiteral("Value")] = QVariant::fromValue(QByteArray());
    descrProps[QStringLiteral("Flags")] = QStringList({QStringLiteral("read"), QStringLiteral("write")});
    descrProps[QStringLiteral("Handle")] = QVariant::fromValue(qint16(6));
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-gatt-descriptor"), descrProps);


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

                    for (const GattDescriptorRemotePtr &gattDescriptor : gattCharacteristic->descriptors()) {
                        QVERIFY(!gattDescriptor->ubi().isEmpty());

                        GattDescriptorRemoteUnit u;
                        u.descriptor = gattDescriptor;
                        u.dbusDescriptor = new org::bluez::GattDescriptor1(service, gattDescriptor->ubi(), connection, this);
                        u.dbusProperties = new org::freedesktop::DBus::Properties(service, gattDescriptor->ubi(), connection, this);
                        m_units.append(u);
                    }

                    QCOMPARE(gattCharacteristic->descriptors().count(), 1);
                }

                QCOMPARE(gattService->characteristics().count(), 1);
            }

            QCOMPARE(device->gattServices().count(), 1);
        }
    }

    QCOMPARE(m_manager->adapters().count(), 2);
    QCOMPARE(m_manager->devices().count(), 2);
}

void GattDescriptorRemoteTest::cleanupTestCase()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        delete unit.dbusDescriptor;
        delete unit.dbusProperties;
    }

    delete m_manager;

    FakeBluez::stop();
}

void GattDescriptorRemoteTest::getPropertiesTest()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        QCOMPARE(unit.descriptor->ubi(), unit.dbusDescriptor->path());
        QCOMPARE(unit.descriptor->uuid(), unit.dbusDescriptor->uUID());
        QCOMPARE(unit.descriptor->characteristic()->ubi(), unit.dbusDescriptor->characteristic().path());
        QCOMPARE(unit.descriptor->value(), unit.dbusDescriptor->value());
        QCOMPARE(unit.descriptor->flags(), unit.dbusDescriptor->flags());
        QCOMPARE(unit.descriptor->handle(), unit.dbusDescriptor->handle());
    }
}

void GattDescriptorRemoteTest::setHandleTest()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy descriptorSpy(unit.descriptor.data(), SIGNAL(handleChanged(quint16)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));

        quint16 value = unit.descriptor->handle() + 3;

        unit.descriptor->setHandle(value);
        QTRY_COMPARE(descriptorSpy.count(), 1);

        QList<QVariant> arguments = descriptorSpy.takeFirst();
        QCOMPARE(arguments.at(0).toUInt(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Handle"), value);

        QCOMPARE(unit.descriptor->handle(), value);
        QCOMPARE(unit.dbusDescriptor->handle(), value);
    }
}

void GattDescriptorRemoteTest::readValueTest()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy descriptorSpy(unit.descriptor.data(), SIGNAL(valueChanged(const QByteArray)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));


        // Test initial value
        QByteArray value = QByteArray();
        QCOMPARE(unit.descriptor->value(), value);
        QCOMPARE(unit.dbusDescriptor->value(), value);


        // Test read
        value = QByteArray("TEST");
        unit.descriptor->readValue({});
        QTRY_COMPARE(descriptorSpy.count(), 1);

        QList<QVariant> arguments = descriptorSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<QByteArray>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Value"), value);

        QCOMPARE(unit.descriptor->value(), value);
        QCOMPARE(unit.dbusDescriptor->value(), value);
    }
}

void GattDescriptorRemoteTest::writeValueTest()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy descriptorSpy(unit.descriptor.data(), SIGNAL(valueChanged(const QByteArray)));
        QSignalSpy dbusSpy(unit.dbusProperties, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)));

        // Test write
        QByteArray value = QByteArray("WRITE");
        unit.descriptor->writeValue(value, {});
        QTRY_COMPARE(descriptorSpy.count(), 1);

        QList<QVariant> arguments = descriptorSpy.takeFirst();
        QCOMPARE(arguments.at(0).value<QByteArray>(), value);
        Autotests::verifyPropertiesChangedSignal(dbusSpy, QStringLiteral("Value"), value);

        QCOMPARE(unit.descriptor->value(), value);
        QCOMPARE(unit.dbusDescriptor->value(), value);
    }
}

void GattDescriptorRemoteTest::descriptorRemovedTest()
{
    for (const GattDescriptorRemoteUnit &unit : std::as_const(m_units)) {
        QSignalSpy characteristicSpy(unit.descriptor->characteristic().data(), SIGNAL(gattDescriptorRemoved(GattDescriptorRemotePtr)));

        QVariantMap properties;
        properties[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(unit.descriptor->ubi()));
        FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("remove-gatt-descriptor"), properties);

        QTRY_COMPARE(characteristicSpy.count(), 1);

        QCOMPARE(characteristicSpy.at(0).at(0).value<GattDescriptorRemotePtr>(), unit.descriptor);
    }
}

QTEST_MAIN(GattDescriptorRemoteTest)

#include "moc_gattdescriptorremotetest.cpp"
