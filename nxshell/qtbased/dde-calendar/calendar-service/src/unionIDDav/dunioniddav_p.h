// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUNIONIDDAV_P_H
#define DUNIONIDDAV_P_H

#include "ddatabase.h"
#include "ddatasyncbase.h"
#include "units.h"

#include <QObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

/**
     * @brief The SyncStack struct 记录单次上传的信息和相关操作
     */
struct SyncStack {
    SyncStack() {}
    SyncStack(QString _accountId, QString _accountName, int _accountState, DDataSyncBase::SyncTypes _syncType)
        : accountId(_accountId)
        , dbname_account(_accountName)
        , dbname_manager(DDataBase::NameAccountManager)
        , dbname_account_thread(DDataBase::createUuid())
        , dbname_manager_thread(DDataBase::createUuid())
        , dbname_sync_thread(DDataBase::createUuid())
        , accountState(_accountState)
        , db_account(QSqlDatabase::database(dbname_account))
        , db_manager(QSqlDatabase::database(dbname_manager))
        , syncType(_syncType) {}

    //data
    QString accountId;              //账户ID
    QString dbname_account;         //主线程的账户数据库别名
    QString dbname_manager;         //主线程的管理数据库别名
    QString dbname_account_thread;  //子线程的账户数据库别名
    QString dbname_manager_thread;  //子线程的管理数据库别名
    QString dbname_sync_thread;     //子线程的同步数据库别名
    int accountState = 0;           //账户状态
    QSqlDatabase  db_account;       //主线程的账户数据库实例
    QSqlDatabase  db_manager;       //主线程的管理数据库实例
    DDataSyncBase::SyncTypes syncType;//记录类型：上传或下载

    //sync
    QString dbpath_sync;

    //func
    //下载云端数据
    int downloadUidData(bool &isInitSyncData, SyncFileManage &fileManger);
    //将本地修改数据移动到下载的临时文件中
    int loadToTmp();
    //将临时文件中的数据移动到本地数据库中
    int tmpToLoad(DDataSyncBase::UpdateTypes &updateType);
    //上传修改过的临时数据
    int uploadTmpData(SyncFileManage &fileManger);
    void deleteTmpData(SyncFileManage &fileManger);

    //sql prepare
    QString prepareQuest(int count);
    //sql bindvalue
    void prepareBinds(QSqlQuery &query, QSqlRecord source);
    //sql sync db1.table to  db2.table
    bool syncIntoTable(const QString &table_name, const QString &connection_name_source, const QString &connection_name_target);
    //sql select first record
    QSqlRecord selectRecord(const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name);
    //sql replace db1.record to  db2.table
    bool replaceIntoRecord(const QString &table_name, QSqlRecord record, const QString &connection_name);
    //sql select first value
    QVariant selectValue(const QString &value_name, const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name);
    //sql select first value
    QVariant selectValue(const QString &sql, const QString &connection_name);
    //sql delete table
    bool deleteTableLine(const QString &table_name, const QString &key_name, const QVariant &key_value, const QString &connection_name);
    //sql delete table
    bool deleteTable(const QString &table_name, const QString &connection_name);
    //sql add column to server
    bool repairTable(const QString &table_name, const QString &connection_name_local, const QString &connection_name_server);

    bool  needUpdateSettingValue();
};


/**
     * @brief The ThrowAccount struct 用于初始化日程类型和类型颜色
     */
struct SyncAccount {
    explicit SyncAccount(const QString &connectionName, QString accountID)
        : _accountID(accountID)
        , _connectionName(connectionName)
    {
    }

    bool defaultScheduleType();
    bool defaultTypeColor();

    bool insertToScheduleType(const DScheduleType::Ptr &scheduleType);
    bool insertToTypeColor(const DTypeColor &typeColor);

    QString _accountID;
    QString _connectionName;
};

/**
 * @brief The SqlTransactionLocker class 用于数据库事务
 */
class SqlTransactionLocker
{
public:
    explicit SqlTransactionLocker(const QStringList &connectionNames)
        : _connectionNames(connectionNames)
    {
        for (auto name : _connectionNames) {
            SqliteQuery(name).transaction();
        }
    }
    ~SqlTransactionLocker()
    {
        if (hasCommited)
            return;

        for (auto name : _connectionNames)
            SqliteQuery(name).rollback();
    }
    void commit()
    {
        hasCommited = true;
        for (auto name : _connectionNames)
            SqliteQuery(name).commit();
    }
    void rollback()
    {
        hasCommited = true;
        for (auto name : _connectionNames)
            SqliteQuery(name).rollback();
    }

private:
    QStringList _connectionNames;
    bool hasCommited = false;
};

/**
 * @brief The DUIDSynDataWorker class 用于多线程上传下载
 */
class DUIDSynDataWorker : public QObject
{
    Q_OBJECT
public:
    DUIDSynDataWorker() : QObject(nullptr)
    {
    }
    ~DUIDSynDataWorker()
    {
    }

public slots:
    void syncData(SyncStack syncType);

public slots:
    void slotSync();

signals:
    void signalUpdate(const DDataSyncBase::UpdateTypes updateType);
    void signalSyncState(const int syncState);

private:
    void startUpdate();

private:
    //sync
    QString dbpath_sync;
    QTimer *mSyncTimer = nullptr;
    //thread
    SyncStack mSync;
    QVector<SyncStack> mSyncList;
};


/**
 * @brief The DUnionIDDav::DUIDSynDataPrivate class DUIDSynData的私有类
 */
class DUnionIDDav::DUIDSynDataPrivate : public QObject
{
    Q_OBJECT
public:
    DUIDSynDataPrivate();
    ~DUIDSynDataPrivate();

    void syncData(QString accountId, QString accountName, int accountState, DDataSyncBase::SyncTypes syncType);

signals:
    void signalUpdate(const DDataSyncBase::UpdateTypes updateType);
    void signalSyncState(const int syncState);

    void signalSyncData(const SyncStack syncType);

private:
    QThread *mThread;
};

#endif // DUNIONIDDAV_P_H
