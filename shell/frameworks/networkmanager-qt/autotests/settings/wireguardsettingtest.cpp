/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wireguardsettingtest.h"

#include "settings/wireguardsetting.h"

#include <libnm/NetworkManager.h>

#include <QTest>

#if !NM_CHECK_VERSION(1, 16, 0)
#define NM_SETTING_WIREGUARD_FWMARK "fwmark"
#define NM_SETTING_WIREGUARD_LISTEN_PORT "listen-port"
#define NM_SETTING_WIREGUARD_PRIVATE_KEY "private-key"
#define NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS "private-key-flags"
#define NM_SETTING_WIREGUARD_PEERS "peers"
#define NM_SETTING_WIREGUARD_MTU "mtu"
#define NM_SETTING_WIREGUARD_PEER_ROUTES "peer-routes"

#define NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY "preshared-key"
#define NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS "preshared-key-flags"
#define NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY "public-key"
#endif

void WireGuardSettingTest::testSetting_data()
{
    QTest::addColumn<qint32>("fwmark");
    QTest::addColumn<qint32>("listenPort");
    QTest::addColumn<qint32>("mtu");
    QTest::addColumn<bool>("peerRoutes");
    QTest::addColumn<NMVariantMapList>("peers");
    QTest::addColumn<QString>("privateKey");
    QTest::addColumn<quint32>("privateKeyFlags");

    NMVariantMapList peers;
    QVariantMap map;
    map.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY), "preshared-key");
    map.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS), (uint)2);
    map.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY), "public-key");

    peers << map;

    QVariantMap map1;
    map1.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY), "preshared-key1");
    map1.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PRESHARED_KEY_FLAGS), (uint)3);
    map1.insert(QLatin1String(NM_WIREGUARD_PEER_ATTR_PUBLIC_KEY), "public-key1");
    peers << map1;

    QTest::newRow("setting1") << (qint32)100 // fwmark
                              << (qint32)101 // listenPort
                              << (qint32)102 // mtu
                              << false // peerRoutes
                              << peers // peers
                              << QString("private-key") // privateKey
                              << (quint32)2; // privateKeyFlags
}

void WireGuardSettingTest::testSetting()
{
    QFETCH(qint32, fwmark);
    QFETCH(qint32, listenPort);
    QFETCH(qint32, mtu);
    QFETCH(bool, peerRoutes);
    QFETCH(NMVariantMapList, peers);
    QFETCH(QString, privateKey);
    QFETCH(quint32, privateKeyFlags);

    QVariantMap map;

    map.insert(QLatin1String(NM_SETTING_WIREGUARD_FWMARK), fwmark);
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_LISTEN_PORT), listenPort);
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_MTU), mtu);
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_PEER_ROUTES), peerRoutes);
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_PEERS), QVariant::fromValue(peers));
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY), privateKey);
    map.insert(QLatin1String(NM_SETTING_WIREGUARD_PRIVATE_KEY_FLAGS), privateKeyFlags);

    NetworkManager::WireGuardSetting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        if (it.key() != QLatin1String(NM_SETTING_WIREGUARD_PEERS)) {
            QCOMPARE(it.value(), map1.value(it.key()));
        }
        ++it;
    }

    NMVariantMapList list = map.value(QLatin1String(NM_SETTING_WIREGUARD_PEERS)).value<NMVariantMapList>();
    NMVariantMapList list1 = map1.value(QLatin1String(NM_SETTING_WIREGUARD_PEERS)).value<NMVariantMapList>();

    QCOMPARE(list.count(), list1.count());

    int comparedMaps = 0;
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap varMap = list.at(i);
        for (int j = 0; j < list1.size(); ++j) {
            QVariantMap varMap1 = list1.at(j);
            QVariantMap::const_iterator ite = varMap.constBegin();
            int comparedvals = 0;
            while (ite != varMap.constEnd()) {
                QVariantMap::const_iterator val1 = varMap1.constFind(ite.key());
                if (val1 != varMap1.constEnd()) {
                    if (varMap.value(ite.key()) == val1.value()) {
                        ++comparedvals;
                    }
                }
                ++ite;
            }
            if (comparedvals == varMap.size()) {
                comparedMaps++;
            }
        }
    }
    QCOMPARE(comparedMaps, list.count());
}

QTEST_MAIN(WireGuardSettingTest)

#include "moc_wireguardsettingtest.cpp"
