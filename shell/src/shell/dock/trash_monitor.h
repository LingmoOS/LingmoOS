// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

namespace Lingmo::Shell {

// Whether any of the user's trashes holds something, for the dock's trash
// icon: the home trash and every volume's $topdir/.Trash/$uid and
// $topdir/.Trash-$uid, found from the mount table as the file manager finds
// them. Looked into on a worker, since a slow mount must not hold the
// compositor; followed through the trashes' files/ folders, the entries that
// would create one, and the mount table.
class TrashMonitor
{
public:
    // Tests give a mount table and a home trash of their own.
    explicit TrashMonitor(std::string mountTable = "/proc/self/mountinfo", std::string homeTrash = {});
    ~TrashMonitor();

    TrashMonitor(const TrashMonitor &) = delete;
    TrashMonitor &operator=(const TrashMonitor &) = delete;

    // Until the first look, the trash counts as empty.
    bool isEmpty() const { return m_empty; }
    // Whether a watch could not be added and is tried again, for tests.
    bool isRetrying() const { return m_retry.isActive(); }

    // What the dock's trash does, through the session's file manager.
    static void moveToTrash(const std::vector<std::string> &paths,
                            base::dbus::Connection *bus = base::dbus::Connection::session());
    static void emptyTrash(base::dbus::Connection *bus = base::dbus::Connection::session());
    static void openTrash(base::dbus::Connection *bus = base::dbus::Connection::session());

    base::Signal<> emptyChanged;

    struct State {
        bool empty = true;
        // files/ of each trash.
        std::vector<std::string> paths;
        // A directory, and the names in it whose coming or going matters.
        std::map<std::string, std::vector<std::string>> entries;
    };
    // The trashes and whether one holds something; any thread.
    static State look(const std::string &mountTable, const std::string &homeTrash);

private:
    struct Unwatched {
        std::string directory;
        // Empty for a trash's files/, whose every change matters.
        std::vector<std::string> names;
    };

    void refresh();
    void watch(const State &state);
    bool addWatch(const std::string &directory, const std::vector<std::string> &names);
    void retryWatches();

    std::string m_mountTable;
    std::string m_homeTrash;
    bool m_empty = true;
    bool m_pending = false;
    base::Job m_look;
    base::Timer m_debounce;
    base::Timer m_retry;
    std::vector<base::FileWatch> m_watches;
    std::vector<Unwatched> m_unwatched;
    std::chrono::milliseconds m_retryDelay;
    int m_mountWatch = 0;
    int m_mountFd = -1;
};

} // namespace Lingmo::Shell

