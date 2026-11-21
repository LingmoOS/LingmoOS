// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include "editorpage.h"
#include "session.h"

#include "base/signal.h"
#include "ui/geometry.h"
#include "ui/glyph_run.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class ConfirmDialog;
class FileChooser;
class HeaderButton;
class Menu;
class Toast;
class Window;
class WindowTabBar;
class WindowHeader;
}

class AboutDialog;
class EditorApplication;
class PreferencesDialog;
class SaveChangesDialog;
class ShortcutsDialog;

// A window of tabs, each a document.
class EditorWindow
{
public:
    struct Options
    {
        // Tabs to open with, from a restored session.
        std::vector<TabState> initialTabs;
        int initialIndex = 0;
        // A tab pulled out of another window, which this window opens with.
        std::unique_ptr<EditorPage> detachedPage;
        Lingmo::ui::SizeF size;
        double zoom = 1;
        bool restoresSize = true;
    };

    EditorWindow(EditorApplication &application, Options options);
    ~EditorWindow();

    EditorWindow(const EditorWindow &) = delete;
    EditorWindow &operator=(const EditorWindow &) = delete;

    EditorApplication &application() const { return m_application; }
    Lingmo::ui::Window &window() const { return *m_window; }

    // Zoom is per window and not kept.
    double zoom() const { return m_zoom; }
    void zoomBy(int step);
    Lingmo::ui::FontSpec editorFont() const;

    EditorPage *currentPage() const;
    int currentIndex() const { return m_currentIndex; }
    int tabCount() const { return int(m_pages.size()); }
    const std::vector<EditorPage *> &pages() const { return m_pages; }
    int indexOfPage(const EditorPage *page) const;

    EditorPage *newTab(const EditorPage::Options &options = {}, int index = -1);
    void selectTab(int index);
    void cycleTab(int step);
    void moveTab(int from, int to);

    int indexOfPath(const std::string &absolute, const std::string &canonical) const;
    // A pristine untitled tab is reused instead of left behind.
    static bool isPristine(const EditorPage *page);
    // `line` from 1, or 0 for none.
    void openFile(const std::string &path, int line);
    void openText(const std::string &text);
    void openPaths(const std::vector<std::string> &paths);
    std::vector<TabState> sessionTabs() const;

    // Asks about unsaved pages, then runs `action` unless the user cancels.
    void confirmDiscard(const std::vector<EditorPage *> &dirty, std::function<void()> action);
    void closeTab(int index, bool ask = true);
    void closeCurrentTab();
    void closeOtherTabs(int index);
    void closeTabsToRight(int index);
    void reopenClosedTab();
    void quitApplication();
    // The close button, or Ctrl+Shift+W.
    void requestClose();
    // Closes the window for good: its pages are already dealt with.
    void finishClose();

    // Saves the page, through Save As when it has no file; `done(saved, cancelled)` follows.
    void savePage(EditorPage *page, std::function<void(bool saved, bool cancelled)> done = {});
    void saveAs(EditorPage *page, std::function<void(bool saved, bool cancelled)> done = {});
    void saveAll();
    void openDialog();

    void showMainMenu();
    // Move to New Window, Copy Path, Show in Folder and the closing items.
    void openTabMenu(int index, Lingmo::ui::PointF position);
    void showPreferences();
    void showShortcuts();
    void showStatistics();
    void showAbout();
    void showNotification(const std::string &text, int milliseconds);

    void insertTab(std::unique_ptr<EditorPage> page, int index);
    std::unique_ptr<EditorPage> takePage(int index);
    void moveToNewWindow(int index);
    void detachTab(int index, Lingmo::ui::PointF position);
    // The page this window was carried with, taken by the tab bar it was dropped on.
    std::unique_ptr<EditorPage> takeDetachedPage();

    bool isFullScreen() const;
    void toggleFullScreen();

    Lingmo::ui::WindowTabBar *tabBar() const;
    Lingmo::ui::View *headerContent() const;

    // The preferences changed.
    void applySettings();
    // Every page's tab and status, as the window title.
    void pagesChanged();

private:
    class Content;
    class HeaderContent;

    void addShortcuts();
    void showPage(int index);
    void attach(EditorPage *page);
    void updateTitle();
    // The tabs keep their documents and text areas; everything around them
    // takes the language of the moment.
    void retranslate();

    EditorApplication &m_application;
    std::unique_ptr<Lingmo::ui::Window> m_window;
    Content *m_content = nullptr;
    HeaderContent *m_header = nullptr;
    std::vector<EditorPage *> m_pages;
    std::vector<Lingmo::base::Subscription> m_pageConnections;
    int m_currentIndex = -1;
    double m_zoom = 1;
    bool m_closeConfirmed = false;

    std::unique_ptr<Lingmo::ui::Menu> m_mainMenu;
    std::unique_ptr<Lingmo::ui::Menu> m_tabMenu;
    int m_tabMenuIndex = -1;
    std::unique_ptr<Lingmo::ui::FileChooser> m_openChooser;
    std::unique_ptr<Lingmo::ui::FileChooser> m_saveChooser;
    std::unique_ptr<SaveChangesDialog> m_saveDialog;
    std::unique_ptr<PreferencesDialog> m_preferences;
    std::unique_ptr<ShortcutsDialog> m_shortcuts;
    std::unique_ptr<AboutDialog> m_about;
    std::unique_ptr<Lingmo::ui::ConfirmDialog> m_statistics;
    std::unique_ptr<Lingmo::ui::Toast> m_toast;
    std::vector<Lingmo::base::Subscription> m_connections;
    Lingmo::base::Subscription m_languageChanged;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

#endif
