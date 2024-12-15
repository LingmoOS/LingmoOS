// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderversioninfo.h"

bool DefenderVersionInfo::operator!=(const DefenderVersionInfo &versionInfo)
{
    return versionInfo.time != time || versionInfo.description != description || versionInfo.version != version;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderVersionInfo &versionInfo)
{
    argument.beginStructure();
    argument << versionInfo.version << versionInfo.description << versionInfo.time;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderVersionInfo &versionInfo)
{
    argument.beginStructure();
    argument >> versionInfo.version >> versionInfo.description >> versionInfo.time;
    argument.endStructure();
    return argument;
}

void registerDefenderVersionInfoMetaType() {
    qRegisterMetaType<DefenderVersionInfo>("DefenderVersionInfo");
    qDBusRegisterMetaType<DefenderVersionInfo>();
}
