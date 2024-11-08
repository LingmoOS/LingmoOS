/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>

#include "CSingleton.h"

/*
    现主线系统仅能使用一个蓝牙适配器，因此仅能设置默认蓝牙适配器；
    暂未适配使用多个蓝牙适配器；
    使用无阻塞方式调用和获取其他dbus服务提供的接口，防止线程长时间阻塞；
*/
class Bluetooth: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.device.bluetooth")
protected:
    explicit Bluetooth(QObject *parent = nullptr);
    virtual ~Bluetooth();

public Q_SLOTS:
    bool setDeviceState(QString device, bool enable);
    
    QStringList getAllDevices(void);

    void enable(bool enable);

private:
    QDBusPendingCallWatcher * __lingmobluetooth_dbus_call(const QString & method, const QList<QVariant> & args);

    friend class SingleTon<Bluetooth>;
};

typedef SingleTon<Bluetooth>  BLUETOOTHDBUS;
#endif

