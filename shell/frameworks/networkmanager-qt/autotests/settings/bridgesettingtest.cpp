/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bridgesettingtest.h"

#include "settings/bridgesetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

void BridgeSettingTest::testSetting_data()
{
    QTest::addColumn<QString>("interfaceName");
    QTest::addColumn<bool>("stp");
    QTest::addColumn<quint16>("priority");
    QTest::addColumn<quint16>("forwardDelay");
    QTest::addColumn<quint16>("helloTime");
    QTest::addColumn<quint16>("maxAge");
    QTest::addColumn<quint32>("ageintTime");
    QTest::addColumn<bool>("multicastSnooping");
    QTest::addColumn<QByteArray>("macAddress");

    QTest::newRow("setting1") << QString("eth0") // interfaceName
                              << false // stp
                              << (quint16)10 // priority
                              << (quint16)5 // forwardDelay
                              << (quint16)5 // helloTime
                              << (quint16)10 // maxAge
                              << (quint32)10000 // ageintTime
                              << false // multicastSnooping
                              << QByteArray("54:EE:75:53:E9:E4"); // macAddress
}

void BridgeSettingTest::testSetting()
{
    QFETCH(QString, interfaceName);
    QFETCH(bool, stp);
    QFETCH(quint16, priority);
    QFETCH(quint16, forwardDelay);
    QFETCH(quint16, helloTime);
    QFETCH(quint16, maxAge);
    QFETCH(quint32, ageintTime);
    QFETCH(bool, multicastSnooping);
    QFETCH(QByteArray, macAddress);

    QVariantMap map;

    map.insert(QLatin1String("interface-name"), interfaceName);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_STP), stp);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_PRIORITY), priority);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_FORWARD_DELAY), forwardDelay);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_HELLO_TIME), helloTime);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_MAX_AGE), maxAge);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_AGEING_TIME), ageintTime);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_MULTICAST_SNOOPING), multicastSnooping);
    map.insert(QLatin1String(NM_SETTING_BRIDGE_MAC_ADDRESS), macAddress);

    NetworkManager::BridgeSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(BridgeSettingTest)

#include "moc_bridgesettingtest.cpp"
