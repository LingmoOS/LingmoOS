// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSACCOUNTREQUEST_H
#define DBUSACCOUNTREQUEST_H

#include "dbusrequestbase.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dschedulequerypar.h"

//单项账户信息请求类
class DbusAccountRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusAccountRequest(const QString &path, const QString &interface, QObject *parent = nullptr);

    /**
     * @brief getAccountInfo        获取帐户信息
     * @return
     */
    void getAccountInfo();
    /**
     * @brief updateAccountInfo     更新帐户信息
     * @param accountInfo           帐户信息
     */
    void updateAccountInfo(const DAccount::Ptr &accountInfo);
    /**
     * @brief getScheduleTypeList      获取日程类型信息集
     * @return
     */
    DScheduleType::List getScheduleTypeList();

    /**
     * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
     * @param typeID                日程类型ID
     * @return
     */
    void getScheduleTypeByID(const QString &typeID);

    /**
     * @brief createScheduleType         创建日程类型
     * @param typeInfo              类型信息
     * @return                      日程类型ID
     */
    void createScheduleType(const DScheduleType::Ptr &typeInfo);

    /**
     * @brief updateScheduleType         更新日程类型
     * @param typeInfo              类型信息
     * @return                      是否成功，true:更新成功
     */
    void updateScheduleType(const DScheduleType::Ptr &typeInfo);

    /**
     * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
     * @param typeID                日程类型ID
     * @return                      是否成功，true:更新成功
     */
    void deleteScheduleTypeByID(const QString &typeID);

    /**
     * @brief scheduleTypeByUsed         日程类型是否被使用
     * @param typeID                日程类型ID
     * @return
     */
    void scheduleTypeByUsed(const QString &typeID);

    /**
     * @brief createSchedule             创建日程
     * @param ScheduleInfo               日程信息
     * @return                      返回日程ID
     */
    QString createSchedule(const DSchedule::Ptr &scheduleInfo);

    /**
     * @brief updateSchedule             更新日程
     * @param ScheduleInfo               日程信息
     * @return                      是否成功，true:更新成功
     */
    void updateSchedule(const DSchedule::Ptr &scheduleInfo);

    DSchedule::Ptr getScheduleByID(const QString &scheduleID);

    /**
     * @brief deleteScheduleByScheduleID      根据日程ID删除日程
     * @param ScheduleID                 日程ID
     * @return                      是否成功，true:删除成功
     */
    void deleteScheduleByScheduleID(const QString &scheduleID);

    /**
     * @brief deleteSchedulesByScheduleTypeID 根据日程类型ID删除日程
     * @param typeID                日程类型ID
     * @return                      是否成功，true:删除成功
     */
    void deleteSchedulesByScheduleTypeID(const QString &typeID);

    /**
     * @brief querySchedulesWithParameter        根据查询参数查询日程
     * @param params                        具体的查询参数
     * @return                              查询到的日程集
     */
    DSchedule::Map querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params);

    DTypeColor::List getSysColors();

signals:
    void signalCreateScheduleFinish(const QString &scheduleID);
    void signalGetAccountInfoFinish(DAccount::Ptr);
    void signalGetScheduleFinish(const DSchedule::Ptr &);
    void signalGetScheduleTypeListFinish(DScheduleType::List);
    void signalGetScheduleListFinish(QMap<QDate, DSchedule::List>);
    void signalSearchScheduleListFinish(QMap<QDate, DSchedule::List>);
    void signalGetSysColorsFinish(DTypeColor::List);

public slots:

    void slotCallFinished(CDBusPendingCallWatcher *) override;

private:
    DScheduleQueryPar::Ptr m_priParams; //上一次查询日程的数据
};

#endif // DBUSACCOUNTREQUEST_H
