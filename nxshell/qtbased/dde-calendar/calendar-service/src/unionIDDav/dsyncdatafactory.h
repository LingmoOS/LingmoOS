// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYNCDATAFACTORY_H
#define DSYNCDATAFACTORY_H

#include "ddatasyncbase.h"

/**
 * @brief The DSyncDataFactory class 根据不同类型的账号，生产不同的上传实例
 */
class DSyncDataFactory
{
public:
    DSyncDataFactory();
    static DDataSyncBase *createDataSync(const DAccount::Ptr &account);
};

#endif // DSYNCDATAFACTORY_H
