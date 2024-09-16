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

//单项帐户信息请求类
class DbusAccountRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusAccountRequest(const QString &path, const QString &interface, QObject *parent = nullptr);

    /**
     * @brief setAccountExpandStatus
     * 设置帐户列表展开状态
     * @param expandStatus 展开状态
     */
    void setAccountExpandStatus(bool expandStatus);

    void setAccountState(DAccount::AccountStates state);
    void setSyncFreq(const QString &freq);
    DAccount::AccountStates getAccountState();
    DAccount::AccountSyncState getSyncState();
    QString getSyncFreq();
    /**
     * @brief getAccountInfo        获取帐户信息
     * @return
     */
    void getAccountInfo();

    /**
     * @brief getScheduleTypeList      获取日程类型信息集
     * @return
     */
    void getScheduleTypeList();

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
     * @brief updateScheduleTypeShowState
     * 更新类型显示状态
     * @param typeInfo
     */
    void updateScheduleTypeShowState(const DScheduleType::Ptr &typeInfo);

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
    bool scheduleTypeByUsed(const QString &typeID);

    /**
     * @brief createSchedule             创建日程
     * @param ScheduleInfo               日程信息
     * @return                      返回日程ID
     */
    void createSchedule(const DSchedule::Ptr &scheduleInfo);

    /**
     * @brief updateSchedule             更新日程
     * @param ScheduleInfo               日程信息
     * @return                      是否成功，true:更新成功
     */
    void updateSchedule(const DSchedule::Ptr &scheduleInfo);

    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);

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
    void querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params);
    //对外查询接口，同步
    bool querySchedulesByExternal(const DScheduleQueryPar::Ptr &params, QString &json);

    void getSysColors();

    //获取最后一次同步时间
    QString getDtLastUpdate();

    void importSchedule(QString icsFilePath, QString TypeID, bool cleanExists);
    void exportSchedule(QString icsFilePath, QString TypeID);
signals:
    void signalGetAccountInfoFinish(DAccount::Ptr);
    void signalGetScheduleTypeListFinish(DScheduleType::List);
    void signalGetScheduleListFinish(QMap<QDate, DSchedule::List>);
    void signalSearchScheduleListFinish(QMap<QDate, DSchedule::List>);
    void signalGetSysColorsFinish(DTypeColor::List);
    void signalDtLastUpdate(QString);
    void signalSyncStateChange(DAccount::AccountSyncState);
    void signalAccountStateChange(DAccount::AccountStates);
    void signalSearchUpdate();

public slots:
    //dbus服务端调用
    void slotDbusCall(const QDBusMessage &msg) override;
    void slotCallFinished(CDBusPendingCallWatcher *) override;

private:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);

private:
    DScheduleQueryPar::Ptr m_priParams; //上一次查询日程的数据

};

#endif // DBUSACCOUNTREQUEST_H
