// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanappinfo.h"

QDBusArgument &operator<<(QDBusArgument &argument, const TrashCleanAppInfo &appInfo)
{
    argument.beginStructure();
    argument << appInfo.isUnistalled << appInfo.appPkgName << appInfo.appDisplayName
             << appInfo.appConfigPaths << appInfo.appCachePaths;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, TrashCleanAppInfo &appInfo)
{
    argument.beginStructure();
    argument >> appInfo.isUnistalled >> appInfo.appPkgName >> appInfo.appDisplayName
        >> appInfo.appConfigPaths >> appInfo.appCachePaths;
    argument.endStructure();
    return argument;
}

void registerTrashCleanAppInfoMetaType()
{
    qRegisterMetaType<TrashCleanAppInfo>("TrashCleanAppInfo");
    qDBusRegisterMetaType<TrashCleanAppInfo>();
}

void registerTrashCleanAppInfoListMetaType()
{
    qRegisterMetaType<TrashCleanAppInfoList>("TrashCleanAppInfoList");
    qDBusRegisterMetaType<TrashCleanAppInfoList>();
}
