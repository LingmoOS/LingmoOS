// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/application.h"
#include "app/chooserservice.h"
#include "app/dialogs.h"
#include "app/previewservice.h"
#include "desktop/desktopwindow.h"
#include "desktop/desktopview.h"
#include "core/launcher.h"
#include "core/lister.h"
#include "core/standardfolders.h"
#include "model/foldermodel.h"
#include "ops/operations.h"

#include "base/event_loop.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/trusted_connection.h"
#include "ui/window.h"

#include <algorithm>
#include <map>
#include <thread>

#ifdef __GLIBC__
#include <malloc.h>
#endif

namespace fm {

static constexpr const char* kServiceName = "org.lingmo.FileManager";
static constexpr const char* kFreedesktopServiceName = "org.freedesktop.FileManager1";
static constexpr const char* kPreviewServiceName = "org.lingmo.FilePreview";
static constexpr const char* kChooserServiceName = "org.lingmo.FileChooser";

Application::Application(ui::Application& app, bool daemon, bool dbusActivated)
    : m_app(app)
    , m_bus(app.sessionBus())
    , m_daemon(daemon)
    , m_dbusActivated(dbusActivated)
    , m_dialogs(std::make_unique<Dialogs>())
{
    m_dialogs->onChanged = [this] { scheduleIdleCheck(); };
    launcher::setActivationTokenSource([](std::function<void(const std::string&)> done) {
        for (ui::Window* window : ui::Application::instance()->windows()) {
            if (window->isActive() && !window->isPopup()) {
                window->requestActivationToken(std::move(done));
                return;
            }
        }
        done(std::string());
    });
    m_subscriptions.push_back(
            OperationsClient::instance().activeTaskCountChanged.connect([this](int) { scheduleIdleCheck(); }));

    m_heapTrim.setSingleShot(true);
    // Past the deferred deletes of the windows that just went.
    m_heapTrim.setInterval(std::chrono::seconds(10));
    m_heapTrim.onTimeout = [this] {
        if (!isIdle())
            return;
#ifdef __GLIBC__
        // glibc's arenas keep what closed windows freed; the trim locks one
        // arena after another, so not on the GUI thread.
        std::thread([] { malloc_trim(0); }).detach();
#endif
    };
}

Application::~Application()
{
    *m_alive = false;
    m_windows.clear();
    // Before the connection it lives on.
    m_desktop.reset();
}

void Application::exportObjects()
{
    if (m_fileManager)
        return;

    m_fileManager = std::make_unique<base::dbus::Object>(m_bus, "/FileManager", kServiceName);
    m_fileManager->addMethod("openFiles", [this](const std::vector<std::string>& paths) { openFiles(paths); });
    m_fileManager->addMethod("moveToTrash", [this](const std::vector<std::string>& paths) { moveToTrash(paths); });
    m_fileManager->addMethod("emptyTrash", [this] { emptyTrash(); });
    m_fileManager->addMethod("EditWidgets", [this] {
        if (m_desktop)
            m_desktop->editWidgets();
    });
    m_fileManager->addMethod("EditNotificationWidgets", [this] {
        if (m_desktop) m_desktop->editNotificationWidgets();
    });
    m_fileManager->addMethod("RefreshWidgets", [this] {
        if (m_desktop)
            m_desktop->refreshWidgets();
    });
    m_fileManager->addMethod("SetIconCorner", [this](int32_t corner) {
        if (!m_desktop)
            return;
        if (corner < 0 || corner > 3)
            return;
        const auto c = static_cast<DesktopGridView::IconCorner>(corner);
        m_desktop->view()->setIconCorner(c);
        base::Settings settings(base::paths::lingmoConfig("filemanager"));
        settings.setValue("Desktop/iconCorner", int(corner));
    });
    m_fileManager->publish();

    m_freedesktop = std::make_unique<base::dbus::Object>(m_bus, "/org/freedesktop/FileManager1",
                                                         kFreedesktopServiceName);
    m_freedesktop->addMethod("ShowFolders", [this](const std::vector<std::string>& uris, const std::string&) {
        std::vector<std::string> folders;
        for (const std::string& url : toUrls(uris))
            folders.push_back(base::url::toLocalFile(url).value_or(url));
        openFiles(folders);
    });
    m_freedesktop->addMethod("ShowItems", [this](const std::vector<std::string>& uris, const std::string&) {
        showItems(toUrls(uris));
    });
    m_freedesktop->addMethod("ShowItemProperties", [this](const std::vector<std::string>& uris, const std::string&) {
        showItemProperties(toUrls(uris));
    });
    m_freedesktop->publish();
}

bool Application::registerServices()
{
    if (m_servicesRegistered)
        return true;
    if (!m_bus)
        return false;

    // Objects first, so a call arriving as soon as the name is ours finds them.
    exportObjects();
    if (!m_bus->requestName(kServiceName))
        return false;

    // Queued: another file manager may hold it, and this one takes over when it goes.
    m_bus->requestName(kFreedesktopServiceName, 0);

    // Only the instance that serves the file manager hosts the preview.
    if (!m_preview) {
        m_preview = std::make_unique<PreviewService>(m_bus);
        m_preview->onVisibilityChanged = [this](bool) { scheduleIdleCheck(); };
    }
    m_bus->requestName(kPreviewServiceName);

    // Only the session daemon serves the portal's dialogs; the session
    // restarts it when it dies.
    if (m_daemon) {
        if (!m_chooser)
            m_chooser = std::make_unique<ChooserService>(m_bus);
        m_bus->requestName(kChooserServiceName);
    }
    m_servicesRegistered = true;
    return true;
}

void Application::unregisterServices()
{
    if (!m_servicesRegistered)
        return;
    m_bus->releaseName(kServiceName);
    m_bus->releaseName(kPreviewServiceName);
    m_bus->releaseName(kChooserServiceName);
    m_bus->releaseName(kFreedesktopServiceName);
    m_servicesRegistered = false;
}

void Application::startDaemon()
{
    if (registerServices())
        return;
    takeNamesWhenFree();
}

// An instance started before the session's daemon gives the names up once it
// is idle, and the daemon takes them over then.
void Application::takeNamesWhenFree()
{
    m_nameWatch = m_bus->watchName(kServiceName, [this, alive = m_alive](const std::string& owner) {
        if (!*alive || !owner.empty())
            return;
        base::EventLoop::current()->post([this, alive] {
            if (*alive && registerServices())
                m_nameWatch = {};
        });
    });
}

bool Application::startDesktop(int socket)
{
    if (socket >= 0) {
        m_trustedConnection = ui::TrustedConnection::fromFd(socket);
        if (!m_trustedConnection || !m_trustedConnection->hasShell())
            return false;
    }
    m_desktop = std::make_unique<DesktopWindow>(m_trustedConnection.get(), *this);
    return true;
}

void Application::openFiles(const std::vector<std::string>& paths)
{
    for (const std::string& path : paths)
        openWindow(path);
}

void Application::moveToTrash(const std::vector<std::string>& paths)
{
    if (paths.empty())
        return;
    std::vector<std::string> urls;
    for (const std::string& path : paths)
        urls.push_back(base::url::fromLocalFile(path));
    OperationsClient::instance().trash(urls);
}

void Application::emptyTrash()
{
    m_dialogs->confirmEmptyTrash(nullptr);
}

void Application::showItems(const std::vector<std::string>& urls)
{
    // One window per folder, with every item asked for in it selected.
    std::map<std::string, std::vector<std::string>> folders;
    for (const std::string& url : urls) {
        std::string folder;
        if (const auto local = base::url::toLocalFile(url)) {
            folder = parentPath(cleanPath(*local));
        } else {
            std::string trimmed = url;
            while (trimmed.size() > 1 && trimmed.back() == '/')
                trimmed.pop_back();
            folder = trimmed.substr(0, trimmed.rfind('/') + 1);
        }
        folders[folder].push_back(url);
    }
    for (const auto& [folder, selection] : folders)
        openWindow(folder, selection);
}

void Application::showItemProperties(const std::vector<std::string>& urls)
{
    FileItemList items;
    for (const std::string& url : urls) {
        const std::string path = DirectoryLister::localPathForUrl(url);
        if (!path.empty())
            items.push_back(std::make_shared<const FileItem>(DirectoryLister::makeItem(path)));
    }
    m_dialogs->properties(items, nullptr);
}

FolderWindow* Application::openWindow(const std::string& path, const std::vector<std::string>& selection)
{
    FolderPage::Start start;
    start.url = FolderModel::resolve(path);
    start.selection = selection;
    auto window = std::make_unique<FolderWindow>(this, start);
    FolderWindow* raw = window.get();
    m_windows.push_back(std::move(window));
    raw->show();
    return raw;
}

FolderWindow* Application::openFolderWindow(const FolderPage::Start& start, ui::Window* from)
{
    auto window = std::make_unique<FolderWindow>(this, start, from ? from->size() : ui::SizeF {});
    FolderWindow* raw = window.get();
    m_windows.push_back(std::move(window));
    raw->show();
    return raw;
}

void Application::openFolders(const std::vector<std::string>& paths)
{
    for (const std::string& path : paths)
        openWindow(path);
}

void Application::showProperties(const FileItemList& items, ui::Window* parent)
{
    m_dialogs->properties(items, parent);
}

void Application::showOpenWith(const std::string& url, ui::Window* parent)
{
    m_dialogs->openWith(url, parent);
}

void Application::confirmDelete(const std::vector<std::string>& urls, ui::Window* parent)
{
    m_dialogs->confirmDelete(urls, parent);
}

void Application::confirmEmptyTrash(ui::Window* parent)
{
    m_dialogs->confirmEmptyTrash(parent);
}

void Application::confirmLaunch(const std::string& path, ui::Window* parent)
{
    m_dialogs->confirmLaunch(path, parent);
}

void Application::windowClosed(FolderWindow* window)
{
    m_dialogs->closeOwnedBy(&window->window());
    auto it = std::find_if(m_windows.begin(), m_windows.end(),
                           [window](const auto& entry) { return entry.get() == window; });
    if (it == m_windows.end())
        return;
    std::shared_ptr<FolderWindow> gone(std::move(*it));
    m_windows.erase(it);
    base::EventLoop::current()->post([gone] {});
    scheduleIdleCheck();
}

std::vector<std::string> Application::toUrls(const std::vector<std::string>& inputs)
{
    std::vector<std::string> urls;
    for (const std::string& input : inputs) {
        if (!input.empty())
            urls.push_back(FolderModel::resolve(input));
    }
    return urls;
}

bool Application::isIdle() const
{
    return m_windows.empty() && m_dialogs->isEmpty() && (!m_preview || !m_preview->isVisible())
            && OperationsClient::instance().activeTaskCount() == 0;
}

void Application::scheduleIdleCheck()
{
    if (m_idleCheckScheduled)
        return;
    m_idleCheckScheduled = true;
    base::EventLoop::current()->post([this, alive = m_alive] {
        if (!*alive)
            return;
        m_idleCheckScheduled = false;
        quitIfIdle();
    });
}

void Application::quitIfIdle()
{
    if (!isIdle() || m_quitPending)
        return;

    if (m_daemon) {
        scheduleHeapTrim();
        return;
    }

    // Give up the names first, then look again: a request already on its
    // way is delivered in between and keeps the process alive.
    unregisterServices();
    m_quitPending = true;
    base::EventLoop::current()->post([this, alive = m_alive] {
        if (!*alive)
            return;
        m_quitPending = false;
        if (isIdle()) {
            m_app.quit(m_exitCode);
            return;
        }
        registerServices();
    });
}

void Application::scheduleHeapTrim()
{
    m_heapTrim.start();
}

} // namespace fm
