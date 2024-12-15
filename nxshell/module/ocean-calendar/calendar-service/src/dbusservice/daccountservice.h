// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTSERVICE_H
#define ACCOUNTSERVICE_H

#include "dservicebase.h"
#include "daccountmodule.h"
#include "units.h"

#include <QSharedPointer>

class DAccountService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", accountServiceInterface)
    Q_PROPERTY(bool isExpand READ getExpand WRITE setExpand)
    Q_PROPERTY(int accountState READ getAccountState WRITE setAccountState)
    Q_PROPERTY(int syncState READ getSyncState)
    Q_PROPERTY(QString syncFreq READ getSyncFreq WRITE setSyncFreq)
    Q_PROPERTY(QString dtLastUpdate READ getDtLastUpdate)
public:
    typedef QSharedPointer<DAccountService> Ptr;

    DAccountService(const QString &path, const QString &interface, const DAccountModule::Ptr &accountModule, QObject *parent = nullptr);
public slots:
    /**
     * @brief getAccountInfo        获取帐户信息
     * @return
     */
    Q_SCRIPTABLE QString getAccountInfo();
    /**
     * @brief getScheduleTypeList      获取日程类型信息集
     * @return
     */
    Q_SCRIPTABLE QString getScheduleTypeList();

    /**
     * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
     * @param typeID                日程类型ID
     * @return
     */
    Q_SCRIPTABLE QString getScheduleTypeByID(const QString &typeID);

    /**
     * @brief createScheduleType         创建日程类型
     * @param typeInfo              类型信息
     * @return                      日程类型ID
     */
    Q_SCRIPTABLE QString createScheduleType(const QString &typeInfo);

    /**
     * @brief updateScheduleType         更新日程类型
     * @param typeInfo              类型信息
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool updateScheduleType(const QString &typeInfo);

    /**
     * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
     * @param typeID                日程类型ID
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool deleteScheduleTypeByID(const QString &typeID);

    /**
     * @brief scheduleTypeByUsed         日程类型是否被使用
     * @param typeID                日程类型ID
     * @return
     */
    Q_SCRIPTABLE bool scheduleTypeByUsed(const QString &typeID);

    /**
     * @brief createSchedule             创建日程
     * @param ScheduleInfo               日程信息
     * @return                      返回日程ID
     */
    Q_SCRIPTABLE QString createSchedule(const QString &scheduleInfo);

    /**
     * @brief updateSchedule             更新日程
     * @param ScheduleInfo               日程信息
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool updateSchedule(const QString &scheduleInfo);

    /**
     * @brief getScheduleByScheduleID       根据日程id获取日程信息
     * @param scheduleID                    日程id
     * @return                              日程信息
     */
    Q_SCRIPTABLE QString getScheduleByScheduleID(const QString &scheduleID);

    /**
     * @brief deleteScheduleByScheduleID      根据日程ID删除日程
     * @param ScheduleID                 日程ID
     * @return                      是否成功，true:删除成功
     */
    Q_SCRIPTABLE bool deleteScheduleByScheduleID(const QString &scheduleID);

    /**
     * @brief querySchedulesWithParameter       根据查询参数查询日程
     * @param params                            具体的查询参数
     * @return                                  查询到的日程集
     */
    Q_SCRIPTABLE QString querySchedulesWithParameter(const QString &params);

    /**
     * @brief querySchedulesWithParameter       导出日程到ics文件
     * @param icsFilePath                       ics文件路径
     * @param typeID                            导入到的类型ID
     * @return                                  是否成功，true: 成功
     */
    Q_SCRIPTABLE bool exportSchedule(const QString &icsFilePath, const QString &typeID);

    /**
     * @brief querySchedulesWithParameter       导入ics文件到日程
     * @param icsFilePath                       ics文件路径
     * @param typeID                            导入到的类型ID
     * @param cleanExists                       是否清理已存在的日程
     * @return                                  是否成功，true: 成功
     */
    Q_SCRIPTABLE bool importSchedule(const QString &icsFilePath, const QString &typeID, const bool cleanExists);

    ///////获取内置类型颜色

    Q_SCRIPTABLE QString getSysColors();

signals:
    //日程更新信号，日程颜色更新信号
    void scheduleUpdate();
    void scheduleTypeUpdate();

private:
    //帐户列表是否展开
    bool getExpand();
    void setExpand(const bool &isExpand);
    //帐户状态。
    int getAccountState();
    void setAccountState(const int accountState);
    //获取同步状态
    int getSyncState();
    //设置同步频率相关信息
    QString getSyncFreq();
    void setSyncFreq(const QString &freq);
    //获取最后一次同步时间
    QString getDtLastUpdate();

public slots:
private:
    DAccountModule::Ptr m_accountModel;
};

#endif // ACCOUNTSERVICE_H
