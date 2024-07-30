/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_ADSL_SETTING_P_H
#define NETWORKMANAGERQT_ADSL_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class AdslSettingPrivate
{
public:
    AdslSettingPrivate();

    QString name;
    QString username;
    QString password;
    NetworkManager::Setting::SecretFlags passwordFlags;
    NetworkManager::AdslSetting::Protocol protocol;
    NetworkManager::AdslSetting::Encapsulation encapsulation;
    quint32 vpi;
    quint32 vci;
};

}

#endif // NETWORKMANAGERQT_ADSL_SETTING_P_H
