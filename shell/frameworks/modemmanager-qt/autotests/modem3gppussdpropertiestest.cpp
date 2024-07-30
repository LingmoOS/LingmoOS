/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gppussdpropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modem3gppussd.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void Modem3gppUssdPropertiesTest::initTestCase()
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

    modem3gppUssd = new Modem3gppUssd(modem);
    modem3gppUssd->setModemPath(modem->modemPath());
    modem3gppUssd->setNetworkNotification(QLatin1String("notification"));
    modem3gppUssd->setNetworkRequest(QLatin1String("request"));
    modem3gppUssd->setState(MM_MODEM_3GPP_USSD_SESSION_STATE_UNKNOWN);

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD), modem3gppUssd->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modem3gppUssd->setEnableNotifications(true);
}

void Modem3gppUssdPropertiesTest::testModem3gppUssdProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::GsmUssdInterface));
    ModemManager::Modem3gppUssd::Ptr modem3gppUssdInterface =
        modemDevice->interface(ModemManager::ModemDevice::GsmUssdInterface).objectCast<ModemManager::Modem3gppUssd>();
    QVERIFY(modem3gppUssdInterface);

    modem3gppUssd->setNetworkNotification(QLatin1String("notification1"));
    QSignalSpy networkNotificationChangedSpy(modem3gppUssdInterface.data(), SIGNAL(networkNotificationChanged(QString)));
    QVERIFY(networkNotificationChangedSpy.wait());
    QCOMPARE(modem3gppUssdInterface->networkNotification(), networkNotificationChangedSpy.at(0).at(0).toString());

    modem3gppUssd->setNetworkRequest(QLatin1String("notification1"));
    QSignalSpy networkRequestChangedSpy(modem3gppUssdInterface.data(), SIGNAL(networkRequestChanged(QString)));
    QVERIFY(networkRequestChangedSpy.wait());
    QCOMPARE(modem3gppUssdInterface->networkRequest(), networkRequestChangedSpy.at(0).at(0).toString());

    modem3gppUssd->setState(MM_MODEM_3GPP_USSD_SESSION_STATE_IDLE);
    QSignalSpy stateChangedSpy(modem3gppUssdInterface.data(), SIGNAL(stateChanged(MMModem3gppUssdSessionState)));
    QVERIFY(stateChangedSpy.wait());
    QCOMPARE(modem3gppUssdInterface->state(), stateChangedSpy.at(0).at(0).value<MMModem3gppUssdSessionState>());
}

QTEST_MAIN(Modem3gppUssdPropertiesTest)

#include "moc_modem3gppussdpropertiestest.cpp"
