// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUNIONIDDAV_H
#define DUNIONIDDAV_H

#include "ddatasyncbase.h"

/**
 * @brief The DUnionIDDav class
 * Union ID 云同步
 */
class DUnionIDDav : public DDataSyncBase
{
    Q_OBJECT
public:
    DUnionIDDav();
    ~DUnionIDDav() override;
    void syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType) override;

private:
    class DUIDSynDataPrivate;
    DUIDSynDataPrivate *const d;
};

#endif // DUNIONIDDAV_H
