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

#include "cpuinfo.h"

Cpuinfo::Cpuinfo(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/cpuinfo";
    ServerInterface = "com.lingmo.lingmosdk.cpuinfo";
}

Cpuinfo::Cpuinfo(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/cpuinfo";
    ServerInterface = "com.lingmo.lingmosdk.cpuinfo";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Cpuinfo::deal_cpuinfo()
{
    qDebug()<<"cpuinfo url path is "<<path;
    return deal_cpuinfo_path();
}

QJsonObject Cpuinfo::deal_cpuinfo_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/cpuinfo/getCpuArch")
    {
        QString res = getCpuArch();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuVendor")
    {
        QString res = getCpuVendor();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuModel")
    {
        QString res = getCpuModel();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuFreqMHz")
    {
        QString res = getCpuFreqMHz();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuCorenums")
    {
        int res = getCpuCorenums();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuVirt")
    {
        QString res = getCpuVirt();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/cpuinfo/getCpuProcess")
    {
        int res = getCpuProcess();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

QString Cpuinfo::getCpuArch()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuArch");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Cpuinfo::getCpuVendor()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuVendor");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Cpuinfo::getCpuModel()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuModel");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Cpuinfo::getCpuFreqMHz()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuFreqMHz");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

int Cpuinfo::getCpuCorenums()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuCorenums");
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

QString Cpuinfo::getCpuVirt()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuVirt");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

int Cpuinfo::getCpuProcess()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuProcess");
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}
