// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "ui/geometry.h"

namespace Lingmo::ui {
class Animation;
class ConfirmDialog;
class HeaderButton;
class Label;
class Menu;
class View;
class Window;
class WindowHeader;
}

#include "terminalpage.h"
#include "globalmenu.h"

class SettingsDialog;
class TerminalSettings;
class Windows;

// A terminal window: its tabs in the title bar, each a shell.
class TerminalWindow
{
public:
    struct Options
    {
        // A tab pulled out of another window, which this window opens with
        // instead of a new shell.
        std::unique_ptr<TerminalPage> detachedPage;
        // Where the first shell starts; empty takes the command line's.
        std::string startDirectory;
        std::vector<std::string> command;
        Lingmo::ui::SizeF size;
        int fontZoom = 0;
        bool restoresSize = true;
    };

    TerminalWindow(Windows& windows, TerminalSettings& settings, Options options);
    ~TerminalWindow();

    Lingmo::ui::Window& window() const { return *m_window; }
    TerminalSettings& settings() const { return m_settings; }

    // The scheme's own background, so cells in the default colour and the
    // window around the text are one surface; the title bar's text and
    // buttons follow the scheme rather than the system theme.
    const std::string& colorScheme() const { return m_colorScheme; }
    Lingmo::ui::Color terminalBackground() const { return m_terminalBackground; }
    Lingmo::ui::Color chromeForeground() const { return m_chromeForeground; }
    bool chromeDark() const { return m_chromeDark; }
    Lingmo::ui::Color schemeColor(int index, Lingmo::ui::Color fallback) const;

    // Zoom is for this window only; the size in Settings stays the default.
    int fontPointSize() const;
    void zoom(int step);

    int tabCount() const { return int(m_pages.size()); }
    TerminalPage* currentPage() const;

    void openNewTab();
    void openTab(const std::string& path, const std::vector<std::string>& command = {}, int index = -1);
    void insertTab(std::unique_ptr<TerminalPage> page, int index);
    std::unique_ptr<TerminalPage> takeTab(int index);
    void closeCurrentTab();
    void closeTab(int index);
    void closeOtherTabs(int index);
    void selectTab(int index);
    void cycleTab(int step);
    void moveCurrentTab(int step);
    void moveTab(int from, int to);
    void newWindow();

    void close();
    void toggleFullScreen();
    bool isFullScreen() const;
    void flashBell();
    void showSettings();
    void openLink(const std::string& url);
    void confirmPaste(const std::string& text, std::function<void()> action);

    // The only tab of a window pulled out of another, for a window it is
    // dropped on. The empty window hides at once and goes when the drag ends.
    std::unique_ptr<TerminalPage> takePage();

private:
    class Content;
    class HeaderContent;
    class Overlay;

    void applySettings();
    void retranslate();
    void updateBackground();
    void updateTitle();
    void confirmClose(const std::vector<TerminalPage*>& pages, std::function<void()> action);
    void showExitPrompt();
    void showLinkConfirm();
    void showPasteConfirm();
    void closeProtection(int index);
    void detachTab(int index, Lingmo::ui::PointF position);
    void openTabMenu(int index, Lingmo::ui::PointF position);
    void attach(TerminalPage* page);
    void showPage(int index);
    int indexOfPage(const TerminalPage* page) const;
    void finishClose();
    std::vector<GlobalMenu::Entry> menuEntries() const;

    Windows& m_windows;
    TerminalSettings& m_settings;
    std::unique_ptr<Lingmo::ui::Window> m_window;
    Lingmo::ui::WindowHeader* m_header = nullptr;
    HeaderContent* m_headerContent = nullptr;
    Content* m_content = nullptr;
    Overlay* m_overlay = nullptr;

    std::vector<TerminalPage*> m_pages;
    int m_currentIndex = -1;
    int m_fontZoom = 0;

    std::string m_colorScheme;
    Lingmo::ui::Color m_terminalBackground;
    Lingmo::ui::Color m_chromeForeground;
    bool m_chromeDark = true;
    std::vector<Lingmo::ui::Color> m_schemeColors;

    // Each dialog is a window of its own, made the first time it is needed.
    std::unique_ptr<Lingmo::ui::ConfirmDialog> m_exitPrompt;
    std::unique_ptr<Lingmo::ui::ConfirmDialog> m_pasteConfirm;
    std::unique_ptr<Lingmo::ui::ConfirmDialog> m_linkConfirm;
    std::unique_ptr<SettingsDialog> m_settingsDialog;
    std::function<void()> m_pendingAction;
    std::function<void()> m_pendingPaste;
    std::string m_pendingLink;
    size_t m_pasteLineCount = 0;
    std::string m_pastePreview;

    // Set once the close has been confirmed, so closing does not ask again.
    bool m_quitting = false;
    // A tab of this window is being dragged out, or one of another window is
    // over the tab bar; the bar stays shown.
    bool m_detaching = false;
    bool m_incoming = false;
    std::unique_ptr<Lingmo::ui::Menu> m_tabMenu;
    std::unique_ptr<GlobalMenu> m_globalMenu;
    int m_tabMenuIndex = -1;

    std::vector<Lingmo::base::Subscription> m_connections;
    // Dropped when the tab leaves this window, closed or moved to another.
    std::map<const TerminalPage*, std::vector<Lingmo::base::Subscription>> m_pageConnections;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
