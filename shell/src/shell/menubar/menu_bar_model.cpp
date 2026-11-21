// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/menubar/menu_bar_model.h"

#include <algorithm>

#include "base/i18n.h"
#include "shell/common/task_windows.h"
#include "shell/shell.h"
#include "shell/statusbar/app_menu_registrar.h"
#include "shell/statusbar/application_menu.h"
#include "shell/statusbar/dbus_menu.h"
#include "shell/statusbar/dbus_menu_popup.h"
#include "shell/statusbar/power_actions.h"
#include "ui/application.h"
#include "ui/menu.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace Lingmo::Shell {

MenuBarModel::MenuBarModel(Shell *shell, AppMenuRegistrar *registrar, PowerActions *power)
    : m_shell(shell)
    , m_registrar(registrar)
    , m_power(power)
    , m_userMenu(std::make_unique<ui::Menu>())
    , m_applicationRowMenu(std::make_unique<ui::Menu>())
{
    m_applicationMenu = std::make_unique<ApplicationMenu>(shell->bus(), shell->taskWindows(), shell->tasks(),
                                                          registrar);
    buildUserMenu();
    // The application entry's menu names the application as it is when it
    // opens.
    m_applicationRowMenu->onAboutToShow = [this] { fillApplicationMenu(); };
    m_userMenu->onClosed = [this] { menuClosed(); };
    m_applicationRowMenu->onClosed = [this] { menuClosed(); };

    m_subscriptions.push_back(m_applicationMenu->switched.connect([this] { updateShown(); }));
    m_subscriptions.push_back(m_applicationMenu->titleChanged.connect([this] { updateRows(); }));
    m_subscriptions.push_back(m_applicationMenu->activationRequested.connect([this](int32_t id) {
        if (m_shown != m_applicationMenu->menu())
            return;
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (m_rows[i].kind == Kind::Menu && m_rows[i].id == id)
                activationRequested(i);
        }
    }));

    // ShellState's desktop: no application window has the keyboard and the
    // launcher does not cover the desktop.
    TaskWindows &windows = shell->taskWindows();
    m_subscriptions.push_back(windows.activeWindowChanged.connect([this](uint64_t) { updateDesktop(); }));
    m_subscriptions.push_back(windows.windowChanged.connect([this](uint64_t, uint32_t) { updateDesktop(); }));
    m_subscriptions.push_back(windows.windowRemoved.connect([this](uint64_t) { updateDesktop(); }));
    if (wm::WindowManager *manager = shell->windowManager()) {
        m_subscriptions.push_back(manager->activeWindowChanged.connect([this](wm::Window *) { updateDesktop(); }));
        m_subscriptions.push_back(manager->windowAdded.connect([this](wm::Window *) {
            updateDesktop();
            updateDesktopImport();
        }));
        m_subscriptions.push_back(manager->windowRemoved.connect([this](wm::Window *) {
            updateDesktop();
            updateDesktopImport();
        }));
    }
    if (m_registrar) {
        m_subscriptions.push_back(m_registrar->windowRegistered.connect(
                [this](uint32_t, const std::string &, const std::string &) { updateDesktopImport(); }));
        m_subscriptions.push_back(
                m_registrar->windowUnregistered.connect([this](uint32_t) { updateDesktopImport(); }));
    }

    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));

    updateDesktop();
    updateDesktopImport();
    updateShown();
}

MenuBarModel::~MenuBarModel()
{
    m_subscriptions.clear();
    m_shownSubscriptions.clear();
    m_desktopSubscriptions.clear();
    m_popups.clear();
    m_userMenu->onClosed = nullptr;
    m_applicationRowMenu->onClosed = nullptr;
    m_userMenu->close();
    m_applicationRowMenu->close();
}

void MenuBarModel::buildUserMenu()
{
    ui::Menu &menu = *m_userMenu;
    Shell *shell = m_shell;
    menu.addItem(tr("About"), [shell] { shell->openSettings({"-m", "about"}); });
    menu.addItem(tr("Settings"), [shell] { shell->openSettings(); });
    menu.addSeparator();
    PowerActions *power = m_power;
    menu.addItem(tr("Suspend"), [power] {
        if (power)
            power->suspend();
    });
    menu.addItem(tr("Restart"), [power] {
        if (power)
            power->request(PowerActions::Reboot);
    });
    menu.addItem(tr("Shutdown"), [power] {
        if (power)
            power->request(PowerActions::Shutdown);
    });
    menu.addSeparator();
    menu.addItem(tr("Lock screen"), [power] {
        if (power)
            power->lockScreen();
    });
    menu.addItem(tr("Log out"), [power] {
        if (power)
            power->request(PowerActions::Logout);
    });
}

// The user's menu is made again, closed if it was open; the application
// entry's menu is made as it opens. The applications' own menus are theirs.
void MenuBarModel::retranslate()
{
    m_userMenu->close();
    m_userMenu->clear();
    buildUserMenu();
    m_applicationRowMenu->close();
    m_applicationMenu->retranslate();
}

void MenuBarModel::fillApplicationMenu()
{
    ui::Menu &menu = *m_applicationRowMenu;
    menu.clear();
    ApplicationMenu *application = m_applicationMenu.get();
    // Quit, Hide and Hide Others are the compositor's; on the desktop there
    // is nothing to quit or hide.
    const size_t hide = menu.addItem(format(tr("Hide {name}"), {{"name", application->title()}}),
                                     [application] { application->hideApplication(); });
    menu.addItem(tr("Hide Others"), [application] { application->hideOtherApplications(); });
    menu.addSeparator();
    const size_t quit = menu.addItem(tr("Quit"), [application] { application->quitApplication(); });
    menu.setItemEnabled(hide, !m_desktop);
    menu.setItemEnabled(quit, !m_desktop);
}

void MenuBarModel::updateDesktop()
{
    bool launcher = false;
    if (const wm::WindowManager *manager = m_shell->windowManager()) {
        for (const wm::Window *window : manager->windows()) {
            if (window->role() == wm::WindowRole::Launcher && window->isVisible())
                launcher = true;
        }
    }
    TaskWindows &windows = m_shell->taskWindows();
    const TaskWindow *active = windows.window(windows.activeWindow());
    const bool desktop = !launcher && !(active && active->isTaskListWindow());
    if (desktop == m_desktop)
        return;
    m_desktop = desktop;
    m_applicationMenu->setDesktop(desktop);
    // An application without menus leaves the application menu as it was
    // (none), so nothing else would take the desktop's menus down.
    updateShown();
}

void MenuBarModel::updateDesktopImport()
{
    // The desktop's process registered its menu with the registrar.
    std::string service;
    std::string path;
    const wm::WindowManager *manager = m_shell->windowManager();
    if (m_registrar && manager && m_shell->bus()) {
        for (const wm::Window *window : manager->windows()) {
            if (window->role() != wm::WindowRole::Desktop || window->processId() <= 0)
                continue;
            service = m_registrar->desktopServiceForPid(uint32_t(window->processId()));
            path = m_registrar->desktopPathForPid(uint32_t(window->processId()));
            if (!service.empty() && !path.empty())
                break;
        }
        // The session hands the file manager's desktop its Wayland connection
        // as a socket pair, so the desktop carries the session's process id:
        // its menu is found by the path it exports it at.
        if (service.empty() || path.empty()) {
            path = kDesktopMenuPath;
            service = m_registrar->serviceForPath(path);
        }
    }

    if (m_desktopMenu ? m_desktopMenu->service() == service && m_desktopMenu->path() == path
                      : service.empty() || path.empty())
        return;

    const bool wasShown = m_desktopMenu && m_shown == m_desktopMenu.get();
    m_desktopSubscriptions.clear();
    if (wasShown) {
        m_shownSubscriptions.clear();
        // Its menus may still be open; they reach the exporter no more.
        for (auto &[id, popup] : m_popups)
            popup->detach();
    }
    m_desktopMenu.reset();
    m_desktopReady = false;

    if (!service.empty() && !path.empty()) {
        m_desktopMenu = std::make_unique<DBusMenu>(m_shell->bus(), service, path);
        m_desktopSubscriptions.push_back(
                m_desktopMenu->menuUpdated.connect([this](int32_t id) { onDesktopMenuUpdated(id); }));
        m_desktopSubscriptions.push_back(m_desktopMenu->activationRequested.connect([this](int32_t id) {
            if (m_shown != m_desktopMenu.get())
                return;
            for (size_t i = 0; i < m_rows.size(); ++i) {
                if (m_rows[i].kind == Kind::Menu && m_rows[i].id == id)
                    activationRequested(i);
            }
        }));
    }
    if (wasShown || m_desktop)
        updateShown();
}

void MenuBarModel::onDesktopMenuUpdated(int32_t id)
{
    if (id != 0 || m_desktopReady)
        return;
    m_desktopReady = true;
    // The first level of submenus is what the bar pops up: have it ready.
    for (int32_t child : m_desktopMenu->root().children) {
        const DBusMenuItem *item = m_desktopMenu->item(child);
        if (item && item->hasSubmenu)
            m_desktopMenu->aboutToShow(child);
    }
    if (m_desktop)
        updateShown();
}

void MenuBarModel::updateShown()
{
    DBusMenu *shown = m_applicationMenu->menu();
    if (m_desktop)
        shown = m_desktopReady ? m_desktopMenu.get() : nullptr;

    // ApplicationMenu says switched for any change of application, even to
    // the same menu; the open menu goes either way.
    // The previous menu's popups go after the view closed the open one, and
    // once the model is consistent: closing a menu reaches the window
    // manager, which may call back into here.
    std::map<int32_t, std::unique_ptr<DBusMenuPopup>> previous;
    if (shown != m_shown) {
        m_shownSubscriptions.clear();
        m_shown = shown;
        watchShown();
        previous = std::move(m_popups);
        m_popups.clear();
    }
    switched();
    previous.clear();
    updateRows();
}

void MenuBarModel::watchShown()
{
    if (!m_shown)
        return;
    // The bar's entries are the root's children: a new layout of the root or
    // a change to one of them renames, adds or removes an entry.
    m_shownSubscriptions.push_back(m_shown->menuUpdated.connect([this](int32_t id) {
        if (id == 0)
            updateRows();
    }));
    m_shownSubscriptions.push_back(m_shown->itemChanged.connect([this](int32_t id) {
        const DBusMenuItem *item = m_shown->item(id);
        if (item && item->parent == 0)
            updateRows();
    }));
}

void MenuBarModel::updateRows()
{
    std::vector<Row> rows;
    rows.push_back({Kind::User, {}, -1});
    const std::string &title = m_applicationMenu->title();
    if (!title.empty()) {
        rows.push_back({Kind::Application, title, -1});
        // Top-level menus the application hid have no entry.
        if (m_shown) {
            for (int32_t child : m_shown->root().children) {
                const DBusMenuItem *item = m_shown->item(child);
                if (item && item->visible)
                    rows.push_back({Kind::Menu, DBusMenuPopup::plainLabel(item->label), child});
            }
        }
    }
    if (rows == m_rows)
        return;
    m_rows = std::move(rows);

    // Menus of entries that are gone go with them.
    std::vector<std::unique_ptr<DBusMenuPopup>> gone;
    for (auto it = m_popups.begin(); it != m_popups.end();) {
        const bool kept = std::any_of(m_rows.begin(), m_rows.end(),
                                      [&](const Row &row) { return row.kind == Kind::Menu && row.id == it->first; });
        if (kept) {
            ++it;
            continue;
        }
        gone.push_back(std::move(it->second));
        it = m_popups.erase(it);
    }
    rowsChanged();
    gone.clear();
}

ui::Menu *MenuBarModel::menuAt(size_t row)
{
    if (row >= m_rows.size())
        return nullptr;
    switch (m_rows[row].kind) {
    case Kind::User:
        return m_userMenu.get();
    case Kind::Application:
        return m_applicationRowMenu.get();
    case Kind::Menu:
        break;
    }
    const DBusMenuItem *item = m_shown ? m_shown->item(m_rows[row].id) : nullptr;
    if (!item || !(item->hasSubmenu || !item->children.empty()))
        return nullptr;
    std::unique_ptr<DBusMenuPopup> &popup = m_popups[item->id];
    if (!popup) {
        popup = std::make_unique<DBusMenuPopup>(m_shown, item->id);
        popup->onClosed = [this] { menuClosed(); };
    }
    return &popup->menu();
}

void MenuBarModel::trigger(size_t row)
{
    if (row >= m_rows.size() || m_rows[row].kind != Kind::Menu || !m_shown)
        return;
    if (const DBusMenuItem *item = m_shown->item(m_rows[row].id); item && item->enabled)
        m_shown->activate(item->id);
}

} // namespace Lingmo::Shell
