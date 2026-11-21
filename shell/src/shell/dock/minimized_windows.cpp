// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/minimized_windows.h"

#include "shell/common/task_windows.h"
#include "shell/dock/dock_model.h"
#include "shell/dock/dock_settings.h"

namespace Lingmo::Shell {

MinimizedWindows::MinimizedWindows(TaskWindows &windows, DockModel &dock, DockSettings &settings)
    : m_windows(windows)
    , m_dock(dock)
    , m_settings(settings)
{
    m_subscriptions.push_back(m_windows.windowAdded.connect([this](uint64_t id) { update(id); }));
    m_subscriptions.push_back(m_windows.windowChanged.connect([this](uint64_t id, uint32_t) { update(id); }));
    m_subscriptions.push_back(m_windows.windowRemoved.connect([this](uint64_t id) { remove(id, true); }));
    m_subscriptions.push_back(m_windows.minimizedSnapshotChanged.connect([this](uint64_t id) {
        const int row = indexOf(id);
        if (row >= 0)
            dataChanged(row, row);
    }));
    m_subscriptions.push_back(m_windows.minimizeLanded.connect([this](uint64_t id) { setLanded(id); }));
    m_subscriptions.push_back(m_settings.minimizedAreaChanged.connect([this] { onMinimizedAreaChanged(); }));
    // The dock resolves icons from desktop entries, which can arrive after
    // the window itself.
    m_subscriptions.push_back(m_dock.dataChanged.connect([this](int, int) {
        if (!m_entries.empty())
            dataChanged(0, count() - 1);
    }));

    // Windows minimized before the dock started are already in place.
    for (uint64_t id : m_windows.windows()) {
        update(id);
        setLanded(id);
    }
}

MinimizedWindows::~MinimizedWindows() = default;

int MinimizedWindows::indexOf(uint64_t id) const
{
    for (size_t row = 0; row < m_entries.size(); ++row) {
        if (m_entries[row].id == id)
            return int(row);
    }
    return -1;
}

std::string MinimizedWindows::title(int row) const
{
    const TaskWindow *window = m_windows.window(m_entries[size_t(row)].id);
    if (!window)
        return {};
    return window->title.empty() ? window->appId : window->title;
}

std::string MinimizedWindows::iconName(int row) const
{
    return m_dock.iconNameForWindow(m_entries[size_t(row)].id);
}

const ui::Bitmap *MinimizedWindows::thumbnail(int row) const
{
    return m_windows.minimizedSnapshot(m_entries[size_t(row)].id);
}

double MinimizedWindows::aspect(int row) const
{
    const TaskWindow *window = m_windows.window(m_entries[size_t(row)].id);
    if (!window || window->geometry.isEmpty())
        return 1.0;
    return double(window->geometry.width()) / window->geometry.height();
}

void MinimizedWindows::restore(uint64_t id)
{
    if (hideLauncher)
        hideLauncher();
    // Activating restores a minimized window first.
    m_windows.activate(id);
}

void MinimizedWindows::close(uint64_t id)
{
    m_windows.close(id);
}

void MinimizedWindows::move(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= count() || to >= count())
        return;
    const Entry entry = m_entries[size_t(from)];
    m_entries.erase(m_entries.begin() + from);
    m_entries.insert(m_entries.begin() + to, entry);
    rowMoved(from, to);
}

void MinimizedWindows::update(uint64_t id)
{
    const TaskWindow *window = m_windows.window(id);
    const bool minimized = window && window->isTaskListWindow() && window->minimized;
    const int row = indexOf(id);

    if (minimized && window->hidden)
        m_hiddenMinimized.insert(id);
    else if (!minimized)
        m_hiddenMinimized.erase(id);

    if (!minimized || window->hidden || !m_settings.minimizedArea()) {
        if (row >= 0)
            remove(id, false);
        return;
    }

    if (row >= 0) {
        dataChanged(row, row);
        return;
    }

    append(id, m_hiddenMinimized.erase(id) > 0);
}

void MinimizedWindows::append(uint64_t id, bool landed)
{
    m_entries.push_back({id, landed, !landed});
    rowsInserted(count() - 1, 1);
    if (landed)
        return;

    auto timer = std::make_unique<base::Timer>();
    timer->setSingleShot(true);
    timer->onTimeout = [this, id] { setLanded(id); };
    timer->start(kLandingFallback);
    m_landingTimers[id] = std::move(timer);
}

void MinimizedWindows::onMinimizedAreaChanged()
{
    if (!m_settings.minimizedArea()) {
        while (!m_entries.empty())
            remove(m_entries.back().id, false);
        return;
    }

    // Already minimized into their icons: revealed in the area, not flown in.
    for (uint64_t id : m_windows.windows()) {
        const TaskWindow *window = m_windows.window(id);
        if (window && window->isTaskListWindow() && window->minimized && !window->hidden && !contains(id))
            append(id, true);
    }
}

void MinimizedWindows::remove(uint64_t id, bool closed)
{
    const TaskWindow *window = m_windows.window(id);
    if (!window || !window->minimized)
        m_hiddenMinimized.erase(id);

    const int row = indexOf(id);
    if (row < 0)
        return;

    // A restored window flies out of the slot itself, so its thumbnail goes
    // at once; a closed one fades while the slot closes.
    if (!closed && window && !window->minimized && m_entries[size_t(row)].landed) {
        m_entries[size_t(row)].landed = false;
        dataChanged(row, row);
    }

    m_landingTimers.erase(id);
    m_entries.erase(m_entries.begin() + row);
    rowsRemoved(row, 1);
}

void MinimizedWindows::setLanded(uint64_t id)
{
    m_landingTimers.erase(id);
    const int row = indexOf(id);
    if (row < 0 || m_entries[size_t(row)].landed)
        return;
    m_entries[size_t(row)].landed = true;
    dataChanged(row, row);
}

} // namespace Lingmo::Shell
