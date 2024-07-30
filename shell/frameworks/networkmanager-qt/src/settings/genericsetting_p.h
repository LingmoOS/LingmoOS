/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GENERIC_SETTING_P_H
#define NETWORKMANAGERQT_GENERIC_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class GenericSettingPrivate
{
public:
    GenericSettingPrivate();

    QString name;
};

}

#endif // NETWORKMANAGERQT_GENERIC_SETTING_P_H
