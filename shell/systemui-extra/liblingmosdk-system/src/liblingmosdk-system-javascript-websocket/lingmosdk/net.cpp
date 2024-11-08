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

#include "net.h"

Net::Net(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.net";
    ServerPath = "/com/lingmo/lingmosdk/net";
}


int Net::getPortState(QString port)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPortState",port);
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(QString::number(res.toInt())));
    return res.toInt();
}

QStringList Net::getMultiplePortStat(QString beginport, QString endport)
{
    QString port = beginport + "," + endport;
    qDebug() << "getMultiplePortStat" << "port" << port;
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getMultiplePortStat",port);
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QStringList Net::getGatewayInfo()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getGatewayInfo");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

 QStringList Net::getFirewallState(void)
 {
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getFirewallState");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
 }
