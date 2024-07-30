/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gpppropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modem3gpp.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void Modem3gppPropertiesTest::initTestCase()
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

    modem3gpp = new Modem3gpp(modem);
    modem3gpp->setModemPath(modem->modemPath());
    modem3gpp->setEnabledFacilityLocks(MM_MODEM_3GPP_FACILITY_NONE);
    modem3gpp->setImei(QLatin1String("IMEI"));
    modem3gpp->setOperatorCode(QLatin1String("31026"));
    modem3gpp->setOperatorName(QLatin1String("op-name"));
    modem3gpp->setRegistrationState(MM_MODEM_3GPP_REGISTRATION_STATE_HOME);
#if MM_CHECK_VERSION(1, 2, 0)
    modem3gpp->setSubscriptionState(MM_MODEM_3GPP_SUBSCRIPTION_STATE_UNKNOWN);
#endif

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP), modem3gpp->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modem3gpp->setEnableNotifications(true);
}

void Modem3gppPropertiesTest::testModem3gppProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::GsmInterface));
    ModemManager::Modem3gpp::Ptr modem3gppInterface = modemDevice->interface(ModemManager::ModemDevice::GsmInterface).objectCast<ModemManager::Modem3gpp>();
    QVERIFY(modem3gppInterface);

    modem3gpp->setEnabledFacilityLocks(MM_MODEM_3GPP_FACILITY_SIM);
    QSignalSpy enabledFacilityLocksChangedSpy(modem3gppInterface.data(), SIGNAL(enabledFacilityLocksChanged(QFlags<MMModem3gppFacility>)));
    QVERIFY(enabledFacilityLocksChangedSpy.wait());
    QCOMPARE(modem3gppInterface->enabledFacilityLocks(), enabledFacilityLocksChangedSpy.at(0).at(0).value<QFlags<MMModem3gppFacility>>());

    modem3gpp->setImei(QLatin1String("IMEI2"));
    QSignalSpy imeiChangedSpy(modem3gppInterface.data(), SIGNAL(imeiChanged(QString)));
    QVERIFY(imeiChangedSpy.wait());
    QCOMPARE(modem3gppInterface->imei(), imeiChangedSpy.at(0).at(0).toString());

    modem3gpp->setOperatorCode(QLatin1String("21025"));
    QSignalSpy operatorCodeChangedSpy(modem3gppInterface.data(), SIGNAL(operatorCodeChanged(QString)));
    QVERIFY(operatorCodeChangedSpy.wait());
    QCOMPARE(modem3gppInterface->operatorCode(), operatorCodeChangedSpy.at(0).at(0).toString());

    modem3gpp->setOperatorCode(QLatin1String("25011"));
    QSignalSpy countryCodeChangedSpy(modem3gppInterface.data(), SIGNAL(countryCodeChanged(QString)));
    QVERIFY(countryCodeChangedSpy.wait());
    QCOMPARE(modem3gppInterface->countryCode(), countryCodeChangedSpy.at(0).at(0).toString());

    modem3gpp->setOperatorName(QLatin1String("op-name2"));
    QSignalSpy operatorNameChangedSpy(modem3gppInterface.data(), SIGNAL(operatorNameChanged(QString)));
    QVERIFY(operatorNameChangedSpy.wait());
    QCOMPARE(modem3gppInterface->operatorName(), operatorNameChangedSpy.at(0).at(0).toString());

    modem3gpp->setRegistrationState(MM_MODEM_3GPP_REGISTRATION_STATE_IDLE);
    QSignalSpy registrationStateChangedSpy(modem3gppInterface.data(), SIGNAL(registrationStateChanged(MMModem3gppRegistrationState)));
    QVERIFY(registrationStateChangedSpy.wait());
    QCOMPARE(modem3gppInterface->registrationState(), registrationStateChangedSpy.at(0).at(0).value<MMModem3gppRegistrationState>());

#if MM_CHECK_VERSION(1, 2, 0)
    modem3gpp->setSubscriptionState(MM_MODEM_3GPP_SUBSCRIPTION_STATE_PROVISIONED);
    QSignalSpy subscriptionStateChangedSpy(modem3gppInterface.data(), SIGNAL(subscriptionStateChanged(MMModem3gppSubscriptionState)));
    QVERIFY(subscriptionStateChangedSpy.wait());
    QCOMPARE(modem3gppInterface->subscriptionState(), subscriptionStateChangedSpy.at(0).at(0).value<MMModem3gppSubscriptionState>());
#endif
}

QTEST_MAIN(Modem3gppPropertiesTest)

#include "moc_modem3gpppropertiestest.cpp"
