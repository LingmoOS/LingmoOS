// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock_model.h"

#include <algorithm>
#include <filesystem>

#include "base/i18n.h"
#include "base/settings.h"
#include "shell/common/app_launcher.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/common/task_windows.h"
#include "shell/common/text_match.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

namespace pinned_applications {

static constexpr const char *kKey = "Dock/Pinned";

const std::vector<std::string> &defaults()
{
    static const std::vector<std::string> list = {
            "lingmo-filemanager", "lingmo-terminal",     "chromium",            "lingmo-settings",
            "lingmo-calculator",  "lingmo-debinstaller", "lingmo-screenshot", "lingmo-videoplayer",
    };
    return list;
}

std::optional<std::vector<std::string>> load(const std::string &path)
{
    const base::Settings settings(path);
    if (!settings.contains(kKey))
        return std::nullopt;
    return settings.stringListValue(kKey);
}

void save(const std::string &path, const std::vector<std::string> &desktopIds)
{
    writeSettings(path, [desktopIds](base::Settings &settings) { settings.setValue(kKey, desktopIds); });
}

} // namespace pinned_applications

static std::string absolutePath(const std::string &path)
{
    std::error_code error;
    return std::filesystem::absolute(path, error).lexically_normal().string();
}

DockModel::DockModel(TaskManager &tasks, ApplicationLauncher &launcher, std::string pinsPath)
    : m_tasks(tasks)
    , m_windows(tasks.taskWindows())
    , m_applications(tasks.applicationSource())
    , m_launcher(launcher)
    , m_pinsPath(std::move(pinsPath))
{
    m_installedTimer.setSingleShot(true);
    m_installedTimer.onTimeout = [this] { checkInstalled(); };

    m_subscriptions.push_back(m_tasks.readyChanged.connect([this] { load(); }));
    m_subscriptions.push_back(
            m_tasks.applicationAdded.connect([this](const std::string &a) { syncApplication(a); }));
    m_subscriptions.push_back(
            m_tasks.applicationRemoved.connect([this](const std::string &a) { syncApplication(a); }));
    m_subscriptions.push_back(
            m_tasks.applicationChanged.connect([this](const std::string &a) { syncApplication(a); }));
    m_subscriptions.push_back(m_windows.activeWindowChanged.connect([this](uint64_t id) { onActiveChanged(id); }));
    // The task manager was connected first, so windows are matched to a new
    // scan before the pins are.
    m_subscriptions.push_back(m_applications.changed.connect([this] { onApplicationsChanged(); }));

    m_pinsJob = runOnFileThread([path = m_pinsPath] { return pinned_applications::load(path); },
                                [this](std::optional<std::vector<std::string>> saved) {
                                    m_savedPins = std::move(saved);
                                    load();
                                });
}

DockModel::~DockModel() = default;

int DockModel::indexOf(const std::string &id) const
{
    for (size_t row = 0; row < m_items.size(); ++row) {
        if (m_items[row]->id == id)
            return int(row);
    }
    return -1;
}

int DockModel::rowOf(const DockItem *item) const
{
    for (size_t row = 0; row < m_items.size(); ++row) {
        if (m_items[row].get() == item)
            return int(row);
    }
    return -1;
}

DockItem *DockModel::findById(const std::string &id) const
{
    const int row = indexOf(id);
    return row < 0 ? nullptr : m_items[size_t(row)].get();
}

DockItem *DockModel::findByWindow(uint64_t window) const
{
    for (const auto &item : m_items) {
        if (std::find(item->windows.begin(), item->windows.end(), window) != item->windows.end())
            return item.get();
    }
    return nullptr;
}

DockItem *DockModel::findByDesktop(const std::string &desktopFile, DesktopEntryPtr *entry) const
{
    DesktopEntryPtr found = m_applications.byPath(desktopFile);
    if (!found)
        found = m_applications.byId(desktopFile);
    if (entry)
        *entry = found;
    if (found)
        return findById(found->id);

    // No longer installed: an item can still name the file it came from.
    if (!desktopFile.empty()) {
        const std::string path = absolutePath(desktopFile);
        for (const auto &item : m_items) {
            if (!item->desktopPath.empty() && absolutePath(item->desktopPath) == path)
                return item.get();
        }
    }
    return findById(desktopFile);
}

void DockModel::updateFromEntry(DockItem *item, const base::DesktopEntry &entry) const
{
    item->id = entry.id;
    if (!entry.icon.empty())
        item->iconName = entry.icon;
    if (!entry.name.empty())
        item->visibleName = entry.name;
    item->desktopPath = entry.path;
    if (entry.shouldShow())
        item->uninstalled = false;
}

void DockModel::changed(DockItem *item)
{
    const int row = rowOf(item);
    if (row >= 0) {
        dataChanged(row, row);
        sectionsChanged();
    }
}

int DockModel::pinnedCount() const
{
    return int(std::count_if(m_items.begin(), m_items.end(),
                             [](const auto &item) { return item->fixed || item->isPinned; }));
}

int DockModel::firstMovableRow() const
{
    int row = 0;
    while (row < count() && m_items[size_t(row)]->fixed)
        ++row;
    return row;
}

void DockModel::addItem(const std::string &desktopFile)
{
    insert(desktopFile, count());
}

void DockModel::removeItem(const std::string &desktopFile)
{
    if (DockItem *item = findByDesktop(desktopFile))
        unPin(item->id);
}

bool DockModel::isDesktopPinned(const std::string &desktopFile) const
{
    const DockItem *item = findByDesktop(desktopFile);
    return item && item->isPinned;
}

int DockModel::desktopRow(const std::string &desktopFile)
{
    DesktopEntryPtr entry;
    DockItem *item = findByDesktop(desktopFile, &entry);
    if (item && entry) {
        updateFromEntry(item, *entry);
        changed(item);
    }
    return rowOf(item);
}

void DockModel::clicked(const std::string &id)
{
    DockItem *item = findById(id);
    if (!item)
        return;

    if (item->id != kLauncherId && hideLauncher)
        hideLauncher();

    if (item->windows.empty()) {
        // A pinned application that is not running starts.
        openNewInstance(item->id);
    } else if (const uint64_t active = m_windows.activeWindow();
               std::find(item->windows.begin(), item->windows.end(), active) != item->windows.end()) {
        // Clicking the active application's icon toggles its current window
        // away; the next click restores and activates it again.
        m_windows.minimize(active);
    } else if (unhide(item)) {
        // A hidden application comes back whole, to the window it was on.
        if (item->currentActive >= item->windows.size())
            item->currentActive = 0;
        m_windows.activate(item->windows[item->currentActive]);
    } else if (m_windows.activeWindow() == 0) {
        // No application is active after the previous click minimized one;
        // restore that same window instead of cycling to the next.
        if (item->currentActive >= item->windows.size())
            item->currentActive = 0;
        m_windows.activate(item->windows[item->currentActive]);
    } else if (item->windows.size() > 1) {
        // Several windows: each click shows the next.
        if (++item->currentActive == item->windows.size())
            item->currentActive = 0;
        m_windows.activate(item->windows[item->currentActive]);
    } else {
        m_windows.activate(item->windows.front());
    }
}

void DockModel::raiseWindow(const std::string &id)
{
    DockItem *item = findById(id);
    // Pinned items and the launcher have no windows to raise.
    if (!item || item->windows.empty())
        return;
    if (item->currentActive >= item->windows.size())
        item->currentActive = 0;
    if (hideLauncher)
        hideLauncher();
    unhide(item);
    m_windows.activate(item->windows[item->currentActive]);
}

std::vector<DockModel::WindowInfo> DockModel::windows(const std::string &id) const
{
    std::vector<WindowInfo> list;
    const DockItem *item = findById(id);
    if (!item)
        return list;
    const uint64_t active = m_windows.activeWindow();
    for (uint64_t window : item->windows) {
        const TaskWindow *client = m_windows.window(window);
        const std::string title = client ? client->title : std::string();
        list.push_back({window, title.empty() ? item->visibleName : title, window == active});
    }
    return list;
}

void DockModel::activateWindow(uint64_t window)
{
    DockItem *item = findByWindow(window);
    if (!item)
        return;
    if (hideLauncher)
        hideLauncher();
    // A click on the icon then cycles on from this window.
    item->currentActive = size_t(std::find(item->windows.begin(), item->windows.end(), window) - item->windows.begin());
    // Activating unhides and restores the window.
    m_windows.activate(window);
}

bool DockModel::unhide(DockItem *item)
{
    bool wasHidden = false;
    for (uint64_t window : item->windows) {
        const TaskWindow *client = m_windows.window(window);
        if (!client || !client->hidden)
            continue;
        m_windows.setHidden(window, false);
        wasHidden = true;
    }
    return wasHidden;
}

bool DockModel::openNewInstance(const std::string &id)
{
    DockItem *item = findById(id);
    if (!item || (!item->windows.empty() && !canOpenNewInstance(id)))
        return false;

    if (item->id == kLauncherId) {
        if (toggleLauncher)
            toggleLauncher();
        return true;
    }

    if (hideLauncher)
        hideLauncher();

    // Without a desktop entry there is no command: the app id is the
    // client's own choice, and a sandboxed one could name a program it wrote.
    const DesktopEntryPtr entry = m_applications.byId(item->id);
    if (!entry)
        return false;
    m_launcher.launch(*entry);
    return true;
}

bool DockModel::canOpenNewInstance(const std::string &id) const
{
    if (id == kLauncherId)
        return true;
    // These applications are single-window by design: a second launch would
    // only activate the existing window through their application runtime.
    return id != "lingmo-filemanager" && id != "lingmo-settings";
}

void DockModel::quit(const std::string &id)
{
    DockItem *item = findById(id);
    if (!item)
        return;

    // Quit is the window manager's, as for the menu bar and Command+Q: it
    // closes the application's windows and later ends what keeps running
    // without one. One request per application id among the windows.
    std::vector<std::string> quitIds;
    bool anyProcess = false;
    for (uint64_t window : std::vector<uint64_t>(item->windows)) {
        const TaskWindow *client = m_windows.window(window);
        if (!client)
            continue;
        anyProcess |= client->pid != 0;
        const std::string appId = toLower(client->appId);
        if (!appId.empty()) {
            if (std::find(quitIds.begin(), quitIds.end(), appId) != quitIds.end())
                continue;
            quitIds.push_back(appId);
        }
        m_windows.quitApplication(window);
    }

    // Without a process the window manager cannot end anything; the runtime
    // still knows the application by its desktop entry.
    if (anyProcess)
        return;
    std::string runtimeId;
    if (const DesktopEntryPtr entry = m_applications.byPath(item->desktopPath))
        runtimeId = entry->id;
    if (runtimeId.empty())
        return;
    auto timer = std::make_unique<base::Timer>();
    base::Timer *raw = timer.get();
    raw->setSingleShot(true);
    raw->onTimeout = [this, runtimeId, id] {
        const DockItem *current = findById(id);
        if (!current || current->windows.empty())
            m_launcher.quitApplication(runtimeId);
    };
    raw->start(kQuitGrace);
    // Timers that went off are dropped here, never from their own callback.
    std::erase_if(m_quitTimers, [](const auto &timer) { return !timer->isActive(); });
    m_quitTimers.push_back(std::move(timer));
}

void DockModel::pin(const std::string &id)
{
    DockItem *item = findById(id);
    if (!item || item->fixed || item->isPinned)
        return;
    pinAt(rowOf(item), pinnedCount());
}

void DockModel::unPin(const std::string &id)
{
    DockItem *item = findById(id);
    if (!item || item->fixed || !item->isPinned)
        return;
    unPinAt(rowOf(item), pinnedCount() - 1);
}

void DockModel::unPinAt(int from, int row)
{
    if (from < firstMovableRow() || from >= count())
        return;

    DockItem *item = m_items[size_t(from)].get();
    item->isPinned = false;
    if (item->windows.empty()) {
        m_items.erase(m_items.begin() + from);
        rowsRemoved(from, 1);
        sectionsChanged();
    } else {
        moveItem(from, std::clamp(row, pinnedCount(), count() - 1));
        changed(item);
    }
    savePins();
}

bool DockModel::dropApplication(const std::string &id, int row, bool pinned)
{
    DockItem *item = findById(id);
    if (!item || item->fixed || (item->isPinned && !pinned))
        return false;

    const int from = rowOf(item);
    const int to = from < row ? row - 1 : row;
    if (pinned) {
        if (item->desktopPath.empty())
            return false;
        pinAt(from, to);
    } else {
        moveItem(from, std::clamp(to, pinnedCount(), count() - 1));
        savePins();
    }
    return true;
}

bool DockModel::pinAt(int from, int row)
{
    if (from < firstMovableRow() || from >= count())
        return false;
    DockItem *item = m_items[size_t(from)].get();
    if (item->desktopPath.empty() || item->uninstalled)
        return false;

    const int end = pinnedCount() - (item->isPinned ? 1 : 0);
    item->isPinned = true;
    moveItem(from, std::clamp(row, firstMovableRow(), std::max(firstMovableRow(), end)));
    changed(item);
    savePins();
    return true;
}

void DockModel::move(int from, int to, bool commit)
{
    if (from < firstMovableRow() || from >= count() || to < firstMovableRow() || to >= count())
        return;

    const int boundary = pinnedCount();
    const bool pinned = m_items[size_t(from)]->isPinned;
    if (pinned != (to < boundary)) {
        // Only unpinned applications may cross the divider, and only on
        // release.
        if (commit && !pinned)
            pinAt(from, to);
        return;
    }
    moveItem(from, to);
}

void DockModel::moveItem(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= count() || to >= count())
        return;
    if (m_items[size_t(from)]->fixed || m_items[size_t(to)]->fixed)
        return;
    auto item = std::move(m_items[size_t(from)]);
    m_items.erase(m_items.begin() + from);
    m_items.insert(m_items.begin() + to, std::move(item));
    rowMoved(from, to);
    sectionsChanged();
}

bool DockModel::insert(const std::string &desktopFile, int row, int existingRow)
{
    DesktopEntryPtr entry;
    DockItem *existing = findByDesktop(desktopFile, &entry);
    if (existingRow >= 0) {
        if (existingRow < firstMovableRow() || existingRow >= count())
            return false;
        // The item the preview used, even if the registry changed during the
        // drag.
        existing = m_items[size_t(existingRow)].get();
    }

    row = std::clamp(row, firstMovableRow(), std::max(firstMovableRow(), pinnedCount()));
    if (existing) {
        if (entry)
            updateFromEntry(existing, *entry);
        const int from = rowOf(existing);
        return pinAt(from, from < row ? row - 1 : row);
    }
    if (!entry)
        return false;

    auto item = std::make_unique<DockItem>();
    updateFromEntry(item.get(), *entry);
    item->isPinned = true;
    m_items.insert(m_items.begin() + row, std::move(item));
    rowsInserted(row, 1);
    sectionsChanged();
    savePins();
    return true;
}

std::string DockModel::iconNameForWindow(uint64_t window) const
{
    if (const DockItem *item = findByWindow(window))
        return item->iconName;
    const TaskWindow *client = m_windows.window(window);
    return client ? toLower(client->appId) : std::string();
}

std::string DockModel::appIdForWindow(uint64_t window) const
{
    const DockItem *item = findByWindow(window);
    return item ? item->id : std::string();
}

void DockModel::load()
{
    if (m_loaded || !m_tasks.isReady() || !m_savedPins)
        return;

    m_loaded = true;
    loadPins(*m_savedPins);
    for (const std::string &application : std::vector<std::string>(m_tasks.applications()))
        syncApplication(application);

    // Saved pins are checked at once; later changes may be an upgrade in
    // progress.
    checkInstalled();
    loaded();
}

void DockModel::retranslate()
{
    if (DockItem *launcher = findById(kLauncherId)) {
        launcher->visibleName = tr("Launcher");
        changed(launcher);
    }
}

void DockModel::loadPins(const std::optional<std::vector<std::string>> &saved)
{
    auto launcher = std::make_unique<DockItem>();
    launcher->id = kLauncherId;
    launcher->iconName = kLauncherId;
    launcher->visibleName = tr("Launcher");
    launcher->fixed = true;

    std::vector<std::unique_ptr<DockItem>> items;
    items.push_back(std::move(launcher));
    const std::vector<std::string> &ids = saved ? *saved : pinned_applications::defaults();
    for (const std::string &id : ids) {
        const DesktopEntryPtr entry = m_applications.byId(id);
        const bool listed = std::any_of(items.begin(), items.end(), [&id](const auto &item) { return item->id == id; });
        if (!entry || listed)
            continue;
        auto item = std::make_unique<DockItem>();
        updateFromEntry(item.get(), *entry);
        item->isPinned = true;
        items.push_back(std::move(item));
    }

    const int first = count();
    const int added = int(items.size());
    for (auto &item : items)
        m_items.push_back(std::move(item));
    rowsInserted(first, added);
    sectionsChanged();

    // Forget pins uninstalled while the shell was not running.
    if (saved && size_t(added - 1) < saved->size())
        savePins();
}

void DockModel::savePins()
{
    std::vector<std::string> ids;
    for (const auto &item : m_items) {
        if (item->isPinned)
            ids.push_back(item->id);
    }
    pinned_applications::save(m_pinsPath, ids);
}

void DockModel::onApplicationsChanged()
{
    if (!m_loaded || m_applications.isLoading())
        return;

    // Running applications follow the task list; the others are kept
    // current here.
    for (const auto &item : m_items) {
        if (item->fixed || !item->windows.empty())
            continue;
        if (const DesktopEntryPtr entry = m_applications.byId(item->id)) {
            updateFromEntry(item.get(), *entry);
            changed(item.get());
        }
    }

    m_installedTimer.start(kUninstallGrace);
}

void DockModel::checkInstalled()
{
    // A scan is running; its end starts the check again.
    if (m_applications.isLoading())
        return;

    const std::vector<std::string> desktops = base::DesktopEntry::currentDesktops();
    for (int row = count() - 1; row >= 0; --row) {
        DockItem *item = m_items[size_t(row)].get();
        if (item->fixed || item->desktopPath.empty())
            continue;

        DesktopEntryPtr entry = m_applications.byPath(item->desktopPath);
        if (!entry)
            entry = m_applications.byId(item->id);
        // Uninstalled means the launcher no longer lists it.
        const bool uninstalled = !entry || !entry->shouldShow(desktops);
        if (item->uninstalled == uninstalled)
            continue;

        item->uninstalled = uninstalled;
        // Removed when nothing runs, otherwise moved among the running ones.
        if (uninstalled && item->isPinned)
            unPinAt(row, pinnedCount() - 1);
        else
            changed(item);
    }
}

void DockModel::syncApplication(const std::string &application)
{
    if (!m_loaded)
        return;

    const std::vector<uint64_t> windows = m_tasks.windows(application);
    DockItem *item = findById(application);

    if (windows.empty()) {
        if (!item || item->fixed)
            return;
        if (item->isPinned) {
            item->windows.clear();
            item->currentActive = 0;
            item->isActive = false;
            changed(item);
            return;
        }
        const int row = rowOf(item);
        m_items.erase(m_items.begin() + row);
        rowsRemoved(row, 1);
        sectionsChanged();
        return;
    }

    std::unique_ptr<DockItem> created;
    if (!item) {
        created = std::make_unique<DockItem>();
        created->id = application;
        item = created.get();
    }

    item->windows = windows;
    const uint64_t active = m_windows.activeWindow();
    const auto activeIt = std::find(windows.begin(), windows.end(), active);
    item->isActive = activeIt != windows.end();
    if (item->isActive)
        item->currentActive = size_t(activeIt - windows.begin());
    else if (item->currentActive >= windows.size())
        item->currentActive = 0;

    if (const DesktopEntryPtr entry = m_tasks.entry(application)) {
        updateFromEntry(item, *entry);
    } else if (item->desktopPath.empty()) {
        // No entry names it, so its window does. One that had an entry keeps
        // its name through an upgrade; checkInstalled() decides if it is gone.
        const TaskWindow *client = m_windows.window(windows.front());
        const std::string title = client ? client->title : std::string();
        item->visibleName = title.empty() ? application : title;
        item->iconName = toLower(application);
    }

    if (!created) {
        changed(item);
        return;
    }
    m_items.push_back(std::move(created));
    rowsInserted(count() - 1, 1);
    sectionsChanged();
}

void DockModel::onActiveChanged(uint64_t window)
{
    for (size_t row = 0; row < m_items.size(); ++row) {
        DockItem *item = m_items[row].get();
        const auto it = std::find(item->windows.begin(), item->windows.end(), window);
        const bool active = it != item->windows.end();
        // A click on the icon cycles on from the window that is active now,
        // however it came to be active.
        if (active)
            item->currentActive = size_t(it - item->windows.begin());
        if (item->isActive == active)
            continue;
        item->isActive = active;
        dataChanged(int(row), int(row));
    }
}

} // namespace Lingmo::Shell
