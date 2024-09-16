// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatasyncbase.h"
#include "dunioniddav.h"

DDataSyncBase::DDataSyncBase()
{
    qRegisterMetaType<DDataSyncBase::UpdateTypes>("DDataSyncBase::UpdateTypes");
    qRegisterMetaType<SyncTypes>("SyncTypes");
}

DDataSyncBase::~DDataSyncBase()
{
}


