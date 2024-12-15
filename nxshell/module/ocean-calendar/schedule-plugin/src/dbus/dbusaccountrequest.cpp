// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountrequest.h"
#include "commondef.h"

#include <QDBusReply>

DbusAccountRequest::DbusAccountRequest(const QString &path, const QString &interface, QObject *parent)
    : DbusRequestBase(path, interface, QDBusConnection::sessionBus(), parent)
{
}

/**
 * @brief getAccountInfo        获取帐户信息
 * @return
 */
void DbusAccountRequest::getAccountInfo()
{
    asyncCall("getAccountInfo");
}

void DbusAccountRequest::updateAccountInfo(const DAccount::Ptr &account)
{
    QString jsonStr;
    DAccount::toJsonString(account, jsonStr);
    asyncCall("updateAccountInfo", QVariant(jsonStr));
}

/**
 * @brief getScheduleTypeList      获取日程类型信息集
 * @return
 */
DScheduleType::List DbusAccountRequest::getScheduleTypeList()
{
    DScheduleType::List typeList;
    QList<QVariant> argumentList;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getScheduleTypeList"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(CommonLogger) << "getScheduleTypeList error ," << reply;
        return typeList;
    }
    QDBusReply<QString> scheduleReply = reply;
    DScheduleType::fromJsonListString(typeList, scheduleReply.value());
    return typeList;
}

/**
 * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
 * @param typeID                日程类型ID
 * @return
 */
void DbusAccountRequest::getScheduleTypeByID(const QString &typeID)
{
    asyncCall("getScheduleTypeByID", QVariant(typeID));
}

/**
 * @brief createScheduleType         创建日程类型
 * @param typeInfo              类型信息
 * @return                      日程类型ID
 */
void DbusAccountRequest::createScheduleType(const DScheduleType::Ptr &typeInfo)
{
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("createScheduleType", QVariant(jsonStr));
}

/**
 * @brief updateScheduleType         更新日程类型
 * @param typeInfo              类型信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateScheduleType(const DScheduleType::Ptr &typeInfo)
{
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("updateScheduleType", QVariant(jsonStr));
}

/**
 * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
 * @param typeID                日程类型ID
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::deleteScheduleTypeByID(const QString &typeID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleTypeByID", QVariant(typeID));
}

/**
 * @brief scheduleTypeByUsed         日程类型是否被使用
 * @param typeID                日程类型ID
 * @return
 */
void DbusAccountRequest::scheduleTypeByUsed(const QString &typeID)
{
    asyncCall("scheduleTypeByUsed", QVariant(typeID));
}

/**
 * @brief createSchedule             创建日程
 * @param ScheduleInfo               日程信息
 * @return                      返回日程ID
 */
QString DbusAccountRequest::createSchedule(const DSchedule::Ptr &scheduleInfo)
{
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jsonStr);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("createSchedule"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(CommonLogger) << "getScheduleTypeByID error ," << reply;
        return nullptr;
    }
    QDBusReply<QString> scheduleReply = reply;
    return scheduleReply.value();
}

/**
 * @brief updateSchedule             更新日程
 * @param ScheduleInfo               日程信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateSchedule(const DSchedule::Ptr &scheduleInfo)
{
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);
    asyncCall("updateSchedule", QVariant(jsonStr));
}

DSchedule::Ptr DbusAccountRequest::getScheduleByID(const QString &scheduleID)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(scheduleID);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getScheduleByScheduleID"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(CommonLogger) << "getScheduleTypeByID error ," << reply;
        return nullptr;
    }
    QDBusReply<QString> scheduleReply = reply;

    QString scheduleStr = scheduleReply.value();
    DSchedule::Ptr schedule;
    DSchedule::fromJsonString(schedule, scheduleStr);
    return schedule;
}

/**
 * @brief deleteScheduleByScheduleID      根据日程ID删除日程
 * @param ScheduleID                 日程ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteScheduleByScheduleID(const QString &scheduleID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleByScheduleID", QVariant(scheduleID));
}

/**
 * @brief deleteSchedulesByScheduleTypeID 根据日程类型ID删除日程
 * @param typeID                日程类型ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteSchedulesByScheduleTypeID(const QString &typeID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteSchedulesByScheduleTypeID", QVariant(typeID));
}

/**
 * @brief querySchedulesWithParameter        根据查询参数查询日程
 * @param params                        具体的查询参数
 * @return                              查询到的日程集
 */
DSchedule::Map DbusAccountRequest::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params)
{
    DSchedule::Map scheduleMap;
    QList<QVariant> argumentList;
    QString jsonStr = DScheduleQueryPar::toJsonString(params);
    argumentList << jsonStr;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("querySchedulesWithParameter"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(CommonLogger) << "getSysColors error ," << reply;
        return scheduleMap;
    }
    QDBusReply<QString> scheduleReply = reply;
    scheduleMap = DSchedule::fromMapString(scheduleReply.value());
    return scheduleMap;
}

DTypeColor::List DbusAccountRequest::getSysColors()
{
    DTypeColor::List colorList;
    QList<QVariant> argumentList;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getSysColors"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(CommonLogger) << "getSysColors error ," << reply;
        return colorList;
    }
    QDBusReply<QString> scheduleReply = reply;
    colorList = DTypeColor::fromJsonString(scheduleReply.value());
    return colorList;
}

void DbusAccountRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    int ret = 0;
    bool canCall = true;
    QString msg = "";

    if (call->isError()) {
        qCWarning(CommonLogger) << call->reply().member() << call->error().message();
        ret = 1;
    } else {
        QDBusPendingReply<QVariant> reply = *call;
        QVariant str = reply.argumentAt<0>();
        if (call->getmember() == "getAccountInfo") {
            DAccount::Ptr ptr;
            ptr.reset(new DAccount());
            if (DAccount::fromJsonString(ptr, str.toString())) {
                emit signalGetAccountInfoFinish(ptr);
            } else {
                qCWarning(CommonLogger) << "AccountInfo Parsing failed!";
                ret = 2;
            }
        } else if (call->getmember() == "getScheduleTypeList") {
            DScheduleType::List stList;
            if (DScheduleType::fromJsonListString(stList, str.toString())) {
                emit signalGetScheduleTypeListFinish(stList);
            } else {
                qCWarning(CommonLogger) << "ScheduleTypeList Parsing failed!";
                ret = 2;
            }
        } else if (call->getmember() == "querySchedulesWithParameter") {
            QMap<QDate, DSchedule::List> map = DSchedule::fromMapString(str.toString());
            emit signalGetScheduleListFinish(map);
        } else if (call->getmember() == "searchSchedulesWithParameter") {
            QMap<QDate, DSchedule::List> map = DSchedule::fromMapString(str.toString());
            emit signalSearchScheduleListFinish(map);
        } else if (call->getmember() == "getSysColors") {
            DTypeColor::List list = DTypeColor::fromJsonString(str.toString());
            emit signalGetSysColorsFinish(list);
        } else if (call->getmember() == "createScheduleType") {
            //创建日程类型结束
            canCall = false;
            //在发起数据获取刷新数据，并将本回调函数和数据传到下一个事件中
            CallbackFunc func = call->getCallbackFunc();
            setCallbackFunc([=](CallMessge) {
                func({0, str.toString()});
            });
            getScheduleTypeList();
        } else if (call->getmember() == "createSchedule") {
            //创建日程结束
            //            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
            msg = str.toString();

        } else if (call->getmember() == "deleteScheduleByScheduleID") {
            //删除日程结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        } else if (call->getmember() == "getScheduleByID") {
            DSchedule::Ptr schedule(new DSchedule);
            DSchedule::fromJsonString(schedule, str.toString());
            emit signalGetScheduleFinish(schedule);
        }

        if (canCall && call->getCallbackFunc() != nullptr) {
            call->getCallbackFunc()({ret, msg});
        }
    }
    call->deleteLater();
}
