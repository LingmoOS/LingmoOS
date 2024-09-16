// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbustartmanager.h"

/*
 * Implementation of interface class DBusStartManager
 */

DBusStartManager::DBusStartManager(QObject *parent)
    : QDBusAbstractInterface("com.deepin.SessionManager", "/com/deepin/StartManager", staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged", "sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DBusStartManager::~DBusStartManager()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}
