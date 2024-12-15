// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "envinfolist.h"

void registerEnvInfoListMetaType()
{
    qRegisterMetaType<EnvInfoList>("EnvInfoList");
    qDBusRegisterMetaType<EnvInfoList>();
}
