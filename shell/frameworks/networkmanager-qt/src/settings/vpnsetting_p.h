/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VPN_SETTING_P_H
#define NETWORKMANAGERQT_VPN_SETTING_P_H

#include "generictypes.h"

#include <QString>

namespace NetworkManager
{
class VpnSettingPrivate
{
public:
    VpnSettingPrivate();

    QString name;
    QString serviceType;
    QString username;
    NMStringMap data;
    NMStringMap secrets;
    bool persistent;
    uint timeout;
};

}

#endif // NETWORKMANAGERQT_VPN_SETTING_P_H
