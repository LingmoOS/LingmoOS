/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPV6_SETTING_P_H
#define NETWORKMANAGERQT_IPV6_SETTING_P_H

#include "../ipconfig.h"

#include <QHostAddress>
#include <QStringList>

namespace NetworkManager
{
class Ipv6SettingPrivate
{
public:
    Ipv6SettingPrivate();

    QString name;
    NetworkManager::Ipv6Setting::ConfigMethod method;
    QList<QHostAddress> dns;
    QStringList dnsSearch;
    QList<NetworkManager::IpAddress> addresses;
    QList<NetworkManager::IpRoute> routes;
    int routeMetric;
    bool ignoreAutoRoutes;
    bool ignoreAutoDns;
    bool neverDefault;
    bool mayFail;
    NetworkManager::Ipv6Setting::IPv6Privacy privacy;
    qint32 dadTimeout;
    NetworkManager::Ipv6Setting::IPv6AddressGenMode addressGenMode;
    qint32 dhcpTimeout;
    QString dhcpHostname;
    QString dhcpDuid;
    QString token;
    QStringList dnsOptions;
    NMVariantMapList addressData;
    NMVariantMapList routeData;
    qint32 routeTable;
};

}
#endif // NETWORKMANAGERQT_IPV6_SETTING_P_H
