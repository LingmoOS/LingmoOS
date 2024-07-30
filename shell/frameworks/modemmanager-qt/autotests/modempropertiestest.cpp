/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modempropertiestest.h"

#include "generictypes.h"
#include "manager.h"
#include "modem.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void ModemPropertiesTest::initTestCase()
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
}

void ModemPropertiesTest::testModemProperties()
{
    ModemManager::Modem::Ptr modemInterface = ModemManager::modemDevices().first()->modemInterface();
    QVERIFY(modemInterface);

    modem->setAccessTechnologies(5);
    QSignalSpy accessTechnologiesChangedSpy(modemInterface.data(), SIGNAL(accessTechnologiesChanged(QFlags<MMModemAccessTechnology>)));
    QVERIFY(accessTechnologiesChangedSpy.wait());
    QCOMPARE(modemInterface->accessTechnologies(), accessTechnologiesChangedSpy.at(0).at(0).value<QFlags<MMModemAccessTechnology>>());

    modem->SetCurrentBands({1});
    QSignalSpy currentBandsChangedSpy(modemInterface.data(), SIGNAL(currentBandsChanged(QList<MMModemBand>)));
    QVERIFY(currentBandsChangedSpy.wait());
    QCOMPARE(modemInterface->currentBands().first(), currentBandsChangedSpy.at(0).at(0).value<QList<MMModemBand>>().first());

    modem->SetCurrentCapabilities(5);
    QSignalSpy currentCapabilitiesChangedSpy(modemInterface.data(), SIGNAL(currentCapabilitiesChanged(QFlags<MMModemCapability>)));
    QVERIFY(currentCapabilitiesChangedSpy.wait());
    QCOMPARE(modemInterface->currentCapabilities(), currentCapabilitiesChangedSpy.at(0).at(0).value<QFlags<MMModemCapability>>());

    modem->SetCurrentModes({MM_MODEM_MODE_2G, MM_MODEM_MODE_NONE});
    QSignalSpy currentModesChangedSpy(modemInterface.data(), SIGNAL(currentModesChanged(ModemManager::CurrentModesType)));
    QVERIFY(currentModesChangedSpy.wait());
    QCOMPARE(modemInterface->currentModes().allowed, currentModesChangedSpy.at(0).at(0).value<ModemManager::CurrentModesType>().allowed);
    QCOMPARE(modemInterface->currentModes().preferred, currentModesChangedSpy.at(0).at(0).value<ModemManager::CurrentModesType>().preferred);

    modem->setDevice(QLatin1String("/sys/devices/pci0000:00/0000:00:1d.0/usb5/5-1/5-1.2"));
    QSignalSpy deviceChangedSpy(modemInterface.data(), SIGNAL(deviceChanged(QString)));
    QVERIFY(deviceChangedSpy.wait());
    QCOMPARE(modemInterface->device(), deviceChangedSpy.at(0).at(0).toString());

    modem->setDeviceIdentifier(QLatin1String("1c435eb6d74494b5f98d7221e2c6ae9ec526a981"));
    QSignalSpy deviceIdentifierChangedSpy(modemInterface.data(), SIGNAL(deviceIdentifierChanged(QString)));
    QVERIFY(deviceIdentifierChangedSpy.wait());
    QCOMPARE(modemInterface->deviceIdentifier(), deviceIdentifierChangedSpy.at(0).at(0).toString());

    modem->setDrivers({QLatin1String("option2")});
    QSignalSpy driversChangedSpy(modemInterface.data(), SIGNAL(driversChanged(QStringList)));
    QVERIFY(driversChangedSpy.wait());
    QCOMPARE(modemInterface->drivers(), driversChangedSpy.at(0).at(0).toStringList());

    modem->setEquipmentIdentifier(QLatin1String("353895021085110"));
    QSignalSpy equipmentIdentifierChangedSpy(modemInterface.data(), SIGNAL(equipmentIdentifierChanged(QString)));
    QVERIFY(equipmentIdentifierChangedSpy.wait());
    QCOMPARE(modemInterface->equipmentIdentifier(), equipmentIdentifierChangedSpy.at(0).at(0).toString());

    modem->setManufacturer(QLatin1String("huawey"));
    QSignalSpy manufacturerChangedSpy(modemInterface.data(), SIGNAL(manufacturerChanged(QString)));
    QVERIFY(manufacturerChangedSpy.wait());
    QCOMPARE(modemInterface->manufacturer(), manufacturerChangedSpy.at(0).at(0).toString());

    modem->setMaxActiveBearers(2);
    QSignalSpy maxActiveBearersChangedSpy(modemInterface.data(), SIGNAL(maxActiveBearersChanged(uint)));
    QVERIFY(maxActiveBearersChangedSpy.wait());
    QCOMPARE(modemInterface->maxActiveBearers(), maxActiveBearersChangedSpy.at(0).at(0).toUInt());

    modem->setMaxBearers(2);
    QSignalSpy maxBearersChangedSpy(modemInterface.data(), SIGNAL(maxBearersChanged(uint)));
    QVERIFY(maxBearersChangedSpy.wait());
    QCOMPARE(modemInterface->maxBearers(), maxBearersChangedSpy.at(0).at(0).toUInt());

    modem->setModel(QLatin1String("K5658"));
    QSignalSpy modelChangedSpy(modemInterface.data(), SIGNAL(modelChanged(QString)));
    QVERIFY(modelChangedSpy.wait());
    QCOMPARE(modemInterface->model(), modelChangedSpy.at(0).at(0).toString());

    modem->setPlugin(QLatin1String("Huawey"));
    QSignalSpy pluginChangedSpy(modemInterface.data(), SIGNAL(pluginChanged(QString)));
    QVERIFY(pluginChangedSpy.wait());
    QCOMPARE(modemInterface->plugin(), pluginChangedSpy.at(0).at(0).toString());

    modem->setPorts({{QLatin1String("ttyUSB5"), MM_MODEM_PORT_TYPE_QCDM},
                     {QLatin1String("ttyUSB6"), MM_MODEM_PORT_TYPE_AT},
                     {QLatin1String("ttyUSB7"), MM_MODEM_PORT_TYPE_AT}});
    QSignalSpy portsChangedSpy(modemInterface.data(), SIGNAL(portsChanged(ModemManager::PortList)));
    QVERIFY(portsChangedSpy.wait());
    ModemManager::PortList ports = modemInterface->ports();
    ModemManager::PortList signalPorts = portsChangedSpy.at(0).at(0).value<ModemManager::PortList>();
    QCOMPARE(ports.count(), signalPorts.count());
    Q_FOREACH (const ModemManager::Port port, ports) {
        bool found = false;
        Q_FOREACH (const ModemManager::Port signalPort, signalPorts) {
            if (port.name == signalPort.name) {
                found = true;
                QCOMPARE(port.type, signalPort.type);
            }
        }
        QVERIFY(found);
    }
    QCOMPARE(modemInterface->plugin(), pluginChangedSpy.at(0).at(0).toString());

    modem->SetPowerState(2);
    QSignalSpy powerStateChangedSpy(modemInterface.data(), SIGNAL(powerStateChanged(MMModemPowerState)));
    QVERIFY(powerStateChangedSpy.wait());
    QCOMPARE(modemInterface->powerState(), powerStateChangedSpy.at(0).at(0).value<MMModemPowerState>());

    modem->setPrimaryPort(QLatin1String("ttyUSB5"));
    QSignalSpy primaryPortChangedSpy(modemInterface.data(), SIGNAL(primaryPortChanged(QString)));
    QVERIFY(primaryPortChangedSpy.wait());
    QCOMPARE(modemInterface->primaryPort(), primaryPortChangedSpy.at(0).at(0).toString());

    modem->setRevision(QLatin1String("11.002.06.00.11"));
    QSignalSpy revisionChangedSpy(modemInterface.data(), SIGNAL(revisionChanged(QString)));
    QVERIFY(revisionChangedSpy.wait());
    QCOMPARE(modemInterface->revision(), revisionChangedSpy.at(0).at(0).toString());

    modem->setSignalQuality({85, true});
    QSignalSpy signalQualityChangedSpy(modemInterface.data(), SIGNAL(signalQualityChanged(ModemManager::SignalQualityPair)));
    QVERIFY(signalQualityChangedSpy.wait());
    QCOMPARE(modemInterface->signalQuality().recent, signalQualityChangedSpy.at(0).at(0).value<ModemManager::SignalQualityPair>().recent);
    QCOMPARE(modemInterface->signalQuality().signal, signalQualityChangedSpy.at(0).at(0).value<ModemManager::SignalQualityPair>().signal);

    modem->setSim(QDBusObjectPath("/org/kde/fakemodem/SIM/2"));
    QSignalSpy simChangedSpy(modemInterface.data(), SIGNAL(simPathChanged(QString, QString)));
    QVERIFY(simChangedSpy.wait());
    QCOMPARE(modemInterface->simPath(), simChangedSpy.at(0).at(1).toString());

    modem->setState(5);
    QSignalSpy stateChangedSpy(modemInterface.data(), SIGNAL(stateChanged(MMModemState, MMModemState, MMModemStateChangeReason)));
    QVERIFY(stateChangedSpy.wait());
    QCOMPARE(MM_MODEM_STATE_REGISTERED, stateChangedSpy.at(0).at(0).value<MMModemState>());
    QCOMPARE(MM_MODEM_STATE_ENABLING, stateChangedSpy.at(0).at(1).value<MMModemState>());
    QCOMPARE(MM_MODEM_STATE_CHANGE_REASON_UNKNOWN, stateChangedSpy.at(0).at(2).value<MMModemStateChangeReason>());

    modem->setStateFailedReason(1);
    QSignalSpy stateFailedReasonChangedSpy(modemInterface.data(), SIGNAL(stateFailedReasonChanged(MMModemStateFailedReason)));
    QVERIFY(stateFailedReasonChangedSpy.wait());
    QCOMPARE(modemInterface->stateFailedReason(), stateFailedReasonChangedSpy.at(0).at(0).value<MMModemStateFailedReason>());

    modem->setSupportedBands({1});
    QSignalSpy supportedBandsChangedSpy(modemInterface.data(), SIGNAL(supportedBandsChanged(QList<MMModemBand>)));
    QVERIFY(supportedBandsChangedSpy.wait());
    QCOMPARE(modemInterface->supportedBands().first(), supportedBandsChangedSpy.at(0).at(0).value<QList<MMModemBand>>().first());

    modem->setSupportedCapabilities({3});
    QSignalSpy supportedCapabilitiesChangedSpy(modemInterface.data(), SIGNAL(supportedCapabilitiesChanged(QList<MMModemCapability>)));
    QVERIFY(supportedCapabilitiesChangedSpy.wait());
    QCOMPARE(modemInterface->supportedCapabilities().first(), supportedCapabilitiesChangedSpy.at(0).at(0).value<QList<MMModemCapability>>().first());

    modem->setSupportedIpFamilies(2);
    QSignalSpy supportedIpFamiliesChangedSpy(modemInterface.data(), SIGNAL(supportedIpFamiliesChanged(QFlags<MMBearerIpFamily>)));
    QVERIFY(supportedIpFamiliesChangedSpy.wait());
    QCOMPARE(modemInterface->supportedIpFamilies(), supportedIpFamiliesChangedSpy.at(0).at(0).value<QFlags<MMBearerIpFamily>>());

    ModemManager::SupportedModesType supportedModes;
    ModemManager::CurrentModesType supportedMode1 = {MM_MODEM_MODE_2G, MM_MODEM_MODE_NONE};
    supportedModes << supportedMode1;
    modem->setSupportedModes(supportedModes);
    QSignalSpy supportedModesChangedSpy(modemInterface.data(), SIGNAL(supportedModesChanged(ModemManager::SupportedModesType)));
    QVERIFY(supportedModesChangedSpy.wait());
    QCOMPARE(modemInterface->supportedModes().first().allowed, supportedModesChangedSpy.at(0).at(0).value<ModemManager::SupportedModesType>().first().allowed);
    QCOMPARE(modemInterface->supportedModes().first().preferred,
             supportedModesChangedSpy.at(0).at(0).value<ModemManager::SupportedModesType>().first().preferred);

    modem->setUnlockRequired(2);
    QSignalSpy unlockRequiredChangedSpy(modemInterface.data(), SIGNAL(unlockRequiredChanged(MMModemLock)));
    QVERIFY(unlockRequiredChangedSpy.wait());
    QCOMPARE(modemInterface->unlockRequired(), unlockRequiredChangedSpy.at(0).at(0).value<MMModemLock>());

    modem->setUnlockRetries({{MM_MODEM_LOCK_SIM_PIN, 2}});
    QSignalSpy unlockRetriesChangedSpy(modemInterface.data(), SIGNAL(unlockRetriesChanged(ModemManager::UnlockRetriesMap)));
    QVERIFY(unlockRetriesChangedSpy.wait());
    QCOMPARE(modemInterface->unlockRetries().count(), 1);
    QVERIFY(modemInterface->unlockRetries().contains(MM_MODEM_LOCK_SIM_PIN));
    QCOMPARE(modemInterface->unlockRetries().value(MM_MODEM_LOCK_SIM_PIN),
             unlockRetriesChangedSpy.at(0).at(0).value<ModemManager::UnlockRetriesMap>().value(MM_MODEM_LOCK_SIM_PIN));
}

QTEST_MAIN(ModemPropertiesTest)

#include "moc_modempropertiestest.cpp"
