/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "inputtest.h"
#include "autotests.h"
#include "initmanagerjob.h"
#include "pendingcall.h"

#include <QTest>

namespace BluezQt
{
extern void bluezqt_initFakeBluezTestRun();
}

using namespace BluezQt;

Q_DECLARE_METATYPE(Input::ReconnectMode)

InputTest::InputTest()
    : m_manager(nullptr)
{
    Autotests::registerMetatypes();

    qRegisterMetaType<Input::ReconnectMode>("ReconnectMode");
}

void InputTest::initTestCase()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString service = QStringLiteral("org.kde.bluezqt.fakebluez");

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

    // Create devices
    QVariantMap deviceProps;
    QVariantMap inputProps;

    QString device1 = adapter + QLatin1String("/dev_40_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device1));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("40:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("00001124-0000-1000-8000-00805F9B34FB"));
    inputProps[QStringLiteral("ReconnectMode")] = QStringLiteral("none");
    deviceProps[QStringLiteral("Input")] = inputProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    QString device2 = adapter + QLatin1String("/dev_50_79_6A_0C_39_75");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device2));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("50:79:6A:0C:39:75");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice2");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("00001124-0000-1000-8000-00805F9B34FB"));
    inputProps[QStringLiteral("ReconnectMode")] = QStringLiteral("host");
    deviceProps[QStringLiteral("Input")] = inputProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    QString device3 = adapter + QLatin1String("/dev_60_79_6B_0C_39_55");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device3));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("60:79:6B:0C:39:55");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice3");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("00001124-0000-1000-8000-00805F9B34FB"));
    inputProps[QStringLiteral("ReconnectMode")] = QStringLiteral("device");
    deviceProps[QStringLiteral("Input")] = inputProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    QString device4 = adapter + QLatin1String("/dev_70_79_6B_0C_39_55");
    deviceProps[QStringLiteral("Path")] = QVariant::fromValue(QDBusObjectPath(device4));
    deviceProps[QStringLiteral("Adapter")] = QVariant::fromValue(QDBusObjectPath(adapter));
    deviceProps[QStringLiteral("Address")] = QStringLiteral("70:79:6B:0C:39:55");
    deviceProps[QStringLiteral("Name")] = QStringLiteral("TestDevice4");
    deviceProps[QStringLiteral("UUIDs")] = QStringList(QStringLiteral("00001124-0000-1000-8000-00805F9B34FB"));
    inputProps[QStringLiteral("ReconnectMode")] = QStringLiteral("any");
    deviceProps[QStringLiteral("Input")] = inputProps;
    FakeBluez::runAction(QStringLiteral("devicemanager"), QStringLiteral("create-device"), deviceProps);

    m_manager = new Manager();
    InitManagerJob *initJob = m_manager->init();
    initJob->exec();
    QVERIFY(!initJob->error());

    for (DevicePtr device : m_manager->devices()) {
        QVERIFY(device->input());

        InputUnit u;
        u.device = device;
        u.dbusInput = new org::bluez::Input1(service, device->ubi(), connection, this);
        m_units.append(u);
    }

    QCOMPARE(m_manager->adapters().count(), 1);
    QCOMPARE(m_manager->devices().count(), 4);
}

void InputTest::cleanupTestCase()
{
    for (const InputUnit &unit : m_units) {
        delete unit.dbusInput;
    }

    delete m_manager;

    FakeBluez::stop();
}

void InputTest::getPropertiesTest()
{
    for (const InputUnit &unit : m_units) {
        QCOMPARE(reconnectModeString(unit.device->input()), unit.dbusInput->reconnectMode());
    }
}

QString InputTest::reconnectModeString(const InputPtr &input) const
{
    switch (input->reconnectMode()) {
    case Input::NoReconnect:
        return QStringLiteral("none");
    case Input::HostReconnect:
        return QStringLiteral("host");
    case Input::DeviceReconnect:
        return QStringLiteral("device");
    default:
        return QStringLiteral("any");
    }
}

QTEST_MAIN(InputTest)

#include "moc_inputtest.cpp"
