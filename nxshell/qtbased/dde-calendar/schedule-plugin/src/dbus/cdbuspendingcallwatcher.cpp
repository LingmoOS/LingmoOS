// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdbuspendingcallwatcher.h"

CDBusPendingCallWatcher::CDBusPendingCallWatcher(const QDBusPendingCall &call, QString member, QObject *parent)
    : QDBusPendingCallWatcher(call, parent)
    , m_member(member)
{
    connect(this, &QDBusPendingCallWatcher::finished, this, [this]() {
        //转发调用完成事件
        emit this->signalCallFinished(this);
    });
}

/**
 * @brief CDBusPendingCallWatcher::setCallbackFunc
 * 设置回调函数
 * @param func 回调函数
 */
void CDBusPendingCallWatcher::setCallbackFunc(CallbackFunc func)
{
    m_func = func;
}

/**
 * @brief CDBusPendingCallWatcher::getCallbackFunc
 * 获取回调函数
 * @return 回调函数
 */
CallbackFunc CDBusPendingCallWatcher::getCallbackFunc()
{
    return m_func;
}

/**
 * @brief CDBusPendingCallWatcher::getmember
 * 设置调用方法名
 * @return 方法名
 */
QString CDBusPendingCallWatcher::getmember()
{
    return m_member;
}
