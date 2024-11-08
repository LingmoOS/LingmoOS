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

#include "resource.h"

Resource::Resource(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/resource";
    ServerInterface = "com.lingmo.lingmosdk.resource";
}

double Resource::getMemTotalKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemTotalKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemUsagePercent()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemUsagePercent") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemUsageKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemUsageKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemAvailableKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemAvailableKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemFreeKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemFreeKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemVirtAllocKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemVirtAllocKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemSwapTotalKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapTotalKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemSwapUsagePercent()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapUsagePercent") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemSwapUsageKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapUsageKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getMemSwapFreeKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapFreeKiB") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

double Resource::getCpuCurrentUsage()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuCurrentUsage") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

QString Resource::getUpTime()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getUpTime") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}
