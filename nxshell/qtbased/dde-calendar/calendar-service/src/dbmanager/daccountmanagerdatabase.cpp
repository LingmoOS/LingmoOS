// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerdatabase.h"

#include "units.h"

#include "commondef.h"
#include <QSqlQuery>
#include <QtDebug>
#include <QSqlError>
#include <QFile>

DAccountManagerDataBase::DAccountManagerDataBase(QObject *parent)
    : DDataBase(parent)
{
    setConnectionName(NameAccountManager);
}

void DAccountManagerDataBase::initDBData()
{
    createDB();
    initAccountManagerDB();
}

DAccount::List DAccountManagerDataBase::getAccountList()
{
    DAccount::List accountList;
    QString strSql("SELECT accountID,accountName, displayName, accountState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, expandStatus, dtDelete, dtUpdate, isDeleted         \
                   FROM accountManager");
    SqliteQuery query(m_database);
    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(query.value("accountID").toString());
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setAccountState(static_cast<DAccount::AccountState>(query.value("accountState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setIsExpandDisplay(query.value("expandStatus").toBool());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            accountList.append(account);
        }
    } else {
        qCWarning(ServiceLogger) << "getAccountList error:" << query.lastError();
    }
    return accountList;
}

DAccount::Ptr DAccountManagerDataBase::getAccountByID(const QString &accountID)
{
    QString strSql("SELECT accountName, displayName, accountState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, dtDelete, dtUpdate, expandStatus, isDeleted         \
                   FROM accountManager WHERE accountID = ?");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(accountID);
        if (query.exec() && query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(accountID);
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setAccountState(static_cast<DAccount::AccountState>(query.value("accountState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setIsExpandDisplay(query.value("expandStatus").toBool());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            return account;
        } else {
            qCWarning(ServiceLogger) << "getAccountByID error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "getAccountByID error:" << query.lastError();
    }

    return nullptr;
}

QString DAccountManagerDataBase::addAccountInfo(const DAccount::Ptr &accountInfo)
{
    SqliteQuery query(m_database);
    //生成唯一标识
    if (accountInfo->accountID().isEmpty()) {
        accountInfo->setAccountID(DDataBase::createUuid());
    }
    QString strSql("INSERT INTO accountManager                                          \
                   (accountID, accountName, displayName, accountState, accountAvatar,  \
                    accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate,         \
                     expandStatus, isDeleted)                                                \
                   VALUES(?,?, ?, ?,?,?,?,?,?,?,?,?,?)");
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountID());
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(int(accountInfo->accountState()));
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(dtToString(accountInfo->dtCreate()));
        query.addBindValue(accountInfo->isExpandDisplay());
        query.addBindValue(0);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << "addAccountInfo error:" << query.lastError();
            accountInfo->setAccountID("");
        }
    } else {
        qCWarning(ServiceLogger) << "addAccountInfo error:" << query.lastError();
        accountInfo->setAccountID("");
    }

    return accountInfo->accountID();
}

bool DAccountManagerDataBase::updateAccountInfo(const DAccount::Ptr &accountInfo)
{
    QString strSql("UPDATE accountManager                                                           \
                   SET accountName=?, displayName=?, accountState= ?,                   \
                   accountAvatar=?, accountDescription=?, accountType=?, dbName=?,               \
                   dBusPath = ? ,dBusInterface = ?, expandStatus = ? WHERE accountID=?");
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(int(accountInfo->accountState()));
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(accountInfo->isExpandDisplay());
        query.addBindValue(accountInfo->accountID());
        res = query.exec();
    }
    if (!res) {
        qCWarning(ServiceLogger) << "updateAccountInfo error:" << query.lastError();
    }

    return res;
}

bool DAccountManagerDataBase::deleteAccountInfo(const QString &accountID)
{
    QString strSql("DELETE FROM accountManager      \
                   WHERE accountID=?");
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountID);
        res = query.exec();
    }

    if (!res) {
        qCWarning(ServiceLogger) << "deleteAccountInfo error:" << query.lastError();
    }
    return res;
}
// 保存通用设置
DCalendarGeneralSettings::Ptr DAccountManagerDataBase::getCalendarGeneralSettings()
{
    DCalendarGeneralSettings::Ptr cgSet(new DCalendarGeneralSettings);
    SqliteQuery query(m_database);
    query.exec("select vch_value from calendargeneralsettings where vch_key = 'firstDayOfWeek' ");
    if (query.next())
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(query.value(0).toInt()));

    query.exec("select vch_value from calendargeneralsettings where vch_key = 'timeShowType' ");
    if (query.next())
        cgSet->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(query.value(0).toInt()));

    return cgSet;
}
// 获取通用设置
void DAccountManagerDataBase::setCalendarGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet)
{
    SqliteQuery query(m_database);
    query.prepare("update calendargeneralsettings set vch_value = ? where vch_key = 'firstDayOfWeek' ");
    query.addBindValue(cgSet->firstDayOfWeek());
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "UPDATE calendargeneralsettings error," << query.lastError();
    }

    query.prepare("update calendargeneralsettings set vch_value = ? where vch_key = 'timeShowType' ");
    query.addBindValue(cgSet->timeShowType());
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "UPDATE calendargeneralsettings error," << query.lastError();
    }
}

void DAccountManagerDataBase::createDB()
{
    dbOpen();
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(getDBPath());
    //如果不存在该文件则创建
    if (!file.exists()) {
        m_database.open();
        m_database.close();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qCWarning(ServiceLogger) << "permissions cannot be modified，error:" << file.errorString();
    }

    if (m_database.open()) {
        SqliteQuery query(m_database);
        bool res = true;
        //创建帐户管理表
        res = query.exec(sql_create_accountManager);
        if (!res) {
            qCWarning(ServiceLogger) << "accountManager create failed.error:" << query.lastError();
        }


        //日历通用设置
        res = query.exec(sql_create_calendargeneralsettings);
        if (!res) {
            qCWarning(ServiceLogger) << "uploadTask create failed.error:" << query.lastError();
        }

        //创建calendargeneralsettings的触发器，数据有变动时，更新dt_update
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_insert'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_insert AFTER INSERT "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_update'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_update AFTER UPDATE "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_delete'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_delete AFTER DELETE "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        if (query.isActive()) {
            query.finish();
        }
    }
}

void DAccountManagerDataBase::initAccountManagerDB()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    m_database = QSqlDatabase::database(NameAccountManager);
    m_database.setDatabaseName(getDBPath());
    //帐户管理表
    {
        SqliteQuery query(m_database);
        QString strsql("INSERT INTO accountManager                              \
                       (accountID, accountName, displayName,                    \
                       accountState, accountAvatar, accountDescription,            \
                       accountType, dbName,dBusPath,dBusInterface,dtCreate, dtUpdate,      \
                       expandStatus, isDeleted)                                \
                   VALUES(:accountID,:accountName,:displayName,:accountState,:accountAvatar,   \
                   :accountDescription,:accountType,:dbName,:dBusPath,:dBusInterface,:dtCreate,:dtUpdate,            \
                   :expandStatus, :isDeleted);");
        if (query.prepare(strsql)) {
            query.bindValue(":accountID", DDataBase::createUuid());
            query.bindValue(":accountName", "localAccount");
            query.bindValue(":displayName", "localAccount");
            query.bindValue(":accountState", 0);
            query.bindValue(":accountAvatar", "");
            query.bindValue(":accountDescription", "");
            query.bindValue(":accountType", 0);
            query.bindValue(":dbName", m_loaclDB);
            query.bindValue(":dBusPath", serviceBasePath + "/account_local");
            query.bindValue(":dBusInterface", accountServiceInterface);
            query.bindValue(":dtCreate", dtToString(currentDateTime));
            query.bindValue(":expandStatus", 1);
            query.bindValue(":isDeleted", 0);

            if (query.exec()) {
                if (query.isActive()) {
                    query.finish();
                }
            } else {
                qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
            }
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    }

    //通用设置
    {
        SqliteQuery query(m_database);
        if (query.exec("insert into calendargeneralsettings values"
                       "('firstDayOfWeek',  '7'),"
                       "('timeShowType',    '0')")) {
            m_database.commit();
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    }
}

void DAccountManagerDataBase::setLoaclDB(const QString &loaclDB)
{
    m_loaclDB = loaclDB;
}
