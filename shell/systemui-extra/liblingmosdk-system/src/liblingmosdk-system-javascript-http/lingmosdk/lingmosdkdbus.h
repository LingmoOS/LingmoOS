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

#ifndef LINGMOSDKDBUS_H
#define LINGMOSDKDBUS_H
#include <QString>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>


class Kysdkdbus
{
public:
    Kysdkdbus();
    QString ServerName;
    QString ServerInterface;
    QString ServerPath;
    int dbusConn;

    QVariant dbusSend(QString SName, QString SPath, QString SInterface, QString method);
    QVariant dbusSendpara(QString SName, QString SPath, QString SInterface, QString method, QVariant para);
    QVariant dbusSendpara2(QString SName, QString SPath, QString SInterface, QString method, QVariant para1,QVariant para2);
    QVariant dbusSendpara4(QString SName, QString SPath, QString SInterface, QString method,QVariant para1,QVariant para2,QVariant para3,QVariant para4);
    QVariant dbusSession(QString SName, QString SPath, QString SInterface, QString method);

};

#endif // LINGMOSDKDBUS_H
