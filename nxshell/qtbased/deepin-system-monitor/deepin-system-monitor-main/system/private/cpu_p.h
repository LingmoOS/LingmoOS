// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_P_H
#define CPU_P_H

#include <QSharedData>

#include <memory>

namespace core {
namespace system {

struct cpu_cache_t {
    QByteArray name;
    QByteArray type;
    qulonglong size;
    int level;
    int shared_cpu_map;
};
using CPUCache = std::unique_ptr<struct cpu_cache_t>;
using CPUCacheList = std::list<CPUCache>;

class CPUInfo;

class CPUInfoPrivate : public QSharedData
{
public:
    CPUInfoPrivate()
        : QSharedData()
        , index {0}
        , package_id {0}
        , book_id {0}
        , drawer_id {0}
        , cpu {}
        , cache {}
    {
    }

    CPUInfoPrivate(const CPUInfoPrivate &other)
        : QSharedData(other)
        , index {other.index}
        , package_id {other.package_id}
        , book_id {other.book_id}
        , drawer_id {other.drawer_id}
        , cpu {other.cpu}
        , cache {}
    {
        for (auto &citem : other.cache) {
            CPUCache cp(new cpu_cache_t());
            *cp = *citem;
            cache.push_back(std::move(cp));
        }
    }
    ~CPUInfoPrivate() {}

private:
    int index = -1;          // logical index
    int m_core_id = -1;   // number of cores
    QString m_model_name; // processor model
    QString m_vendor_id; // processor vendor
    QString m_cpu_freq;  //m hz
    QString m_cache_size;

    int package_id; // cpu package index
    int book_id; // cpu book index
    int drawer_id; // cpu drawer index

    QByteArray cpu;
    CPUCacheList cache; // cpu cache list

    friend class CPUInfo;
};

} // namespace system
} // namespace core

#endif // CPU_P_H
