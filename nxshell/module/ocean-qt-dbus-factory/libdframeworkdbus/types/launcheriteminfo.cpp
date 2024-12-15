// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheriteminfo.h"

bool LauncherItemInfo::operator!=(const LauncherItemInfo &itemInfo)
{
    return itemInfo.ID != ID;
}

QDBusArgument &operator<<(QDBusArgument &argument, const LauncherItemInfo &itemInfo)
{
    argument.beginStructure();
    argument << itemInfo.Path << itemInfo.Name << itemInfo.ID << itemInfo.Icon << itemInfo.CategoryID << itemInfo.TimeInstalled;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LauncherItemInfo &itemInfo)
{
    argument.beginStructure();
    argument >> itemInfo.Path >> itemInfo.Name >> itemInfo.ID >> itemInfo.Icon >> itemInfo.CategoryID >> itemInfo.TimeInstalled;
    argument.endStructure();
    return argument;
}

void registerLauncherItemInfoMetaType()
{
    qRegisterMetaType<LauncherItemInfo>("ItemInfo");
    qDBusRegisterMetaType<LauncherItemInfo>();
}
