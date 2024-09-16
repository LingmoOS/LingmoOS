// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkaccounts_global.h"

#include <QString>

DACCOUNTS_BEGIN_NAMESPACE

// Common Custom Type

enum class AccountTypes : qint32 { Default = 0, Admin, Udcp, Unknown };

enum class PasswdStatus { Password, NoPassword, Locked, Unknown };

enum class PasswdExpirInfo { Normal, Closed, Expired, Unknown };

struct ShadowInfo
{
    qint32 lastChange;
    qint32 min;
    qint32 max;
    qint32 warn;
    qint32 inactive;
    qint32 expired;
    friend bool operator==(const ShadowInfo &lhs, const ShadowInfo &rhs)
    {
        return lhs.lastChange == rhs.lastChange && lhs.min == rhs.min && lhs.max == rhs.max
            && lhs.warn == rhs.warn && lhs.inactive == rhs.inactive && lhs.expired == rhs.expired;
    }
};

struct LoginUtmpx
{
    QByteArray inittabID;
    QByteArray line;
    QByteArray host;
    QByteArray address;
    QByteArray time;
    friend bool operator==(const LoginUtmpx &lhs, const LoginUtmpx &rhs)
    {
        return lhs.inittabID == rhs.inittabID && lhs.line == rhs.line && lhs.host == rhs.host
            && lhs.address == rhs.address && lhs.time == rhs.time;
    }
};

struct ReminderInfo
{
    QByteArray userName;
    ShadowInfo spent;
    LoginUtmpx currentLogin;
    LoginUtmpx lastLogin;
    qint32 failCountSinceLastLogin;
    friend bool operator==(const ReminderInfo &lhs, const ReminderInfo &rhs)
    {
        return lhs.userName == rhs.userName && lhs.spent == rhs.spent
            && lhs.currentLogin == rhs.currentLogin && lhs.lastLogin == rhs.lastLogin
            && lhs.failCountSinceLastLogin == rhs.failCountSinceLastLogin;
    }
};

struct ValidMsg
{
    bool valid;
    qint32 code;
    QString msg;
    friend bool operator==(const ValidMsg &lhs, const ValidMsg &rhs)
    {
        return lhs.valid == rhs.valid && lhs.code == rhs.code && lhs.msg == rhs.msg;
    }
};

DACCOUNTS_END_NAMESPACE
