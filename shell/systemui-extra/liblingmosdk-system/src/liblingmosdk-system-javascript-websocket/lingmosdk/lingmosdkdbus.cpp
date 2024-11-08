/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "lingmosdkdbus.h"

Kysdkdbus::Kysdkdbus()
{
    dbusConn = -1;
}

QVariant Kysdkdbus::dbusSend(QString SName, QString SPath, QString SInterface, QString method)
{
    QVariant res;
    QDBusInterface interface (SName,SPath,SInterface,QDBusConnection::systemBus());
    if(!interface.isValid())
    {
       res = "dbus 连接失败";
       return res;
    }
    QDBusMessage reply = interface.call(method);
    qDebug() << "reply:" << reply;
    if(reply.arguments()[0].isValid())
    {
        qDebug()<<"连接成功";
        dbusConn = 0;
        res=reply.arguments()[0];
        return res;
    }
}

QVariant Kysdkdbus::dbusSend2(QString SName, QString SPath, QString SInterface, QString method)
{
    QVariant res;
    QDBusInterface interface (SName,SPath,SInterface,QDBusConnection::sessionBus());
    if(!interface.isValid())
    {
       res = "dbus 连接失败";
       return res;
    }
    QDBusMessage reply = interface.call(method);
    qDebug() << "reply:" << reply;
    if(reply.arguments()[0].isValid())
    {
        qDebug()<<"连接成功";
        dbusConn = 0;
        res=reply.arguments()[0];
        return res;
    }
}

QVariant Kysdkdbus::dbusSendpara(QString SName, QString SPath, QString SInterface, QString method, QVariant para)
{
    QVariant res;
    QDBusInterface interface (SName,SPath,SInterface,QDBusConnection::systemBus());
    if(!interface.isValid())
    {
       res = "dbus 连接失败";
       return res;
    }
    QDBusMessage reply = interface.call(method,para);
    qDebug() << "reply:" << reply;
    if(reply.arguments()[0].isValid())
    {
        qDebug()<<"连接成功";
        dbusConn = 0;
        res=reply.arguments()[0];
        return res;
    }
}

QVariant Kysdkdbus::dbusSendpara2(QString SName, QString SPath, QString SInterface, QString method, QVariant para1, QVariant para2)
{
    QVariant res;
    QDBusInterface interface (SName,SPath,SInterface,QDBusConnection::systemBus());
    if(!interface.isValid())
    {
       res = "dbus 连接失败";
       return res;
    }
    QDBusMessage reply = interface.call(method,para1, para2);
    qDebug() << "reply:" << reply;
    if(reply.arguments()[0].isValid())
    {
        qDebug()<<"连接成功";
        dbusConn = 0;
        res=reply.arguments()[0];
        return res;
    }
}

QVariant Kysdkdbus::dbusSendpara4(QString SName, QString SPath, QString SInterface, QString method, QVariant para1, QVariant para2, QVariant para3, QVariant para4)
{
    QVariant res;
    QDBusInterface interface (SName,SPath,SInterface,QDBusConnection::systemBus());
    if(!interface.isValid())
    {
       res = "dbus 连接失败";
       return res;
    }
    QDBusMessage reply = interface.call(method,para1,para2, para3, para4);
    qDebug() << "reply:" << reply;
    if(reply.arguments()[0].isValid())
    {
        qDebug()<<"连接成功";
        dbusConn = 0;
        res=reply.arguments()[0];
        return res;
    }
}
