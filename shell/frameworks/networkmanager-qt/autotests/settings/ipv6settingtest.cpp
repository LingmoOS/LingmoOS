/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ipv6settingtest.h"

#include "generictypes.h"
#include "settings/ipv6setting.h"
#include "utils.h"

#include <arpa/inet.h>

#include <libnm/NetworkManager.h>

#include <QHostAddress>
#include <QTest>

using namespace NetworkManager;

// TODO: Test DNS,IPv6Addresses and IPv6Routes
void IPv6SettingTest::testSetting_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<IpV6DBusNameservers>("dns");
    QTest::addColumn<QStringList>("dnsSearch");
    QTest::addColumn<IpV6DBusAddressList>("addresses");
    QTest::addColumn<IpV6DBusRouteList>("routes");
    QTest::addColumn<int>("routeMetric");
    QTest::addColumn<bool>("ignoreAutoRoutes");
    QTest::addColumn<bool>("ignoreAutoDns");
    QTest::addColumn<bool>("neverDefault");
    QTest::addColumn<bool>("mayFail");
    QTest::addColumn<quint32>("privacy");
    QTest::addColumn<quint32>("dadTimeout");
    QTest::addColumn<quint32>("addressGenMode");
    QTest::addColumn<quint32>("dhcpTimeout");
    QTest::addColumn<QString>("dhcpHostname");
    QTest::addColumn<QString>("dhcpDuid");
    QTest::addColumn<QString>("token");
    QTest::addColumn<QStringList>("dnsOptions");
    QTest::addColumn<NMVariantMapList>("addressData");
    QTest::addColumn<NMVariantMapList>("routeData");
    QTest::addColumn<quint32>("routeTable");

    QStringList dnsSearch;
    dnsSearch << "foo.com";
    dnsSearch << "foo.bar";

    IpV6DBusNameservers dns;
    QByteArray dnsAddr1 = ipv6AddressFromHostAddress(QHostAddress("2607:f0d0:1002:0051:0000:0000:0000:0004"));
    dns << dnsAddr1;

    IpV6DBusAddressList addresses;
    IpV6DBusAddress address;
    address.address = ipv6AddressFromHostAddress(QHostAddress("2001:0db8:0000:0000:0000::1428:57ab"));
    address.prefix = 64;
    address.gateway = ipv6AddressFromHostAddress(QHostAddress("2001:0db8:0:f101::1"));
    addresses << address;

    IpV6DBusRouteList routes;
    IpV6DBusRoute route;
    route.destination = ipv6AddressFromHostAddress(QHostAddress("2001:0db8:0000:0000:0000::1428:57ab"));
    route.prefix = 48;
    route.nexthop = ipv6AddressFromHostAddress(QHostAddress("2001:638:500:101:2e0:81ff:fe24:37c6"));
    route.metric = 1024;
    routes << route;

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
                              << true // neverDefault
                              << false // mayFail
                              << (quint32)0 // privacy
                              << (quint32)1101 // dadTimeout
                              << (quint32)0 // addressGenMode
                              << (quint32)110 // dhcpTimeout
                              << QString("dhcp-hostname") // dhcpHostname
                              << QString("dhcp-duid") // dhcpDuid
                              << QString("token") // token
                              << QStringList{QString("option1"), QString("option2")} // dnsOptions
                              << addressData // addressData
                              << routeData // routeData
                              << (quint32)1; // routeTable
}

void IPv6SettingTest::testSetting()
{
    QFETCH(QString, method);
    QFETCH(IpV6DBusNameservers, dns);
    QFETCH(QStringList, dnsSearch);
    QFETCH(IpV6DBusAddressList, addresses);
    QFETCH(IpV6DBusRouteList, routes);
    QFETCH(int, routeMetric);
    QFETCH(bool, ignoreAutoRoutes);
    QFETCH(bool, ignoreAutoDns);
    QFETCH(bool, neverDefault);
    QFETCH(bool, mayFail);
    QFETCH(quint32, privacy);
    QFETCH(quint32, dadTimeout);
    QFETCH(quint32, addressGenMode);
    QFETCH(quint32, dhcpTimeout);
    QFETCH(QString, dhcpHostname);
    QFETCH(QString, dhcpDuid);
    QFETCH(QString, token);
    QFETCH(QStringList, dnsOptions);
    QFETCH(NMVariantMapList, addressData);
    QFETCH(NMVariantMapList, routeData);
    QFETCH(quint32, routeTable);

    QVariantMap map;

    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_METHOD), method);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS), QVariant::fromValue(dns));
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_SEARCH), dnsSearch);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES), QVariant::fromValue(addresses));
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES), QVariant::fromValue(routes));
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_METRIC), routeMetric);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES), ignoreAutoRoutes);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_DNS), ignoreAutoDns);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_NEVER_DEFAULT), neverDefault);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_MAY_FAIL), mayFail);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_IP6_PRIVACY), privacy);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DAD_TIMEOUT), dadTimeout);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_GEN_MODE), addressGenMode);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_TIMEOUT), dhcpTimeout);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_HOSTNAME), dhcpHostname);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DHCP_DUID), dhcpDuid);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_TOKEN), token);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS_OPTIONS), dnsOptions);
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA), QVariant::fromValue(addressData));
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA), QVariant::fromValue(routeData));
    map.insert(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_TABLE), routeTable);

    NetworkManager::Ipv6Setting setting;
    setting.fromMap(map);

    QVariantMap map1 = setting.toMap();

    // Will fail if set some default values, because they are skipped in toMap() method
    QVariantMap::const_iterator it = map.constBegin();
    while (it != map.constEnd()) {
        /* clang-format off */
        if (it.key() != QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)
            && it.key() != QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES)
            && it.key() != QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTES)
            && it.key() != QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA)
            && it.key() != QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA)) { /* clang-format on */
            QCOMPARE(it.value(), map1.value(it.key()));
        }
        ++it;
    }

    IpV6DBusNameservers nameServers1 = map.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).value<IpV6DBusNameservers>();
    IpV6DBusNameservers nameServers2 = map1.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_DNS)).value<IpV6DBusNameservers>();

    for (int i = 0; i < nameServers1.size(); i++) {
        QHostAddress dnsAddr1 = ipv6AddressAsHostAddress(nameServers1.at(i));
        QHostAddress dnsAddr2 = ipv6AddressAsHostAddress(nameServers2.at(i));
        QCOMPARE(dnsAddr1.protocol(), QAbstractSocket::IPv6Protocol);
        QCOMPARE(dnsAddr2.protocol(), QAbstractSocket::IPv6Protocol);
        QCOMPARE(dnsAddr1, dnsAddr2);
    }

    IpV6DBusAddressList addresses1 = map.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES)).value<IpV6DBusAddressList>();
    IpV6DBusAddressList addresses2 = map1.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESSES)).value<IpV6DBusAddressList>();

    for (int i = 0; i < addresses1.size(); i++) {
        QHostAddress addr1(QString(addresses1.at(i).address));
        QHostAddress addr2(QString(addresses2.at(i).address));
        QHostAddress gateway1(QString(addresses1.at(i).gateway));
        QHostAddress gateway2(QString(addresses2.at(i).gateway));

        QCOMPARE(addr1, addr2);
        QCOMPARE(addresses1.at(i).prefix, addresses2.at(i).prefix);
        QCOMPARE(gateway1, gateway2);
    }

    IpV6DBusRouteList routes1 = map.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES)).value<IpV6DBusRouteList>();
    IpV6DBusRouteList routes2 = map1.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_IGNORE_AUTO_ROUTES)).value<IpV6DBusRouteList>();

    for (int i = 0; i < routes1.size(); i++) {
        QHostAddress destination1(QString(routes1.at(i).destination));
        QHostAddress destination2(QString(routes2.at(i).destination));
        QHostAddress nexthop1(QString(routes1.at(i).nexthop));
        QHostAddress nexthop2(QString(routes2.at(i).nexthop));

        QCOMPARE(destination1, destination2);
        QCOMPARE(routes1.at(i).prefix, routes2.at(i).prefix);
        QCOMPARE(nexthop1, nexthop2);
        QCOMPARE(routes1.at(i).metric, routes2.at(i).metric);
    }

    QCOMPARE(qdbus_cast<NMVariantMapList>(map.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA))),
             qdbus_cast<NMVariantMapList>(map1.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ADDRESS_DATA))));
    QCOMPARE(qdbus_cast<NMVariantMapList>(map.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA))),
             qdbus_cast<NMVariantMapList>(map1.value(QLatin1String(NMQT_SETTING_IP6_CONFIG_ROUTE_DATA))));
}

QTEST_MAIN(IPv6SettingTest)

#include "moc_ipv6settingtest.cpp"
