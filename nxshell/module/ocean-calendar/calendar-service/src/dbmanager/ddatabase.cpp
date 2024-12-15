// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatabase.h"

#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QDebug>

static QMap<QString, SqliteMutex> DbpathMutexMap;//记录所有用到的数据库文件锁
static QMutex DbpathMutexMapMutex;               //DbpathMutexMap的锁

/**
 * @brief getDbMutexRef 根据dbpath获取数据库文件锁的引用
 */
SqliteMutex &getDbMutexRef(const QString &dbpath)
{
    QMutexLocker locker(&DbpathMutexMapMutex);

    if (!DbpathMutexMap.contains(dbpath)) {
        DbpathMutexMap.insert(dbpath, SqliteMutex());
    }
    return DbpathMutexMap[dbpath];
}


const QString DDataBase::NameAccountManager = "AccountManager";
const QString DDataBase::NameSync = "SyncManager";

const QString DDataBase::sql_create_account =
    " CREATE TABLE if not exists account(    "
    " id integer not null primary key,       "
    " syncState integer not null ,           "
    " accountState integer not null,         "
    " accountName text not null,             "
    " displayName text not null,             "
    " cloudPath text  ,                      "
    " accountType integer not null,          "
    " syncFreq integer not null,             "
    " intervalTime integer,                  "
    " syncTag    integer,                    "
    " expandStatus  integer,                 "
    " dtLastUpdate DATETIME                  "
    " )";

//日程表
const QString DDataBase::sql_create_schedules =
    " CREATE TABLE if not exists schedules ("
    " scheduleID TEXT not null primary key, "
    " scheduleTypeID TEXT not null,         "
    " summary TEXT not null,                "
    " description TEXT,                     "
    " allDay BOOL not null,                 "
    " dtStart DATETIME not null,            "
    " dtEnd DATETIME not null,              "
    " isAlarm   INTEGER  ,                  "
    " titlePinyin TEXT,                     "
    " isLunar INTEGER not null,             "
    " ics TEXT not null,                    "
    " fileName  TEXT,                       "
    " dtCreate DATETIME not null,           "
    " dtUpdate DATETIME ,                   "
    " dtDelete DATETIME,                    "
    " isDeleted INTEGER not null)";

//类型表
const QString DDataBase::sql_create_scheduleType =
    " CREATE TABLE if not exists scheduleType (            "
    " typeID TEXT not null PRIMARY KEY,                   "
    " typeName TEXT not null,                 "
    " typeDisplayName TEXT,                   "
    " typePath TEXT,                          "
    " typeColorID TEXT not null,           "
    " description TEXT ,                      "
    " privilege INTEGER not null,             "
    " showState INTEGER not null,             "
    " syncTag INTEGER,                        "
    " dtCreate DATETIME not null,             "
    " dtUpdate DATETIME,                      "
    " dtDelete DATETIME,                      "
    " isDeleted INTEGER not null)";
//颜色表
const QString DDataBase::sql_create_typeColor =
    " CREATE TABLE if not exists typeColor (              "
    " ColorID TEXT not null PRIMARY KEY,              "
    " ColorHex TEXT not null,                "
    " privilege INTEGER not null,"
    " dtCreate DATETIME not null)";

//创建上传任务表
const QString DDataBase::sql_create_uploadTask =
    " CREATE TABLE if not exists uploadTask (                "
    " taskID TEXT NOT NULL PRIMARY KEY,                     "
    " uploadType integer NOT NULL,         "
    " uploadObject integer NOT NULL,            "
    " objectID TEXT NOT NULL,                   "
    " dtCreate DATETIME NOT NULL)";

//创建提醒任务表
const QString DDataBase::sql_create_remindTask =
    " CREATE TABLE if not exists remindTask (            "
    " alarmID TEXT NOT NULL PRIMARY KEY,                "
    " scheduleID TEXT NOT NULL,             "
    " recurID DATETIME ,                    "
    " remindCount INTEGER,                  "
    " notifyID INTEGER ,                    "
    " dtRemind DATETIME NOT NULL,           "
    " dtStart DATETIME NOT NULL,            "
    " dtEnd DATETIME NOT NULL)";

//创建帐户管理表
const QString DDataBase::sql_create_accountManager =
    " CREATE TABLE  if not exists accountManager (    "
    " accountID TEXT NOT NULL PRIMARY KEY,             "
    " accountName TEXT NOT NULL,           "
    " displayName TEXT NOT NULL,           "
    " accountState INTEGER not null,       "
    " accountAvatar TEXT,                  "
    " accountDescription TEXT ,            "
    " accountType INTEGER not null,        "
    " dbName TEXT not null,                "
    " dBusPath TEXT not null,              "
    " dBusInterface TEXT not null,         "
    " dtCreate DATETIME not null,    "
    " dtDelete DATETIME,             "
    " dtUpdate DATETIME,             "
    " expandStatus  integer,               "
    " isDeleted INTEGER not null)";

//日历通用设置
const QString DDataBase::sql_create_calendargeneralsettings =
    " CREATE TABLE  if not exists calendargeneralsettings(     "
    " vch_key TEXT NOT NULL PRIMARY KEY,           "
    " vch_value TEXT NOT NULL           "
    " )";

const QString DDataBase::GWorkColorID = "0cecca8a-291b-46e2-bb92-63a527b77d46";
const QString DDataBase::GLifeColorID = "6cfd1459-1085-47e9-8ca6-379d47ec319a";
const QString DDataBase::GOtherColorID = "35e70047-98bb-49b9-8ad8-02d1c942f5d0";
const QString DDataBase::GFestivalColorID = "10af78a1-3c25-4744-91db-6fbe5e88083b";

DDataBase::DDataBase(QObject *parent)
    : QObject(parent)
    , m_DBPath("")
    , m_connectionName("")
{
}

DDataBase::~DDataBase()
{
}

QString DDataBase::getDBPath() const
{
    return m_DBPath;
}

void DDataBase::setDBPath(const QString &DBPath)
{
    m_DBPath = DBPath;
}

QString DDataBase::createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString DDataBase::getConnectionName() const
{
    return m_connectionName;
}

void DDataBase::setConnectionName(const QString &connectionName)
{
    m_connectionName = connectionName;
}

void DDataBase::initDBData()
{
    createDB();
}

void DDataBase::dbOpen()
{
    QStringList cntNames = QSqlDatabase::connectionNames();
    if (cntNames.contains(getConnectionName())) {
        m_database = QSqlDatabase::database(getConnectionName());
        //如果数据库不一致则设置新的数据库
        if (m_database.databaseName() != getDBPath()) {
            m_database.setDatabaseName(getDBPath());
        }
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE", getConnectionName());
        m_database.setDatabaseName(getDBPath());
        m_database.open();
    }
}

bool DDataBase::dbFileExists()
{
    QFile file;
    file.setFileName(getDBPath());
    return file.exists();
}

void DDataBase::removeDB()
{
    QFile::remove(getDBPath());
}

void SqliteMutex::lock()
{
    if (transactionLocked && transactionThreadId == qint64(QThread::currentThreadId())) {
        return;
    }
    m.lock();
}

void SqliteMutex::unlock()
{
    if (transactionLocked && transactionThreadId == qint64(QThread::currentThreadId())) {
        return;
    }
    m.unlock();
}

void SqliteMutex::transactionLock()
{
    m.lock();
    transactionLocked = true;
    transactionThreadId = qint64(QThread::currentThreadId());
}

void SqliteMutex::transactionUnlock()
{
    transactionLocked = false;
    transactionThreadId = 0;
    m.unlock();
}

SqliteQuery::SqliteQuery(QSqlDatabase db)
    : QSqlQuery(db)
    , _db(db)
{
}

SqliteQuery::SqliteQuery(const QString &connectionName)
    : SqliteQuery(QSqlDatabase::database(connectionName))
{
}

SqliteQuery::SqliteQuery(const QString &query, QSqlDatabase db)
    : QSqlQuery(query, db)
    , _db(db)
{
}

bool SqliteQuery::exec(QString sql)
{
    getDbMutexRef(_db.databaseName()).lock();
    bool f = QSqlQuery::exec(sql);
    getDbMutexRef(_db.databaseName()).unlock();
    return f;
}

bool SqliteQuery::exec()
{
    getDbMutexRef(_db.databaseName()).lock();
    bool f = QSqlQuery::exec();
    getDbMutexRef(_db.databaseName()).unlock();
    return f;
}

void SqliteQuery::transaction()
{
    getDbMutexRef(_db.databaseName()).transactionLock();
    _db.transaction();
}

void SqliteQuery::commit()
{
    _db.commit();
    getDbMutexRef(_db.databaseName()).transactionUnlock();
}

void SqliteQuery::rollback()
{
    _db.rollback();
    getDbMutexRef(_db.databaseName()).transactionUnlock();
}


void SqliteMutex::UnCopyMutex::lock()
{
    m.lock();
}

void SqliteMutex::UnCopyMutex::unlock()
{
    m.unlock();
}
