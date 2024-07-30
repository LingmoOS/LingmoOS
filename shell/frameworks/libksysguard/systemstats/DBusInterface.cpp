/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "DBusInterface.h"

namespace KSysGuard
{
namespace SystemStats
{
DBusInterface::DBusInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : org::kde::ksystemstats1(service, path, connection, parent)
{
}

}

}
