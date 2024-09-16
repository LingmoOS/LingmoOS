// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddatabasemanagement.h"

#include "commondef.h"
#include "ddatabase.h"
#include "vcalformat.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "units.h"
#include "recurrence.h"
#include "alarm.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QJsonArray>

const QString localDBName = "localAccount.db";

DDataBaseManagement::DDataBaseManagement()
    : m_newDatabaseName("accountmanager.db")
    , m_oldDatabaseName("scheduler.db")
{
    //旧文件路径
    QString oldDbPath = getHomeConfigPath().append("/deepin/dde-daemon/calendar");
    setOldDatabasePath(oldDbPath);
    //新文件路径
    QString newDbPath = getDBPath();
    setNewDatabasePath(newDbPath);
    QString newDB(newDatabasePath() + "/" + m_newDatabaseName);
    //如果新数据库不存在
    if (!databaseExists(newDatabasePath())) {
        QString localAccountDB(newDatabasePath() + "/" + localDBName);
        DAccountManagerDataBase accountManager;
        accountManager.setDBPath(newDB);
        accountManager.setLoaclDB(localDBName);
        accountManager.initDBData();

        DAccount::List accountList = accountManager.getAccountList();
        DAccount::Ptr localAccount;
        for (auto account : accountList) {
            if (account->accountType() == DAccount::Account_Local) {
                localAccount = account;
                break;
            }
        }
        DAccountDataBase localDB(localAccount);
        localDB.setDBPath(localAccountDB);
        localDB.initDBData();

        DTypeColor::List  sysColorList = localDB.getSysColor();

        QMap<QString,int > oldSysColor={
            {"#FF5E97",1}
            ,{"#FF9436",2}
            ,{"#FFDC00",3}
            ,{"#5BDD80",4}
            ,{"#00B99B",5}
            ,{"#4293FF",6}
            ,{"#5D51FF",7}
            ,{"#A950FF",8}
            ,{"#717171",9}
        };

        foreach (const auto &sysColor, sysColorList) {
            if(oldSysColor.contains(sysColor->colorCode())){
                m_sysColorID.insert(oldSysColor[sysColor->colorCode()],sysColor->colorID());
            }
        }

        //判断是否存在旧的数据库
        QString oldDBFile(oldDatabasePath() + "/" + m_oldDatabaseName);
        if (databaseExists(oldDBFile)) {
            //对数据进行迁移
            QSqlDatabase oldDB = QSqlDatabase::addDatabase("QSQLITE", "oldDB");
            oldDB.setDatabaseName(oldDBFile);
            oldDB.open();

            m_typeMap.insert(1, "107c369e-b13a-4d45-9ff3-de4eb3c0475b");
            m_typeMap.insert(2, "24cf3ae3-541d-487f-83df-f068416b56b6");
            m_typeMap.insert(3, "403bf009-2005-4679-9c76-e73d9f83a8b4");

            //获取类型
            if (hasTypeDB(oldDB)) {
                DScheduleType::List typeList = queryOldJobTypeData(oldDB);
                foreach (auto &type, typeList) {
                    int &&typeid_int = type->typeID().toInt();
                    QString &&type_ID = localDB.createScheduleType(type);
                    m_typeMap.insert(typeid_int, type_ID);
                }

                //获取颜色
                QVector<DTypeColor> colorList = queryOldTypeColorData(oldDB);
                foreach (auto color, colorList) {
                    localDB.addTypeColor(color);
                }
            }

            DSchedule::List scheduleList = queryOldJobData(oldDB, hasLunnarField(oldDB));
            //将原数据库中日程编号与当前日程编号匹配上
            foreach (auto schedule, scheduleList) {
                int scheduleID = schedule->uid().toInt();
                m_schedule.insert(scheduleID, localDB.createSchedule(schedule));
            }

            //提醒任务
            if (hasRemindDB(oldDB)) {
                DRemindData::List remindList = querOldRemindData(oldDB);
                foreach (auto remind, remindList) {
                    remind->setAccountID(localAccount->accountID());
                    localDB.createRemindInfo(remind);
                }
            }

            m_hasTransfer = true;
        }
    }
}

QString DDataBaseManagement::newDatabasePath() const
{
    return m_newDatabasePath;
}

void DDataBaseManagement::setNewDatabasePath(const QString &newDatabasePath)
{
    m_newDatabasePath = newDatabasePath;
}

QString DDataBaseManagement::oldDatabasePath() const
{
    return m_oldDatabasePath;
}

void DDataBaseManagement::setOldDatabasePath(const QString &oldDatabasePath)
{
    m_oldDatabasePath = oldDatabasePath;
}

bool DDataBaseManagement::databaseExists(const QString &databasePath, bool create)
{
    QDir dir;
    bool exist = false;
    if (dir.exists(databasePath)) {
        exist = true;
    } else {
        if (create) {
            dir.mkpath(databasePath);
        }
    }
    return exist;
}

bool DDataBaseManagement::hasLunnarField(QSqlDatabase &db)
{
    SqliteQuery query(db);
    bool haslunnar = false;
    QString hasIsLunarField = "select count(1) from sqlite_master where type='table' and "
                              "tbl_name = 'jobs' and sql like '%is_Lunar%'";
    if (query.exec(hasIsLunarField) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        haslunnar = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return haslunnar;
}

bool DDataBaseManagement::hasTypeDB(QSqlDatabase &db)
{
    SqliteQuery query(db);
    bool hasType = false;
    QString strSql = "select count(1) from sqlite_master where type='table' and "
                     "tbl_name = 'JobType'";
    if (query.exec(strSql) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        hasType = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return hasType;
}

bool DDataBaseManagement::hasRemindDB(QSqlDatabase &db)
{
    SqliteQuery query(db);
    bool hasRemind = false;
    QString strSql = "select count(1) from sqlite_master where type='table' and "
                     "tbl_name = 'jobsReminder'";
    if (query.exec(strSql) && query.next()) {
        //获取是否存在为农历标识字段，若存在则返回1,不存在则返回0
        hasRemind = query.value(0).toInt();
    }
    if (query.isActive()) {
        query.finish();
    }
    return hasRemind;
}

DScheduleType::List DDataBaseManagement::queryOldJobTypeData(QSqlDatabase &db)
{
    SqliteQuery query(db);
    //旧数据日程默认包含一个节假日日程类型,其他的都为用户创建的类型
    QString strSql("SELECT TypeNo, TypeName, ColorTypeNo, CreateTime, Authority                 \
                   FROM JobType where Authority >0;");
    DScheduleType::List typeList;
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DScheduleType::Ptr type = DScheduleType::Ptr(new DScheduleType);
            type->setTypeID(QString::number(query.value("TypeNo").toInt()));
            type->setTypeName(query.value("TypeName").toString());
            type->setDisplayName(query.value("TypeName").toString());
            type->setDtCreate(query.value("CreateTime").toDateTime());
            type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("Authority").toInt()));
            int oldColorTypeID = query.value("ColorTypeNo").toInt();
            //如果为系统色
            if(oldColorTypeID <10){
                type->setColorID(m_sysColorID[oldColorTypeID]);
            }else {
                //如果为用户自定义颜色
                if (!m_typeColorID.contains(oldColorTypeID)) {
                    m_typeColorID[oldColorTypeID] = DDataBase::createUuid();
                }
                type->setColorID(m_typeColorID[oldColorTypeID]);
            }
            typeList.append(type);
        }
    }
    return typeList;
}

DSchedule::List DDataBaseManagement::queryOldJobData(QSqlDatabase &db, const bool haslunar)
{
    SqliteQuery query(db);
    QString strSql;
    if (haslunar) {
        strSql = "SELECT id, created_at, \"type\", title, description, all_day,                     \
                \"start\", \"end\", r_rule, remind, \"ignore\", title_pinyin, is_Lunar          \
               FROM jobs;";
    } else {
        strSql = "SELECT id, created_at, \"type\", title, description, all_day,                     \
                \"start\", \"end\", r_rule, remind, \"ignore\", title_pinyin                    \
               FROM jobs;";
    }
    DSchedule::List scheduleList;
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule);
            schedule->setUid(QString::number(query.value("id").toInt()));
            schedule->setDtStart(dtFromString(query.value("start").toString()));
            schedule->setCreated(dtFromString(query.value("created_at").toString()));
            schedule->setSummary(query.value("title").toString());
            schedule->setAllDay(query.value("all_day").toBool());
            schedule->setDtEnd(dtFromString(query.value("end").toString()));
            //如果有农历信息则设置相关信息
            if(haslunar){
                schedule->setLunnar(query.value("is_Lunar").toBool());
            }
            QString rrule = query.value("r_rule").toString();
            if (!rrule.isEmpty()) {
                //重复规则
                KCalendarCore::Recurrence *recurrence = schedule->recurrence();
                KCalendarCore::ICalFormat ical;
                KCalendarCore::RecurrenceRule *rule = new KCalendarCore::RecurrenceRule;

                if (ical.fromString(rule, query.value("r_rule").toString())) {
                    recurrence->addRRule(rule);
                }

                //添加忽略列表
                QJsonArray ignore = query.value("Ignore").toJsonArray();
                foreach (auto ignoreTime, ignore) {
                    if (schedule->allDay()) {
                        recurrence->addExDate(dtFromString(ignoreTime.toString()).date());
                    } else {
                        recurrence->addExDateTime(dtFromString(ignoreTime.toString()));
                    }
                }
            }
            QString remind = query.value("remind").toString();
            if (!remind.isEmpty()) {
                //提醒规则
                QStringList strList = remind.split(";", QString::SkipEmptyParts);

                int remindNum = strList.at(0).toInt();
                //小于0表示不提醒
                if (remindNum >= 0) {
                    KCalendarCore::Alarm::Ptr alarm = KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(schedule.data()));
                    alarm->setEnabled(true);
                    alarm->setType(KCalendarCore::Alarm::Display);
                    alarm->setDisplayAlarm(schedule->summary());

                    if (schedule->allDay()) {
                        //提前多少秒
                        int offset = 0;
                        if (strList.size() > 1) {
                            QTime time = QTime::fromString(strList.at(1), "hh:mm");
                            offset = time.hour() * 60 * 60 + time.second() * 60;
                        }
                        KCalendarCore::Duration duration(-(24 * 60 * 60 * remindNum - offset));
                        alarm->setStartOffset(duration);
                    } else {
                        KCalendarCore::Duration duration(-(60 * remindNum));
                        alarm->setStartOffset(duration);
                    }
                    schedule->addAlarm(alarm);
                }
            }
            int type = query.value("type").toInt();
            if (m_typeMap.contains(type)) {
                schedule->setScheduleTypeID(m_typeMap[type]);
            } else {
                qCWarning(ServiceLogger) << " can not find type:" << type;
            }
            scheduleList.append(schedule);
        }
    }

    if (query.isActive()) {
        query.finish();
    }
    return scheduleList;
}

QVector<DTypeColor> DDataBaseManagement::queryOldTypeColorData(QSqlDatabase &db)
{
    SqliteQuery query(db);
    QVector<DTypeColor> colorVector;
    //获取用户创建颜色
    QString strSql("SELECT TypeNo, ColorHex, Authority              \
                   FROM ColorType WHERE ID  >9;");
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            DTypeColor color;
            int oldTpyeID = query.value("TypeNo").toInt();
            //如果包含则添加,没有则丢弃
            if (m_typeColorID.contains(oldTpyeID)) {
                color.setColorID(m_typeColorID[oldTpyeID]);
                color.setColorCode(query.value("ColorHex").toString());
                color.setPrivilege(DTypeColor::PriUser);
                colorVector.append(color);
            }
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return colorVector;
}

DRemindData::List DDataBaseManagement::querOldRemindData(QSqlDatabase &db)
{
    SqliteQuery query(db);
    DRemindData::List remindList;
    QString strSql("SELECT jobid, recurid, remindCount, notifyid, remindTime, jobStartTime, jobEndTime FROM jobsReminder;");
    query.prepare(strSql);
    if (query.exec()) {
        while (query.next()) {
            int jobid = query.value("jobid").toInt();
            if (m_schedule.contains(jobid)) {
                DRemindData::Ptr remind = DRemindData::Ptr(new DRemindData);
                remind->setScheduleID(m_schedule[jobid]);
                remind->setRemindCount(query.value("remindCount").toInt());
                remind->setNotifyid(query.value("notifyid").toInt());
                remind->setDtRemind(dtFromString(query.value("remindTime").toString()));
                remind->setDtStart(dtFromString(query.value("jobStartTime").toString()));
                remind->setDtEnd(dtFromString(query.value("jobEndTime").toString()));

                int recurid = query.value("recurid").toInt();
                //如果重复id大于0，则表示为重复日程的提醒，设置提醒id为日程开始时间
                if (recurid > 0) {
                    remind->setRecurrenceId(remind->dtStart());
                }
                remindList.append(remind);
            }
        }
    }
    if (query.isActive()) {
        query.finish();
    }
    return remindList;
}

bool DDataBaseManagement::hasTransfer() const
{
    return m_hasTransfer;
}
