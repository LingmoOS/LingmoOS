// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "lingmo/files/file_watcher.h"

namespace Lingmo::Files {

// Whether anything is in the trash: the home trash or that of any mounted volume.
class TrashMonitor
{
public:
    TrashMonitor();

    TrashMonitor(const TrashMonitor&) = delete;
    TrashMonitor& operator=(const TrashMonitor&) = delete;

    bool isEmpty() const { return m_empty; }

    base::Signal<> emptyChanged;

private:
    void refresh();

    struct State
    {
        bool empty = true;
        std::vector<std::string> paths;
        FileWatcher::Entries entries;
    };

    FileWatcher m_watch;
    base::Subscription m_watchChanged;
    base::Timer m_debounce;
    base::Timer m_retry;
    base::Job m_job;
    bool m_pending = false;
    bool m_empty = true;
};

} // namespace Lingmo::Files
