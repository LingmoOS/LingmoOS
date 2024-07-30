/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MACSECSETTING_P_H
#define NETWORKMANAGERQT_MACSECSETTING_P_H

#include <QString>

namespace NetworkManager
{
class MacsecSettingPrivate
{
public:
    MacsecSettingPrivate();

    QString name;

    bool encrypt;
    QString mkaCak;
    QString mkaCkn;
    NetworkManager::MacsecSetting::Mode mode;
    QString parent;
    qint32 port;
    bool sendSci;
    NetworkManager::MacsecSetting::Validation validation;
    NetworkManager::Setting::SecretFlags mkaCakFlags;
};

}

#endif // NETWORKMANAGERQT_MACSECSETTING_P_H
