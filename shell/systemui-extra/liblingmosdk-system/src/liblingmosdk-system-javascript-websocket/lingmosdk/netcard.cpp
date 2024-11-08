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

#include "netcard.h"

NetCard::NetCard(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.netcard";
    ServerPath = "/com/lingmo/lingmosdk/netcard";
}

QStringList NetCard::getNetCardName()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getNetCardName") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

int NetCard::getNetCardUp(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardUp",NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

QStringList NetCard::getNetCardUpcards()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getNetCardUpcards") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QString NetCard::getNetCardPhymac(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardPhymac", NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString NetCard::getNetCardPrivateIPv4(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardPrivateIPv4", NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QStringList NetCard::getNetCardIPv4(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardIPv4", NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QString NetCard::getNetCardPrivateIPv6(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardPrivateIPv6", NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int NetCard::getNetCardType(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardType",NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(QString::number(res.toInt())));
    return res.toInt();
}

QStringList NetCard::getNetCardProduct(QString NetCardName)
{
    qDebug() << "getNetCardProduct" << NetCardName;
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardProduct",NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QStringList NetCard::getNetCardIPv6(QString NetCardName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetCardIPv6",NetCardName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}
