// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderlauncherinfo.h"

// 重载launcher dbus GetAllItemInfos获取的结构体操作符函数
const QDBusArgument &operator>>(const QDBusArgument &argument, LauncherItemInfo &iteminfos)
{
    argument.beginStructure();
    argument >> iteminfos.Path >> iteminfos.Name >> iteminfos.ID >> iteminfos.Icon >> iteminfos.CategoryID >> iteminfos.TimeInstalled;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator<<(QDBusArgument &argument, const LauncherItemInfo &iteminfo)
{
    argument.beginStructure();
    argument << iteminfo.Path << iteminfo.Name << iteminfo.ID << iteminfo.Icon << iteminfo.CategoryID << iteminfo.TimeInstalled;
    argument.endStructure();
    return argument;
}

void registerLauncherItemInfoMetaType()
{
    qRegisterMetaType<LauncherItemInfo>("LauncherItemInfo");
    qDBusRegisterMetaType<LauncherItemInfo>();
}

void registerLauncherItemInfoListMetaType()
{
    qRegisterMetaType<LauncherItemInfoList>("LauncherItemInfoList");
    qDBusRegisterMetaType<LauncherItemInfoList>();
}
