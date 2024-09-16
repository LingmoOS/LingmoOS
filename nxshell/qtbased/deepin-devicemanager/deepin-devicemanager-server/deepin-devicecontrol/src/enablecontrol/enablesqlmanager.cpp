// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enablesqlmanager.h"
#include "DDLog.h"

#include <QtSql>
#include <QLoggingCategory>
#include <QDir>
#include <QSqlError>
#define DB_PATH "/usr/share/deepin-devicemanager/"
#define DB_FILE "enable.db"
#define DB_CONNECT_NAME "device-enable"
#define DB_TABLE_AUTHORIZED "authorized"
#define DB_TABLE_REMOVE "remove"
#define DB_TABLE_PRINTER "printer"
#define DB_TABLE_WAKEUP "wake"
#define DB_TABLE_NETWORK_WAKEUP "net_wake"
#define DB_TABLE_MONITOR_DEV "monitor_dev"

using namespace DDLog;

std::atomic<EnableSqlManager *> EnableSqlManager::s_Instance;
std::mutex EnableSqlManager::m_mutex;
void EnableSqlManager::insertDataToRemoveTable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, const QString &strDriver)
{
    // QString sql = QString("INSERT INTO %1 (class, name, path, unique_id, driver) VALUES (%2, %3, %4, %5, %6);")
    //               .arg(DB_TABLE_REMOVE).arg(":hclass").arg(":name").arg(":path").arg(":unique_id").arg(":strDriver");
    if(!m_sqlQuery.prepare("INSERT INTO remove (class, name, path, unique_id, driver) VALUES (:hclass, :name, :path, :unique_id, :strDriver);")) return;
    m_sqlQuery.bindValue(":hclass", QVariant(hclass));
    m_sqlQuery.bindValue(":name", QVariant(name));
    m_sqlQuery.bindValue(":path", QVariant(path));
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    m_sqlQuery.bindValue(":strDriver", QVariant(strDriver));

    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDateFromRemoveTable(const QString &path)
{
    QString sql = QString("DELETE FROM %1 WHERE path=%2;").arg(DB_TABLE_REMOVE).arg(":path");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":path", QVariant(path));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::insertDataToAuthorizedTable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool exist, const QString &strDriver)
{
    // 数据库已经存在该设备记录
    if (uniqueIDExistedEX(unique_id)) {
        return;
    }

    // 数据库没有该设备记录，则直接插入
    // QString sql = QString("INSERT INTO %1 (class, name, path, unique_id, exist, driver) VALUES (%2, %3, %4, %5, %6, %7);")
    //               .arg(DB_TABLE_AUTHORIZED).arg(":hclass").arg(":name").arg(":path").arg(":unique_id").arg(":exist").arg(":strDriver");
    if(!m_sqlQuery.prepare("INSERT INTO authorized (class, name, path, unique_id, exist, driver) VALUES (:hclass, :name, :path, :unique_id, :exist, :strDriver);")) return;
    m_sqlQuery.bindValue(":hclass", QVariant(hclass));
    m_sqlQuery.bindValue(":name", QVariant(name));
    m_sqlQuery.bindValue(":path", QVariant(path));
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    m_sqlQuery.bindValue(":exist", QVariant(exist));
    m_sqlQuery.bindValue(":strDriver", QVariant(strDriver));

    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDataFromAuthorizedTable(const QString &key)
{
    QString sql = QString("DELETE FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_AUTHORIZED).arg(":key");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":key", QVariant(key));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::updateDataToAuthorizedTable(const QString &unique_id, const QString &path)
{
    QString sql = QString("UPDATE %1 SET path=%2 WHERE unique_id=%3;").arg(DB_TABLE_AUTHORIZED).arg(":path").arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":path", QVariant(path));
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::clearEnableFromAuthorizedTable()
{
    QString sql = QString("DELETE FROM %1 WHERE enable='%2';").arg(DB_TABLE_AUTHORIZED).arg(true);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::insertDataToPrinterTable(const QString &hclass, const QString &name, const QString &path)
{
    QString sql = QString("INSERT INTO %1 (class, name, path) VALUES (%2, %3, %4);").arg(DB_TABLE_PRINTER).arg(":hclass").arg(":name").arg(":path");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":hclass", QVariant(hclass));
    m_sqlQuery.bindValue(":name", QVariant(name));
    m_sqlQuery.bindValue(":path", QVariant(path));

    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDataFromPrinterTable(const QString &name)
{
    QString sql = QString("DELETE FROM %1 WHERE name=%2;").arg(DB_TABLE_PRINTER).arg(":name");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":name", QVariant(name));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

bool EnableSqlManager::uniqueIDExisted(const QString &key)
{
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_AUTHORIZED).arg(":param");
    if(!m_sqlQuery.prepare(sql)) return false;
    m_sqlQuery.bindValue(":param", QVariant(key));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
            return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

bool EnableSqlManager::uniqueIDExistedEX(const QString &key)
{
    return uniqueIDExisted(key);
}

bool EnableSqlManager::isUniqueIdEnabled(const QString &key)
{
    QString sql = QString("SELECT enable FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(":key");
    if(!m_sqlQuery.prepare(sql)) return false;
    m_sqlQuery.bindValue(":key", QVariant(key));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
            return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

QString EnableSqlManager::removedInfo()
{
    QString info = "";
    QString sql = QString("SELECT class,name,path,unique_id,driver FROM %1;").arg(DB_TABLE_REMOVE);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        return info;
    }

    while (m_sqlQuery.next()) {
        info += "Hardware Class : " + m_sqlQuery.value(0).toString() + "\n";
        info += "name : " + m_sqlQuery.value(1).toString() + "\n";
        info += "path : " + m_sqlQuery.value(2).toString() + "\n";
        info += "unique_id : " + m_sqlQuery.value(3).toString() + "\n";
        info += "driver : " + m_sqlQuery.value(4).toString() + "\n\n";
    }
    return info;
}

QString EnableSqlManager::authorizedInfo()
{
    QString info = "";
    QString sql = QString("SELECT class,name,path,unique_id,driver FROM %1;").arg(DB_TABLE_AUTHORIZED);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        return info;
    }

    while (m_sqlQuery.next()) {
        info += "Hardware Class : " + m_sqlQuery.value(0).toString() + "\n";
        info += "name : " + m_sqlQuery.value(1).toString() + "\n";
        info += "path : " + m_sqlQuery.value(2).toString() + "\n";
        info += "unique_id : " + m_sqlQuery.value(3).toString() + "\n";
        info += "driver : " + m_sqlQuery.value(4).toString() + "\n\n";
    }
    return info;
}

QString EnableSqlManager::authorizedPath(const QString &unique_id)
{
    QString sql = QString("SELECT path FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_AUTHORIZED).arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return "";
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toString();
    }
    return "";
}


void EnableSqlManager::authorizedPathUniqueIDList(QList<QPair<QString, QString> > &lstPair)
{
    QString sql = QString("SELECT path,unique_id FROM %1;").arg(DB_TABLE_AUTHORIZED);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        return;
    }
    while (m_sqlQuery.next()) {
        QPair<QString, QString> pair;
        pair.first = m_sqlQuery.value(0).toString();
        pair.second = m_sqlQuery.value(1).toString();
        lstPair.append(pair);
    }
}

void EnableSqlManager::removePathList(QStringList &lsPath)
{
    QString sql = QString("SELECT path FROM %1;").arg(DB_TABLE_REMOVE);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        return;
    }
    while (m_sqlQuery.next()) {
        lsPath.append(m_sqlQuery.value(0).toString());
    }
}

void EnableSqlManager::removePathUniqueIDList(QList<QPair<QString, QString> > &lstPair)
{
    QString sql = QString("SELECT path,unique_id FROM %1;").arg(DB_TABLE_REMOVE);
    if (!m_sqlQuery.exec(sql)) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        return;
    }
    while (m_sqlQuery.next()) {
        QPair<QString, QString> pair;
        pair.first = m_sqlQuery.value(0).toString();
        pair.second = m_sqlQuery.value(1).toString();
        lstPair.append(pair);
    }
}

void EnableSqlManager::insertWakeupData(const QString &unique_id, const QString &path, bool wakeup)
{
    QString sql = QString("INSERT INTO %1 (unique_id, path, wakeup) VALUES (%2, %3, %4);").arg(DB_TABLE_WAKEUP).arg(":unique_id").arg(":path").arg(":wakeup");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    m_sqlQuery.bindValue(":path", QVariant(path));
    m_sqlQuery.bindValue(":wakeup", QVariant(wakeup));

    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

bool EnableSqlManager::isWakeupUniqueIdExisted(const QString &unique_id)
{
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_WAKEUP).arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return false;
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

void EnableSqlManager::updateWakeData(const QString &unique_id, const QString &path, bool wakeup)
{
    QString sql = QString("UPDATE %1 SET path=%2, wakeup=%3 WHERE unique_id=%4;").arg(DB_TABLE_WAKEUP).arg(":path").arg(":wakeup").arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return;
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    m_sqlQuery.bindValue(":path", QVariant(path));
    m_sqlQuery.bindValue(":wakeup", QVariant(wakeup));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << m_sqlQuery.lastError();
    }
}

QString EnableSqlManager::wakeupPath(const QString &unique_id)
{
    QString sql = QString("SELECT path FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_WAKEUP).arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return "";
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toString();
    }
    return "";
}

bool EnableSqlManager::isWakeup(const QString &unique_id)
{
    QString sql = QString("SELECT wakeup FROM %1 WHERE unique_id=%2;").arg(DB_TABLE_WAKEUP).arg(":unique_id");
    if(!m_sqlQuery.prepare(sql)) return false;
    m_sqlQuery.bindValue(":unique_id", QVariant(unique_id));
    if (m_sqlQuery.exec() && m_sqlQuery.next())
        return m_sqlQuery.value(0).toBool();
    return false;
}

void EnableSqlManager::insertNetworkWakeup(const QString &logical_name, bool wake)
{
    // 先判断是否已经存在
    QString sqlAdd;
    QString sqlExist = QString("SELECT wakeup FROM %1 WHERE logical_name=%2;").arg(DB_TABLE_NETWORK_WAKEUP).arg(":logical_name");
    if(!m_sqlQuery.prepare(sqlExist)) return;
    m_sqlQuery.bindValue(":logical_name", QVariant(logical_name));
    if (m_sqlQuery.exec() && m_sqlQuery.next()) {
        sqlAdd = QString("UPDATE %1 SET wakeup=%2 WHERE logical_name=%3;").arg(DB_TABLE_NETWORK_WAKEUP).arg(":wake").arg(":logical_name");
    } else {
        sqlAdd = QString("INSERT INTO %1 (logical_name, wakeup) VALUES (%2, %3);").arg(DB_TABLE_NETWORK_WAKEUP).arg(":logical_name").arg(":wake");
    }

    if(!m_sqlQuery.prepare(sqlAdd)) return;
    m_sqlQuery.bindValue(":wake", QVariant(wake));
    m_sqlQuery.bindValue(":logical_name", QVariant(logical_name));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

bool EnableSqlManager::isNetworkWakeup(const QString &logical_name)
{
    QString sql = QString("SELECT wakeup FROM %1 WHERE logical_name=%2;").arg(DB_TABLE_NETWORK_WAKEUP).arg(":logical_name");
    if(!m_sqlQuery.prepare(sql)) return false;
    m_sqlQuery.bindValue(":logical_name", QVariant(logical_name));
    if (m_sqlQuery.exec() && m_sqlQuery.next())
        return m_sqlQuery.value(0).toBool();
    return false;
}

bool EnableSqlManager::monitorWorkingFlag()
{
    QString sql = QString("SELECT working_flag FROM %1 WHERE monitor_name='usb';").arg(DB_TABLE_MONITOR_DEV);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next())
        return m_sqlQuery.value(0).toBool();
    return true;
}

void EnableSqlManager::setMonitorWorkingFlag(const bool &flag)
{
    // 先判断是否已经存在
    QString sqlAdd;
    QString sqlExist = QString("SELECT working_flag FROM %1 WHERE monitor_name='usb';").arg(DB_TABLE_MONITOR_DEV);
    if (m_sqlQuery.exec(sqlExist) && m_sqlQuery.next()) {
        sqlAdd = QString("UPDATE %1 SET working_flag=%2 WHERE monitor_name='usb';").arg(DB_TABLE_MONITOR_DEV).arg(":flag");
    } else {
        sqlAdd = QString("INSERT INTO %1 (monitor_name, working_flag) VALUES ('usb', %2);").arg(DB_TABLE_MONITOR_DEV).arg(":flag");
    }

    if(!m_sqlQuery.prepare(sqlAdd)) return;
    m_sqlQuery.bindValue(":flag", QVariant(flag));
    if (!m_sqlQuery.exec()) {
        qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

EnableSqlManager::EnableSqlManager(QObject *parent)
    : QObject(parent)
{
    initDB();
}

void EnableSqlManager::initDB()
{
    //初始化数据库
    QDir dbDir;
    if (!dbDir.exists(DB_PATH)) {
        dbDir.mkpath(DB_PATH);
    }
    m_db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECT_NAME);
    m_db.setDatabaseName(QString("%1%2").arg(DB_PATH).arg(DB_FILE));
    if (!m_db.open()) {
        qCDebug(appLog) << Q_FUNC_INFO << "local db open error!";
        return;
    }

    // 初始化查询接口
    m_sqlQuery = QSqlQuery(m_db);

    // 创建数据库表
    QStringList tableStrList = m_db.tables();

    if (!tableStrList.contains(DB_TABLE_AUTHORIZED)) {
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text, unique_id text, exist boolean, driver text);").arg(DB_TABLE_AUTHORIZED);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if (!tableStrList.contains(DB_TABLE_REMOVE)) {
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text, unique_id text, driver text);").arg(DB_TABLE_REMOVE);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if (!tableStrList.contains(DB_TABLE_PRINTER)) {
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text)").arg(DB_TABLE_PRINTER);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if (!tableStrList.contains(DB_TABLE_WAKEUP)) {
        QString sql = QString("CREATE TABLE %1 (unique_id text, path text, wakeup boolean)").arg(DB_TABLE_WAKEUP);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if (!tableStrList.contains(DB_TABLE_NETWORK_WAKEUP)) {
        QString sql = QString("CREATE TABLE %1 (logical_name text, wakeup boolean)").arg(DB_TABLE_NETWORK_WAKEUP);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if (!tableStrList.contains(DB_TABLE_MONITOR_DEV)) {
        QString sql = QString("CREATE TABLE %1 (monitor_name text, working_flag boolean)").arg(DB_TABLE_MONITOR_DEV);
        bool res = m_sqlQuery.exec(sql);
        if (!res) {
            qCInfo(appLog) << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
}
