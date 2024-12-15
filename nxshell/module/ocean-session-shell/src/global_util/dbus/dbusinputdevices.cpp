// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusinputdevices.h"

/*
 * Implementation of interface class DBusInputDevices
 */

QDBusArgument &operator<<(QDBusArgument &argument, const InputDevice &device)
{
    argument.beginStructure();
    argument << device.interface << device.deviceType;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InputDevice &device)
{
    argument.beginStructure();
    argument >> device.interface >> device.deviceType;
    argument.endStructure();
    return argument;
}

DBusInputDevices::DBusInputDevices(QObject *parent)
    : QDBusAbstractInterface(staticServiceName(), staticObjectPath(), staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    qDBusRegisterMetaType<InputDevice>();
    qDBusRegisterMetaType<InputDeviceList>();

    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusInputDevices::~DBusInputDevices()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

