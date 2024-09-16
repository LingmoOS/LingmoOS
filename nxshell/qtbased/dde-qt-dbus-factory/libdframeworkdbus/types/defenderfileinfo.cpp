// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderfileinfo.h"

bool DefenderFileInfo::operator!=(const DefenderFileInfo &versionInfo)
{
    return versionInfo.type != type || versionInfo.description != description;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderFileInfo &versionInfo)
{
    argument.beginStructure();
    argument << versionInfo.type << versionInfo.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderFileInfo &versionInfo)
{
    argument.beginStructure();
    argument >> versionInfo.type >> versionInfo.description;
    argument.endStructure();
    return argument;
}

void registerDefenderFileInfoMetaType()
{
    qRegisterMetaType<DefenderFileInfo>("DefenderFileInfo");
    qDBusRegisterMetaType<DefenderFileInfo>();
}
