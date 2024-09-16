// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GUI_APPS_CACHE_H
#define GUI_APPS_CACHE_H

#include <QList>

namespace core {
namespace process {

class GuiAppsCache
{
public:
    explicit GuiAppsCache();
    virtual ~GuiAppsCache() = default;

    bool isGuiApp(pid_t pid) const;
    void updateCache(const QList<pid_t> &cache);

private:
    QList<pid_t> m_cache;
};

inline bool GuiAppsCache::isGuiApp(pid_t pid) const
{
    return m_cache.contains(pid);
}

inline void GuiAppsCache::updateCache(const QList<pid_t> &cache)
{
    m_cache = cache;
}

} // namespace process
} // namespace core

#endif // GUI_APPS_CACHE_H
