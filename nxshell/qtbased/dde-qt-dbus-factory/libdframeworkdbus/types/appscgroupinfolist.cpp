// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appscgroupinfolist.h"

void registerAppsCGroupInfoMetaType()
{
    qRegisterMetaType<AppsCGroupInfo>("AppsCGroupInfo");
    qDBusRegisterMetaType<AppsCGroupInfo>();
}

void registerAppsCGroupInfoListMetaType()
{
    registerAppsCGroupInfoMetaType();

    qRegisterMetaType<AppsCGroupInfoList>("AppsCGroupInfoList");
    qDBusRegisterMetaType<AppsCGroupInfoList>();
}
