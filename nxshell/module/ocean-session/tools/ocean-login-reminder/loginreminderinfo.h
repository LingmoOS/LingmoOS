// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGINREMINDERINFO_H
#define LOGINREMINDERINFO_H

#include <QDBusMetaType>
#include <QDBusObjectPath>
struct Spent{
    int LastChange;
    int Min;
    int Max;
    int Warn;
    int Inactive;
    int Expire;

    bool operator ==(const Spent &other);
};
Q_DECLARE_METATYPE(Spent)
QDBusArgument &operator<<(QDBusArgument &arg, const Spent &other);
const QDBusArgument &operator>>(const QDBusArgument &arg, Spent &other);

struct LoginUtmpx {
    QString InittabID;
    QString Line;
    QString Host;
    QString Address;
    QString Time;

    bool operator ==(const LoginUtmpx &other);
};
Q_DECLARE_METATYPE(LoginUtmpx)
QDBusArgument &operator<<(QDBusArgument &arg, const LoginUtmpx &other);
const QDBusArgument &operator>>(const QDBusArgument &arg, LoginUtmpx &other);

struct LoginReminderInfo {
    QString Username;
    Spent spent;
    LoginUtmpx CurrentLogin;
    LoginUtmpx LastLogin;
    int FailCountSinceLastLogin;

    bool operator ==(const LoginReminderInfo &other);
};
Q_DECLARE_METATYPE(LoginReminderInfo)
QDBusArgument &operator<<(QDBusArgument &arg, const LoginReminderInfo &other);
const QDBusArgument &operator>>(const QDBusArgument &arg, LoginReminderInfo &other);

void registerLoginReminderInfoMetaType();

#endif // LOGINREMINDERINFO_H
