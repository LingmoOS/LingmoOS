/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TC_SETTING_P_H
#define NETWORKMANAGERQT_TC_SETTING_P_H

#include <QString>

#include <networkmanagerqt/networkmanagerqt_export.h>

typedef QList<QVariantMap> NMVariantMapList;

namespace NetworkManager
{
class TcSettingPrivate
{
public:
    TcSettingPrivate();

    QString name;

    NMVariantMapList qdiscs;
    NMVariantMapList tfilters;
};

}

#endif // NETWORKMANAGERQT_TC_SETTING_P_H
