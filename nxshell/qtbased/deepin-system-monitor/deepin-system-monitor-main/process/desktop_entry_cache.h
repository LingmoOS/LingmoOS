// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ENTRY_CACHE_H
#define DESKTOP_ENTRY_CACHE_H

#include <QHash>

#include <memory>

namespace core {
namespace process {

struct desktop_entry_t {
    QString name;
    QString displayName;
    QStringList exec;
    QString icon;
    QString startup_wm_class;
};
using DesktopEntry = std::shared_ptr<struct desktop_entry_t>;

class DesktopEntryCache
{
public:
    explicit DesktopEntryCache();
    virtual ~DesktopEntryCache() = default;

    bool contains(const QString &name) const;
    const DesktopEntry entry(const QString &name) const;
    const DesktopEntry entryWithDesktopFile(const QString &desktopFile);
    const DesktopEntry entryWithSubName(const QString &subName) const;
    QHash<QString, DesktopEntry> getCache();

    void updateCache();


private:
    QHash<QString, DesktopEntry> m_cache;
};

inline bool DesktopEntryCache::contains(const QString &name) const
{
    return m_cache.contains(name);
}

inline const DesktopEntry DesktopEntryCache::entry(const QString &name) const
{
    return m_cache[name];
}

inline const DesktopEntry DesktopEntryCache::entryWithSubName(const QString &subName) const
{
    for (auto &key : m_cache.keys()) {
        if (key.toLower().compare(subName) == 0)
            return m_cache[key];
    }
    for (auto &key : m_cache.keys()) {
        if (key.toLower().contains(subName))
            return m_cache[key];
    }
    return {};
}

} // namespace process
} // namespace core

#endif // DESKTOP_ENTRY_CACHE_H
