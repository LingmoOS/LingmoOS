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
#ifndef BLUEZ_ADAPTOR_H
#define BLUEZ_ADAPTOR_H

#include <QGSettings>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#define ERROR -1

class Bluetooth_Dbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.media")
public:
    Bluetooth_Dbus();
    ~Bluetooth_Dbus();
    int batteryLevel = -1;//蓝牙耳机电量
    QString macAddress = "";
    void UkmediaDbusRegister();
    void sendBatteryChangedSignal(QString macAddr, int battery);

Q_SIGNALS:
    void batteryChanged(QString macAddr, int battery);//蓝牙电量改变信号

public Q_SLOTS:
    int getBatteryLevel(QString macAddr);//获取蓝牙耳机电量
};

#endif // BLUEZ_ADAPTOR_H
