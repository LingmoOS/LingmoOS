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

#include "disk.h"

Disk::Disk(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerInterface = "com.lingmo.lingmosdk.disk";
    ServerPath = "/com/lingmo/lingmosdk/disk";
}

QStringList Disk::getDiskList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getDiskList") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

int Disk::getDiskSectorSize(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskSectorSize", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

double Disk::getDiskTotalSizeMiB(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskTotalSizeMiB", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    qDebug() << res.toDouble();
    sendText(QString("%1").arg(res.toDouble()));
    return res.toDouble();
}

QString Disk::getDiskModel(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskModel", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString Disk::getDiskSerial(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskSerial", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int Disk::getDiskPartitionNums(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskPartitionNums", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

QString Disk::getDiskType(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskType", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

QString Disk::getDiskVersion(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskVersion", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

// unsigned int Disk::getDiskSpeed(QString DiskName)
// {
//     QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskSpeed", DiskName) ;
//     dbusConnect = kdbus.dbusConn;
//     sendText(QString("%1").arg(res.toString()));
//     return res.toUInt();
// }

unsigned long long Disk::getDiskSectorNum(QString DiskName)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getDiskSectorNum", DiskName) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toULongLong();
}
