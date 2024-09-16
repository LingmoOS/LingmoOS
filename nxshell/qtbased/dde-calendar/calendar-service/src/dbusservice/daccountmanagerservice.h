// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTMANAGERSERVICE_H
#define ACCOUNTMANAGERSERVICE_H

#include "dservicebase.h"
#include "daccountmanagemodule.h"

#include <QString>

/**
 * @brief The DAccountManagerService class      帐户管理服务
 */

class DAccountManagerService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.AccountManager")
    Q_PROPERTY(int firstDayOfWeek READ getfirstDayOfWeek WRITE setFirstDayOfWeek)
    Q_PROPERTY(int timeFormatType READ getTimeFormatType WRITE setTimeFormatType)
    Q_PROPERTY(int firstDayOfWeekSource READ getFirstDayOfWeekSource WRITE setFirstDayOfWeekSource)
    Q_PROPERTY(int timeFormatTypeSource READ getTimeFormatTypeSource WRITE setTimeFormatTypeSource)
public:
    explicit DAccountManagerService(QObject *parent = nullptr);
public slots:
    /**
     * @brief getAccountList        获取帐户列表
     * @return
     */
    Q_SCRIPTABLE QString getAccountList();
    //稍后提醒相关接口
    Q_SCRIPTABLE void remindJob(const QString &accountID, const QString &alarmID);

    /**
     * @brief updateRemindJob       每隔10分钟更新提醒日程
     * @param isClear               是否情况日程定时任务数据库
     */
    Q_SCRIPTABLE void updateRemindJob(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param alarmID               提醒任务id
     * @param operationNum          操作编号 ， 1：打开日历，2：稍后提醒 3： 明天提醒 4： 提前1天提醒 5:关闭按钮
     */
    Q_SCRIPTABLE void notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum);

    Q_SCRIPTABLE void downloadByAccountID(const QString &accountID);
    Q_SCRIPTABLE void uploadNetWorkAccountData();
    //获取通用设置
    Q_SCRIPTABLE QString getCalendarGeneralSettings();
    //设置通用设置
    Q_SCRIPTABLE void setCalendarGeneralSettings(const QString &cgSet);
    //日历打开关闭
    Q_SCRIPTABLE void calendarIsShow(const bool &isShow);
    //账户登录
    Q_SCRIPTABLE void login();
    //账户登出
    Q_SCRIPTABLE void logout();
    //是否支持云同步
    Q_SCRIPTABLE bool isSupportUid();

signals:
    Q_SCRIPTABLE void accountUpdate();
private:
    int getfirstDayOfWeek() const;
    void setFirstDayOfWeek(const int firstday);
    int getTimeFormatType() const;
    void setTimeFormatType(const int timeType);

    int getFirstDayOfWeekSource();
    void setFirstDayOfWeekSource(const int source);
    int getTimeFormatTypeSource();
    void setTimeFormatTypeSource(const int source);

private:
    DAccountManageModule::Ptr m_accountManager;
};

#endif // ACCOUNTMANAGERSERVICE_H
