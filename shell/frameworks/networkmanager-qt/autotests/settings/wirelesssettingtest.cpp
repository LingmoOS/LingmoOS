/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wirelesssettingtest.h"

#include "settings/wirelesssetting.h"

#include <libnm/NetworkManager.h>

#define NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS "assigned-mac-address"

#include <QTest>

void WirelessSettingTest::testSetting_data()
{
    QTest::addColumn<QByteArray>("ssid");
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("band");
    QTest::addColumn<quint32>("channel");
    QTest::addColumn<QByteArray>("bssid");
    QTest::addColumn<quint32>("rate");
    QTest::addColumn<quint32>("txPower");
    QTest::addColumn<QByteArray>("macAddress");
    QTest::addColumn<QString>("generateMacAddressMask");
    QTest::addColumn<QStringList>("macAddressBlacklist");
    QTest::addColumn<uint>("macAddressRandomization");
    QTest::addColumn<quint32>("mtu");
    QTest::addColumn<QStringList>("seenBssids");
    QTest::addColumn<uint>("powerSave");
    QTest::addColumn<QString>("security");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<QString>("assignedMacAddress");

    QStringList macAddressBlacklist;
    macAddressBlacklist << "00:08:C7:1B:8C:02";

    QStringList seenBssids;
    seenBssids << "0005.9a3e.7c0f";

    QTest::newRow("setting1") << QByteArray("test") // ssid
                              << QString("adhoc") // mode
                              << QString("bg") // band
                              << (quint32)1 // channel
                              << QByteArray("0011.2161.b7c0") // bssid
                              << (quint32)2 // rate
                              << (quint32)3 // txPower
                              << QByteArray("00-B0-D0-86-BB-F7") // macAddress
                              << QString("FE:FF:FF:00:00:00 68:F7:28:00:00:00") // generateMacAddressMask
                              << macAddressBlacklist // macAddressBlacklist
                              << (uint)1 // macAddressRandomization
                              << (quint32)4 // mtu
                              << seenBssids // seenBssids
                              << (uint)2 // powerSave
                              << QString("802-11-wireless-security") // security
                              << true // hidden
                              << QString("random"); // assignedMacAddress
}

void WirelessSettingTest::testSetting()
{
    QFETCH(QByteArray, ssid);
    QFETCH(QString, mode);
    QFETCH(QString, band);
    QFETCH(quint32, channel);
    QFETCH(QByteArray, bssid);
    QFETCH(quint32, rate);
    QFETCH(quint32, txPower);
    QFETCH(QByteArray, macAddress);
    QFETCH(QString, generateMacAddressMask);
    QFETCH(QStringList, macAddressBlacklist);
    QFETCH(uint, macAddressRandomization);
    QFETCH(quint32, mtu);
    QFETCH(QStringList, seenBssids);
    QFETCH(uint, powerSave);
    QFETCH(QString, security);
    QFETCH(bool, hidden);
    QFETCH(QString, assignedMacAddress);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_WIRELESS_SSID), ssid);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_MODE), mode);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_BAND), band);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_CHANNEL), channel);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_BSSID), bssid);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_RATE), rate);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_TX_POWER), txPower);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS), macAddress);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_BLACKLIST), macAddressBlacklist);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS_RANDOMIZATION), macAddressRandomization);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS), seenBssids);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_POWERSAVE), powerSave);
    map.insert(QLatin1String("security"), security);

    map.insert(QLatin1String(NM_SETTING_WIRELESS_HIDDEN), hidden);
    map.insert(QLatin1String(NM_SETTING_WIRELESS_ASSIGNED_MAC_ADDRESS), assignedMacAddress);

    NetworkManager::WirelessSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(WirelessSettingTest)

#include "moc_wirelesssettingtest.cpp"
