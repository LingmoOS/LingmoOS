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

#include "gps.h"

Gps::Gps(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.gps";
    ServerPath = "/com/lingmo/lingmosdk/gps";
}

Gps::Gps(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.gps";
    ServerPath = "/com/lingmo/lingmosdk/gps";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Gps::deal_gps()
{
    qDebug()<<"gps url is "<<path;
    return deal_gps_path();
}

QJsonObject Gps::deal_gps_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/gps/getGPSInfo")
    {
        QString res = getGPSInfo();
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
}

QString Gps::getGPSInfo()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getGPSInfo") ;
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}
