// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "views/folderwindow.h"

#include "base/dbus.h"
#include "base/timer.h"

#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class Application;
class TrustedConnection;
}

namespace fm {

class Dialogs;
class PreviewService;
class ChooserService;
class DesktopWindow;

// The file manager process: its windows, its D-Bus services
// (org.lingmo.FileManager, org.freedesktop.FileManager1, Quick Look and,
// for the session's daemon, the portal's file dialogs) and when it ends.
class Application : public FileManagerHost
{
public:
    Application(ui::Application& app, bool daemon, bool dbusActivated);
    ~Application() override;

    // Takes the D-Bus names; false if another instance holds them.
    bool registerServices();
    // Serves the session from its start until it ends, with or without windows.
    void startDaemon();
    // The desktop's icons on the session's trusted connection (its socket
    // descriptor), or in an ordinary window without one (-1).
    bool startDesktop(int socket);
    void scheduleIdleCheck();
    int exitCode() const { return m_exitCode; }

    // org.lingmo.FileManager
    void openFiles(const std::vector<std::string>& paths);
    void moveToTrash(const std::vector<std::string>& paths);
    void emptyTrash();
    // org.freedesktop.FileManager1
    void showItems(const std::vector<std::string>& urls);
    void showItemProperties(const std::vector<std::string>& urls);

    FolderWindow* openWindow(const std::string& path, const std::vector<std::string>& selection = {});
    Dialogs& dialogs() const { return *m_dialogs; }
    const std::vector<std::unique_ptr<FolderWindow>>& windows() const { return m_windows; }

    // FileManagerHost.
    FolderWindow* openFolderWindow(const FolderPage::Start& start, ui::Window* from) override;
    void openFolders(const std::vector<std::string>& paths) override;
    void showProperties(const FileItemList& items, ui::Window* parent) override;
    void showOpenWith(const std::string& url, ui::Window* parent) override;
    void confirmDelete(const std::vector<std::string>& urls, ui::Window* parent) override;
    void confirmEmptyTrash(ui::Window* parent) override;
    void confirmLaunch(const std::string& path, ui::Window* parent) override;
    void windowClosed(FolderWindow* window) override;

    // The paths and URLs of a command line or a D-Bus call as URLs: ~ and relative paths resolved.
    static std::vector<std::string> toUrls(const std::vector<std::string>& inputs);

private:
    void unregisterServices();
    void exportObjects();
    bool isIdle() const;
    void quitIfIdle();
    void scheduleHeapTrim();
    void takeNamesWhenFree();

    ui::Application& m_app;
    base::dbus::Connection* m_bus;
    const bool m_daemon;
    const bool m_dbusActivated;
    std::unique_ptr<Dialogs> m_dialogs;
    std::vector<std::unique_ptr<FolderWindow>> m_windows;
    std::unique_ptr<base::dbus::Object> m_fileManager;
    std::unique_ptr<base::dbus::Object> m_freedesktop;
    std::unique_ptr<PreviewService> m_preview;
    std::unique_ptr<ChooserService> m_chooser;
    std::unique_ptr<ui::TrustedConnection> m_trustedConnection;
    std::unique_ptr<DesktopWindow> m_desktop;
    base::dbus::NameWatch m_nameWatch;
    bool m_quitPending = false;
    base::Timer m_heapTrim;
    bool m_servicesRegistered = false;
    bool m_idleCheckScheduled = false;
    int m_exitCode = 0;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
