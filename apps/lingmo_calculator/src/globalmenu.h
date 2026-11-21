// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once



#include "base/dbus.h"
#include "base/signal.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Calculator {

// Application menus for the compositor status bar: exported
// as com.canonical.dbusmenu and announced to com.canonical.AppMenu.Registrar,
// which finds it by this process, as it finds an application's.
class GlobalMenu
{
public:
    struct Entry
    {
        std::string label;
        bool separator = false;
        bool enabled = true;
        bool visible = true;
        std::function<void()> action;
        std::vector<Entry> children;
        std::vector<std::string> shortcut;
        bool checkable = false;
        bool checked = false;
    };

    static constexpr const char* kPath = "/org/lingmo/Calculator/Menu";

    // `menu` gives the entries as they are now; `refresh` brings them up to
    // date before a menu shows, as the context menu's actions are.
    GlobalMenu(base::dbus::Connection* bus, std::function<std::vector<Entry>()> menu,
                        std::function<void()> refresh);
    ~GlobalMenu();

    // Brings the entries up to date and tells a bar what changed.
    void update();
    bool isAvailable() const { return m_available; }
    base::Signal<bool> availabilityChanged;

private:
    using Layout = std::tuple<int32_t, base::dbus::VariantMap, std::vector<base::dbus::Variant>>;

    void rebuild();
    const Entry* find(int32_t id) const;
    base::dbus::VariantMap properties(const Entry& entry) const;
    Layout layout(int32_t id, const std::vector<Entry>& children, int depth) const;
    void registerWindow();
    void setAvailable(bool available);

    base::dbus::Connection* m_bus;
    std::function<std::vector<Entry>()> m_menu;
    std::function<void()> m_refresh;
    base::dbus::Object m_object;
    std::vector<Entry> m_entries;
    // Ids by position: the root is 0, the rest numbered depth first.
    std::map<int32_t, const Entry*> m_ids;
    std::map<const Entry*, int32_t> m_entryIds;
    uint32_t m_revision = 1;
    base::dbus::NameWatch m_registrar;
    base::dbus::PendingCall m_registration;
    bool m_available = false;

    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Calculator
