/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAX_SETTING_P_H
#define NETWORKMANAGERQT_WIMAX_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class WimaxSettingPrivate
{
public:
    WimaxSettingPrivate();

    QString name;
    QString networkName;
    QByteArray macAddress;
};

}

#endif // NETWORKMANAGERQT_WIMAX_SETTING_P_H
