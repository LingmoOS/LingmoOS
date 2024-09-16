// Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSVARIANT
#define DBUSVARIANT

#include <QtCore>
#include <QtDBus>
#include <QtDBus/QDBusArgument>

class Inhibit
{
public:
    QString what;
    QString who;
    QString why;
    QString mode;
    quint32 uid;
    quint32 pid;

    Inhibit();
    ~Inhibit();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const Inhibit &obj);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibit &obj);
    static void registerMetaType();
};

class UsrInfo
{
public:
    qlonglong pid;
    QString id;
    QString userName;

    UsrInfo();
    ~UsrInfo();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const UsrInfo &obj);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, UsrInfo &obj);
    static void registerMetaType();
};

class SeatInfo
{
public:
    QString id;
    QString seat;

    SeatInfo();
    ~SeatInfo();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const SeatInfo &obj);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, SeatInfo &obj);
    static void registerMetaType();
};

class SessionInfo
{
public:
    QString session;
    qlonglong pid;
    QString user;
    QString id;
    QString seat;

    SessionInfo();
    ~SessionInfo();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const SessionInfo &obj);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, SessionInfo &obj);
    static void registerMetaType();
};

typedef QList<Inhibit> InhibitorsList;
Q_DECLARE_METATYPE(Inhibit)
Q_DECLARE_METATYPE(InhibitorsList)

typedef QList<UsrInfo> UserList;
Q_DECLARE_METATYPE(UsrInfo)
Q_DECLARE_METATYPE(UserList)

typedef QList<SeatInfo> SeatList;
Q_DECLARE_METATYPE(SeatInfo)
Q_DECLARE_METATYPE(SeatList)

typedef QList<SessionInfo> SessionList;
Q_DECLARE_METATYPE(SessionInfo)
Q_DECLARE_METATYPE(SessionList)
#endif // DBUSVARIANT
