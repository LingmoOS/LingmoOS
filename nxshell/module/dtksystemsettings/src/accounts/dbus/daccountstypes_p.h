// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkaccounts_global.h"

#include <QDBusArgument>
#include <QMap>
#include <QVariant>

DACCOUNTS_BEGIN_NAMESPACE

struct LoginHistory_p
{
    qint64 loginTime;
    qint64 logoutTime;
    QMap<QString, QVariant> sessionInfo;
    static void registerMetaType();
};

struct ShadowInfo_p
{
    qint32 lastChange;
    qint32 min;
    qint32 max;
    qint32 warn;
    qint32 inactive;
    qint32 expired;
    static void registerMetaType();
};

struct LoginUtmpx_p
{
    QString inittabID;
    QString line;
    QString host;
    QString address;
    QString time;
    static void registerMetaType();
};

struct ReminderInfo_p
{
    QString userName;
    ShadowInfo_p spent;
    LoginUtmpx_p currentLogin;
    LoginUtmpx_p lastLogin;
    qint32 failCountSinceLastLogin;
    static void registerMetaType();
};

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginHistory_p &history);
QDBusArgument &operator<<(QDBusArgument &arg, const LoginHistory_p &history);

const QDBusArgument &operator>>(const QDBusArgument &arg, ReminderInfo_p &info);
QDBusArgument &operator<<(QDBusArgument &arg, const ReminderInfo_p &info);

const QDBusArgument &operator>>(const QDBusArgument &arg, ShadowInfo_p &info);
QDBusArgument &operator<<(QDBusArgument &arg, const ShadowInfo_p &info);

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginUtmpx_p &info);
QDBusArgument &operator<<(QDBusArgument &arg, const LoginUtmpx_p &info);
DACCOUNTS_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_ACCOUNTS_NAMESPACE::LoginHistory_p)
Q_DECLARE_METATYPE(DTK_ACCOUNTS_NAMESPACE::ReminderInfo_p)
Q_DECLARE_METATYPE(DTK_ACCOUNTS_NAMESPACE::ShadowInfo_p)
Q_DECLARE_METATYPE(DTK_ACCOUNTS_NAMESPACE::LoginUtmpx_p)
