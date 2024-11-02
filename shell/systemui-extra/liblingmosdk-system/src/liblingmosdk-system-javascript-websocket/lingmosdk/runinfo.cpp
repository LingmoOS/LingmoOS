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

Runinfo::Runinfo(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.runinfo";
    ServerPath = "/com/lingmo/lingmosdk/runinfo";
}

// double Runinfo::getIoSpeed()
// {
//     QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getIoSpeed");
//     dbusConnect = kdbus.dbusConn;
//     sendText(QString("%1").arg(QString::number(res.toDouble())));
//     return res.toDouble();
// }

double Runinfo::getNetSpeed(QString netcard)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getNetSpeed", netcard);
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(QString::number(res.toDouble())));
    return res.toDouble();
}

int Runinfo::getDiskRate(QString diskname)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskRate", diskname);
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

// double Runinfo::getMainboardTemperature()
// {
//     QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getMainboardTemperature");
//     dbusConnect = kdbus.dbusConn;
//     sendText(QString("%1").arg(QString::number(res.toDouble())));
//     return res.toDouble();
// }

double Runinfo::getCpuTemperature()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuTemperature");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(QString::number(res.toDouble())));
    return res.toDouble();
}

double Runinfo::getDiskTemperature(QString diskpath)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskTemperature",diskpath);
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(QString::number(res.toDouble())));
    return res.toDouble();
}
