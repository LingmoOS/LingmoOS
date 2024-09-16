// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_SET_P_H
#define CPU_SET_P_H

#include "system/cpu.h"

#include <QSharedData>
#include <QMap>

namespace core {
namespace system {

class CPUSet;

enum StatIndex {
    kLastStat = 0,
    kCurrentStat = 1,
    kStatCount = kCurrentStat + 1
};

class CPUSetPrivate : public QSharedData
{
public:
    CPUSetPrivate()
        : QSharedData()
        , ncpus_max {0}
        , ncpus_possible {0}
        , ncpus_present {0}
        , ncpus_online {0}
        , m_cores {0}
        , m_sockets {0}
        , m_processors {0}
        , m_model {}
        , m_vendor {}
        , m_virtualization {}
        , m_stat {std::make_shared<cpu_stat_t>()}
        , m_usage {std::make_shared<cpu_usage_t>()}
        , m_statDB {}
        , m_usageDB {}
        , m_info {}
        , m_infos {}
    {

    }

    CPUSetPrivate(const CPUSetPrivate &other)
        : QSharedData(other)
        , ncpus_max(other.ncpus_max)
        , ncpus_possible(other.ncpus_possible)
        , ncpus_present(other.ncpus_present)
        , ncpus_online(other.ncpus_online)
        , m_cores(other.m_cores)
        , m_sockets(other.m_sockets)
        , m_processors(other.m_processors)
        , m_model(other.m_model)
        , m_vendor(other.m_vendor)
        , m_virtualization(other.m_virtualization)
        , m_stat(std::make_shared<cpu_stat_t>(*(other.m_stat)))
        , m_usage(std::make_shared<cpu_usage_t>(*(other.m_usage)))
        , m_info(other.m_info)
    {
        for (auto &stat : other.m_statDB) {
            if (stat) {
                auto cp = std::make_shared<cpu_stat_t>(*stat);
                m_statDB[stat->cpu] = cp;
            }
        }
        for (auto &usage : other.m_usageDB) {
            if (usage) {
                auto cp = std::make_shared<cpu_usage_t>(*usage);
                m_usageDB[usage->cpu] = cp;
            }
        }
        for (auto &info : other.m_infos) {
            CPUInfo cp(info);
            m_infos << cp;
        }
    }
    ~CPUSetPrivate() {}

private:
    int ncpus_max; // kernel_max
    int ncpus_possible; // /sys/devices/system/cpu/possible
    int ncpus_present; // /sys/devices/system/cpu/present
    int ncpus_online; // /sys/devices/system/cpu/online
    quint32 m_cores; // number of cores
    quint32 m_sockets; // number of sockets
    quint32 m_processors; // number of processors
    QString m_model; // processor model
    QString m_vendor; // processor vendor
    QString m_virtualization; // virtualization tech

    CPUStat m_stat; // overall stat
    CPUUsage m_usage; // overall usage

    QMap<QByteArray, CPUStat> m_statDB; // per cpu stat
    QMap<QByteArray, CPUUsage> m_usageDB; // per cpu usage

    qulonglong cpusageTotal[kStatCount] = {0, 0};
    friend class CPUSet;

    QMap<QString, QString> m_info;   //overall info
    QList<CPUInfo> m_infos;         //per cpu info
};

} // namespace system
} // namespace core

#endif // CPU_SET_P_H
