// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <qdbusargument.h>
#include <qdbusextratypes.h>
#include <qdbusmetatype.h>
#include <qglobal.h>
#include <qlist.h>
#include <qmetatype.h>
#include <qnamespace.h>

DLOGIN_BEGIN_NAMESPACE

struct DBusScheduledShutdownValue
{
    QString type;
    quint64 usec;
    static void registerMetaType();
    inline friend bool operator==(const DBusScheduledShutdownValue &lhs, const DBusScheduledShutdownValue &rhs)
    {
        return lhs.type == rhs.type && lhs.usec == rhs.usec;
    }
    inline friend bool operator!=(const DBusScheduledShutdownValue &lhs, const DBusScheduledShutdownValue &rhs)
    {
        return !(lhs == rhs);
    }
};

struct DBusInhibitor
{
    QString what;
    QString who;
    QString why;
    QString mode;
    quint32 UID;
    quint32 PID;
    static void registerMetaType();
};

struct DBusSeat
{
    QString seatId;
    QDBusObjectPath path;
    static void registerMetaType();
    inline friend bool operator==(const DBusSeat &lhs, const DBusSeat &rhs)
    {
        return lhs.seatId == rhs.seatId && lhs.path == rhs.path;
    }
    inline friend bool operator!=(const DBusSeat &lhs, const DBusSeat &rhs)
    {
        return !(lhs == rhs);
    }
};

struct DBusSeatPath
{
    QString seatId;
    QDBusObjectPath path;
    static void registerMetaType();
    inline friend bool operator==(const DBusSeatPath &lhs, const DBusSeatPath &rhs)
    {
        return lhs.path == rhs.path && lhs.seatId == rhs.seatId;
    }
    inline friend bool operator!=(const DBusSeatPath &lhs, const DBusSeatPath &rhs)
    {
        return !(lhs == rhs);
    }
};

struct DBusSession
{
    QString sessionId;
    quint32 userId;
    QString userName;
    QString seatId;
    QDBusObjectPath path;
    static void registerMetaType();
};

struct DBusUser
{
    quint32 userId;
    QString userName;
    QDBusObjectPath path;
    static void registerMetaType();
};

struct DBusSessionPath
{
    QString sessionId;
    QDBusObjectPath path;
    static void registerMetaType();
    inline friend bool operator==(const DBusSessionPath &lhs, const DBusSessionPath &rhs)
    {
        return lhs.path == rhs.path && lhs.sessionId == rhs.sessionId;
    }
    inline friend bool operator!=(const DBusSessionPath &lhs, const DBusSessionPath &rhs)
    {
        return !(lhs == rhs);
    }
};

struct DBusUserPath
{
    uint userId;
    QDBusObjectPath path;
    static void registerMetaType();
    inline friend bool operator==(const DBusUserPath &lhs, const DBusUserPath &rhs)
    {
        return lhs.path == rhs.path && lhs.userId == rhs.userId;
    }
    inline friend bool operator!=(const DBusUserPath &lhs, const DBusUserPath &rhs)
    {
        return !(lhs == rhs);
    }
};

QDBusArgument &operator<<(QDBusArgument &arg, const DBusScheduledShutdownValue &value);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusScheduledShutdownValue &value);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusInhibitor &inhibitor);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusInhibitor &inhibitor);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusSeat &seat);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusSeat &seat);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusSeatPath &seat);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusSeatPath &seat);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusSession &session);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusSession &session);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusUser &user);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusUser &user);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusSessionPath &path);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusSessionPath &path);
QDBusArgument &operator<<(QDBusArgument &arg, const DBusUserPath &path);
const QDBusArgument &operator>>(const QDBusArgument &arg, DBusUserPath &path);

DLOGIN_END_NAMESPACE
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusScheduledShutdownValue)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusInhibitor)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusSeat)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusSeatPath)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusSession)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusUser)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusSessionPath)
Q_DECLARE_METATYPE(DTK_LOGIN_NAMESPACE::DBusUserPath)
