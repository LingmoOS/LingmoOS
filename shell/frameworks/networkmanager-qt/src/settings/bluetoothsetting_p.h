/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BLUETOOTH_SETTING_P_H
#define NETWORKMANAGERQT_BLUETOOTH_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class BluetoothSettingPrivate
{
public:
    BluetoothSettingPrivate();

    QString name;
    QByteArray bdaddr;
    NetworkManager::BluetoothSetting::ProfileType profileType;
};

}

#endif // NETWORKMANAGERQT_BLUETOOTH_SETTING_P_H
