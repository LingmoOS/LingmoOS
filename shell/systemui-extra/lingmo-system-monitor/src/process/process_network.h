/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PROCESS_NETWORK_H__
#define __PROCESS_NETWORK_H__

#include <QThread>
#include <map>
#include <string>
#include <malloc.h>
#include <iostream>
#include <vector>
using namespace std;

#include <libnethogs.h>

class ProcessNetwork : public QThread
{
    Q_OBJECT
public:
    explicit ProcessNetwork(QObject *parent);
    virtual ~ProcessNetwork();

    static void ProcnetMonitorCallback(int action, NethogsMonitorRecord const *data, void *pVoid);

    void run();
    void stop();

signals:
    void procDetected(const QString& procname, double rcv, double sent, int pid, unsigned int uid, const QString& devname);
    void procDeleted(int pid);
};

#endif // __PROCESS_NETWORK_H__
