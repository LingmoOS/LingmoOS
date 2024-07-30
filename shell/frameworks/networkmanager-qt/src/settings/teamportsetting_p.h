/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_PORT_SETTING_P_H
#define NETWORKMANAGERQT_TEAM_PORT_SETTING_P_H

#include <QString>

typedef QList<QVariantMap> NMVariantMapList;

namespace NetworkManager
{
class TeamPortSettingPrivate
{
public:
    TeamPortSettingPrivate();

    QString name;

    QString config;
    qint32 lacpKey;
    qint32 lacpPrio;
    qint32 prio;
    qint32 queueId;
    bool sticky;
    NMVariantMapList linkWatchers;
};

}

#endif // NETWORKMANAGERQT_TEAM_PORT_SETTING_P_H
