/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SERIAL_SETTING_P_H
#define NETWORKMANAGERQT_SERIAL_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class SerialSettingPrivate
{
public:
    SerialSettingPrivate();

    QString name;
    quint32 baud;
    quint32 bits;
    NetworkManager::SerialSetting::Parity parity;
    quint32 stopbits;
    quint64 sendDelay;
};

}

#endif // NETWORKMANAGERQT_SERIAL_SETTING_P_H
