// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"

namespace Lingmo::ui {
class Menu;
}

namespace Lingmo::Shell {

class ApplicationMenu;
class AppMenuRegistrar;
class DBusMenu;
class DBusMenuPopup;
class PowerActions;
class Shell;

// The entries of the status bar's menu bar, left to right: the user's own
// menu, then the active application's name and its exported menus (on the
// backend's ApplicationMenu). The user row is
// always there; the application rows come and go with the focus.
//
// The desktop is not an application: while it has the focus the bar names
// "Desktop" and shows the desktop's own menus, which the desktop's process
// exports over com.canonical.dbusmenu and announces to the AppMenu registrar
// like any application.
class MenuBarModel
{
public:
    // Where the file manager's desktop exports its menus.
    static constexpr const char *kDesktopMenuPath = "/org/lingmo/DesktopMenu";

    enum class Kind {
        User,
        Application,
        Menu,
    };

    struct Row {
        Kind kind = Kind::Menu;
        // With the exporter's "_" mnemonic markers removed.
        std::string title;
        // The dbusmenu entry of a Menu row; -1 otherwise.
        int32_t id = -1;

        bool operator==(const Row &) const = default;
    };

    // `registrar` and `power` may be null (tests): no exported menus, and a
    // user menu whose power entries do nothing.
    MenuBarModel(Shell *shell, AppMenuRegistrar *registrar, PowerActions *power);
    ~MenuBarModel();

    MenuBarModel(const MenuBarModel &) = delete;
    MenuBarModel &operator=(const MenuBarModel &) = delete;

    const std::vector<Row> &rows() const { return m_rows; }
    size_t count() const { return m_rows.size(); }

    // The menu of the entry at `row`, or null when the entry is a plain
    // action; made on first use and kept until the bar switches.
    ui::Menu *menuAt(size_t row);
    // A plain action entry was clicked.
    void trigger(size_t row);

    // Whether the desktop has the focus (ShellState::isDesktop).
    bool isDesktop() const { return m_desktop; }

    // The rows changed: the bar switched, the name changed, or the shown
    // menu gained, lost or renamed an entry.
    base::Signal<> rowsChanged;
    // The bar now belongs to another application, or none: an open menu of
    // the previous one has to go. Comes before rowsChanged.
    base::Signal<> switched;
    // The application asks for the entry at `row` to be opened, which is how
    // a menu's shortcut (Alt and its mnemonic) arrives.
    base::Signal<size_t> activationRequested;
    // A menu menuAt() handed out closed: by a choice, Escape, a press
    // elsewhere, or close().
    base::Signal<> menuClosed;

    // The desktop's exported menu, while it has one; for tests.
    DBusMenu *desktopMenu() const { return m_desktopMenu.get(); }

private:
    void buildUserMenu();
    void retranslate();
    void fillApplicationMenu();
    void updateDesktop();
    void updateDesktopImport();
    void onDesktopMenuUpdated(int32_t id);
    void updateShown();
    void watchShown();
    void updateRows();

    Shell *m_shell;
    AppMenuRegistrar *m_registrar;
    PowerActions *m_power;
    std::unique_ptr<ApplicationMenu> m_applicationMenu;
    bool m_desktop = false;

    // The desktop's export, found through the registrar by the desktop
    // window's process.
    std::unique_ptr<DBusMenu> m_desktopMenu;
    bool m_desktopReady = false;
    std::vector<base::Subscription> m_desktopSubscriptions;

    // The menu the application rows come from: the application's or the
    // desktop's; null for a bar with the name alone.
    DBusMenu *m_shown = nullptr;
    std::vector<base::Subscription> m_shownSubscriptions;
    std::vector<Row> m_rows;

    std::unique_ptr<ui::Menu> m_userMenu;
    std::unique_ptr<ui::Menu> m_applicationRowMenu;
    // The open-able menus of the shown menu's entries, by entry id.
    std::map<int32_t, std::unique_ptr<DBusMenuPopup>> m_popups;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

