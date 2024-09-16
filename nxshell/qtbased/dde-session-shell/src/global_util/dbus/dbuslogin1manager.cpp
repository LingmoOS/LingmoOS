// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuslogin1manager.h"

/*
 * Implementation of interface class DBusLogin1Manager
 */

DBusLogin1Manager::DBusLogin1Manager(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    QDBusConnection::systemBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
    Inhibit::registerMetaType();
    UserInfo::registerMetaType();
    SeatInfo::registerMetaType();
    SessionInfo::registerMetaType();

}

DBusLogin1Manager::~DBusLogin1Manager()
{
    QDBusConnection::systemBus().disconnect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

