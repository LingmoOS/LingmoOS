/*
    SPDX-FileCopyrightText: 2012-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEMPLATE_SETTING_P_H
#define NETWORKMANAGERQT_TEMPLATE_SETTING_P_H

#include <QString>

namespace NetworkManager
{
class TemplateSettingPrivate
{
public:
    TemplateSettingPrivate();

    QString name;
};

}

#endif // NETWORKMANAGERQT_TEMPLATE_SETTING_P_H
