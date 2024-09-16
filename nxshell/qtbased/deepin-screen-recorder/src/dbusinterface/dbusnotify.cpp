// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusnotify.h"

/*
 * Implementation of interface class DBusNotify
 */

const QString NOTIFY_DBUS_NAME = "org.freedesktop.Notifications";
const QString NOTIFY_DBUS_PATH = "/org/freedesktop/Notifications";

DBusNotify::DBusNotify(QObject *parent)
    : QDBusAbstractInterface(NOTIFY_DBUS_NAME, NOTIFY_DBUS_PATH,
                             staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(),
    "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as",
                                          this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusNotify::~DBusNotify()
{
    QDBusConnection::sessionBus().disconnect(service(), path(),
    "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as",
                                             this, SLOT(propertyChanged(QDBusMessage)));
}

