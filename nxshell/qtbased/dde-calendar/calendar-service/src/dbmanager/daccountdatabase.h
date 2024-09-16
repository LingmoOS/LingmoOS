// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCOUNTDATABASE_H
#define DACCOUNTDATABASE_H

#include "ddatabase.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dreminddata.h"
#include "duploadtaskdata.h"
#include "dtypecolor.h"

#include <QSharedPointer>

class DAccountDataBase : public DDataBase
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountDataBase> Ptr;

    explicit DAccountDataBase(const DAccount::Ptr &account, QObject *parent = nullptr);
    //初始化数据库数据，会创建数据库文件和相关数据表
    void initDBData() override;
    ///////////////日程信息
    //创建日程
    QString createSchedule(const DSchedule::Ptr &schedule);
    bool updateSchedule(const DSchedule::Ptr &schedule);
    //根据日程id获取日程信息
    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);

    //根据日程类型ID获取日程id列表
    QStringList getScheduleIDListByTypeID(const QString &typeID);
    bool deleteScheduleByScheduleID(const QString &scheduleID, const int isDeleted = 0);
    bool deleteSchedulesByScheduleTypeID(const QString &typeID, const int isDeleted = 0);
    //根据关键字查询一定范围内的日程
    DSchedule::List querySchedulesByKey(const QString &key);
    //根据重复规则查询一定范围内的日程
    DSchedule::List querySchedulesByRRule(const QString &key, const int &rruleType);
    //获取需要提醒的日程信息
    DSchedule::List getRemindSchedule();

    ///////////////类型信息
    /**
     * @brief createScheduleType        创建日程类型
     * @param typeInfo
     * @return
     */
    QString createScheduleType(const DScheduleType::Ptr &scheduleType);
    virtual DScheduleType::Ptr getScheduleTypeByID(const QString &typeID, const int isDeleted = 0);
    virtual DScheduleType::List getScheduleTypeList(const int isDeleted = 0);
    bool scheduleTypeByUsed(const QString &typeID, const int isDeleted = 0);
    bool deleteScheduleTypeByID(const QString &typeID, const int isDeleted = 0);
    bool updateScheduleType(const DScheduleType::Ptr &scheduleType);

    //获取节假日类型ID
    QString getFestivalTypeID();
    ///////////////帐户信息
    void getAccountInfo(const DAccount::Ptr &account);
    void updateAccountInfo();

    ///////////////////类型颜色
    bool addTypeColor(const DTypeColor::Ptr &typeColor);
    bool addTypeColor(DTypeColor &typeColor);
    void deleteTypeColor(const QString &colorNo);
    //获取内置类型颜色
    DTypeColor::List getSysColor();

    ////////////////////任务
    /**
     * @brief createRemindInfo          存储提醒日程的相关信息
     * @param remind
     */
    void createRemindInfo(const DRemindData::Ptr &remind);

    /**
     * @brief updateRemindJob       更新对应的稍后提醒日程
     * @param job                   日程信息
     */
    void updateRemindInfo(const DRemindData::Ptr &remind);

    void deleteRemindInfoByAlarmID(const QString &alarmID);

    DRemindData::Ptr getRemindData(const QString &alarmID);

    /**
     * @brief getValidRemindJob     获取未提醒的稍后提醒日程
     * @return
     */
    DRemindData::List getValidRemindJob();
    /**
     * @brief clearRemindJobDatabase    清空稍后提醒表
     */
    void clearRemindJobDatabase();

    DRemindData::List getRemindByScheduleID(const QString &scheduleID);

    /**
     * @brief deleteRemindJobs      根据日程id删除提醒日程信息
     * @param Ids                   日程id集
     */
    void deleteRemindInfos(const QStringList &jobIDs);

    //添加上传任务
    void addUploadTask(const DUploadTaskData::Ptr &uploadTask);
    DUploadTaskData::List getUploadTask();
    void deleteUploadTask(const QString &taskID);

protected:
    virtual void initScheduleType();
    //初始化系统类型
    virtual void initSysType();
    void systemTypeTran(const DScheduleType::Ptr &type);

protected:
    void createDB() override;
    //初始化日程数据库
    void initScheduleDB();
    void initTypeColor();
    void initAccountDB();

protected:
    DAccount::Ptr m_account;
};

#endif // DACCOUNTDATABASE_H
