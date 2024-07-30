/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESS_SETTING_P_H
#define NETWORKMANAGERQT_WIRELESS_SETTING_P_H

#include <QStringList>

namespace NetworkManager
{
class WirelessSettingPrivate
{
public:
    WirelessSettingPrivate();

    QString name;
    QByteArray ssid;
    NetworkManager::WirelessSetting::NetworkMode mode;
    NetworkManager::WirelessSetting::FrequencyBand band;
    quint32 channel;
    QByteArray bssid;
    quint32 rate;
    quint32 txPower;
    QByteArray macAddress;
    QByteArray clonedMacAddress;
    QStringList macAddressBlacklist;
    quint32 mtu;
    QStringList seenBssids;
    QString security;
    bool hidden;
    QString generateMacAddressMask;
    NetworkManager::Setting::MacAddressRandomization macAddressRandomization;
    NetworkManager::WirelessSetting::PowerSave powersave;
    QString assignedMacAddress;
};

}

#endif // NETWORKMANAGERQT_WIRELESS_SETTING_P_H
