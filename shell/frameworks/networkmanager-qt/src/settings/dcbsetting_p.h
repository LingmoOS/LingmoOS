/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DCBSETTING_P_H
#define NETWORKMANAGERQT_DCBSETTING_P_H

#include <QString>

namespace NetworkManager
{
class DcbSettingPrivate
{
public:
    DcbSettingPrivate();

    QString name;

    QString appFcoeMode;
    qint32 appFcoePriority;
    qint32 appFipPriority;
    qint32 appIscsiPriority;
    NetworkManager::DcbSetting::DcbFlags appFcoeFlags;
    NetworkManager::DcbSetting::DcbFlags appFipFlags;
    NetworkManager::DcbSetting::DcbFlags appIscsiFlags;
    NetworkManager::DcbSetting::DcbFlags priorityGroupFlags;
    NetworkManager::DcbSetting::DcbFlags priorityFlowControlFlags;
    UIntList priorityFlowControl;
    UIntList priorityBandwidth;
    UIntList priorityGroupBandwidth;
    UIntList priorityGroupId;
    UIntList priorityStrictBandwidth;
    UIntList priorityTrafficClass;
};

}

#endif // NETWORKMANAGERQT_DCBSETTING_P_H
