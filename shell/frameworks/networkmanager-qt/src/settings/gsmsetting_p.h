/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GSM_SETTING_P_H
#define NETWORKMANAGERQT_GSM_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class GsmSettingPrivate
{
public:
    GsmSettingPrivate();

    QString name;
    QString number;
    QString username;
    QString password;
    NetworkManager::Setting::SecretFlags passwordFlags;
    QString apn;
    QString networkId;
    NetworkManager::GsmSetting::NetworkType networkType;
    QString pin;
    NetworkManager::Setting::SecretFlags pinFlags;
    quint32 allowedBand;
    bool homeOnly;
    QString deviceId;
    QString simId;
    QString simOperatorId;
};

}

#endif // NETWORKMANAGERQT_GSM_SETTING_P_H
