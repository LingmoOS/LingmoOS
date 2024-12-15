// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "persistence.h"

#include <QStandardPaths>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMapIterator>
#include <QSqlTableModel>

#include "notificationentity.h"

static const QString TableName = "notifications";
static const QString TableName_v2 = "notifications2";
static const QString ColumnId = "ID";
static const QString ColumnIcon = "Icon";
static const QString ColumnSummary = "Summary";
static const QString ColumnBody = "Body";
static const QString ColumnAppName = "AppName";
static const QString ColumnCTime = "CTime";
static const QString ColumnAction = "Action";
static const QString ColumnHint = "Hint";
static const QString ColumnReplacesId = "ReplacesId";
static const QString ColumnTimeout = "Timeout";

Persistence::Persistence(QObject *parent)
    : AbstractPersistence(parent)
    , m_recordCount(0)
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    m_dbConnection = QSqlDatabase::addDatabase("QSQLITE", "QSQLITE");
    m_dbConnection.setDatabaseName(dataDir + "/" + "data.db");
    if (!m_dbConnection.open()) {
        qWarning() << "open database error" << m_dbConnection.lastError().text();
    } else {
#ifdef QT_DEBUG
        qDebug() << "database open";
#endif
    }

    m_query = QSqlQuery(m_dbConnection);
    m_query.setForwardOnly(true);

    attemptCreateTable();

    QSqlTableModel tableModel(nullptr, m_dbConnection);
    tableModel.setTable(TableName_v2);
    tableModel.select();

    while (tableModel.canFetchMore())
        tableModel.fetchMore();

    m_recordCount = static_cast<uint>(tableModel.rowCount());
}

void Persistence::addOne(EntityPtr entity)
{
    setRecordCount(recordCount() + doAddOne(entity));
}

uint Persistence::doAddOne(EntityPtr entity)
{
    // "cancel"表示正在发送蓝牙文件,不需要发送到通知中心
    if (entity->body().contains("%") && entity->actions().contains("cancel")) {
        return 0;
    }
    QString sqlCmd =  QString("INSERT INTO %1 (").arg(TableName_v2);
    sqlCmd += ColumnIcon + ",";
    sqlCmd += ColumnSummary + ",";
    sqlCmd += ColumnBody + ",";
    sqlCmd += ColumnAppName + ",";
    sqlCmd += ColumnCTime + ",";
    sqlCmd += ColumnAction + ",";
    sqlCmd += ColumnHint + ",";
    sqlCmd += ColumnReplacesId + ",";
    sqlCmd += ColumnTimeout + ")";
    sqlCmd += "VALUES (:icon, :summary, :body, :appname, :ctime, :action, :hint, :replacesid, :timeout)";

    m_query.prepare(sqlCmd);
    m_query.bindValue(":icon", entity->appIcon());
    m_query.bindValue(":summary", entity->summary());
    m_query.bindValue(":body", entity->body());
    m_query.bindValue(":appname", entity->appName());
    m_query.bindValue(":ctime", entity->ctime());

    //action
    QString action;
    foreach (QString text, entity->actions()) {
        action += text;
        action += ACTION_SEGMENT;
    }
    if (!action.isEmpty())
        action = action.mid(0, action.length() - 1);
    m_query.bindValue(":action", action);

    //hint
    m_query.bindValue(":hint", ConvertMapToString(entity->hints()));
    m_query.bindValue(":replacesid", entity->replacesId());
    m_query.bindValue(":timeout", entity->timeout());

    if (!m_query.exec()) {
        qWarning() << "insert value to database failed: " << m_query.lastError().text() << entity->id() << entity->ctime();
        return 0;
    } else {
#ifdef QT_DEBUG
        qDebug() << "insert value done, time is:" << entity->ctime();
#endif
    }

    // to get entity's id in database
    if (!m_query.exec(QString("SELECT last_insert_rowid() FROM %1;").arg(TableName_v2))) {
        qWarning() << "get entity's id failed: " << m_query.lastError().text() << entity->id() << entity->ctime();
    } else {
        m_query.next();
        entity->setStorageId(m_query.value(0).toString());
#ifdef QT_DEBUG
        qDebug() << "get entity's id done:" << entity->id() << entity->storageId();
#endif
    }
    return 1;
}

void Persistence::addAll(QList<EntityPtr> entities)
{
    uint count = 0;
    for (EntityPtr entity : entities) {
        count += doAddOne(entity);
    }
    setRecordCount(recordCount() + count);
}

void Persistence::removeOne(const QString &id)
{
    m_query.prepare(QString("DELETE FROM %1 WHERE ID = (:id)").arg(TableName_v2));
    m_query.bindValue(":id", id);

    if (!m_query.exec()) {
        qWarning() << "remove value:" << id << "from database failed: " << m_query.lastError().text();
        return;
    } else {
        uint count = qMax(uint(0), recordCount() - static_cast<uint>(m_query.numRowsAffected()));
#ifdef QT_DEBUG
        qDebug() << "remove value:" << id << "record count:" << count;
#endif
        setRecordCount(count);
    }
}

void Persistence::removeApp(const QString &app_name)
{
    m_query.prepare(QString("DELETE FROM %1 WHERE AppName = (:app)").arg(TableName_v2));
    m_query.bindValue(":app", app_name);

    if (!m_query.exec()) {
        qWarning() << "remove value:" << app_name << "from database failed: " << m_query.lastError().text();
        return;
    } else {
        uint count = qMax(uint(0), recordCount() - static_cast<uint>(m_query.numRowsAffected()));
#ifdef QT_DEBUG
        qDebug() << "remove value:" << app_name << "record count:" << count;
#endif
        setRecordCount(count);
    }
}

void Persistence::removeAll()
{
    m_query.prepare(QString("DELETE FROM %1").arg(TableName_v2));

    if (!m_query.exec()) {
        qWarning() << "remove all from database failed: " << m_query.lastError().text();
        return;
    } else {
#ifdef QT_DEBUG
        qDebug() << "remove all done";
#endif
        setRecordCount(0);
    }

    // Remove the unused space
    if (!m_query.exec("VACUUM")) {
        qWarning() << "remove the unused space failed: " << m_query.lastError().text();
        return;
    } else {
#ifdef QT_DEBUG
        qDebug() << "remove the unused space done";
#endif
    }
}

QString Persistence::getAll()
{
    QString sqlCmd = QString("SELECT ");
    sqlCmd += ColumnId + ",";
    sqlCmd += ColumnIcon + ",";
    sqlCmd += ColumnSummary + ",";
    sqlCmd += ColumnBody + ",";
    sqlCmd += ColumnAppName + ",";
    sqlCmd += ColumnCTime + ",";
    sqlCmd += ColumnAction + ",";
    sqlCmd += ColumnHint + ",";
    sqlCmd += ColumnReplacesId + ",";
    sqlCmd += ColumnTimeout + " FROM ";
    sqlCmd += TableName_v2;

    m_query.prepare(sqlCmd);

    if (!m_query.exec()) {
        qWarning() << "get all from database failed: " << m_query.lastError().text();
        return QString();
    } else {
#ifdef QT_DEBUG
        qDebug() << "get all done";
#endif
    }

    QJsonArray array1;
    while (m_query.next()) {
        QJsonObject obj {
            {"id", m_query.value(0).toString()},
            {"icon", m_query.value(1).toString()},
            {"summary", m_query.value(2).toString()},
            {"body", m_query.value(3).toString()},
            {"name", m_query.value(4).toString()},
            {"time", m_query.value(5).toString()},
            {"action", m_query.value(6).toString()},
            {"hint", m_query.value(7).toString()},
            {"replacesid", m_query.value(8).toString()},
            {"timeout", m_query.value(9).toString()},
        };
        array1.append(obj);
    }
    return QJsonDocument(array1).toJson();
}

QList<EntityPtr> Persistence::getAllNotify()
{
    QList<EntityPtr> db_notification;
    QString json = getAll();
    QJsonArray notify_array = QJsonDocument::fromJson(json.toLocal8Bit().data()).array();

    foreach (auto notify, notify_array) {
        db_notification.append(fromJsonValue(notify));
    }
    return db_notification;
}

QString Persistence::getById(const QString &id)
{
    QString sqlCmd = QString("SELECT ");
    sqlCmd += ColumnId + ",";
    sqlCmd += ColumnIcon + ",";
    sqlCmd += ColumnSummary + ",";
    sqlCmd += ColumnBody + ",";
    sqlCmd += ColumnAppName + ",";
    sqlCmd += ColumnCTime + ",";
    sqlCmd += ColumnAction + ",";
    sqlCmd += ColumnHint + ",";
    sqlCmd += ColumnReplacesId + ",";
    sqlCmd += ColumnTimeout + " FROM ";
    sqlCmd += TableName_v2;
    sqlCmd += " WHERE ID = (:id)";

    m_query.prepare(sqlCmd);
    m_query.bindValue(":id", id);

    if (!m_query.exec()) {
        qWarning() << "get data by id:" << id << "failed: " << m_query.lastError().text();
        return QString();
    } else {
#ifdef QT_DEBUG
        qDebug() << "get data by id:" << id << "done";
#endif
    }

    QJsonArray array;
    while (m_query.next()) {
        QJsonObject obj {
            {"id", m_query.value(0).toString()},
            {"icon", m_query.value(1).toString()},
            {"summary", m_query.value(2).toString()},
            {"body", m_query.value(3).toString()},
            {"name", m_query.value(4).toString()},
            {"time", m_query.value(5).toString()},
            {"action", m_query.value(6).toString()},
            {"hint", m_query.value(7).toString()},
            {"replacesid", m_query.value(8).toString()},
            {"timeout", m_query.value(9).toString()}
        };
        array.append(obj);
    }
    qDebug() << array;

    if (array.size() > 1) {
        qWarning() << "more than one data has been obtained by id:" << id;
    }

    return QJsonDocument(array).toJson();
}

EntityPtr Persistence::getNotifyById(const QString &id)
{
    QString json = getById(id);
    if (json.isEmpty()) {
        return EntityPtr();
    }
    QJsonArray notify_array = QJsonDocument::fromJson(json.toLocal8Bit().data()).array();
    return fromJsonValue(notify_array.first());
}

EntityPtr Persistence::fromJsonValue(const QJsonValue &jsonValue)
{
    QJsonObject obj = jsonValue.toObject();
    QStringList actions = obj.value("action").toString().split(ACTION_SEGMENT);
    auto notification = std::make_shared<NotificationEntity>(obj.value("name").toString(),
                                                             obj.value("id").toString(), obj.value("icon").toString(),
                                                             obj.value("summary").toString(),
                                                             obj.value("body").toString(),
                                                             actions, ConvertStringToMap(obj.value("hint").toString()),
                                                             obj.value("time").toString(),
                                                             obj.value("replacesid").toString(),
                                                             obj.value("timeout").toString(),
                                                             this);
    return notification;
}

QString Persistence::getFrom(int rowCount, const QString &offsetId)
{
    // gets the line number of the specified offset
    m_query.prepare(QString("SELECT count() FROM %1 WHERE ID <= (:offsetId)").arg(TableName_v2));
    m_query.bindValue(":offsetId", offsetId);

    if (!m_query.exec()) {
        qWarning() << "get line number failed: " << m_query.lastError().text();
        return QString();
    } else {
#ifdef QT_DEBUG
        qDebug() << "get line number done";
#endif
    }

    m_query.next();
    QString rowNum = m_query.value(0).toString();
    if (rowNum.isEmpty()) {
        qWarning() << "the line number is invalid: ";
        return QString();
    } else {
#ifdef QT_DEBUG
        qDebug() << "line number is valid";
#endif
    }

    // get data from rowNum+1
    QString sqlCmd = QString("SELECT ");
    sqlCmd += ColumnId + ",";
    sqlCmd += ColumnIcon + ",";
    sqlCmd += ColumnSummary + ",";
    sqlCmd += ColumnBody + ",";
    sqlCmd += ColumnAppName + ",";
    sqlCmd += ColumnCTime + ",";
    sqlCmd += ColumnAction + ",";
    sqlCmd += ColumnHint + ",";
    sqlCmd += ColumnReplacesId + ",";
    sqlCmd += ColumnTimeout + " FROM ";
    sqlCmd += TableName_v2;
    sqlCmd += QString(" LIMIT (:rowCount) OFFSET (:offset)");

    m_query.bindValue(":rowCount", rowCount);
    m_query.bindValue(":offset", rowNum);

    if (!m_query.exec()) {
        qWarning() << "get data from database failed: " << m_query.lastError().text();
        return QString();
    } else {
#ifdef QT_DEBUG
        qDebug() << "get data done";
#endif
    }

    QJsonArray array;
    while (m_query.next()) {
        QJsonObject obj {
            {"id", m_query.value(0).toString()},
            {"icon", m_query.value(1).toString()},
            {"summary", m_query.value(2).toString()},
            {"body", m_query.value(3).toString()},
            {"name", m_query.value(4).toString()},
            {"time", m_query.value(5).toString()},
            {"action", m_query.value(6).toString()},
            {"hint", m_query.value(7).toString()},
            {"replacesid", m_query.value(8).toString()},
            {"timeout", m_query.value(9).toString()}
        };
        array.append(obj);
    }
    qDebug() << array;

    return QJsonDocument(array).toJson();
}

uint Persistence::recordCount()
{
    return m_recordCount;
}

void Persistence::setRecordCount(uint count)
{
    if (m_recordCount == count)
        return;
    m_recordCount = count;
    Q_EMIT recordCountChanged(count);
}

void Persistence::attemptCreateTable()
{
    QString text = QString("CREATE TABLE IF NOT EXISTS %1("
                           "%2 INTEGER PRIMARY KEY   AUTOINCREMENT,").arg(TableName_v2, ColumnId);
    text += ColumnIcon + " TEXT,";
    text += ColumnSummary + " TEXT,";
    text += ColumnBody + " TEXT,";
    text += ColumnAppName + " TEXT,";
    text += ColumnCTime + " TEXT,";
    text += ColumnAction + " TEXT,";
    text += ColumnHint + " TEXT,";
    text += ColumnReplacesId + " TEXT,";
    text += ColumnTimeout + " TEXT)";

    m_query.prepare(text);

    if (!m_query.exec()) {
        qWarning() << "create table failed" << m_query.lastError().text();
    }

    if (!IsAttributeValid(TableName_v2, ColumnAction)) {
        AddAttributeToTable(TableName_v2, ColumnAction);
    }

    if (!IsAttributeValid(TableName_v2, ColumnHint)) {
        AddAttributeToTable(TableName_v2, ColumnHint);
    }

    if (!IsAttributeValid(TableName_v2, ColumnReplacesId)) {
        AddAttributeToTable(TableName_v2, ColumnReplacesId);
    }

    if (!IsAttributeValid(TableName_v2, ColumnTimeout)) {
        AddAttributeToTable(TableName_v2, ColumnTimeout);
    }
}

QString Persistence::ConvertMapToString(const QVariantMap &map)
{
    QString text;

    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        text += key;
        text += KEYVALUE_SEGMENT;
        QString value = it.value().toString();
        text += value;
        text += HINT_SEGMENT;
    }

    return text;
}

QVariantMap Persistence::ConvertStringToMap(const QString &text)
{
    QVariantMap map;

    QStringList keyValueList = text.split(HINT_SEGMENT);
    foreach (QString text, keyValueList) {
        QStringList list = text.split(KEYVALUE_SEGMENT);
        if (list.size() != 2)
            continue;
        QString key = list[0];
        QVariant value = QVariant::fromValue(list[1]);

        map.insert(key, value);
    }

    return map;
}

bool Persistence::IsAttributeValid(const QString &tableName, const QString &attributeName)
{
    QString sqlCmd = QString("SELECT * FROM SQLITE_MASTER WHERE TYPE='table' AND NAME='%1'").arg(tableName);
    if (m_query.exec(sqlCmd)) {
        if (m_query.next()) {
            sqlCmd = QString("SELECT * FROM %2").arg(tableName);
            if (m_query.exec(sqlCmd)) {
                QSqlRecord record = m_query.record();
                int index = record.indexOf(attributeName);
                if (index == -1) {
                    return false;
                } else {
                    return true;
                }
            } else {
                qDebug() << sqlCmd << ",lastError:" << m_query.lastError().text();
                return false;
            }
        } else { //table not exist
            return false;
        }
    } else {//sql error
        qDebug() << sqlCmd << ",lastError:" << m_query.lastError().text();
        return false;
    }
}

bool Persistence::AddAttributeToTable(const QString &tableName, const QString &attributeName)
{
    QString sqlCmd = QString("alter table %1 add %2 TEXT").arg(tableName, attributeName);
    if (m_query.exec(sqlCmd)) {
        return true;
    }
    return false;
}
