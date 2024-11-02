/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "app-info-table.h"
#include "app-info-table-private.h"
#include "app-info-dbus-argument.h"

#include <QDebug>
#include <QtGlobal>
#include <string>
#include <QStandardPaths>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QTime>
#include <QSqlQuery>
#include <QSqlError>
#include <mutex>
#include <QFile>
#include <application-property-helper.h>

using namespace LingmoUISearch;
static std::once_flag flag;
static AppInfoTable *global_intance = nullptr;

AppInfoTablePrivate::AppInfoTablePrivate(AppInfoTable *parent) : QObject(parent), q(parent), m_database(new QSqlDatabase())
{
    //dbus接收数据库信号
    qRegisterMetaType<LingmoUISearch::ApplicationProperty::Property>("ApplicationProperty::Property");
    qRegisterMetaType<LingmoUISearch::ApplicationInfoMap>("ApplicationInfoMap");
    qRegisterMetaType<LingmoUISearch::ApplicationPropertyMap>("ApplicationPropertyMap");
    qDBusRegisterMetaType<LingmoUISearch::ApplicationProperty::Property>();
    qDBusRegisterMetaType<ApplicationPropertyMap>();
    qDBusRegisterMetaType<ApplicationInfoMap>();

    m_signalTransInterface = new QDBusInterface("com.lingmo.search.appdb.service",
                                                "/org/lingmo/search/appDataBase/signalTransformer",
                                                "org.lingmo.search.signalTransformer");

    if (!m_signalTransInterface->isValid()) {
        qCritical() << "Create signalTransformer Interface Failed Because: " << QDBusConnection::sessionBus().lastError();
        return;
    } else {
        connect(m_signalTransInterface, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)), this, SLOT(sendAppDBItemsUpdate(ApplicationInfoMap)));
        connect(m_signalTransInterface, SIGNAL(appDBItemsUpdateAll(QStringList)), this, SLOT(sendAppDBItemsUpdateAll(QStringList)));
        connect(m_signalTransInterface, SIGNAL(appDBItemsAdd(QStringList)), this, SLOT(sendAppDBItemsAdd(QStringList)));
        connect(m_signalTransInterface, SIGNAL(appDBItemsDelete(QStringList)), this, SLOT(sendAppDBItemsDelete(QStringList)));
    }

    //使用dbus操作数据库获取数据
    m_appDBInterface = new QDBusInterface("com.lingmo.search.appdb.service",
                                          "/org/lingmo/search/appDataBase/dbManager",
                                          "org.lingmo.search.appDBManager");

    while(1) {
        srand(QTime(0,0,0).secsTo(QTime::currentTime()));
        m_ConnectionName = QString::fromStdString(std::to_string(rand()));//随机生产链接
        if (!QSqlDatabase::contains(m_ConnectionName))
            break;
    }
    qDebug() << "App info database currunt connection name:" << m_ConnectionName;
    if (!this->openDataBase()) {
        Q_EMIT q->DBOpenFailed();
        qWarning() << "Fail to open App DataBase, because:" << m_database->lastError();
    }
}

void AppInfoTablePrivate::setAppFavoritesState(const QString &desktopfp)
{
    m_appDBInterface->call("updateFavoritesState", desktopfp);
}

void AppInfoTablePrivate::setAppFavoritesState(const QString &desktopfp, uint num)
{
    m_appDBInterface->call("updateFavoritesState", desktopfp, num);
}

void AppInfoTablePrivate::setAppTopState(const QString &desktopfp)
{
    m_appDBInterface->call("updateTopState", desktopfp);
}

void AppInfoTablePrivate::setAppTopState(const QString &desktopfp, uint num)
{
    m_appDBInterface->call("updateTopState", desktopfp, num);
}

void AppInfoTablePrivate::setAppLaunchedState(const QString &desktopFilePath, bool launched)
{
    ApplicationInfoMap infoMap;
    infoMap[desktopFilePath].insert(ApplicationProperty::Launched, launched);
    QVariant var;
    var.setValue(infoMap);
    m_appDBInterface->call("setValue", var);
}

bool AppInfoTablePrivate::searchInstallApp(QString &keyWord, QStringList &installAppInfoRes)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd;
        if (keyWord.size() < 2) {
            cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON FROM APPINFO WHERE LOCAL_NAME OR NAME_EN OR NAME_ZH OR FIRST_LETTER_OF_PINYIN LIKE '%%0%' ORDER BY FAVORITES DESC")
                    .arg(keyWord);
        } else {
            cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON FROM APPINFO WHERE LOCAL_NAME OR NAME_EN OR NAME_ZH OR PINYIN_NAME OR FIRST_LETTER_OF_PINYIN LIKE '%%0%' ORDER BY FAVORITES DESC")
                    .arg(keyWord);
        }

        if (sql.exec(cmd)) {
            while (sql.next()) {
                installAppInfoRes << sql.value(0).toString() << sql.value(1).toString() << sql.value(2).toString();
            }
        } else {
            qWarning() << QString("cmd %0 failed!").arg(cmd) << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes)
{
    bool res(true);
    if (m_database->transaction() or !keyWord.isEmpty()) {
        QSqlQuery sql(*m_database);
        QString cmd;
        if (keyWord.at(0).size() < 2) {
            cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON,NAME_EN,NAME_ZH,FIRST_LETTER_OF_PINYIN FROM APPINFO"
                          " WHERE LOCAL_NAME LIKE '%%0%' OR NAME_EN LIKE '%%0%' OR NAME_ZH LIKE '%%0%' OR FIRST_LETTER_OF_PINYIN LIKE '%%0%'")
                    .arg(keyWord.at(0));
        } else {
            cmd = QString("SELECT DESKTOP_FILE_PATH,LOCAL_NAME,ICON,NAME_EN,NAME_ZH,FIRST_LETTER_OF_PINYIN FROM APPINFO"
                          " WHERE LOCAL_NAME LIKE '%%0%' OR NAME_EN LIKE '%%0%' OR NAME_ZH LIKE '%%0%' OR PINYIN_NAME LIKE '%%0%' OR FIRST_LETTER_OF_PINYIN LIKE '%%0%'")
                    .arg(keyWord.at(0));
        }
        for (int i = 0; ++i<keyWord.size();) {
            if (keyWord.at(i).size() < 2) {
                cmd += QString(" AND (LOCAL_NAME LIKE '%%1%' OR NAME_EN LIKE '%%1%' OR NAME_ZH LIKE '%%1%' OR FIRST_LETTER_OF_PINYIN LIKE '%%1%')")
                        .arg(keyWord.at(i));
            } else {
                cmd += QString(" AND (LOCAL_NAME LIKE '%%1%' OR NAME_EN LIKE '%%1%' OR NAME_ZH LIKE '%%1%' OR PINYIN_NAME LIKE '%%1%' OR FIRST_LETTER_OF_PINYIN LIKE '%%1%')")
                        .arg(keyWord.at(i));
            }
        }
        cmd += QString(" ORDER BY LENGTH(LOCAL_NAME)");
        if (sql.exec(cmd)) {
            while (sql.next()) {
                installAppInfoRes << sql.value(0).toString() << sql.value(1).toString() << sql.value(2).toString();
            }
        } else {
            qWarning() << QString("cmd %0 failed!").arg(cmd) << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!! keyword size:" << keyWord.size();
        res = false;
    }
    return res;
}

QString AppInfoTablePrivate::lastError() const
{
    return m_database->lastError().text();
}

bool AppInfoTablePrivate::tranPidToDesktopFp(uint pid, QString &desktopfp)
{
    QDBusReply<QString> reply = m_appDBInterface->call("tranPidToDesktopFp", pid);
    if (reply.isValid()) {
        desktopfp = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp)
{
    QDBusReply<QString> reply = m_appDBInterface->call("tranWinIdToDesktopFilePath", QVariant::fromValue(QDBusVariant(winId)));
    if (reply.isValid()) {
        desktopfp = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath)
{
    QDBusReply<QString> reply = m_appDBInterface->call("desktopFilePathFromName", desktopFileName);
    if (reply.isValid()) {
        desktopFilePath = reply.value();
        return true;
    } else {
        qDebug() << m_appDBInterface->lastError();
        return false;
    }
}

bool AppInfoTablePrivate::setAppLaunchTimes(const QString &desktopfp, size_t num)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LAUNCH_TIMES=%1, LAUNCHED=%2 WHERE DESKTOP_FILE_PATH='%3'")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(num)
                .arg(1)
                .arg(desktopfp);
        if (!sql.exec(cmd)) {
            qWarning() << "Set app favorites state failed!" << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::updateAppLaunchTimes(const QString &desktopfp)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("SELECT LAUNCH_TIMES FROM APPINFO WHERE DESKTOP_FILE_PATH='%1'").arg(desktopfp);
        if (sql.exec(cmd)) {
            if (sql.next()) {
                cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LAUNCH_TIMES=%1, LAUNCHED=%2 WHERE DESKTOP_FILE_PATH='%3'")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(sql.value(0).toInt() + 1)
                        .arg(1)
                        .arg(desktopfp);
                if (!sql.exec(cmd)) {
                    qWarning() << "Set app favorites state failed!" << m_database->lastError();
                    res = false;
                }
            } else {
                qWarning() << "Failed to exec next!" << cmd;
                res = false;
            }
        } else {
            qWarning() << "Failed to exec:" << cmd;
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

bool AppInfoTablePrivate::setAppLockState(const QString &desktopfp, size_t num)
{
    bool res(true);
    if (m_database->transaction()) {
        QSqlQuery sql(*m_database);
        QString cmd = QString("UPDATE appInfo SET MODIFYED_TIME='%0', LOCK=%1 WHERE DESKTOP_FILE_PATH='%2'")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(num)
                .arg(desktopfp);
        if (!sql.exec(cmd)) {
            qWarning() << "Set app favorites state failed!" << m_database->lastError();
            res = false;
        }
        if (!m_database->commit()) {
            qWarning() << "Failed to commit !" << cmd;
            m_database->rollback();
            res = false;
        }
    } else {
        qWarning() << "Failed to start transaction mode!!!";
        res = false;
    }
    return res;
}

AppInfoTablePrivate::~AppInfoTablePrivate()
{
    this->closeDataBase();
    if (m_signalTransInterface)
        delete m_signalTransInterface;
    m_signalTransInterface = nullptr;
    if (m_appDBInterface)
        delete m_appDBInterface;
    m_appDBInterface = nullptr;
}

bool AppInfoTablePrivate::initDateBaseConnection()
{
    m_database->setDatabaseName(APP_DATABASE_PATH + APP_DATABASE_NAME);
    if(!m_database->open()) {
        qWarning() << m_database->lastError();
        return false;
//        QApplication::quit();
    }
    return true;
}

bool AppInfoTablePrivate::openDataBase()
{
    *m_database = QSqlDatabase::addDatabase("QSQLITE", m_ConnectionName);
    m_database->setDatabaseName(APP_DATABASE_PATH + APP_DATABASE_NAME);

    if(!m_database->open()) {
        return false;
    }
    return true;
}

void AppInfoTablePrivate::closeDataBase()
{
    m_database->close();
    delete m_database;
    m_database = nullptr;
    QSqlDatabase::removeDatabase(m_ConnectionName);
}

void AppInfoTablePrivate::sendAppDBItemsUpdate(ApplicationInfoMap results)
{
    Q_EMIT q->appDBItems2BUpdate(results);
}

void AppInfoTablePrivate::sendAppDBItemsUpdateAll(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BUpdateAll(desktopFilePaths);
}

void AppInfoTablePrivate::sendAppDBItemsAdd(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BAdd(desktopFilePaths);
}

void AppInfoTablePrivate::sendAppDBItemsDelete(QStringList desktopFilePaths)
{
    Q_EMIT q->appDBItems2BDelete(desktopFilePaths);
}

AppInfoTable::AppInfoTable(QObject *parent) : QObject(parent), d(new AppInfoTablePrivate(this))
{
}

bool AppInfoTable::query(ApplicationPropertyMap &propertyMap, const QString &desktopFile, const ApplicationProperties& properties)
{
    QString field;
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    field.remove(field.length() - 1, 1);
    if(field.isEmpty()) {
        return true;
    }
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(QString("SELECT %0 FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFile").arg(field));
    query.bindValue(":desktopFile", desktopFile);
    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError();
        return false;
    }
    while (query.next()) {
        for(int i = 0; i< properties.size(); i++) {
            propertyMap.insert(properties.at(i), query.value(i));
        }
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties)
{
    QString field;
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField());
    } else if(!field.isEmpty()) {
        field.remove(field.length() - 1, 1);
    }

    QString sql = QString("SELECT %0 FROM APPINFO").arg(field);
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);

    if (!query.exec(sql)) {
        qWarning() << d->m_database->lastError() << sql;
        return false;
    }
    while (query.next()) {
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));

        }
        infoMap.insert(query.value(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField()).toString(), propertyMap);
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const ApplicationPropertyMap& restrictions)
{
    QString field;
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField());
    } else if(!field.isEmpty()) {
        field.remove(field.length() - 1, 1);
    }

    QString condition;
    for (const ApplicationProperty::Property prop: restrictions.keys()) {
        condition.append(ApplicationPropertyHelper(prop).dataBaseField() + "=? AND ");
    }
    condition = condition.left(condition.lastIndexOf(" AND "));

    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(QString("SELECT %0 FROM APPINFO WHERE %1")
                  .arg(field)
                  .arg(condition));
    int i = 0;
    for (const QVariant &conditionValue : restrictions) {
        query.bindValue(i, conditionValue);
        i++;
    }
    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError();
        return false;
    }
    while (query.next()) {
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));

        }
        infoMap.insert(query.value(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField()).toString(), propertyMap);
    }
    return true;
}

bool AppInfoTable::query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const QStringList &keywords, const ApplicationPropertyMap& restrictions)
{
    QString field;
    for(const ApplicationProperty::Property &pro : properties) {
        field.append(ApplicationPropertyHelper(pro).dataBaseField() + ",");
    }
    if(!properties.contains(ApplicationProperty::Property::DesktopFilePath)) {
        field.append(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField() + ",");
    }
    field.append(ApplicationPropertyHelper(ApplicationProperty::Property::LocalKeywords).dataBaseField());
    QString condition;
    for (const ApplicationProperty::Property prop: restrictions.keys()) {
        condition.append(ApplicationPropertyHelper(prop).dataBaseField() + "=? AND ");
    }
    QString keywordCondition;
    QString findWithLocalKeywords =  "((select count(*) from ("
                                     "WITH RECURSIVE split(str, rest) AS "
                                     "("
                                     "SELECT substr(LOCAL_KEYWORDS || ';', 1, instr(LOCAL_KEYWORDS || ';', ';') - 1),"
                                     "substr(LOCAL_KEYWORDS || ';', instr(LOCAL_KEYWORDS || ';', ';') + 1) "
                                     "UNION ALL "
                                     "SELECT substr(rest, 1, instr(rest, ';') - 1), substr(rest, instr(rest, ';') + 1) "
                                     "FROM split WHERE str <> ''"
                                     ")"
                                     "SELECT str FROM split WHERE lower(str)=?)) <> 0)";
    for(const QString& keyword : keywords) {
        if(keyword.size() < 2) {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        } else {
            keywordCondition.append(QString("(ifnull(LOCAL_NAME, '') like ? or ifnull(replace(LOCAL_NAME,' ',''), '') like ? or ifnull(NAME_EN, '') like ? or ifnull(replace(NAME_EN,' ',''), '') like ? or ifnull(NAME_ZH, '') like ? or ifnull(replace(NAME_ZH,' ',''), '') like ? or ifnull(FIRST_LETTER_OF_PINYIN, '') like ? or ifnull(replace(FIRST_LETTER_OF_PINYIN,' ',''), '') like ? or ifnull(PINYIN_NAME, '') like ? or ifnull(replace(PINYIN_NAME,' ',''), '') like ? or %0) AND").arg(findWithLocalKeywords));
        }
    }
    if(!keywordCondition.isEmpty()) {
        keywordCondition.remove(keywordCondition.length() - 3, 3);
    }

    QString sql = QString("SELECT %0 FROM APPINFO WHERE %1 %2 ORDER BY LENGTH(LOCAL_NAME)").arg(field, condition, keywordCondition);
    QSqlQuery query(*d->m_database);
    query.setForwardOnly(true);
    query.prepare(sql);

    int count = 0;
    for (const QVariant &conditionValue : restrictions) {
        query.bindValue(count, conditionValue);
        count++;
    }

    for(const QString &keyword : keywords) {
        int i = 11;
        if(keyword.size() < 2) {
            i = i - 2;
        }
        for (int bindCount = 0; bindCount < i; bindCount++) {
            query.bindValue(count, (bindCount - i + 1) ? "%" + keyword + "%" : keyword.toLower());
            count++;
        }
    }

    if (!query.exec()) {
        qWarning() << d->m_database->lastError() << query.lastError() << query.lastQuery() << query.boundValues();
        return false;
    }

    while (query.next()) {
        ApplicationPropertyMap propertyMap;
        for(const ApplicationProperty::Property &pro : properties) {
            propertyMap.insert(pro, query.value(ApplicationPropertyHelper(pro).dataBaseField()));

        }
        infoMap.insert(query.value(ApplicationPropertyHelper(ApplicationProperty::Property::DesktopFilePath).dataBaseField()).toString(), propertyMap);
    }
    return true;
}

void AppInfoTable::setAppFavoritesState(const QString &desktopfp)
{
    return d->setAppFavoritesState(desktopfp);
}

void AppInfoTable::setAppFavoritesState(const QString &desktopfp, size_t num)
{
    return d->setAppFavoritesState(desktopfp, num);
}

void AppInfoTable::setAppTopState(const QString &desktopfp, size_t num)
{
    return d->setAppTopState(desktopfp, num);
}

void AppInfoTable::setAppLaunchedState(const QString &desktopFilePath, bool launched)
{
    return d->setAppLaunchedState(desktopFilePath, launched);
}

void AppInfoTable::setAppTopState(const QString &desktopfp)
{
    return d->setAppTopState(desktopfp);
}

bool AppInfoTable::searchInstallApp(QString &keyWord, QStringList &installAppInfoRes)
{
    return d->searchInstallApp(keyWord, installAppInfoRes);
}

bool AppInfoTable::searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes)
{
    return d->searchInstallApp(keyWord, installAppInfoRes);
}

QString AppInfoTable::lastError() const
{
    return d->lastError();
}

bool AppInfoTable::tranPidToDesktopFp(uint pid, QString &desktopfp)
{
    return d->tranPidToDesktopFp(pid, desktopfp);
}

bool AppInfoTable::desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath)
{
    return d->desktopFilePathFromName(desktopFileName, desktopFilePath);
}

bool AppInfoTable::tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp)
{
    return d->tranWinIdToDesktopFilePath(winId, desktopfp);
}

//下面接口暂时没啥用，不外放。
bool AppInfoTable::setAppLaunchTimes(const QString &desktopfp, size_t num)
{
    return d->setAppLaunchTimes(desktopfp, num);
}

bool AppInfoTable::updateAppLaunchTimes(const QString &desktopfp)
{
    return d->updateAppLaunchTimes(desktopfp);
}

bool AppInfoTable::setAppLockState(const QString &desktopfp, size_t num)
{
    return d->setAppLockState(desktopfp, num);
}
