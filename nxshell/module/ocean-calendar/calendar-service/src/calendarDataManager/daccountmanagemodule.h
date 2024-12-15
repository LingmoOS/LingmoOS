// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCOUNTMANAGEMODULE_H
#define DACCOUNTMANAGEMODULE_H

#include "dcalendargeneralsettings.h"
#include "syncfilemanage.h"
#include "daccount.h"
#include "daccountmodule.h"
#include "daccountmanagerdatabase.h"
#include "daccountservice.h"
#include "dbustimedate.h"

#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <DConfig>

//帐户类型总数，若支持的类型增加则需要修改
const int accountTypeCount = 3;

//帐户管理模块
class DAccountManageModule : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountManageModule> Ptr;

    explicit DAccountManageModule(QObject *parent = nullptr);

    //获取所有帐户信息
    QString getAccountList();

    //获取通用设置
    QString getCalendarGeneralSettings();
    //设置通用设置
    void setCalendarGeneralSettings(const QString &cgSet);

    int getfirstDayOfWeek();
    void setFirstDayOfWeek(const int firstday);
    int getTimeFormatType();
    void setTimeFormatType(const int timeType);

    DCalendarGeneralSettings::GeneralSettingSource getFirstDayOfWeekSource();
    void setFirstDayOfWeekSource(const DCalendarGeneralSettings::GeneralSettingSource source);
    DCalendarGeneralSettings::GeneralSettingSource getTimeFormatTypeSource();
    void setTimeFormatTypeSource(const DCalendarGeneralSettings::GeneralSettingSource source);

    void remindJob(const QString &accountID, const QString &alarmID);

    /**
     * @brief updateRemindSchedules     更新未来10分钟的提醒任务
     * @param isClear                   是否清空提醒任务数据库
     */
    void updateRemindSchedules(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param accountID             帐户id
     * @param alarmID               提醒任务id
     * @param operationNum          操作编号 ， 1：打开日历，2：稍后提醒 3： 明天提醒 4： 提前1天提醒 5:关闭按钮
     */
    void notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum);

    void downloadByAccountID(const QString &accountID);
    void uploadNetWorkAccountData();

    //账户登录
    void login();
    //账户登出
    void logout();
    //是否支持UID云同步
    bool isSupportUid();

    void calendarOpen(bool isOpen);

private:
    void unionIDDataMerging();
    void initAccountDBusInfo(const DAccount::Ptr &account);
    //移除uid帐户
    void removeUIdAccount(const DAccount::Ptr &uidAccount);
    //添加uid帐户
    void addUIdAccount(const DAccount::Ptr &uidAccount);
    //更新uid帐户
    void updateUIdAccount(const DAccount::Ptr &oldAccount, const DAccount::Ptr &uidAccount);
    //获取设置开关状态
    void setUidSwitchStatus(const DAccount::Ptr &account);
    // 获取通用配置
    DCalendarGeneralSettings::Ptr getGeneralSettings();
    // 保存通用配置
    void setGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet);

signals:
    void firstDayOfWeekChange();
    void timeFormatTypeChange();
    void signalLoginStatusChange();

public slots:
    void slotFirstDayOfWeek(const int firstDay);
    void slotTimeFormatType(const int timeType);
    //TODO：监听网络帐户管理信号和Union ID登陆退出状态
    void slotUidLoginStatueChange(const int status);
    //控制中心的同步开关
    void slotSwitcherChange(const bool state);
    //通用设置发生改变
    void slotSettingChange();

    //定时判断日历界面是否打开
    void slotClientIsOpen();

private:
    SyncFileManage *m_syncFileManage = nullptr;
    DAccountManagerDataBase::Ptr m_accountManagerDB;
    DAccount::List m_accountList;
    QMap<QString, DAccountModule::Ptr> m_accountModuleMap;
    QMap<QString, DAccountService::Ptr> m_AccountServiceMap[accountTypeCount];
    DCalendarGeneralSettings::Ptr m_generalSetting;
    DTK_CORE_NAMESPACE::DConfig *m_reginFormatConfig;
    QTimer m_timer;
    bool m_isSupportUid = false;
    QSettings m_settings;
    DBusTimedate m_timeDateDbus;
};

#endif // DACCOUNTMANAGEMODULE_H
