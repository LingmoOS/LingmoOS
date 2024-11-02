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

#ifndef RUNINFO_H
#define RUNINFO_H

#include <QWidget>
#include "lingmosdkdbus.h"

class Runinfo : public QObject
{
    Q_OBJECT
public:
    explicit Runinfo(QObject *parent = nullptr);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface;
    QString ServerPath;


public slots:
    // double getIoSpeed();
    double getNetSpeed(QString);
    int getDiskRate(QString diskname);
    // double getMainboardTemperature();
    double getCpuTemperature();
    double getDiskTemperature( QString diskpath);

signals:
    void sendText(const QString &text);
};

#endif // RUNINFO_H
