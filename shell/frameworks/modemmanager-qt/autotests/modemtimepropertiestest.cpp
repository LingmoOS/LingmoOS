/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemtimepropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modemtime.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void ModemTimePropertiesTest::initTestCase()
{
    fakeModem = new FakeModem();

    modem = new Modem();
    modem->setAccessTechnologies(16);
    modem->SetCurrentBands({0});
    modem->SetCurrentCapabilities(4);
    modem->SetCurrentModes({MM_MODEM_MODE_ANY, MM_MODEM_MODE_NONE});
    modem->setDevice(QLatin1String("/sys/devices/pci0000:00/0000:00:1d.0/usb4/4-1/4-1.2"));
    modem->setDeviceIdentifier(QLatin1String("1c435eb6d74494b5f78d7221e2c5ae9ec526a981"));
    modem->setDrivers({QLatin1String("option1")});
    modem->setEquipmentIdentifier(QLatin1String("353475021085110"));
    modem->setManufacturer(QLatin1String("huawei"));
    modem->setMaxActiveBearers(1);
    modem->setMaxBearers(1);
    modem->setModel(QLatin1String("K2540"));
    // modem->setOwnNumbers();
    modem->setPlugin(QLatin1String("Huawei"));
    modem->setPorts({{QLatin1String("ttyUSB0"), MM_MODEM_PORT_TYPE_AT},
                     {QLatin1String("ttyUSB1"), MM_MODEM_PORT_TYPE_QCDM},
                     {QLatin1String("ttyUSB2"), MM_MODEM_PORT_TYPE_AT}});
    modem->SetPowerState(3);
    modem->setPrimaryPort(QLatin1String("ttyUSB2"));
    modem->setRevision(QLatin1String("11.001.05.00.11"));
    modem->setSignalQuality({93, true});
    modem->setSim(QDBusObjectPath("/org/kde/fakemodem/SIM/1"));
    modem->setState(8);
    modem->setStateFailedReason(0);
    modem->setSupportedBands({0});
    modem->setSupportedCapabilities({4});
    modem->setSupportedIpFamilies(3);
    ModemManager::SupportedModesType supportedModes;
    ModemManager::CurrentModesType supportedMode1 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_NONE};
    ModemManager::CurrentModesType supportedMode2 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_2G};
    ModemManager::CurrentModesType supportedMode3 = {MM_MODEM_MODE_4G & MM_MODEM_MODE_3G & MM_MODEM_MODE_2G, MM_MODEM_MODE_3G};
    supportedModes << supportedMode1 << supportedMode2 << supportedMode3;
    modem->setSupportedModes(supportedModes);
    modem->setUnlockRequired(1);
    modem->setUnlockRetries({{MM_MODEM_LOCK_SIM_PIN, 3}, {MM_MODEM_LOCK_SIM_PIN2, 3}, {MM_MODEM_LOCK_SIM_PUK, 10}, {MM_MODEM_LOCK_SIM_PUK2, 10}});

    QSignalSpy addModemSpy(ModemManager::notifier(), SIGNAL(modemAdded(QString)));
    fakeModem->addModem(modem);
    QVERIFY(addModemSpy.wait());

    modemTime = new ModemTime(modem);
    modemTime->setModemPath(modem->modemPath());
    QVariantMap timezone;
    timezone.insert(QLatin1String("offset"), 60);
    timezone.insert(QLatin1String("dst-offset"), 60);
    timezone.insert(QLatin1String("offset"), 60);
    timezone.insert(QLatin1String("leap-seconds"), 1);
    modemTime->setNetworkTimezone(timezone);

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA), modemTime->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modemTime->setEnableNotifications(true);
}

void ModemTimePropertiesTest::testModemTimeProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::TimeInterface));
    ModemManager::ModemTime::Ptr modemTimeInterface = modemDevice->interface(ModemManager::ModemDevice::TimeInterface).objectCast<ModemManager::ModemTime>();
    QVERIFY(modemTimeInterface);

    QVariantMap timezone;
    timezone.insert(QLatin1String("offset"), 120);
    timezone.insert(QLatin1String("dst-offset"), 120);
    timezone.insert(QLatin1String("offset"), 120);
    timezone.insert(QLatin1String("leap-seconds"), 2);
    modemTime->setNetworkTimezone(timezone);
    QSignalSpy networkTimezoneChangedSpy(modemTimeInterface.data(), SIGNAL(networkTimezoneChanged(ModemManager::NetworkTimezone)));
    QVERIFY(networkTimezoneChangedSpy.wait());
    QCOMPARE(modemTimeInterface->networkTimezone().dstOffset(), networkTimezoneChangedSpy.at(0).at(0).value<ModemManager::NetworkTimezone>().dstOffset());
    QCOMPARE(modemTimeInterface->networkTimezone().offset(), networkTimezoneChangedSpy.at(0).at(0).value<ModemManager::NetworkTimezone>().offset());
    QCOMPARE(modemTimeInterface->networkTimezone().leapSecond(), networkTimezoneChangedSpy.at(0).at(0).value<ModemManager::NetworkTimezone>().leapSecond());
}

QTEST_MAIN(ModemTimePropertiesTest)

#include "moc_modemtimepropertiestest.cpp"
