/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upower.h"
#include "upowerdbusinterface.h"

#include <QString>

using DBusInterface = Solid::Backends::UPower::DBusInterface;

namespace {

const auto upowerServiceName = QStringLiteral(UP_DBUS_SERVICE);
const auto upowerDBusPath = QStringLiteral(UP_DBUS_PATH);
const char* upowerInterfaceName = UP_DBUS_INTERFACE;

} // anonymous


DBusInterface::DBusInterface(const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(upowerServiceName, upowerDBusPath, upowerInterfaceName, connection, parent)
{
}

DBusInterface::~DBusInterface() = default;

QDBusPendingReply<QList<QDBusObjectPath>> DBusInterface::EnumerateDevices()
{
    return asyncCall(QStringLiteral("EnumerateDevices"));
}

#include "moc_upowerdbusinterface.cpp"
