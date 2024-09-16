// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDATASYNCBASE_H
#define DDATASYNCBASE_H

#include "daccount.h"
#include "daccountdatabase.h"
#include "syncfilemanage.h"

#include <QObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFlags>

#ifndef Q_MOC_RUN
#define Q_DECLARE_FLAGS(Flags, Enum)\
typedef QFlags<Enum> Flags;
#endif

/**
 * @brief The DDataSyncBase class 云同步的基类
 */
class DDataSyncBase : public QObject
{
    Q_OBJECT
public:
    enum UpdateType {
        Update_None = 0, /*无*/
        Update_Schedule = 1, /*更新日程*/
        Update_ScheduleType = 2, /*更新日程类型*/
        Update_Setting = 4, /*更新设置*/
    };
    Q_DECLARE_FLAGS(UpdateTypes, UpdateType)

    enum SyncType {
        Sync_None = 0, /*无*/
        Sync_Upload = 1, /*上传*/
        Sync_Download = 2, /*下载*/
    };
    Q_DECLARE_FLAGS(SyncTypes, SyncType)
    DDataSyncBase();
    virtual ~DDataSyncBase();
    virtual void syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType) = 0;
signals:
    //
    void signalUpdate(const UpdateTypes updateType);
    void signalSyncState(const int syncState);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DDataSyncBase::UpdateTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(DDataSyncBase::SyncTypes)


#endif // DDATASYNCBASE_H
