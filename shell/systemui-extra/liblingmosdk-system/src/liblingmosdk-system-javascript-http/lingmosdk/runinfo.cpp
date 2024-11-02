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

#include "runinfo.h"
#include <QDBusReply>

Runinfo::Runinfo(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.runinfo";
    ServerPath = "/com/lingmo/lingmosdk/runinfo";
}

Runinfo::Runinfo(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.runinfo";
    ServerPath = "/com/lingmo/lingmosdk/runinfo";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Runinfo::deal_runinfo()
{
    qDebug()<<"runinfo url path is "<<path;

    if(path_list.size()>1)
    {
        return deal_runinfo_path_para();
    }else{
        return deal_runinfo_path();
    }
}

QJsonObject Runinfo::deal_runinfo_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/runinfo/getCpuTemperature")
    {
        double res = getCpuTemperature();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

QJsonObject Runinfo::deal_runinfo_path_para()
{
    QJsonArray jarray;
    QJsonObject json;
    QString para = path_list[1];
    para =para.split("=").last();
    qDebug()<<"para name is "<<para;
    if(path_list[0] == "/runinfo/getNetSpeed")
    {
        double res = getNetSpeed(para);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/runinfo/getDiskRate")
    {
        int res = getDiskRate(para);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/runinfo/getDiskTemperature")
    {
        double res = getDiskTemperature(para);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

//double Runinfo::getIoSpeed()
//{
//    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getIoSpeed");
//    dbusConnect = kdbus.dbusConn;
//    return res.toDouble();
//}

double Runinfo::getNetSpeed(QString netname)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetSpeed",netname);
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

//double Runinfo::getMainboardTemperature()
//{
//    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardTemperature");
//    dbusConnect = kdbus.dbusConn;
//    return res.toDouble();
//}

double Runinfo::getCpuTemperature()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuTemperature");
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

double Runinfo::getDiskTemperature(QString diskpath)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskTemperature",diskpath);
    dbusConnect = kdbus.dbusConn;
    return res.toDouble();
}

int Runinfo::getDiskRate(QString diskpath)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskRate",diskpath);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}
