/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_PROXY_SETTING_P_H
#define NETWORKMANAGERQT_PROXY_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class ProxySettingPrivate
{
public:
    ProxySettingPrivate();

    QString name;

    bool browserOnly;
    NetworkManager::ProxySetting::Mode method;
    QString pacScript;
    QString pacUrl;
};

}

#endif // NETWORKMANAGERQT_PROXY_SETTING_P_H
