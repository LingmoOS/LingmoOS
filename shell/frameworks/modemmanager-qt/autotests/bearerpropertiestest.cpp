/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bearerpropertiestest.h"

#include "generictypes.h"
#include "manager.h"
#include "modem.h"

#include "fakemodem/modem.h"

#include <QSignalSpy>
#include <QTest>

void BearerPropertiesTest::initTestCase()
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

    bearer = new Bearer();
    // We need to set some default values
    bearer->setConnected(false);
    bearer->setInterface(QLatin1String("ttyUSB0"));
    bearer->setIp4Config({{QLatin1String("method"), MM_BEARER_IP_METHOD_PPP}});
    bearer->setIp6Config({{QLatin1String("method"), MM_BEARER_IP_METHOD_UNKNOWN}});
    bearer->setIpTimeout(20);
    bearer->setProperties({{QLatin1String("apn"), QLatin1String("internet")}, {QLatin1String("ip-type"), 1}, {QLatin1String("number"), QLatin1String("*99#")}});
    bearer->setSuspended(false);
#if MM_CHECK_VERSION(1, 2, 0)
    ModemManager::Modem::Ptr modemInterface = ModemManager::modemDevices().constFirst()->modemInterface();
    QCOMPARE(modemInterface->listBearers().count(), 0);
    QSignalSpy bearerAddedSpy(modemInterface.data(), SIGNAL(bearerAdded(QString)));
    fakeModem->addBearer(bearer);
    QVERIFY(bearerAddedSpy.wait());
#endif
}

#if MM_CHECK_VERSION(1, 2, 0)
void BearerPropertiesTest::testBearerProperties()
{
    ModemManager::Modem::Ptr modemInterface = ModemManager::modemDevices().first()->modemInterface();
    ModemManager::Bearer::Ptr modemBearer = modemInterface->listBearers().first();

    bearer->setConnected(true);
    QSignalSpy connectedChangedSpy(modemBearer.data(), SIGNAL(connectedChanged(bool)));
    QVERIFY(connectedChangedSpy.wait());
    QCOMPARE(modemBearer->isConnected(), connectedChangedSpy.at(0).at(0).toBool());

    bearer->setInterface(QLatin1String("ttyUSB1"));
    QSignalSpy interfaceChangedSpy(modemBearer.data(), SIGNAL(interfaceChanged(QString)));
    QVERIFY(interfaceChangedSpy.wait());
    QCOMPARE(modemBearer->interface(), interfaceChangedSpy.at(0).at(0).toString());

    bearer->setIp4Config({{QLatin1String("method"), MM_BEARER_IP_METHOD_UNKNOWN}});
    QSignalSpy ip4ConfigChangedSpy(modemBearer.data(), SIGNAL(ip4ConfigChanged(ModemManager::IpConfig)));
    QVERIFY(ip4ConfigChangedSpy.wait());
    QCOMPARE(modemBearer->ip4Config().method(), ip4ConfigChangedSpy.at(0).at(0).value<ModemManager::IpConfig>().method());

    bearer->setIp6Config({{QLatin1String("method"), MM_BEARER_IP_METHOD_PPP}});
    QSignalSpy ip6ConfigChangedSpy(modemBearer.data(), SIGNAL(ip6ConfigChanged(ModemManager::IpConfig)));
    QVERIFY(ip6ConfigChangedSpy.wait());
    QCOMPARE(modemBearer->ip6Config().method(), ip6ConfigChangedSpy.at(0).at(0).value<ModemManager::IpConfig>().method());

    bearer->setIpTimeout(25);
    QSignalSpy ipTimeoutChangedSpy(modemBearer.data(), SIGNAL(ipTimeoutChanged(uint)));
    QVERIFY(ipTimeoutChangedSpy.wait());
    QCOMPARE(modemBearer->ipTimeout(), ipTimeoutChangedSpy.at(0).at(0).toUInt());

    bearer->setProperties(
        {{QLatin1String("apn"), QLatin1String("internet2")}, {QLatin1String("ip-type"), 2}, {QLatin1String("number"), QLatin1String("*98#")}});
    QSignalSpy propertiesChangedSpy(modemBearer.data(), SIGNAL(propertiesChanged(QVariantMap)));
    QVERIFY(propertiesChangedSpy.wait());
    QVERIFY(modemBearer->properties().contains(QLatin1String("apn")));
    QVERIFY(modemBearer->properties().contains(QLatin1String("number")));
    QVERIFY(modemBearer->properties().contains(QLatin1String("ip-type")));
    QCOMPARE(modemBearer->properties().value(QLatin1String("apn")).toString(), QLatin1String("internet2"));
    QCOMPARE(modemBearer->properties().value(QLatin1String("ip-type")).toUInt(), (uint)2);
    QCOMPARE(modemBearer->properties().value(QLatin1String("number")).toString(), QLatin1String("*98#"));

    bearer->setSuspended(true);
    QSignalSpy suspendedChangedSpy(modemBearer.data(), SIGNAL(suspendedChanged(bool)));
    QVERIFY(suspendedChangedSpy.wait());
    QCOMPARE(modemBearer->isSuspended(), suspendedChangedSpy.at(0).at(0).toBool());
}
#endif
QTEST_MAIN(BearerPropertiesTest)

#include "moc_bearerpropertiestest.cpp"
