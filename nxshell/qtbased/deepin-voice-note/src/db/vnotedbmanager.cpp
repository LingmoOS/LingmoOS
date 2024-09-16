// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "db/vnotedbmanager.h"
#include "db/dbvisitor.h"
#include "globaldef.h"

#include <DLog>

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QSqlError>

#define CRITICAL_SECTION_BEGIN() \
    do { \
        m_dbLock.lock(); \
        /*m_vnoteDB.transaction();*/ \
    } while (0)

#define CRITICAL_SECTION_END() \
    do { \
        /*m_vnoteDB.commit();*/ \
        m_dbLock.unlock(); \
    } while (0)

#define CHECK_DB_INIT() \
    do { \
        if (!m_isDbInitOK) { \
            return false; \
        } \
    } while (0)

VNoteDbManager *VNoteDbManager::_instance = nullptr;

/**
 * @brief VNoteDbManager::VNoteDbManager
 * @param fOldDb true 老数据库
 * @param parent
 */
VNoteDbManager::VNoteDbManager(bool fOldDb, QObject *parent)
    : QObject(parent)
{
    initVNoteDb(fOldDb);
}

/**
 * @brief VNoteDbManager::~VNoteDbManager
 */
VNoteDbManager::~VNoteDbManager()
{
    m_vnoteDB.close();
}

/**
 * @brief VNoteDbManager::instance
 * @return 单例对象
 */
VNoteDbManager *VNoteDbManager::instance()
{
    if (nullptr == _instance) {
        _instance = new VNoteDbManager();
    }

    return _instance;
}

/**
 * @brief VNoteDbManager::getVNoteDb
 * @return 数据库对象
 */
QSqlDatabase &VNoteDbManager::getVNoteDb()
{
    return m_vnoteDB;
}

/**
 * @brief VNoteDbManager::insertData
 * @param visitor
 * @return true 成功
 */
bool VNoteDbManager::insertData(DbVisitor *visitor /*in/out*/)
{
    CHECK_DB_INIT();

    bool insertOK = true;

    if (nullptr == visitor) {
        qCritical() << "insertData invalid parameter: visitor is null";
        return false;
    }

    if (Q_UNLIKELY(!visitor->prepareSqls())) {
        qCritical() << "prepare sqls failed!";
        return false;
    }

    CRITICAL_SECTION_BEGIN();

    for (auto it : visitor->dbvSqls()) {
        if (!it.trimmed().isEmpty()) {
            if (!visitor->sqlQuery()->exec(it)) {
                qCritical() << "insert data failed:" << it
                            << " reason:" << visitor->sqlQuery()->lastError().text();
                insertOK = false;
            }
        }
    }

    CRITICAL_SECTION_END();

    if (!visitor->visitorData()) {
        insertOK = false;
        qCritical() << "Query new data failed: visitorData failed.";
    }

    return insertOK;
}

/**
 * @brief VNoteDbManager::updateData
 * @param visitor
 * @return true 成功
 */
bool VNoteDbManager::updateData(DbVisitor *visitor /*in/out*/)
{
    CHECK_DB_INIT();

    bool updateOK = true;

    if (nullptr == visitor) {
        qCritical() << "updateData invalid parameter: visitor is null";
        return false;
    }

    if (Q_UNLIKELY(!visitor->prepareSqls())) {
        qCritical() << "prepare sqls failed!";
        return false;
    }

    CRITICAL_SECTION_BEGIN();

    for (auto it : visitor->dbvSqls()) {
        if (!it.trimmed().isEmpty()) {
            if (!visitor->sqlQuery()->exec(it)) {
                qCritical() << "Update data failed:" << it
                            << " reason:" << visitor->sqlQuery()->lastError().text();
                updateOK = false;
            }
        }
    }

    CRITICAL_SECTION_END();

    return updateOK;
}

/**
 * @brief VNoteDbManager::queryData
 * @param visitor
 * @return true 成功
 */
bool VNoteDbManager::queryData(DbVisitor *visitor /*in/out*/)
{
    CHECK_DB_INIT();

    bool queryOK = true;

    if (nullptr == visitor) {
        qCritical() << "queryData invalid parameter: visitor is null";
        return false;
    }

    if (Q_UNLIKELY(!visitor->prepareSqls())) {
        qCritical() << "prepare sqls failed!";
        return false;
    }

    CRITICAL_SECTION_BEGIN();

    for (auto it : visitor->dbvSqls()) {
        if (!it.trimmed().isEmpty()) {
            if (!visitor->sqlQuery()->exec(it)) {
                qCritical() << "Query data failed:" << it
                            << " reason:" << visitor->sqlQuery()->lastError().text();
                queryOK = false;
            }
        }
    }

    CRITICAL_SECTION_END();

    if (!visitor->visitorData()) {
        qCritical() << "Query data failed: visitorData failed.";
        queryOK = false;
    }

    return queryOK;
}

/**
 * @brief VNoteDbManager::deleteData
 * @param visitor
 * @return true 成功
 */
bool VNoteDbManager::deleteData(DbVisitor *visitor /*in/out*/)
{
    CHECK_DB_INIT();

    bool deleteOK = true;

    if (nullptr == visitor) {
        qCritical() << "deleteData invalid parameter: visitor is null";
        return false;
    }

    if (Q_UNLIKELY(!visitor->prepareSqls())) {
        qCritical() << "prepare sqls failed!";
        return false;
    }

    CRITICAL_SECTION_BEGIN();

    for (auto it : visitor->dbvSqls()) {
        if (!it.trimmed().isEmpty()) {
            if (!visitor->sqlQuery()->exec(it)) {
                qCritical() << "Delete data failed:" << it
                            << " reason:" << visitor->sqlQuery()->lastError().text();
                deleteOK = false;
            }
        }
    }

    CRITICAL_SECTION_END();

    return deleteOK;
}

/**
 * @brief VNoteDbManager::hasOldDataBase
 * @return true 存在老数据库
 */
bool VNoteDbManager::hasOldDataBase()
{
    QString vnoteDatabasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFileInfo dbDir(vnoteDatabasePath);
    dbDir.setFile(vnoteDatabasePath + QDir::separator());

    QString vnoteDatebaseName = DEEPIN_VOICE_NOTE + QString(".db");

    QString vnoteDbFullPath = dbDir.filePath() + vnoteDatebaseName;

    QFileInfo dbFile(vnoteDbFullPath);

    return dbFile.exists();
}

/**
 * @brief VNoteDbManager::initVNoteDb
 * @param fOldDB
 * @return 0 成功，-1失败
 */
int VNoteDbManager::initVNoteDb(bool fOldDB)
{
    QString vnoteDatabasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFileInfo dbDir(vnoteDatabasePath);
    dbDir.setFile(vnoteDatabasePath + QDir::separator());

    if (!dbDir.exists()) {
        QDir(dbDir.filePath()).mkdir(dbDir.filePath());
        qInfo() << "Create vnote db directory:" << vnoteDatabasePath;
    }

    QString vnoteDatebaseName = DEEPIN_VOICE_NOTE
                                + QString(VNoteDbManager::DBVERSION) + QString(".db");

    if (fOldDB) {
        vnoteDatebaseName = DEEPIN_VOICE_NOTE + QString(".db");
    }

    QString vnoteDbFullPath = dbDir.filePath() + vnoteDatebaseName;

    if (QSqlDatabase::contains(vnoteDatebaseName)) {
        m_vnoteDB = QSqlDatabase::database(vnoteDatebaseName);
    } else {
        m_vnoteDB = QSqlDatabase::addDatabase("QSQLITE", vnoteDatebaseName);
        m_vnoteDB.setDatabaseName(vnoteDbFullPath);
    }

    if (!m_vnoteDB.open()) {
        qCritical() << "Open database failed:" << m_vnoteDB.lastError().text();

        return -1;
    }

    qInfo() << "Database opened:" << vnoteDbFullPath;

    if (!fOldDB) {
        createTablesIfNeed();
    }

    m_isDbInitOK = true;

    return 0;
}

/**
 * @brief VNoteDbManager::createTablesIfNeed
 */
void VNoteDbManager::createTablesIfNeed()
{
    QStringList createTableSqls = QString(CREATETABLE_FMT).split(";");

    QScopedPointer<QSqlQuery> sqlQuery(new QSqlQuery(m_vnoteDB));

    for (auto it : createTableSqls) {
        if (!it.trimmed().isEmpty()) {
            if (!sqlQuery->exec(it)) {
                qCritical() << it << "init tables failed error: " << sqlQuery->lastError().text();
            }
        }
    }
}
