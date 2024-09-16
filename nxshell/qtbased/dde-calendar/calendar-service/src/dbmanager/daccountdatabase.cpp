// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountdatabase.h"

#include "commondef.h"
#include "units.h"
#include "pinyin/pinyinsearch.h"

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QSqlError>
#include <QFile>

DAccountDataBase::DAccountDataBase(const DAccount::Ptr &account, QObject *parent)
    : DDataBase(parent)
    , m_account(account)
{
    setConnectionName(m_account->accountName());
}

QString DAccountDataBase::createSchedule(const DSchedule::Ptr &schedule)
{
    if (!schedule.isNull()) {
        SqliteQuery query(m_database);
        schedule->setUid(DDataBase::createUuid());

        QString strSql("INSERT INTO schedules                                                   \
                       (scheduleID, scheduleTypeID, summary, description, allDay, dtStart   \
                       , dtEnd, isAlarm, titlePinyin,isLunar, ics, fileName, dtCreate, isDeleted)   \
                       VALUES(?, ?, ?, ?, ?, ?, ?, ?,?, ?, ?, ?, ?, ?);");
        if (query.prepare(strSql)) {
            query.addBindValue(schedule->schedulingID());
            query.addBindValue(schedule->scheduleTypeID());
            query.addBindValue(schedule->summary());
            query.addBindValue(schedule->description());
            query.addBindValue(schedule->allDay());
            query.addBindValue(dtToString(schedule->dtStart()));
            query.addBindValue(dtToString(schedule->dtEnd()));
            query.addBindValue(schedule->hasEnabledAlarms());
            query.addBindValue(pinyinsearch::getPinPinSearch()->CreatePinyin(schedule->summary()));
            query.addBindValue(schedule->lunnar());
            query.addBindValue(DSchedule::toIcsString(schedule));
            query.addBindValue(schedule->fileName());
            query.addBindValue(dtToString(schedule->created()));
            query.addBindValue(0);
            if (!query.exec()) {
                schedule->setUid("");
                qCWarning(ServiceLogger) << "createSchedule error:" << query.lastError();
            }
            if (query.isActive()) {
                query.finish();
            }
        } else {
            schedule->setUid("");
            qCWarning(ServiceLogger) << "createSchedule error:" << query.lastError();
        }

    } else {
        schedule->setUid("");
        qCWarning(ServiceLogger) << "schedule is null";
    }

    return schedule->uid();
}

bool DAccountDataBase::updateSchedule(const DSchedule::Ptr &schedule)
{
    bool resbool = false;
    if (!schedule.isNull()) {
        SqliteQuery query(m_database);
        QString strSql("UPDATE schedules                                                  \
                       SET scheduleTypeID=?, summary=?, description=?, allDay=?           \
                , dtStart=?, dtEnd=?, isAlarm=?,titlePinyin=?, isLunar=?, ics=?, fileName=?             \
                , dtUpdate=? WHERE scheduleID= ?;");
        if (query.prepare(strSql)) {
            query.addBindValue(schedule->scheduleTypeID());
            query.addBindValue(schedule->summary());
            query.addBindValue(schedule->description());
            query.addBindValue(schedule->allDay());
            query.addBindValue(dtToString(schedule->dtStart()));
            query.addBindValue(dtToString(schedule->dtEnd()));
            query.addBindValue(schedule->hasEnabledAlarms());
            query.addBindValue(pinyinsearch::getPinPinSearch()->CreatePinyin(schedule->summary()));
            query.addBindValue(schedule->lunnar());
            query.addBindValue(DSchedule::toIcsString(schedule));
            query.addBindValue(schedule->fileName());
            query.addBindValue(dtToString(schedule->lastModified()));
            query.addBindValue(schedule->schedulingID());
            if (query.exec()) {
                resbool = true;
            } else {
                qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
            }
            if (query.isActive()) {
                query.finish();
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    }

    return resbool;
}

DSchedule::Ptr DAccountDataBase::getScheduleByScheduleID(const QString &scheduleID)
{
    QString strSql("SELECT  scheduleID, scheduleTypeID, summary, description, allDay, dtStart, dtEnd,   \
                   isAlarm,titlePinyin,isLunar, ics, fileName, dtCreate, dtUpdate, dtDelete, isDeleted  \
                   FROM schedules WHERE  scheduleID  = ? ;");
    SqliteQuery query(m_database);
    DSchedule::Ptr schedule;
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleID);
        schedule = DSchedule::Ptr(new DSchedule);
        if (query.exec()) {
            if (query.next()) {
                QString &&icsStr = query.value("ics").toString();
                DSchedule::fromIcsString(schedule, icsStr);
                schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
            return schedule;
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        return schedule;
    }

    return schedule;
}

QStringList DAccountDataBase::getScheduleIDListByTypeID(const QString &typeID)
{
    QStringList scheduleIDList;
    QString strSql("SELECT scheduleID FROM schedules WHERE  scheduleTypeID  = ?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        if (query.exec()) {
            while (query.next()) {
                scheduleIDList.append(query.value("scheduleID").toString());
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    return scheduleIDList;
}

bool DAccountDataBase::deleteScheduleByScheduleID(const QString &scheduleID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = '%1' , isDeleted = 1  WHERE scheduleID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    SqliteQuery query(m_database);
    bool resBool = false;
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleID);
        resBool = query.exec();
    }
    if (!resBool) {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }

    return resBool;
}

bool DAccountDataBase::deleteSchedulesByScheduleTypeID(const QString &typeID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleTypeID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = '%1' , isDeleted = 1  WHERE scheduleTypeID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    SqliteQuery query(m_database);
    bool resBool = false;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        resBool = query.exec();
    }

    if (!resBool) {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return resBool;
}

DSchedule::List DAccountDataBase::querySchedulesByKey(const QString &key)
{
    DSchedule::List scheduleList;
    QString strSql("SELECT s.scheduleID, s.scheduleTypeID, s.summary, s.description, s.allDay, s.dtStart, s.dtEnd,   \
             s.isAlarm,s.titlePinyin,s.isLunar, s.ics, s.fileName, s.dtCreate, s.dtUpdate, s.dtDelete, s.isDeleted      \
               FROM  schedules s  inner join scheduleType st WHERE s.isDeleted = 0 and st.showState =1 and s.scheduleTypeID  = st.typeID ");
    //如果关键字不为空，添加查询条件
    pinyinsearch *psearch = pinyinsearch::getPinPinSearch();
    QMap<QString, QString> sqlBindValue;
    QString strKey = key.trimmed();
    if (psearch->CanQueryByPinyin(key)) {
        //可以按照拼音查询
        QString pinyin = psearch->CreatePinyinQuery(strKey.toLower());
        strSql += QString("and ( instr(UPPER(s.summary), UPPER(:key)) OR s.titlePinyin LIKE :pinyin )");
        sqlBindValue[":key"] = key;
        sqlBindValue[":pinyin"] = pinyin;
    } else if (!key.isEmpty()) {
        //按照key查询
        strSql += QString(" and instr(UPPER(s.summary), UPPER(:key))");
        sqlBindValue[":key"] = key;
    }

    strSql.append(QString(" order by :strsort "));
    sqlBindValue[":strsort"] = "s.dtStart asc";

    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        for (auto iter = sqlBindValue.constBegin(); iter != sqlBindValue.constEnd(); iter++) {
            query.bindValue(iter.key(), iter.value());
        }

        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule;
                QString &&icsStr = query.value("ics").toString();
                DSchedule::fromIcsString(schedule, icsStr);
                schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                scheduleList.append(schedule);
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

DSchedule::List DAccountDataBase::querySchedulesByRRule(const QString &key, const int &rruleType)
{
    DSchedule::List scheduleList;
    QString strSql("SELECT  scheduleID, scheduleTypeID, summary, description, allDay, dtStart, dtEnd,   \
                   isAlarm,titlePinyin,isLunar, ics, fileName, dtCreate, dtUpdate, dtDelete, isDeleted  \
                   FROM schedules ");
    SqliteQuery query(m_database);
    if (!key.isEmpty()) {
        strSql += " WHERE  summary  = ? ";
    }
    if (query.prepare(strSql)) {
        if (!key.isEmpty()) {
            query.addBindValue(key);
        }

        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
                QString &&icsStr = query.value("ics").toString();
                DSchedule::fromIcsString(schedule, icsStr);
                schedule->setScheduleTypeID(query.value("scheduleTypeID").toString());
                DSchedule::RRuleType rRuleType = schedule->getRRuleType();
                //如果存在重复规则
                if (schedule->recurs()) {
                    //如果为需要获取的重复规则
                    if (rruleType == rRuleType) {
                        scheduleList.append(schedule);
                    }
                }
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

DSchedule::List DAccountDataBase::getRemindSchedule()
{
    QString strSql("SELECT  scheduleID, scheduleTypeID, summary, description, allDay, dtStart, dtEnd, isAlarm,  \
                   titlePinyin, isLunar, ics, fileName, dtCreate, dtUpdate, dtDelete, isDeleted                 \
                   FROM schedules WHERE  isAlarm =1;");
    SqliteQuery query(m_database);
    DSchedule::List scheduleList;
    if (query.prepare(strSql)) {
        if (query.exec()) {
            while (query.next()) {
                DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
                QString &&icsStr = query.value("ics").toString();
                DSchedule::fromIcsString(schedule, icsStr);
                scheduleList.append(schedule);
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

void DAccountDataBase::initDBData()
{
    //如果不存在对应的数据库则创建
    if (!dbFileExists()) {
        createDB();
        initTypeColor();
        initScheduleDB();
        initScheduleType();
        initAccountDB();
    } else {
        //如果存在则连接数据库
        dbOpen();
    }
}

QString DAccountDataBase::createScheduleType(const DScheduleType::Ptr &scheduleType)
{
    QString strSql("INSERT INTO scheduleType (                      \
                   typeID, typeName, typeDisplayName, typePath,     \
                   typeColorID, description, privilege, showState,  \
                   syncTag,dtCreate,isDeleted)                      \
               VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        if (scheduleType->typeID().size() < 30) {
            scheduleType->setTypeID(DDataBase::createUuid());
        }

        query.addBindValue(scheduleType->typeID());
        query.addBindValue(scheduleType->typeName());
        query.addBindValue(scheduleType->displayName());
        query.addBindValue(scheduleType->typePath());
        query.addBindValue(scheduleType->typeColor().colorID());
        query.addBindValue(scheduleType->description());
        query.addBindValue(int(scheduleType->privilege()));
        query.addBindValue(scheduleType->showState());
        query.addBindValue(scheduleType->syncTag());
        query.addBindValue(dtToString(scheduleType->dtCreate()));
        query.addBindValue(scheduleType->deleted());

        if (!query.exec()) {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
            scheduleType->setTypeID("");
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        scheduleType->setTypeID("");
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleType->typeID();
}

DScheduleType::Ptr DAccountDataBase::getScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    DScheduleType::Ptr type;
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.typeID = ? AND st.isDeleteD = ?");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        query.addBindValue(isDeleted);
        if (query.exec() && query.next()) {
            type = DScheduleType::Ptr(new DScheduleType(m_account->accountID()));
            type->setTypeID(typeID);
            type->setTypeName(query.value("typeName").toString());
            type->setDisplayName(query.value("typeDisplayName").toString());
            type->setTypePath(query.value("typePath").toString());
            DTypeColor color;
            color.setColorID(query.value("typeColorID").toString());
            color.setColorCode(query.value("ColorHex").toString());
            color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
            type->setTypeColor(color);
            type->setDescription(query.value("description").toString());
            type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
            type->setSyncTag(query.value("syncTag").toInt());
            type->setDtCreate(dtFromString(query.value("dtCreate").toString()));
            type->setDtUpdate(dtFromString(query.value("dtUpdate").toString()));
            type->setDtDelete(dtFromString(query.value("dtDelete").toString()));
            type->setShowState(static_cast<DScheduleType::ShowState>(query.value("showState").toInt()));
            type->setDeleted(query.value("isDeleted").toInt());
            systemTypeTran(type);
        }
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }
    query.addBindValue(typeID);
    query.addBindValue(isDeleted);

    if (query.isActive()) {
        query.finish();
    }
    return type;
}

DScheduleType::List DAccountDataBase::getScheduleTypeList(const int isDeleted)
{
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.isDeleted = ?");
    DScheduleType::List typeList;
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(isDeleted);
        if (query.exec()) {
            while (query.next()) {
                DScheduleType::Ptr type = DScheduleType::Ptr(new DScheduleType(m_account->accountID()));
                type->setTypeID(query.value("typeID").toString());
                type->setTypeName(query.value("typeName").toString());
                type->setDisplayName(query.value("typeDisplayName").toString());
                type->setTypePath(query.value("typePath").toString());
                DTypeColor color;
                color.setColorID(query.value("typeColorID").toString());
                color.setColorCode(query.value("ColorHex").toString());
                color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
                type->setTypeColor(color);
                type->setDescription(query.value("description").toString());
                type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
                type->setSyncTag(query.value("syncTag").toInt());
                type->setDtCreate(dtFromString(query.value("dtCreate").toString()));
                type->setDtUpdate(dtFromString(query.value("dtUpdate").toString()));
                type->setDtDelete(dtFromString(query.value("dtDelete").toString()));
                type->setShowState(static_cast<DScheduleType::ShowState>(query.value("showState").toInt()));
                type->setDeleted(query.value("isDeleted").toInt());
                systemTypeTran(type);
                typeList.append(type);
            }
        } else {
            qCWarning(ServiceLogger) << "getScheduleTypeList error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "getScheduleTypeList error:" << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return typeList;
}

bool DAccountDataBase::scheduleTypeByUsed(const QString &typeID, const int isDeleted)
{
    QString strSql("SELECT COUNT(scheduleTypeID) FROM schedules WHERE scheduleTypeID = ? AND isDeleted = ?;");
    SqliteQuery query(m_database);
    int typeCount = 0;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        query.addBindValue(isDeleted);

        if (query.exec() && query.next()) {
            typeCount = query.value(0).toInt();
        }
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return typeCount;
}

bool DAccountDataBase::deleteScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    SqliteQuery query(m_database);
    QString strSql;
    if (isDeleted == 0) {
        //弱删除
        QDateTime &&dtCurrent = QDateTime::currentDateTime();
        strSql = QString("UPDATE scheduleType  SET  dtDelete='%1', isDeleted=1  WHERE typeID=?;")
                 .arg(dtToString(dtCurrent));
    } else {
        //真删除
        strSql = "DELETE FROM scheduleType WHERE typeID = ?";
    }
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(typeID);
        res = query.exec();
        if (!res) {
            qCWarning(ServiceLogger) << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
    }

    return res;
}

bool DAccountDataBase::updateScheduleType(const DScheduleType::Ptr &scheduleType)
{
    bool res = false;
    SqliteQuery query(m_database);
    QString strSql("UPDATE scheduleType                     \
                       SET typeName=?, typeDisplayName=?, typePath=?, typeColorID=?, description=?,     \
                       privilege=?, showState=?, dtUpdate=?, dtDelete=?, isDeleted=?        \
                       WHERE typeID = ?");
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleType->typeName());
        query.addBindValue(scheduleType->displayName());
        query.addBindValue(scheduleType->typePath());
        query.addBindValue(scheduleType->typeColor().colorID());
        query.addBindValue(scheduleType->description());
        query.addBindValue(int(scheduleType->privilege()));
        query.addBindValue(scheduleType->showState());
        query.addBindValue(dtConvert(scheduleType->dtUpdate()));
        query.addBindValue(dtConvert(scheduleType->dtDelete()));
        query.addBindValue(scheduleType->deleted());
        query.addBindValue(scheduleType->typeID());
        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

QString DAccountDataBase::getFestivalTypeID()
{
    QString strSql("SELECT typeID FROM scheduleType WHERE  privilege = 0;");
    SqliteQuery query(m_database);
    QString typeID("");
    if (query.prepare(strSql)) {
        if (query.exec()) {
            if (query.next()) {
                typeID = query.value("typeID").toString();
            }
        } else {
            qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
        }

    } else {
        qCWarning(ServiceLogger) << "updateScheduleType error:" << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }

    return typeID;
}

void DAccountDataBase::getAccountInfo(const DAccount::Ptr &account)
{
    QString strSql("SELECT syncState,accountState, accountName, displayName, cloudPath,      \
                   accountType, syncFreq, intervalTime, syncTag, expandStatus, dtLastUpdate         \
                   FROM account WHERE id = 1;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql) && query.exec() && query.next()) {
        account->setSyncState(static_cast<DAccount::AccountSyncState>(query.value("syncState").toInt()));
        // accountState字段以accountmanager.db中accountManager字段为准,它会在登陆和切换状态时根据最新云同步状态去更新开关状态
//        account->setAccountState(static_cast<DAccount::AccountState>(query.value("accountState").toInt()));
        account->setAccountName(query.value("accountName").toString());
        account->setDisplayName(query.value("displayName").toString());
        account->setCloudPath(query.value("cloudPath").toString());
        account->setAccountType(static_cast<DAccount::Type>(query.value("accountType").toInt()));
        account->setSyncFreq(static_cast<DAccount::SyncFreqType>(query.value("syncFreq").toInt()));
        account->setIntervalTime(query.value("intervalTime").toInt());
        account->setSyncTag(query.value("syncTag").toInt());
        account->setIsExpandDisplay(query.value("expandStatus").toBool());
        account->setDtLastSync(dtFromString(query.value("dtLastUpdate").toString()));
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
}

void DAccountDataBase::updateAccountInfo()
{
    QString strSql("UPDATE account                                                              \
                   SET syncState=?, accountState = ?,accountName=?, displayName=?,                               \
                  cloudPath=?, accountType=?, syncFreq=?, intervalTime=?, syncTag=?             \
                  , expandStatus = ?, dtLastUpdate = ? WHERE id=1;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(m_account->syncState());
        query.addBindValue(int(m_account->accountState()));
        query.addBindValue(m_account->accountName());
        query.addBindValue(m_account->displayName());
        query.addBindValue(m_account->cloudPath());
        query.addBindValue(m_account->accountType());
        query.addBindValue(m_account->syncFreq());
        query.addBindValue(m_account->intervalTime());
        query.addBindValue(m_account->syncTag());
        query.addBindValue(m_account->isExpandDisplay());
        query.addBindValue(dtToString(m_account->dtLastSync()));
        if (!query.exec()) {
            qCWarning(ServiceLogger) << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

bool DAccountDataBase::addTypeColor(const DTypeColor::Ptr &typeColor)
{
    if (typeColor.isNull())
        return false;
    return addTypeColor(*typeColor.data());
}

bool DAccountDataBase::addTypeColor(DTypeColor &typeColor)
{
    QString strSql("INSERT INTO TypeColor                   \
                   (ColorID, ColorHex, privilege,dtCreate)           \
                   VALUES(:ColorID, :ColorHex, :privilege,:dtCreate)");

    //如果为空则创建颜色id
    if (typeColor.colorID().isEmpty()) {
        typeColor.setColorID(createUuid());
    }
    if (typeColor.dtCreate().isNull()) {
        typeColor.setDtCreate(QDateTime::currentDateTime());
    }
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.bindValue(":ColorID", typeColor.colorID());
        query.bindValue(":ColorHex", typeColor.colorCode());
        query.bindValue(":privilege", typeColor.privilege());
        query.bindValue(":dtCreate", dtToString(typeColor.dtCreate()));

        if (query.exec()) {
            res = true;
        } else {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return res;
}

void DAccountDataBase::deleteTypeColor(const QString &colorNo)
{
    QString strSql("DELETE FROM typeColor WHERE ColorID = ?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(colorNo);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

DTypeColor::List DAccountDataBase::getSysColor()
{
    QString strSql("SELECT ColorID, ColorHex, privilege,dtCreate FROM typeColor WHERE  privilege =1;");
    SqliteQuery query(m_database);
    DTypeColor::List typeColorList;

    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DTypeColor::Ptr color = DTypeColor::Ptr(new DTypeColor);
            color->setColorID(query.value("ColorID").toString());
            color->setColorCode(query.value("ColorHex").toString());
            color->setPrivilege(static_cast<DTypeColor::Privilege>(query.value("privilege").toInt()));
            color->setDtCreate(dtFromString(query.value("dtCreate").toString()));
            typeColorList.append(color);
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return typeColorList;
}

void DAccountDataBase::createRemindInfo(const DRemindData::Ptr &remind)
{
    QString strSql("INSERT INTO remindTask                              \
                   (alarmID, scheduleID, recurID,remindCount,notifyID        \
                    , dtRemind, dtStart, dtEnd)                         \
                   VALUES(?,?,?,?,?,?,?,?);");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        remind->setAlarmID(createUuid());
        query.addBindValue(remind->alarmID());
        query.addBindValue(remind->scheduleID());
        query.addBindValue(dtToString(remind->recurrenceId()));
        query.addBindValue(remind->remindCount());
        query.addBindValue(remind->notifyid());
        query.addBindValue(dtToString(remind->dtRemind()));
        query.addBindValue(dtToString(remind->dtStart()));
        query.addBindValue(dtToString(remind->dtEnd()));
        if (!query.exec()) {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }
}

void DAccountDataBase::updateRemindInfo(const DRemindData::Ptr &remind)
{
    QString strSql("UPDATE remindTask                                               \
                   SET scheduleID=?, recurID=?, remindCount=?,         \
                   notifyID=?, dtRemind=?, dtStart=?, dtEnd=? WHERE alarmID = ?");

    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(remind->scheduleID());
        query.addBindValue(dtToString(remind->recurrenceId()));
        query.addBindValue(remind->remindCount());
        query.addBindValue(remind->notifyid());
        query.addBindValue(dtToString(remind->dtRemind()));
        query.addBindValue(dtToString(remind->dtStart()));
        query.addBindValue(dtToString(remind->dtEnd()));
        query.addBindValue(remind->alarmID());
        if (!query.exec()) {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }
}

void DAccountDataBase::deleteRemindInfoByAlarmID(const QString &alarmID)
{
    QString strSql("DELETE FROM remindTask  WHERE alarmID=?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(alarmID);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

DRemindData::Ptr DAccountDataBase::getRemindData(const QString &alarmID)
{
    QString strSql("SELECT alarmID, scheduleID, recurID, remindCount, notifyID, dtRemind, dtStart, dtEnd \
                   FROM remindTask WHERE  alarmID = ? ;");
    SqliteQuery query(m_database);
    DRemindData::Ptr remindData;
    if (query.prepare(strSql)) {
        query.addBindValue(alarmID);

        if (query.exec()) {
            while (query.next()) {
                remindData = DRemindData::Ptr(new DRemindData);
                remindData->setAlarmID(query.value("alarmID").toString());
                remindData->setScheduleID(query.value("scheduleID").toString());
                remindData->setRemindCount(query.value("remindCount").toInt());
                remindData->setNotifyid(query.value("notifyID").toInt());
                remindData->setDtStart(dtFromString(query.value("dtStart").toString()));
                remindData->setDtEnd(dtFromString(query.value("dtEnd").toString()));
                remindData->setDtRemind(dtFromString(query.value("dtRemind").toString()));
                remindData->setRecurrenceId(dtFromString(query.value("recurID").toString()));
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return remindData;
}

DRemindData::List DAccountDataBase::getValidRemindJob()
{
    QString strSql("SELECT alarmID,scheduleID, recurID, remindCount, notifyID, dtRemind, dtStart, dtEnd \
                   FROM remindTask WHERE  dtRemind > ? ;");
    SqliteQuery query(m_database);
    DRemindData::List remindList;
    if (query.prepare(strSql)) {
        query.addBindValue(dtToString(QDateTime::currentDateTime()));

        if (query.exec()) {
            while (query.next()) {
                DRemindData::Ptr remindData = DRemindData::Ptr(new DRemindData);
                remindData->setAccountID(m_account->accountID());
                remindData->setAlarmID(query.value("alarmID").toString());
                remindData->setScheduleID(query.value("scheduleID").toString());
                remindData->setRemindCount(query.value("remindCount").toInt());
                remindData->setNotifyid(query.value("notifyID").toInt());
                remindData->setDtStart(dtFromString(query.value("dtStart").toString()));
                remindData->setDtEnd(dtFromString(query.value("dtEnd").toString()));
                remindData->setDtRemind(dtFromString(query.value("dtRemind").toString()));
                remindData->setRecurrenceId(dtFromString(query.value("recurID").toString()));
                remindList.append(remindData);
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return remindList;
}

void DAccountDataBase::clearRemindJobDatabase()
{
    SqliteQuery query(m_database);
    QString sql("delete from remindTask");
    if (query.exec(sql)) {
        if (query.isActive()) {
            query.finish();
        }
    } else {
        qCWarning(ServiceLogger) << __FUNCTION__ << query.lastError();
    }
}

DRemindData::List DAccountDataBase::getRemindByScheduleID(const QString &scheduleID)
{
    QString strSql("SELECT alarmID, scheduleID, recurID, remindCount, notifyID, dtRemind, dtStart, dtEnd \
                   FROM remindTask WHERE  scheduleID = ? ;");
    SqliteQuery query(m_database);
    DRemindData::List remindList;
    if (query.prepare(strSql)) {
        query.addBindValue(scheduleID);

        if (query.exec()) {
            while (query.next()) {
                DRemindData::Ptr remindData = DRemindData::Ptr(new DRemindData);
                remindData->setAlarmID(query.value("alarmID").toString());
                remindData->setScheduleID(query.value("scheduleID").toString());
                remindData->setRemindCount(query.value("remindCount").toInt());
                remindData->setNotifyid(query.value("notifyID").toInt());
                remindData->setDtStart(dtFromString(query.value("dtStart").toString()));
                remindData->setDtEnd(dtFromString(query.value("dtEnd").toString()));
                remindData->setDtRemind(dtFromString(query.value("dtRemind").toString()));
                remindData->setRecurrenceId(dtFromString(query.value("recurID").toString()));
                remindList.append(remindData);
            }
        } else {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
    return remindList;
}

void DAccountDataBase::addUploadTask(const DUploadTaskData::Ptr &uploadTask)
{
    QString strSql("INSERT INTO uploadTask                                      \
                   (taskID, uploadType, uploadObject, objectID, dtCreate)      \
                   VALUES(?, ?, ?, ?, ?);");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        uploadTask->setTaskID(createUuid());
        query.addBindValue(uploadTask->taskID());
        query.addBindValue(uploadTask->taskType());
        query.addBindValue(uploadTask->taskObject());
        query.addBindValue(uploadTask->objectId());
        query.addBindValue(dtToString(QDateTime::currentDateTime()));
        if (!query.exec()) {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

DUploadTaskData::List DAccountDataBase::getUploadTask()
{
    DUploadTaskData::List uploadList;
    QString strSql("SELECT taskID, uploadType, uploadObject, objectID, dtCreate     \
                  FROM uploadTask;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DUploadTaskData::Ptr upload = DUploadTaskData::Ptr(new DUploadTaskData);
            upload->setTaskID(query.value("taskID").toString());
            upload->setTaskType(static_cast<DUploadTaskData::TaskType>(query.value("uploadType").toInt()));
            upload->setTaskObject(static_cast<DUploadTaskData::TaskObject>(query.value("uploadObject").toInt()));
            upload->setObjectId(query.value("objectID").toString());
            upload->setDtCreate(dtFromString(query.value("dtCreate").toString()));
            uploadList.append(upload);
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return uploadList;
}

void DAccountDataBase::deleteUploadTask(const QString &taskID)
{
    QString strSql("DELETE FROM uploadTask   WHERE taskID = ?;");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(taskID);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << Q_FUNC_INFO << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}

void DAccountDataBase::createDB()
{
    dbOpen();
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(getDBPath());
    //如果不存在该文件则创建
    if (!file.exists() && m_database.open()) {
        m_database.close();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qCWarning(ServiceLogger) << "permissions cannot be modified，error:" << file.errorString();
    }
    if (m_database.open()) {
        SqliteQuery query(m_database);
        bool res = true;
        //帐户信息表
        res = query.exec(sql_create_account);
        if (!res) {
            qCWarning(ServiceLogger) << "account create failed.error:" << query.lastError();
        }

        //日程表
        res = query.exec(sql_create_schedules);
        if (!res) {
            qCWarning(ServiceLogger) << "schedules create failed.error:" << query.lastError();
        }

        //类型表
        res = query.exec(sql_create_scheduleType);
        if (!res) {
            qCWarning(ServiceLogger) << "scheduleType create failed.error:" << query.lastError();
        }

        //颜色表
        res = query.exec(sql_create_typeColor);
        if (!res) {
            qCWarning(ServiceLogger) << "typeColorSql create failed.error:" << query.lastError();
        }

        //创建上传任务表
        res = query.exec(sql_create_uploadTask);
        if (!res) {
            qCWarning(ServiceLogger) << "uploadTask create failed.error:" << query.lastError();
        }

        //创建提醒任务表
        res = query.exec(sql_create_remindTask);
        if (!res) {
            qCWarning(ServiceLogger) << "remindTask create failed.error:" << query.lastError();
        }

        if (query.isActive()) {
            query.finish();
        }
    }
}

void DAccountDataBase::initScheduleDB()
{
    //创建数据库时，需要初始化的日程数据
}

void DAccountDataBase::initScheduleType()
{
    //创建数据库时，需要初始化的类型数据
    initSysType();
}

void DAccountDataBase::initSysType()
{
    //如果为本地帐户则初始化本地日程类型数据
    if (m_account->accountType() == DAccount::Account_Local) {
        QDateTime currentTime = QDateTime::currentDateTime();
        //添加节假日日程类型，不会展示在类型列表中
        DScheduleType::Ptr scheduleType(new DScheduleType);
        scheduleType->setTypeID(createUuid());
        scheduleType->setDtCreate(currentTime.addDays(-3));
        scheduleType->setPrivilege(DScheduleType::None);
        scheduleType->setTypeName("festival");
        scheduleType->setDisplayName("Holiday schedule type");
        scheduleType->setColorID(GFestivalColorID);
        scheduleType->setColorCode("#FF9436");
        scheduleType->setShowState(DScheduleType::Show);
        createScheduleType(scheduleType);

        //工作类型
        DScheduleType::Ptr workType(new DScheduleType(m_account->accountID()));
        workType->setTypeID("107c369e-b13a-4d45-9ff3-de4eb3c0475b");
        workType->setDtCreate(currentTime.addDays(-2));
        workType->setPrivilege(DScheduleType::Read);
        workType->setTypeName("Work");
        workType->setDisplayName("Work");
        DTypeColor workColor;
        workColor.setColorID(GWorkColorID);
        workColor.setColorCode("#ff5e97");
        workColor.setPrivilege(DTypeColor::PriSystem);
        workType->setTypeColor(workColor);
        workType->setShowState(DScheduleType::Show);
        createScheduleType(workType);

        //生活
        DScheduleType::Ptr lifeType(new DScheduleType(m_account->accountID()));
        lifeType->setTypeID("24cf3ae3-541d-487f-83df-f068416b56b6");
        lifeType->setDtCreate(currentTime.addDays(-1));
        lifeType->setPrivilege(DScheduleType::Read);
        lifeType->setTypeName("Life");
        lifeType->setDisplayName("Life");
        DTypeColor lifeColor;
        lifeColor.setColorID(GLifeColorID);
        lifeColor.setColorCode("#5d51ff");
        lifeColor.setPrivilege(DTypeColor::PriSystem);
        lifeType->setTypeColor(lifeColor);
        lifeType->setShowState(DScheduleType::Show);
        createScheduleType(lifeType);

        //其他
        DScheduleType::Ptr otherType(new DScheduleType(m_account->accountID()));
        otherType->setTypeID("403bf009-2005-4679-9c76-e73d9f83a8b4");
        otherType->setDtCreate(currentTime);
        otherType->setPrivilege(DScheduleType::Read);
        otherType->setTypeName("Other");
        otherType->setDisplayName("Other");
        DTypeColor otherColor;
        otherColor.setColorID(GOtherColorID);
        otherColor.setColorCode("#5bdd80");
        otherColor.setPrivilege(DTypeColor::PriSystem);
        otherType->setTypeColor(otherColor);
        otherType->setShowState(DScheduleType::Show);
        createScheduleType(otherType);
    }
}

void DAccountDataBase::systemTypeTran(const DScheduleType::Ptr &type)
{
    //如果为本地帐户且类型为默认类型
    if ((m_account->accountType() == DAccount::Account_Local || m_account->accountType() == DAccount::Account_UnionID) && type->privilege() == DScheduleType::Privilege::Read) {
        if (type->typeName() == "Work") {
            type->setDisplayName(tr("Work"));
        } else if (type->typeName() == "Life") {
            type->setDisplayName(tr("Life"));
        } else if (type->typeName() == "Other") {
            type->setDisplayName(tr("Other"));
        }
    }
}

void DAccountDataBase::initTypeColor()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int index = -10;

    QMap<QString, QString>::const_iterator iter = GTypeColor.constBegin();
    for (; iter != GTypeColor.constEnd(); ++iter) {
        DTypeColor::Ptr typeColor(new DTypeColor);
        typeColor->setDtCreate(currentTime.addDays(++index));
        typeColor->setPrivilege(DTypeColor::PriSystem);
        typeColor->setColorCode(iter.value());
        typeColor->setColorID(iter.key());
        addTypeColor(typeColor);
    }
}

void DAccountDataBase::initAccountDB()
{
    QString strSql("INSERT                                  \
                   INTO                                     \
                   account                                  \
               (syncState,accountState,accountName,         \
                   displayName,cloudPath,accountType,       \
                   syncFreq,intervalTime,syncTag, expandStatus)           \
               VALUES(?,?,?,?,?,?,?,?,?,?);");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(m_account->syncState());
        query.addBindValue(int(m_account->accountState()));
        query.addBindValue(m_account->accountName());
        query.addBindValue(m_account->displayName());
        query.addBindValue(m_account->cloudPath());
        query.addBindValue(m_account->accountType());
        query.addBindValue(m_account->syncFreq());
        query.addBindValue(m_account->intervalTime());
        query.addBindValue(m_account->syncTag());
        query.addBindValue(m_account->isExpandDisplay());
        if (!query.exec()) {
            qCWarning(ServiceLogger) << "initAccountDB error:" << query.lastError();
        }
    } else {
        qCWarning(ServiceLogger) << "initAccountDB error:" << query.lastError();
    }

    if (query.isActive()) {
        query.finish();
    }
}
