// SPDX-FileCopyrightText: 2014 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_daemon_interface.h"

/*
 * Implementation of interface class DBusDaemonInterface
 */

DBusDaemonInterface::DBusDaemonInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DBusDaemonInterface::~DBusDaemonInterface()
{
}

