// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkinputdevices_types.h"
#include "dtkdevice_global.h"
#include "dtkinputdevices_converters.h"
#include <QDebug>
DDEVICE_BEGIN_NAMESPACE

const DeviceInfo DeviceInfoInitializer = {.id = 0xffffffff, .name = "Generic device", .type = DeviceType::Generic};

bool operator==(const DeviceInfo &lhs, const DeviceInfo &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name && lhs.type == rhs.type;
}

QDebug operator<<(QDebug debug, DeviceType type)
{
    debug << deviceTypeToString(type);
    return debug;
}

QDebug operator<<(QDebug debug, AccelerationProfile profile)
{
    debug << profileToString(profile);
    return debug;
}

QDebug operator<<(QDebug debug, const DeviceInfo &info)
{
    debug << "{id: " << info.id << ", name: " << info.name << ", type: " << info.type << "}";
    return debug;
}
DDEVICE_END_NAMESPACE
