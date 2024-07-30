/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IP_TUNNEL_SETTING_P_H
#define NETWORKMANAGERQT_IP_TUNNEL_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class IpTunnelSettingPrivate
{
public:
    IpTunnelSettingPrivate();

    QString name;

    NetworkManager::IpTunnelSetting::Mode mode;
    bool pathMtuDiscovery;
    quint32 encapsulationLimit;
    NetworkManager::IpTunnelSetting::Flags flags;
    quint32 flowLabel;
    quint32 mtu;
    quint32 tos;
    quint32 ttl;
    QString inputKey;
    QString local;
    QString outputKey;
    QString parent;
    QString remote;
};

}

#endif // NETWORKMANAGERQT_IP_TUNNEL_SETTING_P_H
