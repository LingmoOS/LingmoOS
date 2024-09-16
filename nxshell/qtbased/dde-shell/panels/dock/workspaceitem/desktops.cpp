// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktops.h"

const QDBusArgument &operator<<(QDBusArgument &argument, const DBusDesktopDataStruct &desk)
{
    argument.beginStructure();
    argument << desk.position;
    argument << desk.id;
    argument << desk.name;
    argument.endStructure();
    return argument;
}
// Retrieve
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusDesktopDataStruct &desk)
{
    argument.beginStructure();
    argument >> desk.position;
    argument >> desk.id;
    argument >> desk.name;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator<<(QDBusArgument &argument, const DBusDesktopDataVector &deskVector)
{
    argument.beginArray(qMetaTypeId<DBusDesktopDataStruct>());
    for (int i = 0; i < deskVector.size(); ++i) {
        argument << deskVector[i];
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusDesktopDataVector &deskVector)
{
    argument.beginArray();
    deskVector.clear();

    while (!argument.atEnd()) {
        DBusDesktopDataStruct element;
        argument >> element;
        deskVector.append(element);
    }

    argument.endArray();

    return argument;
}

