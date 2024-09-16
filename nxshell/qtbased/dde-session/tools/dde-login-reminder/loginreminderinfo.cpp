// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginreminderinfo.h"

void registerLoginReminderInfoMetaType()
{
    qRegisterMetaType<LoginUtmpx>("LoginUtmpx");
    qDBusRegisterMetaType<LoginUtmpx>();

    qRegisterMetaType<Spent>("Spent");
    qDBusRegisterMetaType<Spent>();

    qRegisterMetaType<LoginReminderInfo>("LoginReminderInfo");
    qDBusRegisterMetaType<LoginReminderInfo>();
}

bool Spent::operator ==(const Spent &other)
{
    return (LastChange == other.LastChange)
            && (Min == other.Min)
            && (Max == other.Max)
            && (Warn == other.Warn)
            && (Inactive == other.Inactive)
            && (Expire == other.Expire);
}

bool LoginUtmpx::operator ==(const LoginUtmpx &other)
{
    return (InittabID == other.InittabID)
            && (Line == other.Line)
            && (Host == other.Host)
            && (Address == other.Address)
            && (Time == other.Time);
}

bool LoginReminderInfo::operator ==(const LoginReminderInfo &other)
{
    return (Username == other.Username)
            && (spent == other.spent)
            && (CurrentLogin == other.CurrentLogin)
            && (LastLogin == other.LastLogin)
            && (FailCountSinceLastLogin == other.FailCountSinceLastLogin);
}

QDBusArgument &operator<<(QDBusArgument &arg, const Spent &other)
{
    arg.beginStructure();
    arg << other.LastChange << other.Min << other.Max << other.Warn << other.Inactive << other.Expire;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Spent &other)
{
    arg.beginStructure();
    arg >> other.LastChange >> other.Min >> other.Max >> other.Warn >> other.Inactive >> other.Expire;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const LoginUtmpx &other)
{
    arg.beginStructure();
    arg << other.InittabID << other.Line << other.Host << other.Address<< other.Time;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginUtmpx &other)
{
    arg.beginStructure();
    arg >> other.InittabID >> other.Line >> other.Host >> other.Address >> other.Time;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const LoginReminderInfo &other)
{
    arg.beginStructure();
    arg << other.Username << other.spent << other.CurrentLogin
        << other.LastLogin << other.FailCountSinceLastLogin;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, LoginReminderInfo &other)
{
    arg.beginStructure();
    arg >> other.Username >> other.spent >> other.CurrentLogin
            >> other.LastLogin >> other.FailCountSinceLastLogin;
    arg.endStructure();
    return arg;
}
