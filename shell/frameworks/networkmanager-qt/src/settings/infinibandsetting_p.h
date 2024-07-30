/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_INFINIBAND_SETTING_P_H
#define NETWORKMANAGERQT_INFINIBAND_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class InfinibandSettingPrivate
{
public:
    InfinibandSettingPrivate();

    QString name;
    QByteArray macAddress;
    quint32 mtu;
    NetworkManager::InfinibandSetting::TransportMode transportMode;
    qint32 pKey;
    QString parent;
};

}

#endif // NETWORKMANAGERQT_INFINIBAND_SETTING_P_H
