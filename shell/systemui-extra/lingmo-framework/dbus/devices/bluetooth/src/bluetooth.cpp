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

#include "bluetooth.h"
#include "bluetooth_define.h"

Bluetooth::Bluetooth(QObject *parent /*= nullptr*/):
    QObject(parent)
{
    qDebug() << "init Bluetooth";
}

 Bluetooth::~Bluetooth(){
    qDebug() << "fini Bluetooth";
}


bool Bluetooth::setDeviceState(QString device, bool enable){
    qInfo() << device << enable;
    
    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();

    QList<QVariant> args;
    QMap<QString, QVariant> arg;
    arg["Powered"] = enable;
    args.append(arg);

    QDBusPendingCallWatcher * watch = this->__lingmobluetooth_dbus_call("setDefaultAdapterAttr", args);
    connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
        QDBusMessage reply = self->reply();
        if(reply.type() == QDBusMessage::ReplyMessage) {
            auto r = msg.createReply(reply.arguments());
            con.send(r);
        } else {
            auto r = msg.createErrorReply(reply.errorName(), reply.errorMessage());
            con.send(r);
        }
    });

    return true;
}

QStringList Bluetooth::getAllDevices(void)
{
    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();
    QList<QVariant> args;

    QDBusPendingCallWatcher * watch = this->__lingmobluetooth_dbus_call("getAllAdapterAddress", args);
    connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
        QDBusMessage reply = self->reply();
        if(reply.type() == QDBusMessage::ReplyMessage) {
            auto r = msg.createReply(reply.arguments());
            con.send(r);
        } else {
            auto r = msg.createErrorReply(reply.errorName(), reply.errorMessage());
            con.send(r);
        }
    });
    return QStringList();
}

void Bluetooth::enable(bool enable)
{
    QList<QVariant> args;
    QMap<QString, QVariant> arg;
    arg["Powered"] = enable;
    args.append(arg);

    this->__lingmobluetooth_dbus_call("setDefaultAdapterAttr", args);
}

QDBusPendingCallWatcher * Bluetooth::__lingmobluetooth_dbus_call(const QString & method, const QList<QVariant> & args)
{
    QDBusInterface iface(LINGMOBLUETHHTH_DBUS_SERVER, LINGMOBLUETHHTH_DBUS_OBJECT, LINGMOBLUETHHTH_DBUS_INTERFACE, QDBusConnection::systemBus());
    QDBusPendingCallWatcher * watch = new QDBusPendingCallWatcher(iface.asyncCallWithArgumentList(method, args), this);
    return watch;
}