// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/timer.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::Files {

// Follows folders through the inotify instance the loop's thread shares with
// every other watch in the process: one instance per process, whose watches
// cost nothing to drop, where an instance of its own blocks for a kernel grace
// period when it closes with watches held.
class FileWatcher
{
public:
    // Folder paths to the names of their entries that count.
    using Entries = std::map<std::string, std::vector<std::string>>;

    // On the calling thread's loop when none is given.
    explicit FileWatcher(base::EventLoop* loop = nullptr);
    ~FileWatcher();

    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    bool watch(const std::string& path) { return watch(std::vector<std::string>{path}); }

    // Watches each folder of paths, and the entry naming it in its parent so that one
    // created, replaced or removed is noticed; in each folder of entries, only those
    // entries; with mounts, the mount table. Returns whether every folder of paths is.
    bool watch(const std::vector<std::string>& paths, const Entries& entries = {}, bool mounts = false);
    void clear();

    // Once for the changes the kernel reported together.
    base::Signal<> changed;
    base::Signal<const std::string&> error;

private:
    // The entries of a watched folder that count, or every one.
    struct Filter
    {
        bool all = false;
        std::vector<std::string> names;
    };

    void noticed(const Filter& filter, const std::string& name);

    base::EventLoop* m_loop;
    std::vector<std::string> m_paths;
    Entries m_entries;
    bool m_mounts = false;
    // Cleared once a watch is removed or its directory moves or goes away.
    bool m_valid = false;
    // Shared with the callbacks, which outlive no watch.
    std::vector<std::unique_ptr<Filter>> m_filters;
    std::vector<base::FileWatch> m_watches;
    int m_mountTable = -1;
    int m_mountWatch = 0;
    base::Timer m_emit;
};

} // namespace Lingmo::Files
