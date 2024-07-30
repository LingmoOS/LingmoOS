/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iptunnelsettingtest.h"

#include "settings/iptunnelsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 12, 0)
#define NM_SETTING_IP_TUNNEL_FLAGS "flags"
#endif

void IpTunnelSettingTest::testSetting_data()
{
    QTest::addColumn<quint32>("mode");
    QTest::addColumn<bool>("pathMtuDiscovery");
    QTest::addColumn<quint32>("encapsulationLimit");
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<quint32>("flowLabel");
    QTest::addColumn<quint32>("mtu");
    QTest::addColumn<quint32>("tos");
    QTest::addColumn<quint32>("ttl");
    QTest::addColumn<QString>("inputKey");
    QTest::addColumn<QString>("local");
    QTest::addColumn<QString>("parent");
    QTest::addColumn<QString>("outputKey");
    QTest::addColumn<QString>("remote");

    QTest::newRow("setting1") << (quint32)2 // mode
                              << false // pathMtuDiscovery
                              << (quint32)1 // encapsulationLimit
                              << (quint32)0x02 // flags
                              << (quint32)1 // flowLabel
                              << (quint32)1 // mtu
                              << (quint32)1 // tos
                              << (quint32)1 // ttl
                              << QString("key") // inputKey
                              << QString("abc") // local
                              << QString("par") // parent
                              << QString("out") // outputKey
                              << QString("rem"); // remote
}

void IpTunnelSettingTest::testSetting()
{
    QFETCH(quint32, mode);
    QFETCH(bool, pathMtuDiscovery);
    QFETCH(quint32, encapsulationLimit);
    QFETCH(quint32, flags);
    QFETCH(quint32, flowLabel);
    QFETCH(quint32, mtu);
    QFETCH(quint32, tos);
    QFETCH(quint32, ttl);
    QFETCH(QString, inputKey);
    QFETCH(QString, local);
    QFETCH(QString, parent);
    QFETCH(QString, outputKey);
    QFETCH(QString, remote);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_MODE), mode);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_PATH_MTU_DISCOVERY), pathMtuDiscovery);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_ENCAPSULATION_LIMIT), encapsulationLimit);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_FLAGS), flags);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_FLOW_LABEL), flowLabel);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_TOS), tos);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_TTL), ttl);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_INPUT_KEY), inputKey);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_LOCAL), local);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_PARENT), parent);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_OUTPUT_KEY), outputKey);
    map.insert(QLatin1String(NM_SETTING_IP_TUNNEL_REMOTE), remote);

    NetworkManager::IpTunnelSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        QCOMPARE(it.value(), map1.value(it.key()));
        ++it;
    }
}

QTEST_MAIN(IpTunnelSettingTest)

#include "moc_iptunnelsettingtest.cpp"
