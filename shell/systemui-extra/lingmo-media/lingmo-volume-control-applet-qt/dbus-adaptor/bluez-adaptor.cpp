/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "bluez-adaptor.h"

Bluetooth_Dbus::Bluetooth_Dbus()
{
    UkmediaDbusRegister();
}

void Bluetooth_Dbus::UkmediaDbusRegister()
{
    QDBusConnection con = QDBusConnection::sessionBus();

    if(!con.registerService("org.lingmo.media"))
        qDebug()<<"registerService failed"<<con.lastError().message();

    if(!con.registerObject("/org/lingmo/media/bluetooth",this,QDBusConnection::ExportAllSlots
                                                              |QDBusConnection::ExportAllSignals))
        qDebug()<<"registerObject failed"<<con.lastError().message();
}

void Bluetooth_Dbus::sendBatteryChangedSignal(QString macAddr, int battery)
{
    Q_EMIT batteryChanged(macAddr, battery);
}

int Bluetooth_Dbus::getBatteryLevel(QString macAddr)
{
    return (macAddress == macAddr) ? batteryLevel : ERROR;
}

Bluetooth_Dbus::~Bluetooth_Dbus()
{

}
