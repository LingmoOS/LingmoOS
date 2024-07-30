/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BRIDGEPORT_SETTING_P_H
#define NETWORKMANAGERQT_BRIDGEPORT_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class BridgePortSettingPrivate
{
public:
    BridgePortSettingPrivate();

    QString name;
    quint32 priority;
    quint32 pathCost;
    bool hairpinMode;
};

}

#endif // NETWORKMANAGERQT_BRIDGEPORT_SETTING_P_H
