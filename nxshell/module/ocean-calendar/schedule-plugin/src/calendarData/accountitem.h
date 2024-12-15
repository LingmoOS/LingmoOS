// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTITEM_H
#define ACCOUNTITEM_H

#include "dbus/dbusaccountrequest.h"
#include <QObject>

//单项账户信息管理类
class AccountItem : public QObject
{
    Q_OBJECT
public:
    explicit AccountItem(const DAccount::Ptr &account, QObject *parent = nullptr);

    typedef std::function<void()> Func;
    typedef QSharedPointer<AccountItem> Ptr;

    void resetAccount();

    //获取账户数据
    DAccount::Ptr getAccount();

    //获取日程
    QMap<QDate, DSchedule::List> getScheduleMap();

    // 获取日程类型信息集
    DScheduleType::List getScheduleTypeList();

    //根据类型ID获取日程类型
    DScheduleType::Ptr getScheduleTypeByID(const QString &typeID);
    DScheduleType::Ptr getScheduleTypeByName(const QString &typeName);

    DSchedule::Ptr getScheduleByID(const QString &scheduleID);

    //获取颜色类型列表
    DTypeColor::List getColorTypeList();

    //更新账户信息
    void updateAccountInfo(CallbackFunc callback = nullptr);

    //创建日程类型
    void createJobType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //更新类型信息
    void updateScheduleType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //根据类型ID删除日程类型
    void deleteScheduleTypeByID(const QString &typeID, CallbackFunc callback = nullptr);

    //类型是否被日程使用
    void scheduleTypeIsUsed(const QString &typeID, CallbackFunc callback);

    //创建日程
    QString createSchedule(const DSchedule::Ptr &scheduleInfo);

    //更新日程
    void updateSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //根据日程ID删除日程
    void deleteScheduleByID(const QString &scheduleID, CallbackFunc callback = nullptr);

    //根据类型ID删除日程
    void deleteSchedulesByTypeID(const QString &typeID, CallbackFunc callback = nullptr);

    //根据查询条件查询数据
    DSchedule::Map querySchedulesWithParameter(const DScheduleQueryPar::Ptr &);

    //监听日程类型数据完成事件
    void monitorScheduleTypeData(Func callback);

signals:
    void signalAccountDataUpdate();
    void signalScheduleUpdate();
    void signalScheduleTypeUpdate();
    void signalSearchScheduleUpdate();
    void signalCreateFinish(const QString &scheduleID);
    void signalGetScheduleFinish(const DSchedule::Ptr &);

public slots:
    //获取账户信息完成事件
    void slotGetAccountInfoFinish(DAccount::Ptr);
    //获取日程类型数据完成事件
    void slotGetScheduleTypeListFinish(DScheduleType::List);
    //获取日程数据完成事件
    void slotGetScheduleListFinish(QMap<QDate, DSchedule::List>);
    //搜索日程数据完成事件
    void slotSearchScheduleListFinish(QMap<QDate, DSchedule::List>);
    //获取系统颜色完成
    void slotGetSysColorsFinish(DTypeColor::List);

private:
    void initConnect();

private:
    DAccount::Ptr m_account; //账户数据
    DScheduleType::List m_scheduleTypeList; //日程类型数据
    DTypeColor::List m_typeColorList; //颜色数据
    DbusAccountRequest *m_dbusRequest = nullptr; //dbus请求实例
    //一年的日程信息
    QMap<QDate, DSchedule::List> m_scheduleMap {};
    //搜索的日程信息
    QMap<QDate, DSchedule::List> m_searchedScheduleMap {};
    //一年是否含有日程
    QMap<QDate, bool> m_fullInfo {};
    //一年的班休信息
    QMap<QDate, int> m_festivalInfo {};
    QVector<DSchedule> m_searchScheduleInfoVector {};

    //回调函数
    QMap<QString, QList<Func>> m_callbackMap;
    QMap<QString, bool> m_dataStatus; //数据状态
};

#endif // ACCOUNTITEM_H
