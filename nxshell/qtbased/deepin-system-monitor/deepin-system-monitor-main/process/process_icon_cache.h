// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_ICON_CACHE_H
#define PROCESS_ICON_CACHE_H

#include <QCache>
#include <QThread>
#include <QPixmapCache>

namespace core {
namespace process {

class ProcessIcon;
class ProcessIconCache : QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProcessIconCache)

public:
    static ProcessIconCache *instance();

    ProcessIcon *getProcessIcon(pid_t pid) const;
    void addProcessIcon(pid_t pid, ProcessIcon *procIcon);
    void removeProcessIcon(pid_t pid);
    bool contains(pid_t pid) const;
    void clear();
    void setMaxCost(int cost);

public:
    QPixmapCache iconPixmapCache;

private:
    explicit ProcessIconCache(QObject *parent = nullptr);

private:
    QCache<pid_t, ProcessIcon> m_cache {200};

    static ProcessIconCache *m_instance;
};

inline ProcessIconCache *ProcessIconCache::instance()
{
    if (!m_instance)
        m_instance = new ProcessIconCache();

    return m_instance;
}

inline ProcessIcon *ProcessIconCache::getProcessIcon(pid_t pid) const
{
    if (m_cache.contains(pid))
        return m_cache[pid];

    return nullptr;
}

inline void ProcessIconCache::addProcessIcon(pid_t pid, ProcessIcon *procIcon)
{
    m_cache.insert(pid, procIcon);
}

inline void ProcessIconCache::removeProcessIcon(pid_t pid)
{
    m_cache.remove(pid);
}

inline bool ProcessIconCache::contains(pid_t pid) const
{
    return m_cache.contains(pid);
}

inline void ProcessIconCache::clear()
{
    m_cache.clear();
}

inline void ProcessIconCache::setMaxCost(int cost)
{
    m_cache.setMaxCost(cost);
}

} // namespace process
} // namespace core

#endif // PROCESS_ICON_CACHE_H
