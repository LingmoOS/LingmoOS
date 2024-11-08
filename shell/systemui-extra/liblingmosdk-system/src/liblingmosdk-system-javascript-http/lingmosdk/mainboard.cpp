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

#include "mainboard.h"

Mainboard::Mainboard(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.mainboard";
    ServerPath = "/com/lingmo/lingmosdk/mainboard";
}

Mainboard::Mainboard(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.mainboard";
    ServerPath = "/com/lingmo/lingmosdk/mainboard";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Mainboard::deal_mainboard()
{
    qDebug()<<"mainboard url path is "<<path;
    return deal_mainboard_path();
}

QJsonObject Mainboard::deal_mainboard_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/mainboard/getMainboardName")
    {
        QString res = getMainboardName();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/mainboard/getMainboardDate")
    {
        QString res = getMainboardDate();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/mainboard/getMainboardSerial")
    {
        QString res = getMainboardSerial();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path == "/mainboard/getMainboardVendor")
    {
        QString res = getMainboardVendor();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}


QString Mainboard::getMainboardName()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardName");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Mainboard::getMainboardDate()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardDate");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Mainboard::getMainboardSerial()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardSerial");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

QString Mainboard::getMainboardVendor()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardVendor");
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}
