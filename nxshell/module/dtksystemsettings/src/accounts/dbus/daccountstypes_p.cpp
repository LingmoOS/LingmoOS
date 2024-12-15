// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountstypes_p.h"

#include <QDBusMetaType>

DACCOUNTS_BEGIN_NAMESPACE

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginHistory_p &history)
{
    arg.beginStructure();
    arg >> history.loginTime;
    arg >> history.logoutTime;
    arg >> history.sessionInfo;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const LoginHistory_p &history)
{
    arg.beginStructure();
    arg << history.loginTime;
    arg << history.logoutTime;
    arg << history.sessionInfo;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ReminderInfo_p &info)
{
    arg.beginStructure();
    arg >> info.userName;
    arg >> info.spent;
    arg >> info.currentLogin;
    arg >> info.lastLogin;
    arg >> info.failCountSinceLastLogin;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const ReminderInfo_p &info)
{
    arg.beginStructure();
    arg << info.userName;
    arg << info.spent;
    arg << info.currentLogin;
    arg << info.lastLogin;
    arg << info.failCountSinceLastLogin;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginUtmpx_p &info)
{
    arg.beginStructure();
    arg >> info.inittabID;
    arg >> info.line;
    arg >> info.host;
    arg >> info.address;
    arg >> info.time;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const LoginUtmpx_p &info)
{
    arg.beginStructure();
    arg << info.inittabID;
    arg << info.line;
    arg << info.host;
    arg << info.address;
    arg << info.time;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ShadowInfo_p &info)
{
    arg.beginStructure();
    arg >> info.lastChange;
    arg >> info.min;
    arg >> info.max;
    arg >> info.warn;
    arg >> info.inactive;
    arg >> info.expired;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const ShadowInfo_p &info)
{
    arg.beginStructure();
    arg << info.lastChange;
    arg << info.min;
    arg << info.max;
    arg << info.warn;
    arg << info.inactive;
    arg << info.expired;
    arg.endStructure();
    return arg;
}

void LoginHistory_p::registerMetaType()
{
    qRegisterMetaType<LoginHistory_p>("LoginHistory_p");
    qDBusRegisterMetaType<LoginHistory_p>();
    qDBusRegisterMetaType<QList<LoginHistory_p>>();
}

void ShadowInfo_p::registerMetaType()
{
    qRegisterMetaType<ShadowInfo_p>("ShadowInfo_p");
    qDBusRegisterMetaType<ShadowInfo_p>();
}

void ReminderInfo_p::registerMetaType()
{
    qRegisterMetaType<ReminderInfo_p>("ReminderInfo_p");
    qDBusRegisterMetaType<ReminderInfo_p>();
}

void LoginUtmpx_p::registerMetaType()
{
    qRegisterMetaType<LoginUtmpx_p>("LoginUtmpx_p");
    qDBusRegisterMetaType<LoginUtmpx_p>();
}

DACCOUNTS_END_NAMESPACE
