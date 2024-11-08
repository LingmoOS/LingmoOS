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

#include "print.h"

Print::Print(QObject *parent) : QObject(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/print";
    ServerInterface = "com.lingmo.lingmosdk.print";
}

QStringList Print::getPrinterList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getPrinterList") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

QStringList Print::getPrinterAvailableList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getPrinterAvailableList") ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toStringList().join(",")));
    return res.toStringList();
}

void Print::setPrinterOptions(int number_up, QString media, QString number_up_layout, QString sides)
{
    QVariant res = kdbus.dbusSendpara4(ServerName,ServerPath,ServerInterface,"setPrinterOptions", QString::number(number_up), media, number_up_layout, sides) ;
    dbusConnect = kdbus.dbusConn;
}

int Print::getPrinterPrintLocalFile(QString printname,QString printpath)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterPrintLocalFile", printname, printpath) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

int Print::getPrinterCancelAllJobs(QString printname)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterCancelAllJobs",printname) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

int Print::getPrinterStatus(QString printname)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterStatus", printname) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

QString Print::getPrinterFilename(QString printpath)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterFilename", printpath) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toString()));
    return res.toString();
}

int Print::getPrinterJobStatus(QString printname, int printid)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterJobStatus", printname, printid) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}

int Print::getPrinterDownloadRemoteFile(QString printfile, QString printpath)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterDownloadRemoteFile", printfile, printpath) ;
    dbusConnect = kdbus.dbusConn;
    sendText(QString("%1").arg(res.toInt()));
    return res.toInt();
}
