// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "commondef.h"
#include "dcalendargeneralsettings.h"
#include <QDBusInterface>
#include <QDebug>

DbusAccountManagerRequest::DbusAccountManagerRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/AccountManager", "com.deepin.dataserver.Calendar.AccountManager", QDBusConnection::sessionBus(), parent)
{

}

/**
 * @brief setFirstDayofWeek
 * 设置一周首日
 */
void DbusAccountManagerRequest::setFirstDayofWeek(int value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("firstDayOfWeek", QVariant(value));
}

/**
 * @brief DbusAccountManagerRequest::setTimeFormatType
 * 设置时间显示格式
 */
void DbusAccountManagerRequest::setTimeFormatType(int value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("timeFormatType", QVariant(value));
}

/**
 * @brief setFirstDayofWeekSource
 * 设置一周首日来源
 */
void DbusAccountManagerRequest::setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("firstDayOfWeekSource", QVariant(value));
}

/**
 * @brief setTimeFormatTypeSource
 * 设置时间显示格式来源
 */
void DbusAccountManagerRequest::setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("timeFormatTypeSource", QVariant(value));
}

/**
 * @brief getFirstDayofWeekSource
 * 获取一周首日来源
 */
DCalendarGeneralSettings::GeneralSettingSource DbusAccountManagerRequest::getFirstDayofWeekSource()
{
    QDBusInterface interface(this->service(),
                             this->path(),
                             this->interface(),
                             QDBusConnection::sessionBus(),
                             this);
    bool ok;
    auto val = interface.property("firstDayOfWeekSource").toInt(&ok);
    if (ok) {
        return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val);
    } else {
        return DCalendarGeneralSettings::Source_Database;
    }
}

/**
 * @brief getTimeFormatTypeSource
 * 获取时间显示格式来源
 */
DCalendarGeneralSettings::GeneralSettingSource DbusAccountManagerRequest::getTimeFormatTypeSource()
{
    QDBusInterface interface(this->service(),
                             this->path(),
                             this->interface(),
                             QDBusConnection::sessionBus(),
                             this);
    bool ok;
    auto val = interface.property("timeFormatTypeSource").toInt(&ok);
    if (ok) {
        return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val);
    } else {
        return DCalendarGeneralSettings::Source_Database;
    }
}

/**
 * @brief DbusAccountManagerRequest::getAccountList
 * 请求帐户列表
 */
void DbusAccountManagerRequest::getAccountList()
{
    asyncCall("getAccountList");
}

/**
 * @brief DbusAccountManagerRequest::downloadByAccountID
 * 根据帐户id下拉数据
 * @param accountID 帐户id
 */
void DbusAccountManagerRequest::downloadByAccountID(const QString &accountID)
{
    asyncCall("downloadByAccountID", QVariant(accountID));
}

/**
 * @brief DbusAccountManagerRequest::uploadNetWorkAccountData
 * 更新网络帐户数据
 */
void DbusAccountManagerRequest::uploadNetWorkAccountData()
{
    asyncCall("uploadNetWorkAccountData");
}

/**
 * @brief DbusAccountManagerRequest::getCalendarGeneralSettings
 * 获取通用设置
 */
void DbusAccountManagerRequest::getCalendarGeneralSettings()
{
    asyncCall("getCalendarGeneralSettings");
}

/**
 * @brief DbusAccountManagerRequest::login
 * 帐户登录
 */
void DbusAccountManagerRequest::login()
{
    asyncCall("login");
}

/**
 * @brief DbusAccountManagerRequest::loginout
 * 帐户登出
 */
void DbusAccountManagerRequest::logout()
{
    asyncCall("logout");
}

void DbusAccountManagerRequest::clientIsShow(bool isShow)
{
    QList<QVariant> argumentList;
    argumentList << isShow;
    //不需要返回结果，发送完直接结束
    callWithArgumentList(QDBus::NoBlock, QStringLiteral("calendarIsShow"), argumentList);
}

bool DbusAccountManagerRequest::getIsSupportUid()
{
    QList<QVariant> argumentList;
    //
    QDBusMessage  msg = callWithArgumentList(QDBus::Block, QStringLiteral("isSupportUid"), argumentList);
    if (msg.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = msg.arguments().first();
        return variant.toBool();
    } else {
        return false;
    }
}

/**
 * @brief DbusAccountManagerRequest::slotCallFinished
 * dbus调用完成事件
 * @param call 回调类
 */
void DbusAccountManagerRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    int ret = 0;
    bool canCall = true;
    //错误处理
    if (call->isError()) {
        //打印错误信息
        qCWarning(ClientLogger) << call->reply().member() << call->error().message();
        ret = 1;
    } else if (call->getmember() == "getAccountList") {
        //"getAccountList"方法回调事件
        QDBusPendingReply<QString> reply = *call;
        //获取返回值
        QString str = reply.argumentAt<0>();
        DAccount::List accountList;
        //解析字符串
        if (DAccount::fromJsonListString(accountList, str)) {
            emit signalGetAccountListFinish(accountList);
        } else {
            qCWarning(ClientLogger) <<"AccountList Parsing failed!";
            ret = 2;
        }
    } else if (call->getmember() == "getCalendarGeneralSettings") {
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
        DCalendarGeneralSettings::Ptr ptr;
        ptr.reset(new DCalendarGeneralSettings());
        if (DCalendarGeneralSettings::fromJsonString(ptr, str)) {
            emit signalGetGeneralSettingsFinish(ptr);
        } else {
            qCWarning(ClientLogger) <<"AccountList Parsing failed!";
            ret = 2;
        }
    } else if (call->getmember() == "setCalendarGeneralSettings") {
        canCall = false;
        setCallbackFunc(call->getCallbackFunc());
        getCalendarGeneralSettings();
    }

    //执行回调函数
    if (canCall && call->getCallbackFunc() != nullptr) {
        call->getCallbackFunc()({ret, ""});
    }
    //释放内存
    call->deleteLater();
}

void DbusAccountManagerRequest::slotDbusCall(const QDBusMessage &msg)
{
    if (msg.member() == "accountUpdate") {
        getAccountList();
    }else if (msg.member() == "PropertiesChanged") {
        QDBusPendingReply<QString, QVariantMap, QStringList> reply = msg;
        onPropertiesChanged(reply.argumentAt<0>(), reply.argumentAt<1>(), reply.argumentAt<2>());
    }
}

void DbusAccountManagerRequest::onPropertiesChanged(const QString &, const QVariantMap &changedProperties, const QStringList &)
{
    for (QVariantMap::const_iterator it = changedProperties.cbegin(), end = changedProperties.cend(); it != end; ++it) {
        if (it.key() == "firstDayOfWeek") {
            getCalendarGeneralSettings();
        } else if (it.key() == "timeFormatType") {
            getCalendarGeneralSettings();
        }
    }
}
