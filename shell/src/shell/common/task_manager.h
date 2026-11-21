// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/signal.h"
#include "shell/common/applications.h"
#include "shell/common/process_info.h"

namespace Lingmo::Shell {

class TaskWindows;

// Which application each window belongs to, for the whole shell.
//
// Every window is matched to its desktop entry once, when it appears and
// again only when its app id or process changes or the installed
// applications do. Task-list windows are grouped by application: the desktop
// id, or the app id of a window no entry claims. A window stays with the
// desktop id that claimed it until its app id or process changes, so an entry
// missing for a moment during an upgrade does not move it.
//
// Applications are announced only once the task list is ready, when the
// first scan of the installed applications is done, so no listener ever sees
// a window grouped under its bare app id merely because its entry had not
// been read yet.
class TaskManager
{
public:
    using CommandReader = std::function<ProcessCommand(int64_t pid)>;
    using ParentReader = std::function<int64_t(int64_t pid)>;

    // Both must outlive the task manager. The readers are /proc's unless a
    // test gives its own.
    TaskManager(TaskWindows &windows, Applications &applications, CommandReader commandReader = {},
                ParentReader parentReader = {});
    ~TaskManager();

    TaskManager(const TaskManager &) = delete;
    TaskManager &operator=(const TaskManager &) = delete;

    TaskWindows &taskWindows() const { return m_taskWindows; }
    Applications &applicationSource() const { return m_applications; }

    bool isReady() const { return m_ready; }

    // The applications with task-list windows, in the order they appeared.
    const std::vector<std::string> &applications() const { return m_order; }
    // The application's task-list windows, in the order they joined it.
    std::vector<uint64_t> windows(const std::string &application) const;
    // Null for an application no desktop entry claims.
    DesktopEntryPtr entry(const std::string &application) const;

    // The application a task-list window belongs to; empty for any other.
    std::string applicationOf(uint64_t window) const;
    // The desktop entry of any application window, task list or not.
    DesktopEntryPtr entryForWindow(uint64_t window) const;

    // The desktop entry of a process that has no window to go by, such as
    // the sender of a notification.
    DesktopEntryPtr resolveProcess(const std::string &appId, int64_t pid, const std::string &windowClass) const;
    // The desktop entry of a process or of the nearest ancestor with a window.
    DesktopEntryPtr entryForProcess(int64_t pid, const std::string &appId = {}) const;
    // Names a helper the shell itself runs for one of its own applications,
    // by that application's desktop id, until it is released.
    void claimProcess(int64_t pid, const std::string &desktopId);
    void releaseProcess(int64_t pid);

    base::Signal<> readyChanged;
    // Sent once the task list is ready.
    base::Signal<const std::string &> applicationAdded;
    base::Signal<const std::string &> applicationRemoved;
    // Its windows or its entry changed, or, without an entry, a window title.
    base::Signal<const std::string &> applicationChanged;
    // Sent whether or not the task list is ready.
    base::Signal<uint64_t> windowEntryChanged;

private:
    struct Window {
        std::string appId;
        std::string title;
        int64_t pid = 0;
        std::string program;
        std::string programName;
        bool taskList = false;
        DesktopEntryPtr entry;
        std::string claimedBy;
        std::string application;
    };

    void onWindowAdded(uint64_t id);
    void onWindowChanged(uint64_t id, uint32_t changes);
    void onWindowRemoved(uint64_t id);
    void onApplicationsChanged();

    void readProcess(Window &window);
    bool resolve(uint64_t id);
    void regroup(uint64_t id, bool changed);
    void join(uint64_t id, const std::string &application);
    void leave(uint64_t id);
    void updateReady();

    TaskWindows &m_taskWindows;
    Applications &m_applications;
    CommandReader m_commandReader;
    ParentReader m_parentReader;
    std::unordered_map<uint64_t, Window> m_windows;
    // Windows in the order they appeared; regrouping in it keeps an
    // application's order.
    std::vector<uint64_t> m_arrival;
    std::unordered_map<std::string, std::vector<uint64_t>> m_groups;
    std::vector<std::string> m_order;
    std::map<int64_t, std::string> m_claims;
    bool m_registryScanned = false;
    bool m_ready = false;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

