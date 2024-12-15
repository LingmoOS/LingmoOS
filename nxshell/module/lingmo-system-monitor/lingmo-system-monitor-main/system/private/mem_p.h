// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEM_P_H
#define MEM_P_H

#include <QSharedData>

namespace core {
namespace system {

class MemInfo;

class MemInfoPrivate : public QSharedData
{
public:
    MemInfoPrivate()
        : QSharedData()
        , mem_total_kb {0}
        , mem_free_kb {0}
        , mem_avail_kb {0}
        , buffers_kb {0}
        , cached_kb {0}
        , active_kb {0}
        , inactive_kb {0}
        , swap_total_kb {0}
        , swap_free_kb {0}
        , swap_cached_kb {0}
        , shmem_kb {0}
        , slab_kb {0}
        , dirty_kb {0}
        , mapped_kb {0}
    {
    }

    MemInfoPrivate(const MemInfoPrivate &other)
        : QSharedData(other)
        , mem_total_kb(other.mem_total_kb)
        , mem_free_kb(other.mem_free_kb)
        , mem_avail_kb(other.mem_avail_kb)
        , buffers_kb(other.buffers_kb)
        , cached_kb(other.cached_kb)
        , active_kb(other.active_kb)
        , inactive_kb(other.inactive_kb)
        , swap_total_kb(other.swap_total_kb)
        , swap_free_kb(other.swap_free_kb)
        , swap_cached_kb(other.swap_cached_kb)
        , shmem_kb(other.shmem_kb)
        , slab_kb(other.slab_kb)
        , dirty_kb(other.dirty_kb)
        , mapped_kb(other.mapped_kb)
    {
    }

private:
    unsigned long long mem_total_kb; // MemTotal
    unsigned long long mem_free_kb; // MemFree
    unsigned long long mem_avail_kb; // MemAvailable
    unsigned long long buffers_kb; // Buffers
    unsigned long long cached_kb; // Cached
    unsigned long long active_kb; // Active
    unsigned long long inactive_kb; // Inactive

    unsigned long long swap_total_kb; // SwapTotal
    unsigned long long swap_free_kb; // SwapFree
    unsigned long long swap_cached_kb; // SwapCached
    unsigned long long shmem_kb; // Shared
    unsigned long long slab_kb; // Slab
    unsigned long long dirty_kb; // Dirty
    unsigned long long mapped_kb; // Mapped

    friend class MemInfo;
};

} // namespace system
} // namespace core

#endif // MEM_P_H
