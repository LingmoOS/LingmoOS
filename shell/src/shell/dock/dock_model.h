// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/thread_pool.h"
#include "base/timer.h"
#include "shell/common/applications.h"
#include "shell/common/list_model.h"

namespace Lingmo::Shell {

class ApplicationLauncher;
class TaskManager;
class TaskWindows;

// The dock's pins, Dock/Pinned in shell.conf: desktop ids in order. Until the
// user changes them, the session's default list. Read and written on the file
// thread.
namespace pinned_applications {

const std::vector<std::string> &defaults();
// Nothing when the file has no list yet.
std::optional<std::vector<std::string>> load(const std::string &path);
void save(const std::string &path, const std::vector<std::string> &desktopIds);

} // namespace pinned_applications

// One entry of the dock: the launcher button, a pinned application, or a
// running one.
struct DockItem {
    // The TaskManager application: the desktop id, or the app id of windows
    // no desktop entry claims.
    std::string id;
    std::string iconName;
    std::string visibleName;
    std::string desktopPath;
    // Its task-list windows, in the order they opened.
    std::vector<uint64_t> windows;
    // Which window a click shows next.
    size_t currentActive = 0;
    bool isActive = false;
    bool isPinned = false;
    // The launcher button: always first, never moved.
    bool fixed = false;
    // The launcher stopped listing it; a running one can no longer be pinned.
    bool uninstalled = false;

    bool pinnable() const { return !fixed && !desktopPath.empty() && !uninstalled; }
};

// The dock's applications: the launcher button, the pinned applications, then
// the running ones in the order they started.
class DockModel : public ListModelSignals
{
public:
    // The dock's built-in launcher button; not backed by a desktop file.
    static constexpr const char *kLauncherId = "lingmo-launcher";
    // Let the application close its windows before the runtime ends what is
    // left of it.
    static constexpr std::chrono::milliseconds kQuitGrace{3000};
    // An upgrade can remove a desktop file and put it back seconds later.
    static constexpr std::chrono::milliseconds kUninstallGrace{3000};

    // `pinsPath` is shell.conf. Everything must outlive the model.
    DockModel(TaskManager &tasks, ApplicationLauncher &launcher, std::string pinsPath);
    ~DockModel();

    DockModel(const DockModel &) = delete;
    DockModel &operator=(const DockModel &) = delete;

    // The launcher shows and hides through these; the dock never takes the
    // keyboard, so the launcher does not lose activation when a dock item
    // brings up an application.
    std::function<void()> hideLauncher;
    std::function<void()> toggleLauncher;

    // The pins and the running applications are in.
    bool isLoaded() const { return m_loaded; }

    const std::vector<std::unique_ptr<DockItem>> &items() const { return m_items; }
    int count() const { return int(m_items.size()); }
    const DockItem &at(int row) const { return *m_items[size_t(row)]; }
    int indexOf(const std::string &id) const;
    int pinnedCount() const;
    // Rows below this hold the fixed items, which nothing can go before.
    int firstMovableRow() const;

    // org.lingmo.Dock: by desktop path or desktop id.
    void addItem(const std::string &desktopFile);
    void removeItem(const std::string &desktopFile);
    bool isDesktopPinned(const std::string &desktopFile) const;
    // The row of the desktop file's item, refreshed from its entry; -1.
    int desktopRow(const std::string &desktopFile);

    void clicked(const std::string &id);
    void raiseWindow(const std::string &id);
    // An application's window, as the window list in its menu chose it.
    void activateWindow(uint64_t window);
    struct WindowInfo {
        uint64_t id;
        std::string title;
        bool active;
    };
    // The application's windows in the order they opened.
    std::vector<WindowInfo> windows(const std::string &id) const;

    bool openNewInstance(const std::string &id);
    bool canOpenNewInstance(const std::string &id) const;
    void quit(const std::string &id);
    void pin(const std::string &id);
    void unPin(const std::string &id);

    // A drag over the dock moves items within their section; only an
    // unpinned application crosses the divider, and only on release.
    void move(int from, int to, bool commit = false);
    bool dropApplication(const std::string &id, int row, bool pinned);
    bool pinAt(int from, int row);
    // Pins a desktop file at `row`: a new entry is inserted there, the one it
    // already has is moved there. The row is counted with that entry still in
    // its old place, the way a gap opened in front of `row` reads.
    bool insert(const std::string &desktopFile, int row, int existingRow = -1);

    // The icon of the entry a window belongs to.
    std::string iconNameForWindow(uint64_t window) const;
    // The id of the entry a window belongs to; empty if none has it yet.
    std::string appIdForWindow(uint64_t window) const;

    // Takes the launcher button's name again after the session language
    // changed.
    void retranslate();

    // pinnedCount() may have changed.
    base::Signal<> sectionsChanged;
    base::Signal<> loaded;

private:
    DockItem *findById(const std::string &id) const;
    DockItem *findByWindow(uint64_t window) const;
    DockItem *findByDesktop(const std::string &desktopFile, DesktopEntryPtr *entry = nullptr) const;
    int rowOf(const DockItem *item) const;
    void updateFromEntry(DockItem *item, const base::DesktopEntry &entry) const;
    void changed(DockItem *item);
    void moveItem(int from, int to);
    void unPinAt(int from, int row);
    bool unhide(DockItem *item);

    void load();
    void loadPins(const std::optional<std::vector<std::string>> &saved);
    void savePins();
    void onApplicationsChanged();
    void checkInstalled();
    void syncApplication(const std::string &application);
    void onActiveChanged(uint64_t window);

    TaskManager &m_tasks;
    TaskWindows &m_windows;
    Applications &m_applications;
    ApplicationLauncher &m_launcher;
    std::string m_pinsPath;
    std::vector<std::unique_ptr<DockItem>> m_items;
    bool m_loaded = false;
    // The file's list, once read.
    std::optional<std::optional<std::vector<std::string>>> m_savedPins;
    base::Timer m_installedTimer;
    base::Job m_pinsJob;
    std::vector<std::unique_ptr<base::Timer>> m_quitTimers;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

