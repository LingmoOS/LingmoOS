// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dnotificationtypes_p.h"
#include <qdbusmetatype.h>

DNOTIFICATIONS_BEGIN_NAMESPACE

void registerMetaType()
{
    qRegisterMetaType<DBusServerInformation>("DBusServerInformation");
    qDBusRegisterMetaType<DBusServerInformation>();
    qDBusRegisterMetaType<QList<DBusServerInformation>>();
}
Q_CONSTRUCTOR_FUNCTION(registerMetaType)

QDBusArgument &operator<<(QDBusArgument &arg, const DBusServerInformation &serverInformation)
{
    arg.beginStructure();
    arg << serverInformation.name;
    arg << serverInformation.vendor;
    arg << serverInformation.version;
    arg << serverInformation.spec_version;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, DBusServerInformation &serverInformation)
{
    arg.beginStructure();
    arg >> serverInformation.name;
    arg >> serverInformation.vendor;
    arg >> serverInformation.version;
    arg >> serverInformation.spec_version;
    arg.endStructure();
    return arg;
}
DNOTIFICATIONS_END_NAMESPACE
