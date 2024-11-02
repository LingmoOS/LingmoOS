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

#include "bios.h"

Bios::Bios(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/bios";
    ServerInterface = "com.lingmo.lingmosdk.bios";

}


QString Bios::getBiosVendor()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getBiosVendor");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    qDebug() << res.toString();
    return res.toString();
}

QString Bios::getBiosVersion()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getBiosVersion");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    qDebug() << res.toString();
    return res.toString();
}
