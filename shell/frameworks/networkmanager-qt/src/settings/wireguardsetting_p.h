/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREGUARD_SETTING_P_H
#define NETWORKMANAGERQT_WIREGUARD_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class WireGuardSettingPrivate
{
public:
    WireGuardSettingPrivate();

    QString name;

    quint32 fwmark;
    quint32 listenPort;
    quint32 mtu;
    bool peerRoutes;
    NMVariantMapList peers;
    QString privateKey;
    NetworkManager::Setting::SecretFlags privateKeyFlags;
};

}

#endif // NETWORKMANAGERQT_WIREGUARD_SETTING_P_H
