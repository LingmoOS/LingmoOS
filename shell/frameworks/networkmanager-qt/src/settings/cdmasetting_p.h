/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_CDMA_SETTING_P_H
#define NETWORKMANAGERQT_CDMA_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class CdmaSettingPrivate
{
public:
    CdmaSettingPrivate();

    QString name;
    QString number;
    QString username;
    QString password;
    NetworkManager::Setting::SecretFlags passwordFlags;
};

}

#endif // NETWORKMANAGERQT_CDMA_SETTINGS_P_H
