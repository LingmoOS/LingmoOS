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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include "lingmosdkdbus.h"

class Resource : public QWidget
{
    Q_OBJECT
public:
    explicit Resource(QWidget *parent = nullptr);
    Resource(QString urlPath);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface;
    QString ServerPath;
    QJsonObject deal_resource();
    QJsonObject deal_resource_path();
//    QJsonObject deal_resource_path_para();
private:
    QStringList path_list;
    QString path;

signals:

public slots:
    double getMemTotalKiB();
    double getMemUsagePercent();
    double getMemUsageKiB();
    double getMemAvailableKiB();
    double getMemFreeKiB();
    double getMemVirtAllocKiB();
    double getMemSwapTotalKiB();
    double getMemSwapUsagePercent();
    double getMemSwapUsageKiB();
    double getMemSwapFreeKiB();
    double getCpuCurrentUsage();
    QString getUpTime();

};

#endif // RESOURCE_H
