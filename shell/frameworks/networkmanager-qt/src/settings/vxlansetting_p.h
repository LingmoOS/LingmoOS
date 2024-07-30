/*
    SPDX-FileCopyrightText: 2018 Billy Laws <blaws05@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VXLAN_SETTING_P_H
#define NETWORKMANAGERQT_VXLAN_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class VxlanSettingPrivate
{
public:
    VxlanSettingPrivate();

    QString name;
    quint32 ageing;
    quint32 destinationPort;
    quint32 id;
    bool l2Miss;
    bool l3Miss;
    bool learning;
    quint32 limit;
    QString local;
    QString parent;
    bool proxy;
    QString remote;
    bool rsc;
    quint32 sourcePortMax;
    quint32 sourcePortMin;
    quint32 tos;
    quint32 ttl;
};

}

#endif // NETWORKMANAGERQT_VXLAN_SETTING_P_H
