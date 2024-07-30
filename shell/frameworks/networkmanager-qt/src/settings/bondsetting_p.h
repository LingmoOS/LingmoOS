/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BOND_SETTING_P_H
#define NETWORKMANAGERQT_BOND_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class BondSettingPrivate
{
public:
    BondSettingPrivate();

    QString name;
    QString interfaceName;
    NMStringMap options;
};

}

#endif // NETWORKMANAGERQT_BOND_SETTING_P_H
