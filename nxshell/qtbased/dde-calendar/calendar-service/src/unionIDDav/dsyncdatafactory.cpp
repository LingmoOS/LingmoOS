// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsyncdatafactory.h"

#include "dunioniddav.h"
#include "commondef.h"

#include <QDebug>

DSyncDataFactory::DSyncDataFactory()
{
}

DDataSyncBase *DSyncDataFactory::createDataSync(const DAccount::Ptr &account)
{
    DDataSyncBase *syncBase = nullptr;
    switch (account->accountType()) {
    case DAccount::Account_UnionID:
        syncBase = new DUnionIDDav();
        qCInfo(ServiceLogger) << "创建同步任务";
        break;
    default:
        syncBase = nullptr;
        break;
    }
    return syncBase;
}
