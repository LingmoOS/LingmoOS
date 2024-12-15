// Copyright (C) 2023 ~ 2023 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "permissions_info.h"
void registerPermissionsInfoMetaType()
{
    qRegisterMetaType<PermissionsInfo>("PermissionsInfo");
    qDBusRegisterMetaType<PermissionsInfo>();
}
