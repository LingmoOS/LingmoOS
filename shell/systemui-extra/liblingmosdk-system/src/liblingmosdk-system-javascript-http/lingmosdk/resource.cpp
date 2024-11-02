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

Resource::Resource(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.resource";
    ServerPath = "/com/lingmo/lingmosdk/resource";
}

Resource::Resource(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.resource";
    ServerPath = "/com/lingmo/lingmosdk/resource";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Resource::deal_resource()
{
    qDebug()<<"resource url path is "<<path;
    return deal_resource_path();
}

QJsonObject Resource::deal_resource_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/resource/getMemTotalKiB")
    {
        double res = getMemTotalKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemUsagePercent")
    {
        double res = getMemUsagePercent();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemUsageKiB")
    {
        double res = getMemUsageKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemAvailableKiB")
    {
        double res = getMemAvailableKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemFreeKiB")
    {
        double res = getMemFreeKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemVirtAllocKiB")
    {
        double res = getMemVirtAllocKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemSwapTotalKiB")
    {
        double res = getMemSwapTotalKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemSwapUsagePercent")
    {
        double res = getMemSwapUsagePercent();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemSwapUsageKiB")
    {
        double res = getMemSwapUsageKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getMemSwapFreeKiB")
    {
        double res = getMemSwapFreeKiB();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getCpuCurrentUsage")
    {
        double res = getCpuCurrentUsage();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/resource/getUpTime")
    {
        QString res = getUpTime();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

double Resource::getMemTotalKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemTotalKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemUsagePercent()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemUsagePercent");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemUsageKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemUsageKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemAvailableKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemAvailableKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemFreeKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemFreeKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemVirtAllocKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemVirtAllocKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemSwapTotalKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapTotalKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemSwapUsagePercent()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapUsagePercent");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemSwapUsageKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapUsageKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getMemSwapFreeKiB()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMemSwapFreeKiB");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Resource::getCpuCurrentUsage()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuCurrentUsage");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

QString Resource::getUpTime()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getUpTime");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}
