/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TUN_SETTING_P_H
#define NETWORKMANAGERQT_TUN_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class TunSettingPrivate
{
public:
    TunSettingPrivate();

    QString name;
    QString group;
    NetworkManager::TunSetting::Mode mode;
    bool multiQueue;
    QString owner;
    bool pi;
    bool vnetHdr;
};

}

#endif // NETWORKMANAGERQT_TUN_SETTING_P_H
