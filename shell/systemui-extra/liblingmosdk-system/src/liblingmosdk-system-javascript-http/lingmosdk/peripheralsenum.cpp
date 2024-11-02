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

#include "peripheralsenum.h"

Peripheralsenum::Peripheralsenum(QWidget *parent) : QWidget(parent)
{

}

Peripheralsenum::Peripheralsenum(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.peripheralsenum";
    ServerPath = "/com/lingmo/lingmosdk/peripheralsenum";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Peripheralsenum::deal_peripheralsenum()
{
    qDebug()<<"mainboard url path is "<<path;
    return deal_peripheralsenum_path();
}

QJsonObject Peripheralsenum::deal_peripheralsenum_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/peripheralsenum/getAllUsbInfo")
    {
        QStringList res = getAllUsbInfo();
        for(int i=0;i<res.size();i++)
        {
            jarray.append(res[i]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
}

QStringList Peripheralsenum::getAllUsbInfo()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getAllUsbInfo");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}
