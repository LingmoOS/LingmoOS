// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTITEM_H
#define ACCOUNTITEM_H

#include "dbus/dbusaccountrequest.h"
#include <QObject>

//单项帐户信息管理类
class AccountItem : public QObject
{
    Q_OBJECT
public:
    explicit AccountItem(const DAccount::Ptr &account, QObject *parent = nullptr);

    typedef QSharedPointer<AccountItem> Ptr;

    //同步状态码转状态说明
    static QString getSyncMsg(DAccount::AccountSyncState code);

    void resetAccount();

    //获取帐户数据
    DAccount::Ptr getAccount();

    //获取日程
    QMap<QDate, DSchedule::List> getScheduleMap();
    QMap<QDate, DSchedule::List> getSearchScheduleMap();

    // 获取日程类型信息集
    DScheduleType::List getScheduleTypeList();

    //根据类型ID获取日程类型
    DScheduleType::Ptr getScheduleTypeByID(const QString &typeID);
    DScheduleType::Ptr getScheduleTypeByName(const QString &typeName);

    //获取颜色类型列表
    DTypeColor::List getColorTypeList();

    //获取日程是否可以同步
    bool isCanSyncShedule();
    //获取通用设置是否可以同步
    bool isCanSyncSetting();

    //获取uos账号是否enable
    bool isEnableForUosAccount();

    //更新帐户列表展开状态
    void setAccountExpandStatus(bool expandStatus);

    //设置帐号状态
    void setAccountState(DAccount::AccountStates state);
    //设置同步频率
    void setSyncFreq(DAccount::SyncFreqType freq);
    //获取帐号状态
    DAccount::AccountStates getAccountState();
    //获取同步状态
    bool getSyncState();
    //获取同步频率
    DAccount::SyncFreqType getSyncFreq();

    //创建日程类型
    void createJobType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //更新类型信息
    void updateScheduleType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //更新类型显示状态
    void updateScheduleTypeShowState(const DScheduleType::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //根据类型ID删除日程类型
    void deleteScheduleTypeByID(const QString &typeID, CallbackFunc callback = nullptr);

    //类型是否被日程使用
    bool scheduleTypeIsUsed(const QString &typeID);

    //创建日程
    void createSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //更新日程
    void updateSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //根据日程ID获取日程
    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);

    //根据日程ID删除日程
    void deleteScheduleByID(const QString &scheduleID, CallbackFunc callback = nullptr);

    //根据类型ID删除日程
    void deleteSchedulesByTypeID(const QString &typeID, CallbackFunc callback = nullptr);

    //根据查询条件查询数据
    void querySchedulesWithParameter(const int year, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const QDateTime &start, const QDateTime &end, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const QString &key, const QDateTime &start, const QDateTime &end, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const DScheduleQueryPar::Ptr &, CallbackFunc callback = nullptr);

    //对外请求接口，同步
    QString querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end);
    bool querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end, QMap<QDate, DSchedule::List>& out);
    // 导入日程
    void importSchedule(QString icsFilePath, QString TypeID, bool cleanExists, CallbackFunc func);
    // 导出日程
    void exportSchedule(QString icsFilePath, QString TypeID, CallbackFunc func);

signals:
    void signalAccountDataUpdate();
    void signalScheduleUpdate();
    void signalScheduleTypeUpdate();
    void signalSearchScheduleUpdate();
    void signalLogout(DAccount::Type);
    void signalDtLastUpdate(QString);
    void signalAccountStateChange();
    void signalSyncStateChange(DAccount::AccountSyncState);

public slots:
    //获取帐户信息完成事件
    void slotGetAccountInfoFinish(DAccount::Ptr);
    //获取日程类型数据完成事件
    void slotGetScheduleTypeListFinish(DScheduleType::List);
    //获取日程数据完成事件
    void slotGetScheduleListFinish(QMap<QDate, DSchedule::List>);
    //搜索日程数据完成事件
    void slotSearchScheduleListFinish(QMap<QDate, DSchedule::List>);
    //获取系统颜色完成
    void slotGetSysColorsFinish(DTypeColor::List);
    //帐户状态改变事件
    void slotAccountStateChange(DAccount::AccountStates);
    //同步状态改变事件
    void slotSyncStateChange(DAccount::AccountSyncState);
    //获取最后一次同步时间
    QString getDtLastUpdate();

    void slotSearchUpdata();
private:
    void initConnect();

private:
    DAccount::Ptr m_account;    //帐户数据
    DScheduleType::List m_scheduleTypeList; //日程类型数据
    DTypeColor::List m_typeColorList;    //颜色数据
    DbusAccountRequest *m_dbusRequest = nullptr; //dbus请求实例
    //一年的日程信息
    QMap<QDate, DSchedule::List> m_scheduleMap{};
    //搜索的日程信息
    QMap<QDate, DSchedule::List> m_searchedScheduleMap{};

    QMap<QString, bool> m_dataStatus;   //数据状态
    DScheduleQueryPar::Ptr    m_preQuery = nullptr;       //上一次查询

};

#endif // ACCOUNTITEM_H
