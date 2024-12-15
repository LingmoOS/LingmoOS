// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRAY_APPS_CACHE_H
#define TRAY_APPS_CACHE_H

#include <QList>

namespace core {
namespace process {

class TrayAppsCache
{
public:
    explicit TrayAppsCache();
    ~TrayAppsCache() = default;

    bool isTrayApp(pid_t pid) const;

    void updateCache(const QList<pid_t> &cache);

private:
    QList<pid_t> m_cache;
};

inline bool TrayAppsCache::isTrayApp(pid_t pid) const
{
    return m_cache.contains(pid);
}

} // namespace process
} // namespace core

#endif // TRAY_APPS_CACHE_H
