// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderprocessinfo.h"

bool DefenderProcessInfo::operator!=(const DefenderProcessInfo &procInfo)
{
    return procInfo.proc != proc || procInfo.title != title || procInfo.desktop != desktop;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderProcessInfo &procInfo)
{
    argument.beginStructure();
    argument << procInfo.proc << procInfo.title << procInfo.desktop;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderProcessInfo &procInfo)
{
    argument.beginStructure();
    argument >> procInfo.proc >> procInfo.title >> procInfo.desktop;
    argument.endStructure();
    return argument;
}

void registerDefenderProcessInfoMetaType()
{
    qRegisterMetaType<DefenderProcessInfoList>("DefenderProcessInfoList");
    qDBusRegisterMetaType<DefenderProcessInfoList>();

    qRegisterMetaType<DefenderProcessInfo>("DefenderProcessInfo");
    qDBusRegisterMetaType<DefenderProcessInfo>();
}
