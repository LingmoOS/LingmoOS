// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkerrors.h"

void registerNetworkErrorsMetaType()
{
    qRegisterMetaType<NetworkErrors>("NetworkErrors");
    qDBusRegisterMetaType<NetworkErrors>();
}
