// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusrequestbase.h"
#include "commondef.h"
#include <QDebug>

DbusRequestBase::DbusRequestBase(const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(DBUS_SERVER_NAME, path, interface.toStdString().c_str(), connection, parent)
{
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), this->interface(), "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qCWarning(CommonLogger) << "the connection was fail!"
                   << "path: " << this->path() << "interface: " << this->interface();
    };
}

void DbusRequestBase::setCallbackFunc(CallbackFunc func)
{
    m_callbackFunc = func;
}

/**
 * @brief DbusRequestBase::asyncCall
 * 异步访问dbus接口
 * @param method    dbus方法名
 * @param args  参数
 */
void DbusRequestBase::asyncCall(const QString &method, const QList<QVariant> &args)
{
    QDBusPendingCall async = QDBusAbstractInterface::asyncCall(method, args);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, method, this);
    //将回调函数放进CallWatcher中，随CallWatcher调用结果返回
    watcher->setCallbackFunc(m_callbackFunc);
    //清楚回调函数，防止多方法调用时混淆
    setCallbackFunc(nullptr);
    connect(watcher, &CDBusPendingCallWatcher::signalCallFinished, this, &DbusRequestBase::slotCallFinished);
}

/**
 * @brief DbusRequestBase::asyncCall
 * 异步访问dbus接口
 * @param method    dbus方法名
 * @param args  参数
 */
void DbusRequestBase::asyncCall(const QString &method,
                                const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4,
                                const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8)
{
    asyncCall(method, method, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

/**
 * @brief DbusRequestBase::asyncCall
 * 异步访问dbus接口
 * @param method    dbus方法名
 * @param args  参数
 */
void DbusRequestBase::asyncCall(const QString &method, QString callName,
                                const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4,
                                const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8)
{
    QDBusPendingCall async = QDBusAbstractInterface::asyncCall(method, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, callName, this);
    //将回调函数放进CallWatcher中，随CallWatcher调用结果返回
    watcher->setCallbackFunc(m_callbackFunc);
    //清楚回调函数，防止多方法调用时混淆
    setCallbackFunc(nullptr);
    connect(watcher, &CDBusPendingCallWatcher::signalCallFinished, this, &DbusRequestBase::slotCallFinished);
}

/**
 * @brief slotDbusCall
 * dbus服务端调用
 * @param msg 调用消息
 */
void DbusRequestBase::slotDbusCall(const QDBusMessage &msg)
{
}
