/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "generictypes.h"

QDBusArgument &operator<<(QDBusArgument &argument, const IpV6DBusAddress &address)
{
    argument.beginStructure();
    argument << address.address << address.prefix << address.gateway;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IpV6DBusAddress &address)
{
    argument.beginStructure();
    argument >> address.address >> address.prefix >> address.gateway;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IpV6DBusRoute &route)
{
    argument.beginStructure();
    argument << route.destination << route.prefix << route.nexthop << route.metric;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IpV6DBusRoute &route)
{
    argument.beginStructure();
    argument >> route.destination >> route.prefix >> route.nexthop >> route.metric;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const NMStringMap &mydict)
{
    argument.beginMap(QMetaType::QString, QMetaType::QString);

    QMapIterator<QString, QString> i(mydict);
    while (i.hasNext()) {
        i.next();
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NMStringMap &mydict)
{
    argument.beginMap();
    mydict.clear();

    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mydict.insert(key, value);
    }

    argument.endMap();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DeviceDBusStateReason &reason)
{
    argument.beginStructure();
    argument << reason.state << reason.reason;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceDBusStateReason &reason)
{
    argument.beginStructure();
    argument >> reason.state >> reason.reason;
    argument.endStructure();
    return argument;
}
