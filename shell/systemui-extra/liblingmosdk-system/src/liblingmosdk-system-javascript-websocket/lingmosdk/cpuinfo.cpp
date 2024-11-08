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
#include <QDBusReply>

Cpuinfo::Cpuinfo(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.cpuinfo";
    ServerPath = "/com/lingmo/lingmosdk/cpuinfo";
}

QString Cpuinfo::getCpuArch()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuArch");
    dbusConnect = kdbus.dbusConn;
    qDebug() << res.toString();
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString Cpuinfo::getCpuVendor()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuVendor");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString Cpuinfo::getCpuModel()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuModel");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString Cpuinfo::getCpuFreqMHz()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuFreqMHz");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int Cpuinfo::getCpuCorenums()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuCorenums");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

QString Cpuinfo::getCpuVirt()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuVirt");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int Cpuinfo::getCpuProcess()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getCpuProcess");
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}
