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

#ifndef DISK_H
#define DISK_H

#include <QWidget>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDBusReply>
#include "lingmosdkdbus.h"

class Disk : public QObject
{
    Q_OBJECT
public:
    explicit Disk(QObject *parent = nullptr);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface ;
    QString ServerPath ;


public slots:
    QStringList getDiskList();
    int getDiskSectorSize(QString DiskName);
    double getDiskTotalSizeMiB(QString DiskName);
    QString getDiskModel(QString DiskName);
    QString getDiskSerial(QString DiskName);
    int getDiskPartitionNums(QString DiskName);
    QString getDiskType(QString DiskName);
    QString getDiskVersion(QString DiskName);
    //unsigned int getDiskSpeed(QString DiskName);
    unsigned long long getDiskSectorNum(QString DiskName);

signals:
    void sendText(const QString &text);

};

#endif // DISK_H
