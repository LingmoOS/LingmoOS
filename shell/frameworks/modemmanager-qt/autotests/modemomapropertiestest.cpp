/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemomapropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modemoma.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void ModemOmaPropertiesTest::initTestCase()
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

    modemOma = new ModemOma(modem);
    modemOma->setModemPath(modem->modemPath());
    modemOma->setFeatures(MM_OMA_FEATURE_NONE);
    ModemManager::OmaSessionType sessionType;
    sessionType.id = 1;
    sessionType.type = MM_OMA_SESSION_TYPE_CLIENT_INITIATED_DEVICE_CONFIGURE;
    ModemManager::OmaSessionTypes sessions;
    sessions << sessionType;
    modemOma->setPendingNetworkInitiatedSessions(sessions);
    modemOma->setSessionState(MM_OMA_SESSION_STATE_STARTED);
    modemOma->setSessionType(MM_OMA_SESSION_TYPE_CLIENT_INITIATED_PRL_UPDATE);

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_OMA), modemOma->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modemOma->setEnableNotifications(true);
}

void ModemOmaPropertiesTest::testModemOmaProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::OmaInterface));
    ModemManager::ModemOma::Ptr modemOmaInterface = modemDevice->interface(ModemManager::ModemDevice::OmaInterface).objectCast<ModemManager::ModemOma>();
    QVERIFY(modemOmaInterface);

    modemOma->setFeatures(MM_OMA_FEATURE_DEVICE_PROVISIONING);
    QSignalSpy featuresChangedSpy(modemOmaInterface.data(), SIGNAL(featuresChanged(QFlags<MMOmaFeature>)));
    QVERIFY(featuresChangedSpy.wait());
    QCOMPARE(modemOmaInterface->features(), featuresChangedSpy.at(0).at(0).value<QFlags<MMOmaFeature>>());

    ModemManager::OmaSessionType sessionType;
    sessionType.id = 2;
    sessionType.type = MM_OMA_SESSION_TYPE_CLIENT_INITIATED_HANDS_FREE_ACTIVATION;
    ModemManager::OmaSessionTypes sessions;
    sessions << sessionType;
    modemOma->setPendingNetworkInitiatedSessions(sessions);
    QSignalSpy sessionsChangedSpy(modemOmaInterface.data(), SIGNAL(pendingNetworkInitiatedSessionsChanged(ModemManager::OmaSessionTypes)));
    QVERIFY(sessionsChangedSpy.wait());
    QCOMPARE(modemOmaInterface->pendingNetworkInitiatedSessions().first().id, sessionsChangedSpy.at(0).at(0).value<ModemManager::OmaSessionTypes>().first().id);
    QCOMPARE(modemOmaInterface->pendingNetworkInitiatedSessions().first().type,
             sessionsChangedSpy.at(0).at(0).value<ModemManager::OmaSessionTypes>().first().type);

    modemOma->setSessionState(MM_OMA_SESSION_STATE_CONNECTING);
    QSignalSpy sessionStateChangedSpy(modemOmaInterface.data(),
                                      SIGNAL(sessionStateChanged(MMOmaSessionState, MMOmaSessionState, MMOmaSessionStateFailedReason)));
    QVERIFY(sessionStateChangedSpy.wait());
    QCOMPARE(modemOmaInterface->sessionState(), sessionStateChangedSpy.at(0).at(1).value<MMOmaSessionState>());

    modemOma->setSessionType(MM_OMA_SESSION_TYPE_NETWORK_INITIATED_PRL_UPDATE);
    QSignalSpy sessionTypeChangedSpy(modemOmaInterface.data(), SIGNAL(sessionTypeChanged(MMOmaSessionType)));
    QVERIFY(sessionTypeChangedSpy.wait());
    QCOMPARE(modemOmaInterface->sessionType(), sessionTypeChangedSpy.at(0).at(0).value<MMOmaSessionType>());
}

QTEST_MAIN(ModemOmaPropertiesTest)

#include "moc_modemomapropertiestest.cpp"
