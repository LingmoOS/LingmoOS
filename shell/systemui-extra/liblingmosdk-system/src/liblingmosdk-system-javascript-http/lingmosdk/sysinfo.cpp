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

#include "sysinfo.h"

Sysinfo::Sysinfo(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.sysinfo";
    ServerPath = "/com/lingmo/lingmosdk/sysinfo";
}

Sysinfo::Sysinfo(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.sysinfo";
    ServerPath = "/com/lingmo/lingmosdk/sysinfo";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Sysinfo::deal_sysinfo()
{
    qDebug()<<"sysinfo url path is "<<path;

    if(path_list.size()>1)
    {
        return deal_sysinfo_path_para();
    }else{
        return deal_sysinfo_path();
    }
}

QJsonObject Sysinfo::deal_sysinfo_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/sysinfo/getSystemArchitecture")
    {
        QString res = getSystemArchitecture();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemName")
    {
        QString res = getSystemName();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemActivationStatus")
    {
        int res = getSystemActivationStatus();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemSerialNumber")
    {
        QString res = getSystemSerialNumber();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemKernelVersion")
    {
        QString res = getSystemKernelVersion();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemEffectUser")
    {
        QString res = getSystemEffectUser();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemProjectName")
    {
        QString res = getSystemProjectName();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemProjectSubName")
    {
        QString res = getSystemProjectSubName();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemProductFeatures")
    {
        int res = getSystemProductFeatures();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemHostVirtType")
    {
        QString res = getSystemHostVirtType();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemHostCloudPlatform")
    {
        QString res = getSystemHostCloudPlatform();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemOSVersion")
    {
        QString res = getSystemOSVersion();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemUpdateVersion")
    {
        QString res = getSystemUpdateVersion();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSystemIsZYJ")
    {
        bool res = getSystemIsZYJ();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/sysinfo/getSysLegalResolution")
    {
        QStringList res = getSysLegalResolution();
        for (int var = 0; var < res.size(); ++var) {
           jarray.append(res[var]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
}

QJsonObject Sysinfo::deal_sysinfo_path_para()
{
    QJsonArray jarray;
    QJsonObject json;
    QString sysinfoname = path_list[1];
    sysinfoname =sysinfoname.split("=").last();
    qDebug()<<"sysinfoname  is "<<sysinfoname;
    if(path_list[0] == "/sysinfo/getSystemVersion")
    {
//        bool version ;
//        if(sysinfoname == )
        QString res = getSystemVersion(sysinfoname.toInt());
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

QString Sysinfo::getSystemArchitecture()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemArchitecture");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemName()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemName");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemVersion(bool version)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getSystemVersion",version);
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

int Sysinfo::getSystemActivationStatus()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemActivationStatus");
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

QString Sysinfo::getSystemSerialNumber()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemSerialNumber");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemKernelVersion()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemKernelVersion");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemEffectUser()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemEffectUser");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemProjectName()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemProjectName");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemProjectSubName()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemProjectSubName");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

int Sysinfo::getSystemProductFeatures()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemProductFeatures");
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

QString Sysinfo::getSystemHostVirtType()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemHostVirtType");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemHostCloudPlatform()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemHostCloudPlatform");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemOSVersion()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemOSVersion");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Sysinfo::getSystemUpdateVersion()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemUpdateVersion");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

bool Sysinfo::getSystemIsZYJ()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getSystemIsZYJ");
    dbusConnect = kdbus.dbusConn;
    return res.toBool();
}

QStringList Sysinfo::getSysLegalResolution()
{
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/resolution";
    ServerInterface = "com.lingmo.lingmosdk.resolution";
    QVariant res = kdbus.dbusSession(ServerName,ServerPath,ServerInterface,"getSysLegalResolution");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}
