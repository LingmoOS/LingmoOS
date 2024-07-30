/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPV4_SETTING_P_H
#define NETWORKMANAGERQT_IPV4_SETTING_P_H

#include "../ipconfig.h"

#include <QHostAddress>
#include <QStringList>
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class Ipv4SettingPrivate
{
public:
    Ipv4SettingPrivate();

    QString name;
    NetworkManager::Ipv4Setting::ConfigMethod method;
    QList<QHostAddress> dns;
    QStringList dnsSearch;
    QList<NetworkManager::IpAddress> addresses;
    QList<NetworkManager::IpRoute> routes;
    int routeMetric;
    bool ignoreAutoRoutes;
    bool ignoreAutoDns;
    QString dhcpClientId;
    bool dhcpSendHostname;
    QString dhcpHostname;
    bool neverDefault;
    bool mayFail;
    qint32 dadTimeout;
    QString dhcpFqdn;
    QStringList dnsOptions;
    qint32 dnsPriority;
    QString gateway;
    NMVariantMapList addressData;
    NMVariantMapList routeData;
};

}
#endif // NETWORKMANAGERQT_IPV4_SETTING_P_H
