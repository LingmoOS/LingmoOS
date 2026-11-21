// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/trash_monitor.h"

#include <algorithm>
#include <chrono>
#include <dirent.h>
#include <sys/stat.h>

#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace std::chrono_literals;

static constexpr std::chrono::milliseconds FirstRetry = 2000ms;
static constexpr std::chrono::milliseconds LastRetry = 60000ms;

static bool hasEntries(const std::string& path)
{
    DIR* dir = ::opendir(path.c_str());
    if (!dir)
        return false;

    bool found = false;
    while (const dirent* entry = ::readdir(dir)) {
        const std::string_view name = entry->d_name;
        if (name != "." && name != "..") {
            found = true;
            break;
        }
    }
    ::closedir(dir);
    return found;
}

TrashMonitor::TrashMonitor()
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(40ms);
    m_retry.setSingleShot(true);
    m_retry.setInterval(FirstRetry);

    m_watchChanged = m_watch.changed.connect([this] {
        m_debounce.start();
    });
    m_debounce.onTimeout = [this] {
        refresh();
    };
    m_retry.onTimeout = [this] {
        refresh();
    };

    refresh();
}

void TrashMonitor::refresh()
{
    if (m_job.isPending()) {
        m_pending = true;
        return;
    }

    // Looking for the trash folders reads the mount table and stats each mount.
    m_job = base::ThreadPool::global().run(
        [] {
            State state;
            internal::trashWatches(&state.paths, &state.entries);

            for (const auto& trash : internal::trashDirectories()) {
                if (hasEntries(trash.path + "/files")) {
                    state.empty = false;
                    break;
                }
            }

            return state;
        },
        [this](State state) {
            // Mounts and unmounts add and remove volumes' trash, so a folder found can be
            // gone by now.
            std::vector<std::string> paths;
            for (const auto& path : state.paths) {
                struct stat st;
                if (::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
                    paths.push_back(path);
            }

            // A watch that keeps failing, past the inotify limit say, is tried less often.
            if (m_watch.watch(paths, state.entries, true)) {
                m_retry.stop();
                m_retry.setInterval(FirstRetry);
            } else {
                m_retry.start();
                m_retry.setInterval(std::min<std::chrono::nanoseconds>(m_retry.interval() * 2, LastRetry));
            }

            if (state.empty != m_empty) {
                m_empty = state.empty;
                emptyChanged.emit();
            }

            if (m_pending) {
                m_pending = false;
                refresh();
            }
        });
}

} // namespace Lingmo::Files
