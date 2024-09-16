// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEINFO_P_H
#define DEVICEINFO_P_H

#include "deviceinfo.h"

namespace cooperation_core {

class DeviceInfoPrivate
{
public:
    explicit DeviceInfoPrivate(DeviceInfo *qq);

    DeviceInfo *q;

    QString deviceName;
    QString ipAddress;
    deepin_cross::BaseUtils::OS_TYPE osType { deepin_cross::BaseUtils::kOther };
    bool isClipboardShared { false };
    bool isPeripheralShared { false };
    bool cooperationEnabled { true };
    DeviceInfo::ConnectStatus conStatus { DeviceInfo::ConnectStatus::Unknown };
    DeviceInfo::TransMode transMode { DeviceInfo::TransMode::Everyone };
    DeviceInfo::DiscoveryMode discoveryMode { DeviceInfo::DiscoveryMode::Everyone };
    DeviceInfo::LinkMode linkMode { DeviceInfo::LinkMode::RightMode };
};

}   // namespace cooperation_core

#endif   // DEVICEINFO_P_H
