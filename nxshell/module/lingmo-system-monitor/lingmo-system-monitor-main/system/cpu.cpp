// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpu.h"
#include "cpu_set.h"
#include "private/cpu_p.h"
#include "common/common.h"

#define PATH_TPL_CPU_CACHE "/sys/devices/system/cpu/%d/cache"
#define PATH_TPL_CPU_CACHE_ID "/sys/devices/system/cpu/%d/cache/index%d/id"
#define PATH_TPL_CPU_CACHE_TYPE "/sys/devices/system/cpu/%d/cache/index%d/type"
#define PATH_TPL_CPU_CACHE_SIZE "/sys/devices/system/cpu/%d/cache/index%d/size"
#define PATH_TPL_CPU_CACHE_LEVEL "/sys/devices/system/cpu/%d/cache/index%d/level"
#define PATH_TPL_CPU_CACHE_SHR_CPU_LIST "/sys/devices/system/cpu/%d/cache/index%d/shared_cpu_list"

#define PATH_TPL_CPU_FREQ_MAX "/sys/devices/system/cpu/%d/cpufreq/cpuinfo_max_freq"
#define PATH_TPL_CPU_FREQ_MIN "/sys/devices/system/cpu/%d/cpufreq/cpuinfo_min_freq"
#define PATH_TPL_CPU_FREQ_CUR "/sys/devices/system/cpu/%d/cpufreq/scaling_cur_freq"

#define PATH_TPL_CPU_TOPO_CORE_ID "/sys/devices/system/cpu/%d/topology/core_id"
#define PATH_TPL_CPU_TOPO_CORE_SIBLINGS "/sys/devices/system/cpu/%d/topology/core_siblings_list"
#define PATH_TPL_CPU_TOPO_PHY_PKG_ID "/sys/devices/system/cpu/%d/topology/physical_package_id"
#define PATH_TPL_CPU_TOPO_THR_SIBLINGS "/sys/devices/system/cpu/%d/topology/thread_siblings_list"

namespace core {
namespace system {

CPUInfo::CPUInfo()
    : d(new CPUInfoPrivate())
{
}
CPUInfo::CPUInfo(int index)
    : d(new CPUInfoPrivate())
{
    d->index = index;
}
CPUInfo::CPUInfo(const CPUInfo &other)
    : d(other.d)
{
}
CPUInfo &CPUInfo::operator=(const CPUInfo &rhs)
{
    if (this == &rhs)
        return *this;

    d = rhs.d;
    return *this;
}
CPUInfo::~CPUInfo()
{
}

int CPUInfo::logicalIndex() const
{
    return d->index;
}

QByteArray CPUInfo::logicalName() const
{
    return d->cpu;
}

QString CPUInfo::coreID() const
{
    return QString::number(d->m_core_id);
}

QString CPUInfo::modelName() const
{
    return d->m_model_name;
}

QString CPUInfo::vendorId() const
{
    return d->m_vendor_id;
}

QString CPUInfo::cpuFreq() const
{
    if( d->m_cpu_freq == "-") {
        return "-";
    }
    return d->m_cpu_freq + "MHz";
}

QString CPUInfo::cacheSize() const
{
    return d->m_cache_size;
}

QString CPUInfo::l1iCache() const
{
    for (auto &ce : d->cache) {
        if (ce->level == 1 && ce->type == "Instruction")
            return common::format::formatUnit_memory_disk({ce->size}, common::format::B, 0);
    }

    return "";
}

QString CPUInfo::l1dCache() const
{
    for (auto &ce : d->cache) {
        if (ce->level == 1 && ce->type == "Data")
            return common::format::formatUnit_memory_disk({ce->size}, common::format::B, 0);
    }

    return "";
}

QString CPUInfo::l2Cache() const
{
    for (auto &ce : d->cache) {
        if (ce->level == 2)
            return common::format::formatUnit_memory_disk({ce->size}, common::format::B, 0);
    }

    return "";
}

QString CPUInfo::l3Cache() const
{
    for (auto &ce : d->cache) {
        if (ce->level == 3)
            return common::format::formatUnit_memory_disk({ce->size}, common::format::B, 0);
    }

    return "";
}

void CPUInfo::setIndex(int index)
{
    d->index = index;
}

void CPUInfo::setCoreId(int coreId)
{
    d->m_core_id = coreId;
}

void CPUInfo::setModelName(QString modelName)
{
    d->m_model_name = modelName;
}

void CPUInfo::setVendorId(QString vendorId)
{
    d->m_vendor_id = vendorId;
}

void CPUInfo::setCpuFreq(QString cpuFreq)
{
    d->m_cpu_freq = cpuFreq;
}

void CPUInfo::setCacheSize(QString cacheSize)
{
    d->m_cache_size = cacheSize;
}

} // namespace system
} // namespace core
