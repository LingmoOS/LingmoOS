// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/file_watcher.h"

#include <algorithm>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/strings.h"
#include "framework_i18n.h"
#include "path_utils_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

FileWatcher::FileWatcher(EventLoop* loop)
    : m_loop(loop ? loop : EventLoop::current())
    , m_emit(m_loop)
{
    m_emit.setSingleShot(true);
    m_emit.setInterval(std::chrono::nanoseconds(0));
    m_emit.onTimeout = [this] {
        changed.emit();
    };
}

FileWatcher::~FileWatcher()
{
    clear();
}

void FileWatcher::clear()
{
    m_watches.clear();
    m_filters.clear();
    m_paths.clear();
    m_entries.clear();
    m_mounts = false;
    m_valid = false;

    if (m_mountWatch)
        m_loop->unwatch(m_mountWatch);
    m_mountWatch = 0;

    if (m_mountTable >= 0)
        ::close(m_mountTable);
    m_mountTable = -1;
}

void FileWatcher::noticed(const Filter& filter, const std::string& name)
{
    // The folder itself was removed, moved or stopped being watched.
    if (name.empty())
        m_valid = false;
    else if (!filter.all && std::find(filter.names.cbegin(), filter.names.cend(), name) == filter.names.cend())
        return;

    if (!m_emit.isActive())
        m_emit.start();
}

bool FileWatcher::watch(const std::vector<std::string>& paths, const Entries& entries, bool mounts)
{
    // Adding a directory watch walks every child dentry; keep one that still holds.
    if (m_valid && paths == m_paths && entries == m_entries && mounts == m_mounts)
        return true;

    clear();

    std::map<std::string, Filter> wanted;
    for (const auto& path : paths) {
        const std::string clean = internal::cleanPath(path);
        const std::string parent = internal::absolutePathOf(clean);
        wanted[clean].all = true;

        if (parent != clean)
            wanted[parent].names.push_back(internal::fileNameOf(clean));
    }

    for (const auto& [path, names] : entries) {
        auto& filter = wanted[internal::cleanPath(path)];
        filter.names.insert(filter.names.end(), names.cbegin(), names.cend());
    }

    std::vector<std::string> unwatched;

    for (auto& [path, filter] : wanted) {
        auto owned = std::make_unique<Filter>(std::move(filter));
        const Filter* shared = owned.get();
        FileWatch watch = FileWatch::directory(path, [this, shared](const std::string& name) {
            noticed(*shared, name);
        }, m_loop);

        if (watch.isActive()) {
            m_filters.push_back(std::move(owned));
            m_watches.push_back(std::move(watch));
        } else if (shared->all) {
            unwatched.push_back(path);
        }
    }

    m_paths = paths;
    m_entries = entries;
    m_mounts = mounts;
    m_valid = unwatched.empty();

    // The mount table reports a change as an exceptional condition.
    if (mounts) {
        m_mountTable = ::open("/proc/self/mountinfo", O_RDONLY | O_CLOEXEC);
        if (m_mountTable >= 0) {
            m_mountWatch = m_loop->watch(m_mountTable, EPOLLPRI, [this](uint32_t) {
                changed.emit();
            });
        }
    }

    if (!unwatched.empty())
        error.emit(format(tr("Cannot watch {paths}; polling for changes"), {{"paths", strings::join(unwatched, ", ")}}));

    return m_valid;
}

} // namespace Lingmo::Files
