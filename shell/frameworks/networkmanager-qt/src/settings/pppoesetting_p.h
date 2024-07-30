/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_PPPOE_SETTING_P_H
#define NETWORKMANAGERQT_PPPOE_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class PppoeSettingPrivate
{
public:
    PppoeSettingPrivate();

    QString name;
    QString service;
    QString username;
    QString password;
    QString parent;
    NetworkManager::Setting::SecretFlags passwordFlags;
};

}

#endif // NETWORKMANAGERQT_PPPOE_SETTING_P_H
