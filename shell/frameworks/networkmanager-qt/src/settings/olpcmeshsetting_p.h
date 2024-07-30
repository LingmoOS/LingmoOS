/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OLPCMESH_SETTING_P_H
#define NETWORKMANAGERQT_OLPCMESH_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class OlpcMeshSettingPrivate
{
public:
    OlpcMeshSettingPrivate();

    QString name;
    QByteArray ssid;
    quint32 channel;
    QByteArray dhcpAnycastAddress;
};

}

#endif // NETWORKMANAGERQT_OLPCMESH_SETTING_P_H
