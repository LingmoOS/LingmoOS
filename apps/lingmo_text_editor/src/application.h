// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H
#define APPLICATION_H

#include "editorwindow.h"
#include "recentfiles.h"
#include "session.h"

#include "base/signal.h"
#include "base/timer.h"
#include "ui/glyph_run.h"

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Lingmo::base::dbus {
class Object;
}

class EditorSettings;

// The editor process: its windows, the session they are saved to, the
// recently used files and the D-Bus service later launches hand their files to.
class EditorApplication
{
public:
    explicit EditorApplication(EditorSettings &settings);
    ~EditorApplication();

    EditorApplication(const EditorApplication &) = delete;
    EditorApplication &operator=(const EditorApplication &) = delete;

    EditorSettings &settings() const { return m_settings; }
    RecentFiles &recentFiles() { return m_recentFiles; }

    std::vector<std::string> monospaceFamilies() const;
    static std::string encodingLabel(const std::string &encoding);
    // Plain Text first, then the languages by name: (id, name).
    std::vector<std::pair<std::string, std::string>> languages() const;

    bool canReopenClosedTab() const { return !m_closedTabs.empty(); }
    int windowCount() const;
    bool isQuitting() const { return m_quitting; }

    EditorWindow *newWindow(EditorWindow::Options options = {});
    // Destroys a closed window once the event it closed in is over.
    void releaseWindow(EditorWindow *window);
    void windowActivated(EditorWindow *window);
    std::vector<EditorWindow *> windows() const;
    // Opens files in the window used last, or focuses their tabs where they are open.
    void openPaths(const std::vector<std::string> &paths, int line = 0, EditorWindow *window = nullptr);
    struct OpenDocument
    {
        EditorWindow *window;
        int index;
    };
    std::optional<OpenDocument> findOpenDocument(const std::string &path) const;

    void pushClosedTab(const TabState &state);
    std::optional<TabState> takeClosedTab();

    void scheduleSessionSave();
    void saveSession();
    // Saves the session and drafts and ends the process, without asking.
    void quit();
    // For logout: keeps everything for the next start, whatever the setting.
    void terminate();

    void showInFileManager(const std::string &path) const;
    void copyText(const std::string &text) const;

    // Restores the last session, then opens `arguments` (paths and +LINE),
    // or an empty document when there is nothing at all.
    void start(const std::vector<std::string> &arguments, const std::string &workingDirectory,
               const std::optional<std::string> &standardInput);

    // The D-Bus methods: a later launch's files, resolved against its working directory.
    void open(const std::vector<std::string> &arguments, const std::string &workingDirectory);
    void openText(const std::string &text);
    void newWindowFromBus();
    // Registers org.lingmo.TextEditor; false when another process has it.
    bool registerService();

    Lingmo::base::Signal<> closedTabsChanged;
    Lingmo::base::Signal<> windowCountChanged;

private:
    EditorWindow *targetWindow(EditorWindow *preferred = nullptr);
    void restore();
    std::set<std::string> liveDraftIds() const;
    void flushDrafts();

    EditorSettings &m_settings;
    RecentFiles m_recentFiles;
    std::vector<std::unique_ptr<EditorWindow>> m_windows;
    // Front to back: the window brought forward last is at the end.
    std::vector<EditorWindow *> m_order;
    EditorWindow *m_lastActive = nullptr;
    std::vector<TabState> m_closedTabs;
    Lingmo::base::Timer m_sessionTimer;
    bool m_quitting = false;
    std::unique_ptr<Lingmo::base::dbus::Object> m_service;
};

#endif
