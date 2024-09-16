// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amdbuslauncherframe.h"

/*
 * Implementation of interface class DBusLauncherFrame
 */

AMDBusLauncherFrame::AMDBusLauncherFrame(QObject *parent)
    : QDBusAbstractInterface(INTERFACE_NAME, SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

AMDBusLauncherFrame::~AMDBusLauncherFrame()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties", "PropertiesChanged",  "sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

