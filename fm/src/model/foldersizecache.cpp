// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/foldersizecache.h"

#include "base/event_loop.h"

#include <dirent.h>

namespace fm {

static base::ThreadPool& countPool()
{
    static base::ThreadPool pool(2);
    return pool;
}

FolderSizeCache::FolderSizeCache()
    : m_cancelled(std::make_shared<std::atomic_bool>(false))
{
}

FolderSizeCache::~FolderSizeCache()
{
    *m_alive = false;
    m_cancelled->store(true);
}

void FolderSizeCache::clear()
{
    m_counts.clear();
}

void FolderSizeCache::reset()
{
    m_cancelled->store(true);
    m_cancelled = std::make_shared<std::atomic_bool>(false);
    m_counts.clear();
}

int64_t FolderSizeCache::count(const std::string& path)
{
    if (const auto found = m_counts.find(path); found != m_counts.end())
        return found->second;

    m_counts[path] = -1;
    const auto cancelled = m_cancelled;
    const base::LoopHandle loop = base::EventLoop::current()->handle();
    countPool().post([this, alive = m_alive, cancelled, loop, path] {
        int64_t count = 0;
        if (DIR* directory = ::opendir(path.c_str())) {
            while (!cancelled->load()) {
                dirent* entry = ::readdir(directory);
                if (!entry)
                    break;
                // Hidden entries, "." and ".." among them, are not counted.
                if (entry->d_name[0] != '.')
                    ++count;
            }
            ::closedir(directory);
        }
        loop.post([this, alive, cancelled, path, count] {
            if (!*alive || cancelled->load())
                return;
            m_counts[path] = count;
            ready.emit(path);
        });
    });
    return -1;
}

} // namespace fm
