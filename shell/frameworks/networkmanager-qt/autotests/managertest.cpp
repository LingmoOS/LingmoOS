/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "managertest.h"

#include "manager.h"
#include "wireddevice.h"

#include "fakenetwork/wireddevice.h"

#include <QSignalSpy>
#include <QTest>

void ManagerTest::initTestCase()
{
    fakeNetwork = new FakeNetwork();
}

void ManagerTest::testDevices()
{
    WiredDevice *device = new WiredDevice();
    /* Device properties */
    device->setAutoconnect(true);
    device->setCapabilities(3);
    device->setDeviceType(1);
    device->setDriver(QLatin1String("e1000e"));
    device->setDriverVersion(QLatin1String("2.3.2-k"));
    device->setFirmwareMissing(false);
    device->setInterfaceFlags(NetworkManager::Device::Interfaceflag::Up);
    device->setFirmwareVersion(QLatin1String("0.13-3"));
    device->setInterface(QLatin1String("em1"));
    device->setManaged(true);
    device->setUdi(QLatin1String("/sys/devices/pci0000:00/0000:00:19.0/net/em1"));

    /* Wired device properties */
    device->setCarrier(true);
    device->setHwAddress(QLatin1String("F0:DE:F1:FB:30:C1"));
    device->setPermanentHwAddress(QLatin1String("F0:DE:F1:FB:30:C1"));

    QSignalSpy addDeviceSpy(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)));
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &ManagerTest::testDeviceAdded);
    fakeNetwork->addDevice(device);
    QVERIFY(addDeviceSpy.wait());
    QCOMPARE(NetworkManager::networkInterfaces().count(), 1);
    QCOMPARE(NetworkManager::networkInterfaces().first()->uni(), addDeviceSpy.at(0).at(0).toString());
    const QString addedDevicePath = NetworkManager::networkInterfaces().first()->uni();

    QSignalSpy removeDeviceSpy(NetworkManager::notifier(), SIGNAL(deviceRemoved(QString)));
    fakeNetwork->removeDevice(device);
    QVERIFY(removeDeviceSpy.wait());
    QVERIFY(NetworkManager::networkInterfaces().isEmpty());
    QCOMPARE(removeDeviceSpy.at(0).at(0).toString(), addedDevicePath);

    addDeviceSpy.clear();

    fakeNetwork->addDevice(device);
    QVERIFY(addDeviceSpy.wait());
    QCOMPARE(NetworkManager::networkInterfaces().count(), 1);
    QCOMPARE(NetworkManager::networkInterfaces().first()->uni(), addDeviceSpy.at(0).at(0).toString());

    addDeviceSpy.clear();
    removeDeviceSpy.clear();

    fakeNetwork->unregisterService();
    QTRY_COMPARE(removeDeviceSpy.count(), 1);

    fakeNetwork->registerService();
    QTRY_COMPARE(addDeviceSpy.count(), 1);

    // Make sure deviceAdded is emitted only once
    addDeviceSpy.wait(100);
    QCOMPARE(addDeviceSpy.count(), 1);

    delete device;
}

void ManagerTest::testDeviceAdded(const QString &dev)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(dev);

    QCOMPARE(device->autoconnect(), true);
    QCOMPARE(device->capabilities(), 3);
    QCOMPARE(device->type(), NetworkManager::Device::Ethernet);
    QCOMPARE(device->driver(), QLatin1String("e1000e"));
    QCOMPARE(device->driverVersion(), QLatin1String("2.3.2-k"));
    QCOMPARE(device->firmwareMissing(), false);
    QCOMPARE(device->firmwareVersion(), QLatin1String("0.13-3"));
    QCOMPARE(device->interfaceName(), QLatin1String("em1"));
    QCOMPARE(device->interfaceFlags(), NetworkManager::Device::Interfaceflag::Up);
    QCOMPARE(device->managed(), true);
    QCOMPARE(device->udi(), QLatin1String("/sys/devices/pci0000:00/0000:00:19.0/net/em1"));

    NetworkManager::WiredDevice::Ptr wiredDevice = device.objectCast<NetworkManager::WiredDevice>();

    QCOMPARE(wiredDevice->carrier(), true);
    QCOMPARE(wiredDevice->hardwareAddress(), QLatin1String("F0:DE:F1:FB:30:C1"));
    QCOMPARE(wiredDevice->permanentHardwareAddress(), QLatin1String("F0:DE:F1:FB:30:C1"));
}

void ManagerTest::testManager()
{
    QCOMPARE(NetworkManager::connectivity(), NetworkManager::NoConnectivity);
    //     QCOMPARE(NetworkManager::status(), NetworkManager::Disconnected);
    QCOMPARE(NetworkManager::version(), QLatin1String("0.9.10.0"));

    QSignalSpy wirelessEnabledChanged(NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)));
    QCOMPARE(NetworkManager::isWirelessEnabled(), true);
    fakeNetwork->setWirelessEnabled(false);
    QVERIFY(wirelessEnabledChanged.wait());
    QCOMPARE(wirelessEnabledChanged.count(), 1);
    QVERIFY(wirelessEnabledChanged.at(0).at(0).toBool() == false);
    QCOMPARE(NetworkManager::isWirelessEnabled(), false);

    QSignalSpy wirelessHwEnabledChanged(NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)));
    QCOMPARE(NetworkManager::isWirelessHardwareEnabled(), true);
    fakeNetwork->setWirelessHardwareEnabled(false);
    QVERIFY(wirelessHwEnabledChanged.wait());
    QCOMPARE(wirelessHwEnabledChanged.count(), 1);
    QVERIFY(wirelessHwEnabledChanged.at(0).at(0).toBool() == false);
    QCOMPARE(NetworkManager::isWirelessHardwareEnabled(), false);

    QSignalSpy wimaxEnabledChanged(NetworkManager::notifier(), SIGNAL(wimaxEnabledChanged(bool)));
    QCOMPARE(NetworkManager::isWimaxEnabled(), true);
    fakeNetwork->setWimaxEnabled(false);
    QVERIFY(wimaxEnabledChanged.wait());
    QCOMPARE(wimaxEnabledChanged.count(), 1);
    QVERIFY(wimaxEnabledChanged.at(0).at(0).toBool() == false);
    QCOMPARE(NetworkManager::isWimaxEnabled(), false);

    QSignalSpy wimaxHwEnabledChanged(NetworkManager::notifier(), SIGNAL(wimaxHardwareEnabledChanged(bool)));
    QCOMPARE(NetworkManager::isWimaxHardwareEnabled(), true);
    fakeNetwork->setWimaxHardwareEnabled(false);
    QVERIFY(wimaxHwEnabledChanged.wait());
    QCOMPARE(wimaxHwEnabledChanged.count(), 1);
    QVERIFY(wimaxHwEnabledChanged.at(0).at(0).toBool() == false);
    QCOMPARE(NetworkManager::isWimaxHardwareEnabled(), false);

    QSignalSpy wwanEnabledChanged(NetworkManager::notifier(), SIGNAL(wwanEnabledChanged(bool)));
    QCOMPARE(NetworkManager::isWwanEnabled(), true);
    fakeNetwork->setWwanEnabled(false);
    QVERIFY(wwanEnabledChanged.wait());
    QCOMPARE(wwanEnabledChanged.count(), 1);
    QVERIFY(wwanEnabledChanged.at(0).at(0).toBool() == false);
    QCOMPARE(NetworkManager::isWwanEnabled(), false);

    // We make it here, so we can set all values back for futher testing
    fakeNetwork->setWirelessEnabled(true);
    fakeNetwork->setWirelessHardwareEnabled(true);
    fakeNetwork->setWimaxEnabled(true);
    fakeNetwork->setWimaxHardwareEnabled(true);
    fakeNetwork->setWwanEnabled(true);
}

QTEST_MAIN(ManagerTest)

#include "moc_managertest.cpp"
