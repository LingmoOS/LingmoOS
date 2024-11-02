/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <QObject>
#include <QDebug>
#include <QString>

#include <QStyle>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSlider>

#include <QFile>
#include <QApplication>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 *
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}
 */


///////////////////////////////////////////////////////////////////////////////
// wifi property struct
 typedef struct structWifiProperty{
    QString objectPath;
    QString bssid;
    int priority;
}structWifiProperty;


///////////////////////////////////////////////////////////////////////////////
// The Utils class, used to do some assist function

class Utils : public QObject
{
    Q_OBJECT
public:
    Utils();

    static int m_system(char *cmd);

public Q_SLOTS:
    void onRequestSendDesktopNotify(QString message);

};


///////////////////////////////////////////////////////////////////////////////
// The UseQssFile class, set control style by using .qss file

class UseQssFile
{
public:
    static void setStyle(const QString &style)
    {
        QString styleName = ":/qss/" + style;

        QFile qss(styleName);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};


///////////////////////////////////////////////////////////////////////////////
// The NetworkSpeed class, get the network upload and download speed

class NetworkSpeed : public QObject
{
    Q_OBJECT
public:
    explicit NetworkSpeed(QObject *parent = nullptr);

    int getCurrentDownloadRates(char *netname,long int * save_rate,long int * tx_rate); //获取当前的流量，参数为将获取到的流量保
};

#endif // UTILS_H
