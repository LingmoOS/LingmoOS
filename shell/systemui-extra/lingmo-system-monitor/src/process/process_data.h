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

#ifndef __PROCESS_DATA_H__
#define __PROCESS_DATA_H__

#include <QObject>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QPixmap>
#include <iostream>
#include <QString>
#include <QPainter>
#include <QDateTime>

using std::string;
#include "process_list.h"

namespace sysmonitor{
namespace process{

class Process;

class ProcessData : public QSharedData
{
public:
    ProcessData()
        : QSharedData()
        , valid(false)
        , pid(0)
        , cpu(0.0)
        , m_memory(0)
        , m_nice(0)
        , m_flownet("")
        , m_diskio("")
        , m_numFlowNet(0)
        , m_numDiskIo(0)
        , strIconPath("")
        , processName("")
        , displayName("")
        , path("")
        , user("")
        , m_status("")
        , m_session("")
        , cpu_duration_time("")
        , m_type(0)
        , m_start_time(0)
        , m_cpu_time(0)
        , m_ustatus(0)
        , m_pcpu(0)
        , m_tooltip("")
        , m_arguments("")
        , m_uid(-1)
        , m_unit("")
        , m_seat("")
        , m_frequency(1)
        , m_preDiskIoCount(0)
        , m_preFlownetCount(0)
        , m_uThreads(1)
    {     
    }
    ProcessData(const ProcessData &other)
        : QSharedData(other)
        , valid(other.valid)
        , pid(other.pid)
        , cpu(other.cpu)
        , m_memory(other.m_memory)
        , m_nice(other.m_nice)
        , m_flownet(other.m_flownet)
        , m_diskio(other.m_diskio)
        , m_numFlowNet(other.m_numFlowNet)
        , m_numDiskIo(other.m_numDiskIo)
        , strIconPath(other.strIconPath)
        , processName(other.processName)
        , displayName(other.displayName)
        , path(other.path)
        , user(other.user)
        , m_status(other.m_status)
        , m_session(other.m_session)
        , cpu_duration_time(other.cpu_duration_time)
        , m_type(other.m_type)
        , m_start_time(other.m_start_time)
        , m_cpu_time(other.m_cpu_time)
        , m_ustatus(other.m_ustatus)
        , m_pcpu(other.m_pcpu)
        , m_tooltip(other.m_tooltip)
        , m_arguments(other.m_arguments)
        , m_uid(other.m_uid)
        , m_unit(other.m_unit)
        , m_seat(other.m_seat)
        , m_frequency(other.m_frequency)
        , m_preDiskIoCount(other.m_preDiskIoCount)
        , m_preFlownetCount(other.m_preFlownetCount)
        , m_uThreads(1)
    {
    }
    ~ProcessData() {}

    inline bool isValid() const
    {
        return valid;
    }

private:
    bool valid;
    pid_t pid;
    double cpu;
    qulonglong m_memory;
    int m_nice;
    QString m_flownet;
    QString m_diskio;
    qulonglong m_numFlowNet;
    qulonglong m_numDiskIo;
    QString strIconPath;
    QString processName;
    QString displayName;
//    QString commandLine;
    QString path;
    QString user;
    QString m_status;
    QString m_session;
    QString cpu_duration_time;
    unsigned int m_type;

    unsigned long m_start_time;
    qulonglong m_cpu_time;
    unsigned m_ustatus;
    double m_pcpu;
    QString m_tooltip;
    QString m_arguments;
    unsigned m_uid;
    QString m_unit;
    QString m_seat;
    unsigned m_frequency;
    QDateTime m_preDiskIoTime;
    QDateTime m_preFlownetTime;
    qint64 m_preDiskIoCount;
    qint64 m_mergePreDiskIoCount;
    qint64 m_preFlownetCount;
    unsigned int m_uThreads;

    friend class Process;
};

} // namespace process
} // namespace sysmonitor

#endif //__PROCESS_DATA_H__
