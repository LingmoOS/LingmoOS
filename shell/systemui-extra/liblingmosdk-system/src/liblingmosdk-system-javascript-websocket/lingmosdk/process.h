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

#ifndef PROCESS_H
#define PROCESS_H

#include <QWidget>
#include "lingmosdkdbus.h"

class Process : public QObject
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = nullptr);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface;
    QString ServerPath;


public slots:
    double getProcInfoCpuUsage(int pid);
    //double getProcInfoIoUsage(int pid);
    double getProcInfoMemUsage(int pid);
    QString getProcInfoStatus(int pid);
    int getProcInfoPort(int pid);
    QString getProcInfoStartTime(int pid);
    QString getProcInfoRunningTime(int pid);
    QString getProcInfoCpuTime(int pid);
    QString getProcInfoCmd(int pid);
    QString getProcInfoUser(int pid);
    QStringList getProcInfo(QString procName);
    QStringList getAllProcInfo(void);

signals:
    void sendText(const QString &text);
};

#endif // PROCESS_H
