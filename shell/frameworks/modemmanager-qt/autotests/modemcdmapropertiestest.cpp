/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemcdmapropertiestest.h"

#include "dbus/fakedbus.h"
#include "generictypes.h"
#include "manager.h"
#include "modem.h"
#include "modemcdma.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void ModemCdmaPropertiesTest::initTestCase()
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

    modemCdma = new ModemCdma(modem);
    modemCdma->setModemPath(modem->modemPath());
    modemCdma->setActivationState(MM_MODEM_CDMA_ACTIVATION_STATE_ACTIVATING);
    modemCdma->setCdma1xRegistrationState(MM_MODEM_CDMA_REGISTRATION_STATE_REGISTERED);
    modemCdma->setEsn(QLatin1String("esn"));
    modemCdma->setEvdoRegistrationState(MM_MODEM_CDMA_REGISTRATION_STATE_HOME);
    modemCdma->setMeid(QLatin1String("meid"));
    modemCdma->setNid(5);
    modemCdma->setSid(10);

    ModemManager::MMVariantMapMap interfaces;
    interfaces.insert(QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA), modemCdma->toMap());
    fakeModem->addInterfaces(QDBusObjectPath(modem->modemPath()), interfaces);
    modemCdma->setEnableNotifications(true);
}

void ModemCdmaPropertiesTest::testModemCdmaProperties()
{
    ModemManager::ModemDevice::Ptr modemDevice = ModemManager::modemDevices().first();
    QVERIFY(modemDevice);
    QVERIFY(modemDevice->hasInterface(ModemManager::ModemDevice::CdmaInterface));
    ModemManager::ModemCdma::Ptr modemCdmaInterface = modemDevice->interface(ModemManager::ModemDevice::CdmaInterface).objectCast<ModemManager::ModemCdma>();
    QVERIFY(modemCdmaInterface);

    modemCdma->setActivationState(MM_MODEM_CDMA_ACTIVATION_STATE_NOT_ACTIVATED);
    QSignalSpy activationChangedSpy(modemCdmaInterface.data(), SIGNAL(activationStateChanged(MMModemCdmaActivationState, MMCdmaActivationError, QVariantMap)));
    QVERIFY(activationChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->activationState(), activationChangedSpy.at(0).at(0).value<MMModemCdmaActivationState>());

    modemCdma->setCdma1xRegistrationState(MM_MODEM_CDMA_REGISTRATION_STATE_HOME);
    QSignalSpy cdma1xRegistrationStateChangedSpy(modemCdmaInterface.data(), SIGNAL(cdma1xRegistrationStateChanged(MMModemCdmaRegistrationState)));
    QVERIFY(cdma1xRegistrationStateChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->cdma1xRegistrationState(), cdma1xRegistrationStateChangedSpy.at(0).at(0).value<MMModemCdmaRegistrationState>());

    modemCdma->setEsn(QLatin1String("esn1"));
    QSignalSpy esnChangedSpy(modemCdmaInterface.data(), SIGNAL(esnChanged(QString)));
    QVERIFY(esnChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->esn(), esnChangedSpy.at(0).at(0).toString());

    modemCdma->setEvdoRegistrationState(MM_MODEM_CDMA_REGISTRATION_STATE_REGISTERED);
    QSignalSpy evdoRegistrationStateChangedSpy(modemCdmaInterface.data(), SIGNAL(evdoRegistrationStateChanged(MMModemCdmaRegistrationState)));
    QVERIFY(evdoRegistrationStateChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->evdoRegistrationState(), evdoRegistrationStateChangedSpy.at(0).at(0).value<MMModemCdmaRegistrationState>());

    modemCdma->setMeid(QLatin1String("meid1"));
    QSignalSpy meidChangedSpy(modemCdmaInterface.data(), SIGNAL(meidChanged(QString)));
    QVERIFY(meidChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->meid(), meidChangedSpy.at(0).at(0).toString());

    modemCdma->setNid(10);
    QSignalSpy nidChangedSpy(modemCdmaInterface.data(), SIGNAL(nidChanged(uint)));
    QVERIFY(nidChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->nid(), nidChangedSpy.at(0).at(0).toUInt());

    modemCdma->setSid(5);
    QSignalSpy sidChangedSpy(modemCdmaInterface.data(), SIGNAL(sidChanged(uint)));
    QVERIFY(sidChangedSpy.wait());
    QCOMPARE(modemCdmaInterface->sid(), sidChangedSpy.at(0).at(0).toUInt());
}

QTEST_MAIN(ModemCdmaPropertiesTest)

#include "moc_modemcdmapropertiestest.cpp"
