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

#ifndef NET_H
#define NET_H

#include <QWidget>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDBusReply>
#include <QJsonArray>
#include <QJsonObject>
#include "lingmosdkdbus.h"

class Net : public QWidget
{
    Q_OBJECT
public:
    explicit Net(QWidget *parent = nullptr);
    Net(QString urlPath);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface;
    QString ServerPath;

    QJsonObject deal_net();
    QJsonObject deal_net_path();
    QJsonObject deal_net_path_para();
private:
    QStringList path_list;
    QString path;

public slots:
    int getPortState(int port);
    QStringList getGatewayInfo();
    QStringList getMultiplePortStat(int startPort,int endPort);
    QStringList getFirewallState();

signals:

};

#endif // NET_H
