// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcheriteminfolist.h"

void registerLauncherItemInfoListMetaType()
{
    qRegisterMetaType<LauncherItemInfo>("ItemInfoList");
    qDBusRegisterMetaType<LauncherItemInfoList>();
}
