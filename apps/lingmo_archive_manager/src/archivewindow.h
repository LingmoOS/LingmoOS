// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "archivecore.h"
#include "globalmenu.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/weak_ptr.h"
#include "ui/window.h"
#include <functional>
#include <memory>
#include <set>
#include <map>
namespace Lingmo::ui { class FileChooser; class Menu; class AboutDialog; }
namespace ArchiveManager {
class ArchiveView;
class ArchiveWindow {
public:
    enum class Page { Browse, Create };
    explicit ArchiveWindow(Lingmo::base::dbus::Connection* menuBus = nullptr);
    ~ArchiveWindow();
    void show();
    Lingmo::ui::Window& window() { return m_window; }
    bool busy() const { return m_busy; }
    Page page() const { return m_page; }
    const Result& listing() const { return m_listing; }
    const std::string& lastOutput() const { return m_output; }
    const std::string& directory() const { return m_directory; }
    bool openArguments(const std::vector<std::string>& paths, bool createMode = false);
    void openArchive(const std::string& path);
    void openChooser();
    void newArchive();
    void showArchive();
    void addSources(const std::vector<std::string>& paths);
    void addChooser(bool folder);
    void removeSelected();
    void createChooser();
    void createAt(const std::string& destination);
    void extractChooser(bool selected);
    void extractTo(const std::string& parent, bool selected = false);
    void verifyArchive();
    void cancel();
    void navigate(const std::string& path);
    void up();
    void select(const std::string& path);
    bool selected(const std::string& path) const;
    void selectAll();
    void clearSelection();
    void setQuery(const std::string& query);
    void revealOutput();
    void showMenu(Lingmo::ui::View* anchor);
    void showAbout();
private:
    friend class ArchiveView;
    enum class Operation { Inspect, Create, Extract, Verify };
    void refresh();
    void refreshMenus();
    void run(Operation operation, std::function<Result(const Options&)> work);
    void finish(Operation operation, Result result);
    void progress(const Progress& progress);
    std::vector<GlobalMenu::Entry> menuEntries();
    Lingmo::ui::Window m_window;
    ArchiveView* m_view = nullptr;
    Result m_listing;
    std::string m_path, m_directory, m_query, m_status, m_output;
    std::vector<std::string> m_sources;
    std::map<std::string, Entry> m_sourceInfo;
    std::set<std::string> m_selection, m_sourceSelection;
    Page m_page = Page::Browse;
    bool m_busy = false, m_closeWhenDone = false;
    uint64_t m_generation = 0;
    std::shared_ptr<std::atomic_bool> m_cancel;
    Lingmo::base::Job m_job;
    std::unique_ptr<Lingmo::ui::FileChooser> m_chooser;
    std::unique_ptr<Lingmo::ui::Menu> m_menu;
    std::unique_ptr<Lingmo::ui::AboutDialog> m_about;
    std::unique_ptr<GlobalMenu> m_globalMenu;
    Lingmo::base::Subscription m_menuAvailability, m_language;
    Lingmo::base::WeakPtrFactory<ArchiveWindow> m_weak{this};
};
}
