/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activeconnectiontest.h"

#include "device.h"
#include "manager.h"
#include "settings.h"
#include "settings/ipv4setting.h"
#include "settings/ipv6setting.h"
#include "settings/wiredsetting.h"

#include "fakenetwork/settings.h"
#include "fakenetwork/wireddevice.h"

#include <QSignalSpy>
#include <QTest>

void ActiveConnectionTest::initTestCase()
{
    fakeNetwork = new FakeNetwork();

    WiredDevice *device = new WiredDevice();
    /* Device properties */
    device->setAutoconnect(true);
    device->setCapabilities(3);
    device->setDeviceType(1);
    device->setDriver(QLatin1String("e1000e"));
    device->setDriverVersion(QLatin1String("2.3.2-k"));
    device->setFirmwareMissing(false);
    device->setFirmwareVersion(QLatin1String("0.13-3"));
    device->setInterface(QLatin1String("em1"));
    device->setInterfaceFlags(NetworkManager::Device::Interfaceflag::Up);
    device->setManaged(true);
    device->setUdi(QLatin1String("/sys/devices/pci0000:00/0000:00:19.0/net/em1"));

    /* Wired device properties */
    device->setCarrier(true);
    device->setHwAddress(QLatin1String("F0:DE:F1:FB:30:C1"));
    device->setPermanentHwAddress(QLatin1String("F0:DE:F1:FB:30:C1"));

    fakeNetwork->addDevice(device);

    NetworkManager::ConnectionSettings::Ptr connectionSettings =
        NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wired));
    connectionSettings->setId("Wired connection");
    connectionSettings->setUuid(QLatin1String("39af79a5-b053-4893-9378-7342a5a30d06"));
    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4Setting->setInitialized(true);
    ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Automatic);
    NetworkManager::Ipv6Setting::Ptr ipv6Setting = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6Setting->setInitialized(true);
    ipv6Setting->setMethod(NetworkManager::Ipv6Setting::Automatic);
    NetworkManager::WiredSetting::Ptr wiredSetting = connectionSettings->setting(NetworkManager::Setting::Wired).dynamicCast<NetworkManager::WiredSetting>();
    wiredSetting->setInitialized(true);
    // Something needs to be set to not use default values, when using default values we get an empty map
    wiredSetting->setSpeed(100);

    NetworkManager::Device::Ptr wiredDevice = NetworkManager::networkInterfaces().first().objectCast<NetworkManager::Device>();
    QSignalSpy availableConnectionAppearedSpy(wiredDevice.data(), SIGNAL(availableConnectionAppeared(QString)));
    NetworkManager::addConnection(connectionSettings->toMap());
    QVERIFY(availableConnectionAppearedSpy.wait());
}

void ActiveConnectionTest::testActiveConnection()
{
    qRegisterMetaType<NetworkManager::ActiveConnection::State>("NetworkManager::ActiveConnection::State");
    qRegisterMetaType<NetworkManager::ActiveConnection::Reason>("NetworkManager::ActiveConnection::Reason");
    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    qRegisterMetaType<NetworkManager::Connectivity>("NetworkManager::Connectivity");
    qRegisterMetaType<NetworkManager::Status>("NetworkManager::Status");

    NetworkManager::Device::Ptr device = NetworkManager::networkInterfaces().first();
    QSignalSpy deviceActiveConnectionChangedSpy(device.data(), SIGNAL(activeConnectionChanged()));
    QSignalSpy deviceIpInterfaceChangedSpy(device.data(), SIGNAL(ipInterfaceChanged()));
    QSignalSpy deviceStateChangedSpy(
        device.data(),
        SIGNAL(stateChanged(NetworkManager::Device::State, NetworkManager::Device::State, NetworkManager::Device::StateChangeReason)));

    NetworkManager::Connection::Ptr connection = device->availableConnections().first();

    QSignalSpy activeConnectionAddedSpy(NetworkManager::notifier(), SIGNAL(activeConnectionAdded(QString)));
    QSignalSpy activatingConnectionChangedSpy(NetworkManager::notifier(), SIGNAL(activatingConnectionChanged(QString)));
    QSignalSpy connectivityChangedSpy(NetworkManager::notifier(), SIGNAL(connectivityChanged(NetworkManager::Connectivity)));
    QSignalSpy primaryConnectionChangedSpy(NetworkManager::notifier(), SIGNAL(primaryConnectionChanged(QString)));
    QSignalSpy stateChangedSpy(NetworkManager::notifier(), SIGNAL(statusChanged(NetworkManager::Status)));

    NetworkManager::activateConnection(connection->path(), device->uni(), QString());
    QVERIFY(activeConnectionAddedSpy.wait());

    NetworkManager::ActiveConnection::Ptr activeConnection = NetworkManager::findActiveConnection(activeConnectionAddedSpy.at(0).at(0).toString());
    QSignalSpy activeConnectionStateChangedSpy(activeConnection.data(), SIGNAL(stateChanged(NetworkManager::ActiveConnection::State)));
    QSignalSpy activeConnectionStateChangedReasonSpy(
        activeConnection.data(),
        SIGNAL(stateChangedReason(NetworkManager::ActiveConnection::State, NetworkManager::ActiveConnection::Reason)));

    QCOMPARE(activeConnection->devices().first(), device->uni());
    QCOMPARE(activeConnection->state(), NetworkManager::ActiveConnection::Activating);
    QCOMPARE(activeConnection->uuid(), connection->uuid());

    QCOMPARE(NetworkManager::activatingConnection()->uuid(), connection->uuid());

    // Wait until device goes through all states (PREPARE, CONFIG, NEED_AUTH, IP_CONFIG, IP_CHECK, ACTIVATED)
    QTest::qWait(800);

    QCOMPARE(activeConnectionStateChangedSpy.count(), 1);
    QCOMPARE(activeConnectionStateChangedReasonSpy.count(), 1);

    QCOMPARE(deviceStateChangedSpy.count(), 6);
    QCOMPARE(device->ipInterfaceName(), device->interfaceName());
    QCOMPARE(device->state(), NetworkManager::Device::Activated);
    QCOMPARE(deviceActiveConnectionChangedSpy.count(), 1);
    QCOMPARE(deviceIpInterfaceChangedSpy.count(), 1);

    QCOMPARE(NetworkManager::connectivity(), NetworkManager::Full);
    QCOMPARE(NetworkManager::primaryConnection()->uuid(), connection->uuid());
    QCOMPARE(NetworkManager::status(), NetworkManager::Status::Connected);
    QCOMPARE(activatingConnectionChangedSpy.count(), 2);
    QCOMPARE(connectivityChangedSpy.count(), 1);
    QCOMPARE(primaryConnectionChangedSpy.count(), 1);
    QCOMPARE(stateChangedSpy.count(), 1);

    NetworkManager::deactivateConnection(activeConnection->path());

    // Wait until we are disconnected
    QTest::qWait(300);

    QVERIFY(device->ipInterfaceName().isEmpty());
    QCOMPARE(device->state(), NetworkManager::Device::Disconnected);

    QCOMPARE(NetworkManager::activeConnections().count(), 0);
    QCOMPARE(NetworkManager::connectivity(), NetworkManager::NoConnectivity);
    QVERIFY(NetworkManager::primaryConnection().isNull());
    QCOMPARE(NetworkManager::status(), NetworkManager::Disconnected);
}

QTEST_MAIN(ActiveConnectionTest)

#include "moc_activeconnectiontest.cpp"
