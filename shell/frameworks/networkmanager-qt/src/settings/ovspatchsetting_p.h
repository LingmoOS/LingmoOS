/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OVS_PATCH_SETTING_P_H
#define NETWORKMANAGERQT_OVS_PATCH_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class OvsPatchSettingPrivate
{
public:
    OvsPatchSettingPrivate();

    QString name;

    QString peer;
};

}

#endif // NETWORKMANAGERQT_OVS_PATCH_SETTING_P_H
