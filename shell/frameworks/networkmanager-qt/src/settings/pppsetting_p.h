/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_PPP_SETTING_P_H
#define NETWORKMANAGERQT_PPP_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class PppSettingPrivate
{
public:
    PppSettingPrivate();

    QString name;
    bool noauth;
    bool refuseEap;
    bool refusePap;
    bool refuseChap;
    bool refuseMschap;
    bool refuseMschapv2;
    bool nobsdcomp;
    bool nodeflate;
    bool noVjComp;
    bool requireMppe;
    bool requireMppe128;
    bool mppeStateful;
    bool crtscts;
    quint32 baud;
    quint32 mru;
    quint32 mtu;
    quint32 lcpEchoFailure;
    quint32 lcpEchoInterval;
};

}

#endif // NETWORKMANAGERQT_PPP_SETTING_P_H
