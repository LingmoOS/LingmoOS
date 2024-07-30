/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_SETTING_P_H
#define NETWORKMANAGERQT_TEAM_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class TeamSettingPrivate
{
public:
    TeamSettingPrivate();

    QString name;
    QString interfaceName;
    QString config;
};

}

#endif // NETWORKMANAGERQT_TEAM_SETTING_P_H
