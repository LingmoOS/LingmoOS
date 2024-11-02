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

#include "process_network.h"

#include <QDateTime>
#include <QDebug>

ProcessNetwork::ProcessNetwork(QObject *parent)
    : QThread(parent)
{
}

ProcessNetwork::~ProcessNetwork()
{
    nethogsmonitor_breakloop();
}

void ProcessNetwork::ProcnetMonitorCallback(int action, NethogsMonitorRecord const *data, void *pVoid)
{
    if (!data || !pVoid)
        return;
    //qDebug()<<"Action:"<<action<<",pid:"<<data->pid<<",send_bytes:"<<data->sent_bytes
    //    <<",resc_bytes:"<<data->recv_bytes<<",sent_kbs:"<<data->sent_kbs<<",recv_kbs:"<<data->recv_kbs;
    ProcessNetwork* pThis = (ProcessNetwork*)pVoid;
    if (action == 1) {
        emit pThis->procDetected(data->name, data->recv_kbs, data->sent_kbs, 
            data->pid, data->uid, data->device_name);
    } else if (action == 2) {
        emit pThis->procDeleted(data->pid);
    }
}

void ProcessNetwork::run()
{
    nethogsmonitor_loop(ProcnetMonitorCallback, this);
}

void ProcessNetwork::stop()
{
    nethogsmonitor_breakloop();
}
