/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRELESSSECURITY_SETTING_P_H
#define NETWORKMANAGERQT_WIRELESSSECURITY_SETTING_P_H

#include <QList>
#include <QString>

namespace NetworkManager
{
class WirelessSecuritySettingPrivate
{
public:
    WirelessSecuritySettingPrivate();

    QString name;
    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt;
    quint32 wepTxKeyidx;
    NetworkManager::WirelessSecuritySetting::AuthAlg authAlg;
    QList<NetworkManager::WirelessSecuritySetting::WpaProtocolVersion> proto;
    QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities> pairwise;
    QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities> group;
    QString leapUsername;
    QString wepKey0;
    QString wepKey1;
    QString wepKey2;
    QString wepKey3;
    NetworkManager::Setting::SecretFlags wepKeyFlags;
    NetworkManager::WirelessSecuritySetting::WepKeyType wepKeyType;
    QString psk;
    NetworkManager::Setting::SecretFlags pskFlags;
    QString leapPassword;
    NetworkManager::Setting::SecretFlags leapPasswordFlags;
    NetworkManager::WirelessSecuritySetting::Pmf pmf;
};

}

#endif // NETWORKMANAGERQT_WIRELESSSECURITY_SETTING_P_H
