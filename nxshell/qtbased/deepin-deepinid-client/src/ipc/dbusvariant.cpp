// Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusvariant.h"

Inhibit::Inhibit()
{
}
Inhibit::~Inhibit()
{
}

void Inhibit::registerMetaType()
{
    qRegisterMetaType<Inhibit>("Inhibit");
    qDBusRegisterMetaType<Inhibit>();
    qRegisterMetaType<InhibitorsList>("InhibitorsList");
    qDBusRegisterMetaType<InhibitorsList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const Inhibit &obj)
{
    argument.beginStructure();
    argument << obj.what << obj.who << obj.why << obj.mode << obj.uid << obj.pid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Inhibit &obj)
{
    argument.beginStructure();
    argument >> obj.what >> obj.who >> obj.why >> obj.mode >> obj.uid >> obj.pid;
    argument.endStructure();
    return argument;
}

UsrInfo::UsrInfo()
{
}
UsrInfo::~UsrInfo()
{
}

void UsrInfo::registerMetaType()
{
    qRegisterMetaType<UsrInfo>("UserInfo");
    qDBusRegisterMetaType<UsrInfo>();
    qRegisterMetaType<UserList>("UserList");
    qDBusRegisterMetaType<UserList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const UsrInfo &obj)
{
    argument.beginStructure();
    argument << obj.pid << obj.id << obj.userName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UsrInfo &obj)
{
    argument.beginStructure();
    argument >> obj.pid >> obj.id >> obj.userName;
    argument.endStructure();
    return argument;
}

SeatInfo::SeatInfo()
{
}
SeatInfo::~SeatInfo()
{
}

void SeatInfo::registerMetaType()
{
    qRegisterMetaType<SeatInfo>("SeatInfo");
    qDBusRegisterMetaType<SeatInfo>();
    qRegisterMetaType<SeatList>("SeatList");
    qDBusRegisterMetaType<SeatList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const SeatInfo &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.seat;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SeatInfo &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.seat;
    argument.endStructure();
    return argument;
}

SessionInfo::SessionInfo()
{
}
SessionInfo::~SessionInfo()
{
}

void SessionInfo::registerMetaType()
{
    qRegisterMetaType<SessionInfo>("SessionInfo");
    qDBusRegisterMetaType<SessionInfo>();
    qRegisterMetaType<SessionList>("SessionList");
    qDBusRegisterMetaType<SessionList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const SessionInfo &obj)
{
    argument.beginStructure();
    argument << obj.session << obj.pid << obj.user << obj.id << obj.seat;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, SessionInfo &obj)
{
    argument.beginStructure();
    argument >> obj.session >> obj.pid >> obj.user >> obj.id >> obj.seat;
    argument.endStructure();
    return argument;
}
