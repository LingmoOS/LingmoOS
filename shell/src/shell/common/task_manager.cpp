// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/task_manager.h"

#include <algorithm>
#include <utility>

#include "shell/common/task_windows.h"

namespace Lingmo::Shell {

TaskManager::TaskManager(TaskWindows &windows, Applications &applications, CommandReader commandReader,
                         ParentReader parentReader)
    : m_taskWindows(windows)
    , m_applications(applications)
    , m_commandReader(commandReader ? std::move(commandReader) : CommandReader(processCommand))
    , m_parentReader(parentReader ? std::move(parentReader) : ParentReader(parentProcess))
    , m_registryScanned(!applications.isLoading())
{
    m_subscriptions.push_back(m_taskWindows.windowAdded.connect([this](uint64_t id) { onWindowAdded(id); }));
    m_subscriptions.push_back(m_taskWindows.windowChanged.connect(
            [this](uint64_t id, uint32_t changes) { onWindowChanged(id, changes); }));
    m_subscriptions.push_back(m_taskWindows.windowRemoved.connect([this](uint64_t id) { onWindowRemoved(id); }));
    m_subscriptions.push_back(m_applications.changed.connect([this] { onApplicationsChanged(); }));

    for (uint64_t id : m_taskWindows.windows())
        onWindowAdded(id);

    updateReady();
}

TaskManager::~TaskManager() = default;

std::vector<uint64_t> TaskManager::windows(const std::string &application) const
{
    const auto it = m_groups.find(application);
    return it == m_groups.end() ? std::vector<uint64_t>() : it->second;
}

DesktopEntryPtr TaskManager::entry(const std::string &application) const
{
    const auto it = m_groups.find(application);
    return it == m_groups.end() || it->second.empty() ? nullptr : entryForWindow(it->second.front());
}

std::string TaskManager::applicationOf(uint64_t window) const
{
    const auto it = m_windows.find(window);
    return it == m_windows.end() ? std::string() : it->second.application;
}

DesktopEntryPtr TaskManager::entryForWindow(uint64_t window) const
{
    const auto it = m_windows.find(window);
    return it == m_windows.end() ? nullptr : it->second.entry;
}

DesktopEntryPtr TaskManager::resolveProcess(const std::string &appId, int64_t pid,
                                            const std::string &windowClass) const
{
    const ProcessCommand command = m_commandReader(pid);
    return m_applications.resolveWindow(appId, windowClass, command.program, command.name);
}

DesktopEntryPtr TaskManager::entryForProcess(int64_t pid, const std::string &appId) const
{
    if (!appId.empty())
        return resolveProcess(appId, pid, {});

    // A helper process, such as a browser's media process, goes by its window's.
    int depth = 0;
    for (int64_t process = pid; process > 1 && depth < 32; process = m_parentReader(process), ++depth) {
        const auto claim = m_claims.find(process);
        if (claim != m_claims.end()) {
            if (DesktopEntryPtr entry = m_applications.byId(claim->second))
                return entry;
        }
        for (const auto &[id, window] : m_windows) {
            if (window.pid == process && window.entry)
                return window.entry;
        }
    }

    return resolveProcess({}, pid, {});
}

void TaskManager::claimProcess(int64_t pid, const std::string &desktopId)
{
    m_claims[pid] = desktopId;
}

void TaskManager::releaseProcess(int64_t pid)
{
    m_claims.erase(pid);
}

void TaskManager::onWindowAdded(uint64_t id)
{
    if (m_windows.count(id)) {
        onWindowChanged(id, kAppIdChange | kPidChange | kTitleChange | kStateChange);
        return;
    }

    const TaskWindow *client = m_taskWindows.window(id);
    if (!client)
        return;

    Window &window = m_windows[id];
    window.appId = client->appId;
    window.title = client->title;
    window.pid = client->pid;
    window.taskList = client->isTaskListWindow();
    m_arrival.push_back(id);
    readProcess(window);

    resolve(id);
    regroup(id, false);
}

void TaskManager::onWindowChanged(uint64_t id, uint32_t changes)
{
    if (!m_windows.count(id)) {
        onWindowAdded(id);
        return;
    }

    constexpr uint32_t relevant = kTitleChange | kAppIdChange | kPidChange | kStateChange | kSkipTaskbarChange;
    if (!(changes & relevant))
        return;

    const TaskWindow *client = m_taskWindows.window(id);
    if (!client)
        return;
    Window &window = m_windows[id];

    bool entryChanged = false;
    if (changes & (kAppIdChange | kPidChange)) {
        window.appId = client->appId;
        window.claimedBy.clear();
        if (window.pid != client->pid) {
            window.pid = client->pid;
            readProcess(window);
        }
        entryChanged = resolve(id);
    }

    const bool titleChanged = window.title != client->title;
    window.title = client->title;
    window.taskList = client->isTaskListWindow();

    // A title only stands for an application that has no entry to name it.
    regroup(id, entryChanged || (titleChanged && !window.entry));
}

void TaskManager::onWindowRemoved(uint64_t id)
{
    if (!m_windows.count(id))
        return;

    leave(id);
    m_windows.erase(id);
    std::erase(m_arrival, id);
}

void TaskManager::onApplicationsChanged()
{
    if (m_applications.isLoading())
        return;
    m_registryScanned = true;

    // Entries come and go, and a new one can claim a window better than the
    // one it had; matching needs no /proc read, so every window is redone.
    const std::vector<uint64_t> arrival = m_arrival;
    for (uint64_t id : arrival) {
        if (m_windows.count(id))
            regroup(id, resolve(id));
    }

    updateReady();
}

void TaskManager::readProcess(Window &window)
{
    const ProcessCommand command = m_commandReader(std::max<int64_t>(0, window.pid));
    window.program = command.program;
    window.programName = command.name;
}

bool TaskManager::resolve(uint64_t id)
{
    Window &window = m_windows[id];
    DesktopEntryPtr entry = window.appId.empty() && window.program.empty()
            ? nullptr
            : m_applications.resolveWindow(window.appId, window.appId, window.program, window.programName);
    if (entry)
        window.claimedBy = entry->id;
    // Every scan reads the files into new entries; one that reads the same
    // is no change.
    const bool same = window.entry == entry
            || (window.entry && entry && window.entry->id == entry->id && window.entry->path == entry->path
                && window.entry->name == entry->name && window.entry->icon == entry->icon);
    window.entry = entry;
    if (same)
        return false;

    windowEntryChanged(id);
    return true;
}

void TaskManager::regroup(uint64_t id, bool changed)
{
    const Window &window = m_windows[id];
    std::string application;
    if (window.taskList && !window.appId.empty())
        application = window.claimedBy.empty() ? window.appId : window.claimedBy;

    if (application == window.application) {
        if (changed && !application.empty() && m_ready)
            applicationChanged(application);
        return;
    }

    leave(id);
    if (!application.empty())
        join(id, application);
}

void TaskManager::join(uint64_t id, const std::string &application)
{
    m_windows[id].application = application;

    const auto it = m_groups.find(application);
    if (it == m_groups.end()) {
        m_groups.emplace(application, std::vector<uint64_t>{id});
        m_order.push_back(application);
        if (m_ready)
            applicationAdded(application);
        return;
    }

    it->second.push_back(id);
    if (m_ready)
        applicationChanged(application);
}

void TaskManager::leave(uint64_t id)
{
    const std::string application = std::exchange(m_windows[id].application, std::string());
    if (application.empty())
        return;

    const auto it = m_groups.find(application);
    if (it == m_groups.end())
        return;

    std::erase(it->second, id);
    if (!it->second.empty()) {
        if (m_ready)
            applicationChanged(application);
        return;
    }

    m_groups.erase(it);
    std::erase(m_order, application);
    if (m_ready)
        applicationRemoved(application);
}

void TaskManager::updateReady()
{
    if (m_ready || !m_registryScanned)
        return;

    m_ready = true;
    readyChanged();
}

} // namespace Lingmo::Shell
