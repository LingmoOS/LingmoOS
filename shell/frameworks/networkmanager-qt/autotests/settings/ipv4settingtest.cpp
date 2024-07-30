/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipv4settingtest.h"

#include "generictypes.h"
#include "settings/ipv4setting.h"

#include <arpa/inet.h>

#include <libnm/NetworkManager.h>

#include <QTest>

void IPv4SettingTest::testSetting_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<UIntList>("dns");
    QTest::addColumn<QStringList>("dnsSearch");
    QTest::addColumn<UIntListList>("addresses");
    QTest::addColumn<UIntListList>("routes");
    QTest::addColumn<int>("routeMetric");
    QTest::addColumn<bool>("ignoreAutoRoutes");
    QTest::addColumn<bool>("ignoreAutoDns");
    QTest::addColumn<QString>("dhcpClientId");
    QTest::addColumn<bool>("dhcpSendHostname");
    QTest::addColumn<QString>("dhcpHostname");
    QTest::addColumn<bool>("neverDefault");
    QTest::addColumn<bool>("mayFail");
    QTest::addColumn<qint32>("dadTimeout");
    QTest::addColumn<QString>("dhcpFqdn");
    QTest::addColumn<QStringList>("dnsOptions");
    QTest::addColumn<qint32>("dnsPriority");
    QTest::addColumn<QString>("gateway");
    QTest::addColumn<NMVariantMapList>("addressData");
    QTest::addColumn<NMVariantMapList>("routeData");

    QList<uint> dns;
    dns << inet_addr("10.0.0.1");
    dns << inet_addr("10.0.1.1");

    QStringList dnsSearch;
    dnsSearch << "foo.com";
    dnsSearch << "foo.bar";

    QList<QList<uint>> addresses;
    QList<uint> addr;
    addr << inet_addr("192.168.1.1");
    addr << 25;
    addr << 0;
    addresses << addr;

    QList<QList<uint>> routes;
    QList<uint> routeAddr;
    routeAddr << inet_addr("192.168.1.1");
    routeAddr << 25;
    routeAddr << inet_addr("192.169.1.1");
    routeAddr << 25;
    routes << routeAddr;

    QStringList dnsOptions;
    dnsOptions << "opt1";
    dnsOptions << "opt2";

    NMVariantMapList addressData;
    QVariantMap addressMap;
    addressMap["address"] = "192.168.1.1";
    addressMap["prefix"] = 25;
    addressData.append(addressMap);

    NMVariantMapList routeData;
    QVariantMap routeMap;
    routeMap["dest"] = "192.168.1.1";
    routeMap["prefix"] = 25;
    routeData.append(routeMap);
    routeMap.clear();
    routeMap["dest"] = "192.168.1.2";
    routeMap["prefix"] = 25;
    routeData.append(routeMap);

    QTest::newRow("setting1") << QString("auto") // method
                              << dns // dns
                              << dnsSearch // dnsSearch
                              << addresses // addresses
                              << routes // routes
                              << 100 // routeMetric
                              << true // ignoreAutoRoutes
                              << true // ignoreAutoDns
                              << QString("home-test") // dhcpClientId
                              << false // dhcpSendHostname
                              << QString("home-test") // dhcpHostname
                              << true // neverDefault
                              << false // mayFail
                              << 100 // dadTimeout
                              << QString("foo.com") // dhcpFqdn
                              << dnsOptions // dnsOptions
                              << 100 // dnsPriority
                              << QString("1.1.1.1") // gateway
                              << addressData // addressData
                              << routeData; // routeData
}

void IPv4SettingTest::testSetting()
{
    QFETCH(QString, method);
    QFETCH(UIntList, dns);
    QFETCH(QStringList, dnsSearch);
    QFETCH(UIntListList, addresses);
    QFETCH(UIntListList, routes);
    QFETCH(int, routeMetric);
    QFETCH(bool, ignoreAutoRoutes);
    QFETCH(bool, ignoreAutoDns);
    QFETCH(QString, dhcpClientId);
    QFETCH(bool, dhcpSendHostname);
    QFETCH(QString, dhcpHostname);
    QFETCH(bool, neverDefault);
    QFETCH(bool, mayFail);
    QFETCH(qint32, dadTimeout);
    QFETCH(QString, dhcpFqdn);
    QFETCH(QStringList, dnsOptions);
    QFETCH(qint32, dnsPriority);
    QFETCH(QString, gateway);
    QFETCH(NMVariantMapList, addressData);
    QFETCH(NMVariantMapList, routeData);

    QVariantMap map;

    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_METHOD), method);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS), QVariant::fromValue(dns));
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_SEARCH), dnsSearch);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES), QVariant::fromValue(addresses));
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES), QVariant::fromValue(routes));
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_METRIC), routeMetric);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_CLIENT_ID), dhcpClientId);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_SEND_HOSTNAME), dhcpSendHostname);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_HOSTNAME), dhcpHostname);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_NEVER_DEFAULT), neverDefault);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_MAY_FAIL), mayFail);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DAD_TIMEOUT), dadTimeout);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DHCP_FQDN), dhcpFqdn);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_OPTIONS), dnsOptions);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS_PRIORITY), dnsPriority);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_GATEWAY), gateway);
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData));
    map.insert(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData));

    NetworkManager::Ipv4Setting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        /* clang-format off */
        if (it.key() != QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)
            && it.key() != QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)
            && it.key() != QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)
            && it.key() != QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA)
            && it.key() != QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA)) { /* clang-format on */
            QCOMPARE(it.value(), map1.value(it.key()));
        }
        ++it;
    }

    QCOMPARE(map.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)).value<UIntList>(), //
             map1.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_DNS)).value<UIntList>());
    QCOMPARE(map.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)).value<UIntListList>(),
             map1.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESSES)).value<UIntListList>());
    QCOMPARE(map.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)).value<UIntListList>(),
             map1.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTES)).value<UIntListList>());
    QCOMPARE(qdbus_cast<NMVariantMapList>(map.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA))),
             qdbus_cast<NMVariantMapList>(map1.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ADDRESS_DATA))));
    QCOMPARE(qdbus_cast<NMVariantMapList>(map.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA))),
             qdbus_cast<NMVariantMapList>(map1.value(QLatin1String(NMQT_SETTING_IP4_CONFIG_ROUTE_DATA))));
}

QTEST_MAIN(IPv4SettingTest)

#include "moc_ipv4settingtest.cpp"
