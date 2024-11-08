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

Net::Net(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.net";
    ServerPath = "/com/lingmo/lingmosdk/net";
}

Net::Net(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.net";
    ServerPath = "/com/lingmo/lingmosdk/net";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Net::deal_net()
{
    qDebug()<<"net url path is "<<path;

    if(path_list.size()>1)
    {
        return deal_net_path_para();
    }else{
        return deal_net_path();
    }
}

QJsonObject Net::deal_net_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/net/getGatewayInfo")
    {
        QStringList res = getGatewayInfo();
        for(int i=0;i<res.size();i++)
        {
            jarray.append(res[i]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
    else if(path == "/net/getFirewallState")
    {
        QStringList res = getFirewallState();
        for(int i=0;i<res.size();i++)
        {
            jarray.append(res[i]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
}

QJsonObject Net::deal_net_path_para()
{
    QJsonArray jarray;
    QJsonObject json;
    QString netName = path_list[1];
    netName =netName.split("=").last();
    qDebug()<<"netName  is "<<netName;
    if(path_list[0] == "/net/getPortState")
    {
        int res = getPortState(netName.toInt());
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/net/getMultiplePortStat")
    {
        QString parainfo = path_list[1];
        QStringList para_list = parainfo.split("&");
        QString para_first = para_list.first();
        QString para_second = para_list.last();
        para_first = para_first.split("=").last();
        para_second = para_second.split("=").last();
        qDebug()<<"first para is "<<para_first;
        qDebug()<<"second para is "<<para_second;
        QStringList res = getMultiplePortStat(para_first.toInt(), para_second.toInt());
        for (int var = 0; var < res.size(); ++var) {
           jarray.append(res[var]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
}


int Net::getPortState(int port)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPortState",port);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

QStringList Net::getGatewayInfo()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getGatewayInfo");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}

QStringList Net::getMultiplePortStat(int startPort, int endPort)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getMultiplePortStat",startPort,endPort);
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}

QStringList Net::getFirewallState()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getFirewallState");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}
