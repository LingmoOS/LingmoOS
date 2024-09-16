// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "commondef.h"
#include <QDebug>

DbusAccountManagerRequest::DbusAccountManagerRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/AccountManager", "com.deepin.dataserver.Calendar.AccountManager", QDBusConnection::sessionBus(), parent)
{
}

/**
 * @brief DbusAccountManagerRequest::getAccountList
 * 请求帐户列表
 */
DAccount::List DbusAccountManagerRequest::getAccountList()
{
    DAccount::List accountList;
    QList<QVariant> argumentList;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getAccountList"), argumentList);
    pCall.waitForFinished();
    QDBusPendingReply<QString> reply = pCall.reply();

    //获取返回值
    QString str = reply.argumentAt<0>();
    //解析字符串
    DAccount::fromJsonListString(accountList, str);
    return accountList;
}

/**
 * @brief DbusAccountManagerRequest::downloadByAccountID
 * 根据帐户id下拉数据
 * @param accountID 帐户id
 */
void DbusAccountManagerRequest::downloadByAccountID(const QString &accountID)
{
    QList<QVariant> argumentList;
    argumentList << QVariant(accountID);
    asyncCall("downloadByAccountID", argumentList);
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
 * @brief DbusAccountManagerRequest::setCalendarGeneralSettings
 * 设置通用设置
 * @param ptr   通用设置
 */
void DbusAccountManagerRequest::setCalendarGeneralSettings(DCalendarGeneralSettings::Ptr ptr)
{
    QString jsonStr;
    DCalendarGeneralSettings::toJsonString(ptr, jsonStr);
    asyncCall("setCalendarGeneralSettings", QVariant(jsonStr));
}

void DbusAccountManagerRequest::clientIsShow(bool isShow)
{
    QList<QVariant> argumentList;
    argumentList << isShow;
    //不需要返回结果，发送完直接结束
    callWithArgumentList(QDBus::NoBlock, QStringLiteral("calendarIsShow"), argumentList);
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
        qCWarning(CommonLogger) << call->reply().member() << call->error().message();
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
            qCWarning(CommonLogger)<< "AccountList Parsing failed!";
            ret = 2;
        }
    } else if (call->getmember() == "getCalendarGeneralSettings") {
        qCInfo(CommonLogger) << "getCalendarGeneralSettings";
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
        DCalendarGeneralSettings::Ptr ptr;
        ptr.reset(new DCalendarGeneralSettings());
        if (DCalendarGeneralSettings::fromJsonString(ptr, str)) {
            emit signalGetGeneralSettingsFinish(ptr);
        } else {
            qCWarning(CommonLogger) <<"AccountList Parsing failed!";
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
