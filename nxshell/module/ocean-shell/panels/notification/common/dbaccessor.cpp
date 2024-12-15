// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbaccessor.h"
#include "notifyentity.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notification {
Q_LOGGING_CATEGORY(notifyDBLog, "ocean.shell.notification.db", QtMsgType::QtWarningMsg)

static const QString TableName = "notifications";
static const QString TableName_v2 = "notifications2";
static const QString ColumnId = "ID";
static const QString ColumnIcon = "Icon";
static const QString ColumnSummary = "Summary";
static const QString ColumnBody = "Body";
static const QString ColumnAppName = "AppName";
static const QString ColumnAppId = "AppId";
static const QString ColumnCTime = "CTime";
static const QString ColumnAction = "Action";
static const QString ColumnHint = "Hint";
static const QString ColumnProcessedType = "ProcessedType";
static const QString ColumnNotifyId = "NotifyId";
static const QString ColumnReplacesId = "ReplacesId";
static const QString ColumnTimeout = "Timeout";

static const QStringList EntityFields {
    ColumnId,
    ColumnIcon,
    ColumnSummary,
    ColumnBody,
    ColumnAppName,
    ColumnAppId,
    ColumnCTime,
    ColumnAction,
    ColumnHint,
    ColumnProcessedType,
    ColumnNotifyId,
    ColumnReplacesId
};

static QString notificationDBPath()
{
    QStringList dataPaths;
    if (qEnvironmentVariableIsSet("DS_NOTIFICATION_DB_PATH")) {
        auto path = qEnvironmentVariable("DS_NOTIFICATION_DB_PATH");
        dataPaths << path.split(";");
    }
    if (dataPaths.isEmpty()) {
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QDir dir(dataDir);
        const auto dbSubFix = QString("%1/%2/data.db").arg(qApp->organizationName()).arg(qApp->applicationName());
        const auto appPath = dir.absoluteFilePath(dbSubFix);
        dataPaths << appPath;
        QString path = dir.absoluteFilePath("lingmo/ocean-osd/data.db");
        dataPaths << path;
    }

    for (auto path : dataPaths) {
        QFile file(path);
        if (!file.exists()) {
            QDir().mkpath(QFileInfo(file.fileName()).path());
        }
        if (!file.open(QIODevice::ReadWrite)) {
            qDebug(notifyLog) << "Failed on open the data path:" << path << ", error:" << file.errorString();
            continue;
        }
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    qWarning(notifyLog) << "Doesn't exist the data path" << dataPaths;
    return {};
}

class Benchmark
{
public:
    explicit Benchmark(const QString &msg)
        : m_msg(msg)
    {
        m_timer.start();
    }
    ~Benchmark()
    {
        const auto time = m_timer.elapsed();
        if (time > 10)
            qWarning(notifyDBLog) << m_msg << " cost more time, elapsed:" << time;
    }
private:
    QElapsedTimer m_timer;
    QString m_msg;
};

#define BENCHMARK() \
    Benchmark __benchmark__(__FUNCTION__);

DBAccessor::DBAccessor(const QString &key)
    : m_key(key)
{
    auto dataPath = notificationDBPath();
    qInfo(notifyLog) << "DBAccessor's key:" << m_key;

    auto isDatabaseCorrupted = [this]() {
        QSqlQuery query(m_connection);
        if (!query.exec("SELECT 1")) {
            qDebug() << "Database maybe corrupted, cannot execute basic query!";
            m_connection.close();
            return true;
        }
        return false;
    };

    if (!dataPath.isEmpty() && open(dataPath)) {
        bool dbOpened = true;
        if (isDatabaseCorrupted()) {
            QString dirPath = dataPath.left(dataPath.lastIndexOf('/'));
            dataPath = dirPath + "/" + "data_new_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".db";
            qWarning() << "original database maybe corrupted, create new one:" << dataPath;
            dbOpened = open(dataPath);
        }

        if (dbOpened) {
            tryToCreateTable();
        }
    }
}

DBAccessor::~DBAccessor()
{
    if (m_connection.isOpen()) {
        m_connection.close();
    }
}

DBAccessor *DBAccessor::instance()
{
    static DBAccessor *instance = nullptr;
    if (!instance) {
        instance = new DBAccessor("Default");
    }
    return instance;
}

bool DBAccessor::open(const QString &dataPath)
{
    m_connection = QSqlDatabase::addDatabase("QSQLITE", "QSQLITE" + m_key);
    m_connection.setDatabaseName(dataPath);
    qDebug(notifyLog) << "Open database path" << dataPath;

    if (!m_connection.open()) {
        qWarning(notifyLog) << "Open database error" << m_connection.lastError().text();
        return false;
    }

    return true;
}

qint64 DBAccessor::addEntity(const NotifyEntity &entity)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString columns = QStringList{
            ColumnIcon,
            ColumnSummary,
            ColumnBody,
            ColumnAppName,
            ColumnAppId,
            ColumnCTime,
            ColumnAction,
            ColumnHint,
            ColumnReplacesId,
            ColumnNotifyId,
            ColumnProcessedType
    }.join(", ");

    QString sqlCmd = QString("INSERT INTO %1 (%2) VALUES (%3)")
            .arg(TableName_v2)
            .arg(columns)
            .arg(":icon, :summary, :body, :appName, :appId, :ctime, :action, :hint, :replacesId, :notifyId, :processedType");

    query.prepare(sqlCmd);
    query.bindValue(":icon", entity.appIcon());
    query.bindValue(":summary", entity.summary());
    query.bindValue(":body", entity.body());
    query.bindValue(":appName", entity.appName());
    query.bindValue(":appId", entity.appId());
    query.bindValue(":ctime", entity.cTime());
    query.bindValue(":action", entity.actionsString());
    query.bindValue(":hint", entity.hintsString());
    query.bindValue(":replacesId", entity.replacesId());
    query.bindValue(":notifyId", entity.bubbleId());
    query.bindValue(":processedType", entity.processedType());

    if (!query.exec()) {
        qWarning(notifyDBLog) << "insert value to database failed: " << query.lastError().text() << query.lastQuery() << entity.bubbleId() << entity.cTime();
        return -1;
    }

    // to get entity's id in database
    qint64 storageId = query.lastInsertId().toLongLong();

    qDebug(notifyDBLog) << "Insert entity bubbleId:" << entity.bubbleId() << ", id:" << storageId;

    return storageId;
}

qint64 DBAccessor::replaceEntity(qint64 id, const NotifyEntity &entity)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString columns = QStringList{QString("%1 = :icon").arg(ColumnIcon),
                                  QString("%1 = :summary").arg(ColumnSummary),
                                  QString("%1 = :body").arg(ColumnBody),
                                  QString("%1 = :appName").arg(ColumnAppName),
                                  QString("%1 = :appId").arg(ColumnAppId),
                                  QString("%1 = :ctime").arg(ColumnCTime),
                                  QString("%1 = :action").arg(ColumnAction),
                                  QString("%1 = :hint").arg(ColumnHint),
                                  QString("%1 = :replacesId").arg(ColumnReplacesId),
                                  QString("%1 = :notifyId").arg(ColumnNotifyId),
                                  QString("%1 = :processedType").arg(ColumnProcessedType)}
                          .join(", ");

    QString sqlCmd = QString("UPDATE %1 SET %2 WHERE ID = :id").arg(TableName_v2).arg(columns);

    query.prepare(sqlCmd);
    query.bindValue(":icon", entity.appIcon());
    query.bindValue(":summary", entity.summary());
    query.bindValue(":body", entity.body());
    query.bindValue(":appName", entity.appName());
    query.bindValue(":appId", entity.appId());
    query.bindValue(":ctime", entity.cTime());
    query.bindValue(":action", entity.actionsString());
    query.bindValue(":hint", entity.hintsString());
    query.bindValue(":replacesId", entity.replacesId());
    query.bindValue(":notifyId", entity.bubbleId());
    query.bindValue(":processedType", entity.processedType());
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Update value to database failed: " << query.lastError().text() << query.lastQuery() << entity.bubbleId() << entity.cTime();
        return -1;
    }

    qDebug(notifyDBLog) << "Update entity bubbleId:" << entity.bubbleId() << ", id:" << id << ", affected rows:" << query.numRowsAffected();

    return id;
}

void DBAccessor::updateEntityProcessedType(qint64 id, int processedType)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString cmd = QString("UPDATE %1 SET ProcessedType = :processed WHERE ID = :id").arg(TableName_v2);
    query.prepare(cmd);
    query.bindValue(":id", id);
    query.bindValue(":processed", processedType);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "update processed type execution error:" << query.lastError().text();
    }
}

NotifyEntity DBAccessor::fetchEntity(qint64 id)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    QString cmd = QString("SELECT %1 FROM notifications2 WHERE ID = :id").arg(EntityFields.join(","));
    query.prepare(cmd);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    if (query.next())
        return parseEntity(query);

    return {};
}

int DBAccessor::fetchEntityCount(const QString &appName, int processedType) const
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    if (appName == DataAccessor::AllApp()) {
        QString cmd = QString("SELECT COUNT(*) FROM notifications2 WHERE (ProcessedType = :processedType OR ProcessedType IS NULL)");
        query.prepare(cmd);
    } else {
        QString cmd = QString("SELECT COUNT(*) FROM notifications2 WHERE AppName = :appName AND (ProcessedType = :processedType OR ProcessedType IS NULL)");
        query.prepare(cmd);
        query.bindValue(":appName", appName);
    }

    query.bindValue(":processedType", processedType);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    if (query.next())
        return query.value(0).toInt();

    return 0;
}

NotifyEntity DBAccessor::fetchLastEntity(const QString &appName, int processedType)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    QString cmd =
        QString(
            "SELECT %1 FROM notifications2 WHERE AppName = :appName AND (ProcessedType = :processedType OR ProcessedType IS NULL) ORDER BY CTime DESC LIMIT 1")
            .arg(EntityFields.join(","));
    query.prepare(cmd);
    query.bindValue(":appName", appName);
    query.bindValue(":processedType", processedType);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    if (query.next()) {
        auto entity = parseEntity(query);

        qDebug(notifyDBLog) << "Fetched last entity" << entity.id();
        return entity;
    }
    return {};
}

QList<NotifyEntity> DBAccessor::fetchEntities(const QString &appName, int processedType, int maxCount)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    if (appName == DataAccessor::AllApp()) {
        if (maxCount >= 0) {
            QString cmd = QString("SELECT %1 FROM notifications2 WHERE (ProcessedType = :processedType OR ProcessedType IS NULL) ORDER BY CTime DESC LIMIT :limit").arg(EntityFields.join(","));
            query.prepare(cmd);
            query.bindValue(":limit", maxCount);
        } else {
            QString cmd = QString("SELECT %1 FROM notifications2 WHERE (ProcessedType = :processedType OR ProcessedType IS NULL) ORDER BY CTime DESC").arg(EntityFields.join(","));
            query.prepare(cmd);
        }
    } else {
        if (maxCount >= 0) {
            QString cmd = QString("SELECT %1 FROM notifications2 WHERE AppName = :appName AND (ProcessedType = :processedType OR ProcessedType IS NULL) ORDER BY CTime DESC LIMIT :limit").arg(EntityFields.join(","));
            query.prepare(cmd);
            query.bindValue(":appName", appName);
            query.bindValue(":limit", maxCount);
        } else {
            QString cmd = QString("SELECT %1 FROM notifications2 WHERE AppName = :appName AND (ProcessedType = :processedType OR ProcessedType IS NULL) ORDER BY CTime DESC").arg(EntityFields.join(","));
            query.prepare(cmd);
            query.bindValue(":appName", appName);
        }
    }

    query.bindValue(":processedType", processedType);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    QList<NotifyEntity> ret;
    while (query.next()) {
        auto entity = parseEntity(query);
        if (!entity.isValid())
            continue;
        ret.append(entity);
    }

    qDebug(notifyDBLog) << "Fetched entities size:" << ret.size();
    return ret;
}

NotifyEntity DBAccessor::fetchLastEntity(uint notifyId)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    QString cmd = QString("SELECT %1 FROM notifications2 WHERE notifyId = :notifyId ORDER BY CTime DESC LIMIT 1").arg(EntityFields.join(","));
    query.prepare(cmd);
    query.bindValue(":notifyId", notifyId);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    if (query.next()) {
        auto entity = parseEntity(query);

        qDebug(notifyDBLog) << "Fetched last entity " << entity.id() <<" by the notifyId" << notifyId;
        return entity;
    }
    return {};
}

QList<QString> DBAccessor::fetchApps(int maxCount) const
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);
    if (maxCount >= 0) {
        QString cmd("SELECT DISTINCT AppName FROM notifications2 ORDER BY CTime DESC LIMIT :limit");
        query.prepare(cmd);
        query.bindValue(":limit", maxCount);
    } else {
        QString cmd("SELECT DISTINCT AppName FROM notifications2 ORDER BY CTime DESC");
        query.prepare(cmd);
    }

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return {};
    }

    QList<QString> ret;
    while (query.next()) {
        const auto name = query.value(0).toString();
        ret.append(name);
    }

    qDebug(notifyDBLog) << "Fetched apps count" << ret.size();

    return ret;
}

void DBAccessor::removeEntity(qint64 id)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString cmd("DELETE FROM notifications2 WHERE ID = :id");
    query.prepare(cmd);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return;
    }

    qDebug(notifyDBLog) << "Delete notify count" << query.numRowsAffected();
}

void DBAccessor::removeEntityByApp(const QString &appName)
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString cmd("DELETE FROM notifications2 WHERE AppName = :appName");
    query.prepare(cmd);
    query.bindValue(":appName", appName);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return;
    }

    qDebug(notifyDBLog) << "Delete notify count" << query.numRowsAffected();
}

void DBAccessor::clear()
{
    BENCHMARK();

    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_connection);

    QString cmd("DELETE FROM notifications2");
    query.prepare(cmd);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "Query execution error:" << query.lastError().text();
        return;
    }

    qDebug(notifyDBLog) << "Delete notify count" << query.numRowsAffected();
}

void DBAccessor::tryToCreateTable()
{
    QSqlQuery query(m_connection);

    QStringList columns = {
            QString("%1 INTEGER PRIMARY KEY AUTOINCREMENT").arg(ColumnId),
            QString("%1 TEXT").arg(ColumnIcon),
            QString("%1 TEXT").arg(ColumnSummary),
            QString("%1 TEXT").arg(ColumnBody),
            QString("%1 TEXT").arg(ColumnAppName),
            QString("%1 TEXT").arg(ColumnAppId),
            QString("%1 TEXT").arg(ColumnCTime),
            QString("%1 TEXT").arg(ColumnAction),
            QString("%1 TEXT").arg(ColumnHint),
            QString("%1 TEXT").arg(ColumnReplacesId),
            QString("%1 TEXT").arg(ColumnNotifyId),
            QString("%1 TEXT").arg(ColumnTimeout),
            QString("%1 INTEGER").arg(ColumnProcessedType)
    };

    QString sql = QString("CREATE TABLE IF NOT EXISTS %1(%2)")
            .arg(TableName_v2)
            .arg(columns.join(", "));

    query.prepare(sql);

    if (!query.exec()) {
        qWarning(notifyDBLog) << "create table failed" << query.lastError().text();
    }

    // add new columns in history
    QMap<QString, QString> newColumns;
    newColumns[ColumnAction] = "TEXT";
    newColumns[ColumnHint] = "TEXT";
    newColumns[ColumnReplacesId] = "TEXT";
    newColumns[ColumnNotifyId] = "TEXT";
    newColumns[ColumnTimeout] = "TEXT";
    newColumns[ColumnProcessedType] = "INTEGER";
    newColumns[ColumnAppId] = "INTEGER";

    for (auto it = newColumns.begin(); it != newColumns.end(); ++it) {
        if (!isAttributeValid(TableName_v2, it.key())) {
            addAttributeToTable(TableName_v2, it.key(), it.value());

            // update processed type value for new columns
            if (it.key() == ColumnProcessedType) {
                updateProcessTypeValue();
            }
        }
    }
}

bool DBAccessor::isAttributeValid(const QString &tableName, const QString &attributeName) const
{
    QSqlQuery query(m_connection);

    QString sqlCmd = QString("SELECT * FROM SQLITE_MASTER WHERE TYPE='table' AND NAME='%1'").arg(tableName);
    if (query.exec(sqlCmd)) {
        if (query.next()) {
            sqlCmd = QString("SELECT * FROM %2").arg(tableName);
            if (query.exec(sqlCmd)) {
                QSqlRecord record = query.record();
                int index = record.indexOf(attributeName);
                return index != -1;
            }
            qDebug(notifyDBLog) << sqlCmd << ",lastError:" << query.lastError().text();
            return false;
        }
        // table not exist
        return false;
    }
    // sql error
    qDebug(notifyDBLog) << sqlCmd << ",lastError:" << query.lastError().text();
    return false;
}

bool DBAccessor::addAttributeToTable(const QString &tableName, const QString &attributeName, const QString &type) const
{
    QSqlQuery query(m_connection);

    QString sqlCmd = QString("alter table %1 add %2 %3").arg(tableName, attributeName, type);
    if (query.exec(sqlCmd)) {
        return true;
    }

    return false;
}

void DBAccessor::updateProcessTypeValue()
{
    QSqlQuery query(m_connection);

    QString updateCmd = QString("UPDATE %1 SET ProcessedType = %2 WHERE ProcessedType IS NULL")
            .arg(TableName_v2, NotifyEntity::Processed);

    if (!query.exec(updateCmd)) {
        qWarning(notifyDBLog) << "Failed to update ProcessedType NULL values:" << query.lastError();
    }
}

NotifyEntity DBAccessor::parseEntity(const QSqlQuery &query)
{
    const auto id = query.value(ColumnId).toLongLong();
    const auto icon = query.value(ColumnIcon).toString();
    const auto summary = query.value(ColumnSummary).toString();
    const auto body = query.value(ColumnBody).toString();
    const auto appName = query.value(ColumnAppName).toString();
    const auto appId = query.value(ColumnAppId).toString();
    const auto time = query.value(ColumnCTime).toString();
    const auto action = query.value(ColumnAction).toString();
    const auto hint = query.value(ColumnHint).toString();
    const auto processedType = query.value(ColumnProcessedType).toUInt();
    const auto notifyId = query.value(ColumnNotifyId).toUInt();
    const auto replacesId = query.value(ColumnReplacesId).toUInt();

    NotifyEntity entity(id, appName);
    entity.setAppId(appId.isEmpty() ? appName : appId);
    entity.setAppIcon(icon);
    entity.setSummary(summary);
    entity.setBody(body);
    entity.setCTime(time.toLongLong());
    entity.setHintString(hint);
    entity.setActionString(action);
    entity.setProcessedType(processedType);
    entity.setBubbleId(notifyId);
    entity.setReplacesId(replacesId);

    return entity;
}

#undef BENCHMARK

}
