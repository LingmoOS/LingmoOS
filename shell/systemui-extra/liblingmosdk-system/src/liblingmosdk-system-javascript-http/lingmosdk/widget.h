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

#ifndef WIDGET_H
#define WIDGET_H
#include "customerevent.h"
#include <QWidget>
// #include <JQHttpServer>
#include "jqhttpserver.h"
#include <QProcess>
#include <QFile>
#include <QTimer>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonObject>
#include "resolutionctl.h"
#include "netlink.h"
#include "peripheralsenum.h"
#include "netcard.h"
#include "bios.h"
#include "mainboard.h"
#include "disk.h"
#include "net.h"
#include "runinfo.h"
#include "gps.h"
#include "process.h"
#include "packageinfo.h"
#include "resource.h"
#include "sysinfo.h"
#include "printer.h"
#include "cpuinfo.h"
#include <QSettings>
#include <QCoreApplication>



bool actByFilepath(QString &filepath);


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void initializeHttpServer(int port);
    void convert2Json();
    void convertFromJson(QByteArray data);//解析
//    static QJsonArray deal_filepath(QString &filepath);
    static QJsonObject deal_filepath(QString &filepath);
    static QJsonObject deal_filepath_para(QString &filepath,QVariant filepathpara);
//    Screenctl *screenctl;
    static void onHttpAccepted(const QPointer<JQHttpServer::Session> &session);
    static Widget* w;
    QMutex mutex;


protected:
    virtual bool event(QEvent *event) override;
//    bool eventFilter(QObject*obj,QEvent *e);

private:

};

#endif // WIDGET_H
