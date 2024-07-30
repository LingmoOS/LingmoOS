/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VLAN_SETTING_P_H
#define NETWORKMANAGERQT_VLAN_SETTING_P_H

#include <QStringList>

namespace NetworkManager
{
class VlanSettingPrivate
{
public:
    VlanSettingPrivate();

    QString name;
    QString interfaceName;
    QString parent;
    quint32 id;
    NetworkManager::VlanSetting::Flags flags;
    QStringList ingressPriorityMap;
    QStringList egressPriorityMap;
};

}

#endif // NETWORKMANAGERQT_VLAN_SETTING_P_H
