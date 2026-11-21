// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "search.h"
#include "base/application_registry.h"
#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "base/weak_ptr.h"
#include "ui/window.h"
namespace Search {
class SearchView;
class SearchWindow {
public:
    explicit SearchWindow(bool liveProviders = true, Lingmo::base::dbus::Connection* runtimeBus = nullptr);
    ~SearchWindow();
    void show(const std::string& token = {});
    void hide();
    void toggle(const std::string& token = {});
    void setQuery(const std::string& query);
    void setCategory(Category category);
    void moveSelection(int delta);
    void activate(size_t index);
    void activateSelected();
    void refresh();
    bool visible() const { return m_visible; }
    bool searching() const { return m_searching; }
    Category category() const { return m_category; }
    const Response& response() const { return m_response; }
    Lingmo::ui::Window& window() { return *m_window; }
    // Deterministic provider data for headless UI tests/captures.
    void setTestSources(std::vector<Application> apps, std::vector<std::string> roots);
private:
    friend class SearchView;
    void createWindow();
    void queueSearch();
    void performSearch();
    void updateSize();
    void launch(const Result& result, const std::string& token);
    void operationFailed(const std::string& error);
    std::unique_ptr<Lingmo::ui::Window> m_window;
    SearchView* m_view = nullptr;
    Lingmo::base::dbus::Connection* m_runtimeBus = nullptr;
    Lingmo::base::Timer m_debounce;
    Lingmo::base::ThreadPool m_workers{1};
    Lingmo::base::Job m_job;
    Lingmo::base::dbus::PendingCall m_launch;
    Lingmo::base::Subscription m_appsChanged, m_languageChanged, m_screensChanged;
    Lingmo::base::ApplicationRegistry* m_registry = nullptr;
    std::vector<Application> m_testApps;
    std::vector<std::string> m_roots;
    Response m_response;
    std::string m_query, m_error;
    Category m_category = Category::All;
    size_t m_selected = 0;
    uint64_t m_generation = 0;
    std::shared_ptr<std::atomic_bool> m_cancel;
    bool m_visible = false, m_searching = false, m_wasActive = false, m_launching = false;
    Lingmo::base::WeakPtrFactory<SearchWindow> m_weak{this};
};
}
