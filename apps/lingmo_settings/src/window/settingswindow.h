// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"

namespace Lingmo::base {
class Timer;
}

namespace Lingmo::ui {
class HeaderButton;
class Label;
class Window;
}

namespace settings {

class AvailabilityWatch;
class Page;
struct PageInfo;
class Sidebar;
class SettingsRoot;

// The settings window: the sidebar, the page area and the header floating
// over both, with a history to go back and forward through.
class SettingsWindow
{
public:
    // Room for the header buttons with as much space below them as above;
    // content starts under it.
    static constexpr double headerHeight = 56;

    SettingsWindow();
    ~SettingsWindow();

    Lingmo::ui::Window& window() const { return *m_window; }

    // Shown once the entries that come and go with hardware have settled, or
    // after half a second.
    void reveal();
    bool isRevealed() const { return m_revealed; }
    Lingmo::base::Signal<> revealedChanged;

    // A sidebar page or a named sub page, as -m and switchToPage() name them.
    // A shown window comes forward, with the activation token another
    // instance was started with.
    void switchToPage(const std::string& name, const std::string& activationToken = {});
    // Opens the Wi-Fi page to join an access point's enterprise network.
    void joinWifi(const std::string& accessPoint, const std::string& activationToken = {});
    // Taken by the Wi-Fi page once it can ask for the network.
    std::string takePendingWifiJoin();
    // After joinWifi() switched pages, also when the Wi-Fi page was already
    // shown and so was not made again.
    Lingmo::base::Signal<> wifiJoinRequested;

    // Opens a sub page of the current sidebar page.
    void pushPage(const std::string& id);
    void goBack();
    void goForward();
    bool canGoBack() const { return historyStep(-1) >= 0; }
    bool canGoForward() const { return historyStep(1) >= 0; }

    Page* currentPage() const { return m_page; }

    bool isActive() const;
    // After the window's active state changed.
    Lingmo::base::Signal<> activeChanged;

private:
    struct HistoryEntry
    {
        std::string page;
        std::string rootPage;
    };

    void navigate(const std::string& page, const std::string& rootPage);
    void showHistoryEntry(int index);
    int historyStep(int direction) const;
    bool isInSidebar(const std::string& id) const;
    void switchToIndex(int index);
    void setAvailable(const PageInfo& page, bool available);
    void updateSidebar();
    void updateHeader();
    void updateBackground();
    bool isReady() const;
    void retranslate();

    std::unique_ptr<Lingmo::ui::Window> m_window;
    SettingsRoot* m_root;
    Sidebar* m_sidebar;
    Lingmo::ui::HeaderButton* m_back;
    Lingmo::ui::HeaderButton* m_forward;
    Lingmo::ui::Label* m_title;
    Page* m_page = nullptr;
    std::vector<HistoryEntry> m_history;
    int m_historyIndex = -1;
    // Sidebar pages that are there, by id.
    std::vector<std::string> m_available;
    std::vector<std::unique_ptr<AvailabilityWatch>> m_watches;
    std::vector<std::string> m_unanswered;
    std::string m_pendingPage;
    std::string m_pendingWifiJoin;
    std::unique_ptr<Lingmo::base::Timer> m_revealTimer;
    bool m_revealed = false;
    Lingmo::base::Subscription m_languageChanged;
};

} // namespace settings
