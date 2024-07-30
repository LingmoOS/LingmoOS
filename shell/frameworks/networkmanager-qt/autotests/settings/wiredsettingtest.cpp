/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wiredsettingtest.h"

#include "generictypes.h"
#include "settings/wiredsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#define NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS "assigned-mac-address"

void WiredSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("port");
    QTest::addColumn<quint32>("speed");
    QTest::addColumn<QString>("duplex");
    QTest::addColumn<bool>("autoNegotiate");
    QTest::addColumn<QString>("generateMacAddressMask");
    QTest::addColumn<QByteArray>("macAddress");
    QTest::addColumn<QByteArray>("clonedMacAddress");
    QTest::addColumn<QStringList>("macAddressBlacklist");
    QTest::addColumn<quint32>("mtu");
    QTest::addColumn<QStringList>("s390Subchannels");
    QTest::addColumn<QString>("s390NetType");
    QTest::addColumn<NMStringMap>("s390Options");
    QTest::addColumn<uint>("wakeOnLan");
    QTest::addColumn<QString>("wakeOnLanPassword");
    QTest::addColumn<QString>("assignedMacAddress");    

    QStringList macAddressBlacklist;
    macAddressBlacklist << "00:08:C7:1B:8C:02";

    QStringList s390Subchannels;
    s390Subchannels << "0.0.09a0";
    s390Subchannels << "0.0.09a1";
    s390Subchannels << "0.0.09a2";

    QMap<QString, QString> s390Options;
    s390Options.insert("portno", "0");

    QTest::newRow("setting1") << QString("tp") // port
                              << (quint32)100 // speed
                              << QString("full") // duplex
                              << false // autoNegotiate
                              << QString("FE:FF:FF:00:00:00 68:F7:28:00:00:00") // generateMacAddressMask
                              << QByteArray("00-B0-D0-86-BB-F7") // macAddress
                              << QByteArray("00-14-22-01-23-4") // clonedMacAddress
                              << macAddressBlacklist // macAddressBlacklist
                              << (quint32)200 // mtu
                              << s390Subchannels // s390Subchannels
                              << QString("qeth") // s390NetType
                              << s390Options // s390Options
                              << (uint)(NetworkManager::WiredSetting::WakeOnLanMulticast | NetworkManager::WiredSetting::WakeOnLanMagic) // wakeOnLan
                              << QString("password") // wakeOnLanPassword
                              << QString("random"); // assignedMacAddress
}

void WiredSettingTest::testSetting()
{
    QFETCH(QString, port);
    QFETCH(quint32, speed);
    QFETCH(QString, duplex);
    QFETCH(bool, autoNegotiate);
    QFETCH(QString, generateMacAddressMask);
    QFETCH(QByteArray, macAddress);
    QFETCH(QByteArray, clonedMacAddress);
    QFETCH(QStringList, macAddressBlacklist);
    QFETCH(quint32, mtu);
    QFETCH(QStringList, s390Subchannels);
    QFETCH(QString, s390NetType);
    QFETCH(NMStringMap, s390Options);
    QFETCH(uint, wakeOnLan);
    QFETCH(QString, wakeOnLanPassword);
    QFETCH(QString, assignedMacAddress);    

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_WIRED_PORT), port);
    map.insert(QLatin1String(NM_SETTING_WIRED_SPEED), speed);
    map.insert(QLatin1String(NM_SETTING_WIRED_DUPLEX), duplex);
    map.insert(QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE), autoNegotiate);
    map.insert(QLatin1String(NM_SETTING_WIRED_GENERATE_MAC_ADDRESS_MASK), generateMacAddressMask);
    map.insert(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS), macAddress);
    map.insert(QLatin1String(NM_SETTING_WIRED_CLONED_MAC_ADDRESS), clonedMacAddress);
    map.insert(QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS_BLACKLIST), macAddressBlacklist);
    map.insert(QLatin1String(NM_SETTING_WIRED_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_WIRED_S390_SUBCHANNELS), s390Subchannels);
    map.insert(QLatin1String(NM_SETTING_WIRED_S390_NETTYPE), s390NetType);
    map.insert(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS), QVariant::fromValue(s390Options));
    map.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN), wakeOnLan);
    map.insert(QLatin1String(NM_SETTING_WIRED_WAKE_ON_LAN_PASSWORD), wakeOnLanPassword);
    map.insert(QLatin1String(NM_SETTING_WIRED_ASSIGNED_MAC_ADDRESS), assignedMacAddress);    

    NetworkManager::WiredSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        if (it.key() != QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)) {
            QCOMPARE(it.value(), map1.value(it.key()));
        }
        ++it;
    }

    QCOMPARE(map.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<NMStringMap>(),
             map1.value(QLatin1String(NM_SETTING_WIRED_S390_OPTIONS)).value<NMStringMap>());
}

QTEST_MAIN(WiredSettingTest)

#include "moc_wiredsettingtest.cpp"
