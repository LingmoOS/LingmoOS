// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/desktop_entry.h"
#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

namespace Lingmo::base {

namespace dbus = Lingmo::base::dbus;

class EventLoop;

// The installed applications: every .desktop file in the user's and the
// system's applications directories, read on a thread and read again when
// they change or the session language does.
class ApplicationRegistry
{
public:
    // The loop's registry, made on first use and gone with the loop.
    static ApplicationRegistry& instance(EventLoop* loop = nullptr);
    // A registry of other applications directories, for tests.
    static std::unique_ptr<ApplicationRegistry> create(EventLoop& loop, std::vector<std::string> directories);

    ApplicationRegistry(const ApplicationRegistry&) = delete;
    ApplicationRegistry& operator=(const ApplicationRegistry&) = delete;

    // Until the first scan has arrived, and while another runs.
    bool isLoading() const { return m_loading; }

    // An exact id first, then one that differs only in case.
    std::shared_ptr<const DesktopEntry> byId(const std::string& id) const;
    std::shared_ptr<const DesktopEntry> byPath(const std::string& path) const;
    // The entry a window or process belongs to, from its app id or window
    // class and its executable's path and name; null when none fits, or when
    // nothing as certain as the app id or class picks one of several entries
    // that fit equally well.
    std::shared_ptr<const DesktopEntry> resolveWindow(const std::string& appId, const std::string& windowClass,
                                                      const std::string& executablePath,
                                                      const std::string& executableName) const;
    // Every entry, the ones menus leave out included.
    const std::vector<std::shared_ptr<const DesktopEntry>>& entries() const { return m_entries; }
    // What menus show, sorted by name.
    const std::vector<std::shared_ptr<const DesktopEntry>>& applications() const { return m_applications; }

    // After each scan.
    Signal<> changed;

    // ~/.local/share/applications, then the system's.
    static std::vector<std::string> desktopPaths();

    void scheduleScan();

private:
    ApplicationRegistry(EventLoop& loop, std::vector<std::string> directories);
    friend struct std::default_delete<ApplicationRegistry>;
    ~ApplicationRegistry() = default;

    void startScan();
    void applyScan(std::vector<DesktopEntry> results);
    void monitorPaths();

    EventLoop& m_loop;
    std::vector<std::string> m_directories;
    Timer m_debounce;
    // Brings back the watches that could not be made, such as when the
    // user's inotify watches ran out.
    Timer m_watchRetry;
    std::chrono::milliseconds m_watchRetryDelay;
    Job m_scan;
    std::vector<FileWatch> m_watches;
    dbus::Match m_languageChanged;
    std::string m_language;
    std::map<std::string, std::shared_ptr<const DesktopEntry>> m_byId;
    std::map<std::string, std::shared_ptr<const DesktopEntry>> m_byPath;
    std::vector<std::shared_ptr<const DesktopEntry>> m_entries;
    std::vector<std::shared_ptr<const DesktopEntry>> m_applications;
    bool m_loading = false;
    bool m_scanQueued = false;
};

} // namespace Lingmo::base
