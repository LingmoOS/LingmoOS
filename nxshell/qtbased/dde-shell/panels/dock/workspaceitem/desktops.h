// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOPS_H
#define DESKTOPS_H

#include <QtDBus>

struct DBusDesktopDataStruct {
    uint position;
    QString id;
    QString name;
    DBusDesktopDataStruct() {
        position = 0;
    }
};
typedef QVector<DBusDesktopDataStruct> DBusDesktopDataVector;

const QDBusArgument &operator<<(QDBusArgument &argument, const DBusDesktopDataStruct &desk);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusDesktopDataStruct &desk);

Q_DECLARE_METATYPE(DBusDesktopDataStruct)

const QDBusArgument &operator<<(QDBusArgument &argument, const DBusDesktopDataVector &deskVector);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusDesktopDataVector &deskVector);

Q_DECLARE_METATYPE(DBusDesktopDataVector)

#endif // DESKTOPS_H
