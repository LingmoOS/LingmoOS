// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_NAME_CACHE_H
#define PROCESS_NAME_CACHE_H

#include "process_name.h"

#include <QCache>
#include <QObject>

namespace core {
namespace process {

/**
 * @brief The ProcessNameCache class
 */
class ProcessNameCache : public QObject
{
    Q_OBJECT

public:
    static ProcessNameCache *instance();

    void insert(pid_t pid, ProcessName *name);
    void remove(pid_t pid);
    void clear();

    const ProcessName *processName(pid_t pid) const;
    bool contains(pid_t pid) const;

protected:
    explicit ProcessNameCache(QObject *parent = nullptr);

private:
    QCache<pid_t, ProcessName> m_cache;

    static ProcessNameCache *m_instance;
};

inline ProcessNameCache *ProcessNameCache::instance()
{
    if (!m_instance)
        m_instance = new ProcessNameCache();

    return m_instance;
}

inline void ProcessNameCache::insert(pid_t pid, ProcessName *name)
{
    m_cache.insert(pid, name);
}

inline void ProcessNameCache::remove(pid_t pid)
{
    m_cache.remove(pid);
}

inline void ProcessNameCache::clear()
{
    m_cache.clear();
}

inline const ProcessName *ProcessNameCache::processName(pid_t pid) const
{
    return m_cache[pid];
}

inline bool ProcessNameCache::contains(pid_t pid) const
{
    return m_cache.contains(pid);
}

} // namespace process
} // namespace core

#endif // PROCESS_NAME_CACHE_H
