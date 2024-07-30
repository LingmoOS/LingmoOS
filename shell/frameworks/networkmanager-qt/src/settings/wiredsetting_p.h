/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIRED_SETTING_P_H
#define NETWORKMANAGERQT_WIRED_SETTING_P_H

#include <QMap>
#include <QStringList>

namespace NetworkManager
{
class WiredSettingPrivate
{
public:
    WiredSettingPrivate();

    QString name;
    NetworkManager::WiredSetting::PortType port;
    quint32 speed;
    NetworkManager::WiredSetting::DuplexType duplex;
    bool autoNegotiate;
    QByteArray macAddress;
    QByteArray clonedMacAddress;
    QStringList macAddressBlacklist;
    quint32 mtu;
    QStringList s390Subchannels;
    NetworkManager::WiredSetting::S390Nettype s390NetType;
    QMap<QString, QString> s390Options;
    QString generateMacAddressMask;
    NetworkManager::WiredSetting::WakeOnLanFlags wakeOnLan;
    QString wakeOnLanPassword;
    QString assignedMacAddress;
};

}

#endif // NETWORKMANAGERQT_WIRED_SETTING_P_H
