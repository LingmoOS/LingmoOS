/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "upowersystemenery.h"
#include <QDBusReply>
#include <QDebug>

namespace {

const char *upower_dbus_service = "org.freedesktop.UPower";
const char *upower_dbus_path = "/org/freedesktop/UPower/devices/DisplayDevice";
const char *upower_dbus_interface = "org.freedesktop.UPower.Device";
const char *upower_dbus_properties_interface = "org.freedesktop.DBus.Properties";

}
UPowerSystemEnery::UPowerSystemEnery()
    : m_upowerDbusInterface(
        upower_dbus_service, upower_dbus_path,
        upower_dbus_properties_interface, QDBusConnection::systemBus())
{
}

double UPowerSystemEnery::energy()
{
    // 瓦时->焦耳
    return getUpowerProperty("Energy").toDouble() * 3600;
}

double UPowerSystemEnery::dischargeRate()
{
    return getUpowerProperty("EnergyRate").toDouble();
}

int UPowerSystemEnery::timeToEmpty()
{
    return getUpowerProperty("TimeToEmpty").toInt();
}

QVariant UPowerSystemEnery::getUpowerProperty(const QString &propertyName)
{
    QDBusReply<QVariant> reply = m_upowerDbusInterface.call("Get", upower_dbus_interface, propertyName);
    if (!reply.isValid()) {
        qWarning() << "Get upower property failed: " << reply.error();
        return QVariant();
    }
    return reply.value();
}
