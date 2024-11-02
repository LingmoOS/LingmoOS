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

#include "printer.h"

Printer::Printer(QWidget *parent) : QWidget(parent)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/print";
    ServerInterface = "com.lingmo.lingmosdk.print";
}

Printer::Printer(QString urlPath)
{
    dbusConnect = -1;
    ServerName = "com.lingmo.lingmosdk.service";
    ServerPath = "/com/lingmo/lingmosdk/print";
    ServerInterface = "com.lingmo.lingmosdk.print";
    path = urlPath;
    path_list = urlPath.split("?");
}

QJsonObject Printer::deal_printer()
{
    qDebug()<<"printer url path is "<<path;

    if(path_list.size()>1)
    {
        return deal_printer_path_para();
    }else{
        return deal_printer_path();
    }
}

QJsonObject Printer::deal_printer_path()
{
    QJsonArray jarray;
    QJsonObject json;
    if(path == "/printer/getPrinterList")
    {
        QStringList res = getPrinterList();
        for(int i=0;i<res.size();i++)
        {
            jarray.append(res[i]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
    else if(path == "/printer/getPrinterAvailableList")
    {
        QStringList res = getPrinterAvailableList();
        for(int i=0;i<res.size();i++)
        {
            jarray.append(res[i]);
        }
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",jarray);
        return json;
    }
    else{
        json.insert("ResultMessage","error url");
        return json;
    }
}

QJsonObject Printer::deal_printer_path_para()
{
    QJsonArray jarray;
    QJsonObject json;
    QString printername = path_list[1];
    printername =printername.split("=").last();
    qDebug()<<"printername  is "<<printername;
    if(path_list[0] == "/printer/getPrinterCancelAllJobs")
    {
        int res = getPrinterCancelAllJobs(printername);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/getPrinterStatus")
    {
        int res = getPrinterStatus(printername);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/getPrinterFilename")
    {
        QString res = getPrinterFilename(printername);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/getPrinterPrintLocalFile")
    {
        QString parainfo = path_list[1];
        QStringList para_list = parainfo.split("&");
        QString para_first = para_list.first();
        QString para_second = para_list.last();
        para_first = para_first.split("=").last();
        para_second = para_second.split("=").last();
        qDebug()<<"first para is "<<para_first;
        qDebug()<<"second para is "<<para_second;
        int res = getPrinterPrintLocalFile(para_first, para_second);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/getPrinterJobStatus")
    {
        QString parainfo = path_list[1];
        QStringList para_list = parainfo.split("&");
        QString para_first = para_list.first();
        QString para_second = para_list.last();
        para_first = para_first.split("=").last();
        para_second = para_second.split("=").last();
        qDebug()<<"first para is "<<para_first;
        qDebug()<<"second para is "<<para_second;
        int res = getPrinterJobStatus(para_first, para_second.toInt());
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/getPrinterDownloadRemoteFile")
    {
        QString parainfo = path_list[1];
        QStringList para_list = parainfo.split("&");
        QString para_first = para_list.first();
        QString para_second = para_list.last();
        para_first = para_first.split("=").last();
        para_second = para_second.split("=").last();
        qDebug()<<"first para is "<<para_first;
        qDebug()<<"second para is "<<para_second;
        int res = getPrinterDownloadRemoteFile(para_first, para_second);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else if(path_list[0] == "/printer/setPrinterOptions")
    {
        QString parainfo = path_list[1];
        QStringList para_list = parainfo.split("&");
        QString para_first = para_list[0];
        QString para_second = para_list[1];
        QString para_third = para_list[2];
        QString para_forth = para_list[3];
        para_first = para_first.split("=").last();
        para_second = para_second.split("=").last();
        para_third = para_third.split("=").last();
        para_forth = para_forth.split("=").last();
        qDebug()<<"first para is "<<para_first;
        qDebug()<<"second para is "<<para_second;
        qDebug()<<"third para is "<<para_third;
        qDebug()<<"forth para is "<<para_forth;
        int res = setPrinterOptions(para_first.toInt(), para_second,para_third,para_forth);
        json.insert("Result",dbusConnect);
        json.insert("ResultMessage",res);
        return json;
    }
    else{
        json.insert("ResultMessage","error url");
        return json;
    }
}

QStringList Printer::getPrinterList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getPrinterList");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}

QStringList Printer::getPrinterAvailableList()
{
    QVariant res = kdbus.dbusSend(ServerName,ServerPath,ServerInterface,"getPrinterAvailableList");
    dbusConnect = kdbus.dbusConn;
    return res.toStringList();
}

int Printer::setPrinterOptions(int num, QString type, QString cups, QString sides)
{
    kdbus.dbusSendpara4(ServerName,ServerPath,ServerInterface,"setPrinterOptions",num,type,cups,sides);
    dbusConnect = 0;
    QVariant res = 0;
    return res.toInt();
}

int Printer::getPrinterPrintLocalFile(QString name, QString savepath)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterPrintLocalFile",name,savepath);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

int Printer::getPrinterCancelAllJobs(QString name)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterCancelAllJobs",name);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

int Printer::getPrinterStatus(QString name)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterStatus",name);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

QString Printer::getPrinterFilename(QString name)
{
    QVariant res = kdbus.dbusSendpara(ServerName,ServerPath,ServerInterface,"getPrinterFilename",name);
    dbusConnect = kdbus.dbusConn;
    return res.toString();
}

int Printer::getPrinterJobStatus(QString name, int jobid)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterJobStatus",name,jobid);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

int Printer::getPrinterDownloadRemoteFile(QString name, QString savepath)
{
    QVariant res = kdbus.dbusSendpara2(ServerName,ServerPath,ServerInterface,"getPrinterDownloadRemoteFile",name,savepath);
    dbusConnect = kdbus.dbusConn;
    return res.toInt();
}

