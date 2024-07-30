/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BRIDGE_SETTING_P_H
#define NETWORKMANAGERQT_BRIDGE_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class BridgeSettingPrivate
{
public:
    BridgeSettingPrivate();

    QString name;
    QString interfaceName;
    bool multicastSnooping;
    bool stp;
    quint32 priority;
    quint32 forwardDelay;
    quint32 helloTime;
    quint32 maxAge;
    quint32 agingTime;
    QByteArray macAddress;
};

}

#endif // NETWORKMANAGERQT_BRIDGE_SETTING_P_H
