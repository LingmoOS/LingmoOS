// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESS_P_H
#define DWIRELESS_P_H

#include "dwirelesssetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWirelessSettingPrivate
{
public:
    DWirelessSettingPrivate();

    bool m_hidden;
    quint32 m_channel;
    quint32 m_rate;
    quint32 m_txPower;
    quint32 m_mtu;
    DWirelessSetting::NetworkMode m_mode;
    DWirelessSetting::FrequencyBand m_band;
    DWirelessSetting::MacAddressRandomization m_macAddressRandomization;
    DWirelessSetting::PowerSave m_powersave;
    QByteArray m_ssid;
    QByteArray m_bssid;
    QByteArray m_macAddress;
    QByteArray m_clonedMacAddress;
    QString m_name;
    QString m_assignedMacAddress;
    QStringList m_macAddressBlacklist;
    QString m_security;
    QString m_generateMacAddressMask;
    QStringList m_seenBssids;
};

DNETWORKMANAGER_END_NAMESPACE

#endif
