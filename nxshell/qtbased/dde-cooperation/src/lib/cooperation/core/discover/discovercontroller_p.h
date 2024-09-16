// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISCOVERYCONTROLLER_P_H
#define DISCOVERYCONTROLLER_P_H

#include "discovercontroller.h"
#include "qzeroconf.h"

namespace cooperation_core {

class DiscoverControllerPrivate
{
    friend class DiscoverController;

public:
    explicit DiscoverControllerPrivate(DiscoverController *qq);

private:
    DiscoverController *q;
    QZeroConf *zeroConf = { nullptr };
    QList<DeviceInfoPointer> onlineDeviceList;
    DeviceInfoPointer searchDevice;
    //过滤非同子网段
    QString ipfilter;
    //发现服务名，需要为局域网唯一
    QString zeroconfname;
};

}   // namespace cooperation_core

#endif   // DISCOVERYCONTROLLER_P_H
