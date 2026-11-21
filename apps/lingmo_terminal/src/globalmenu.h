// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "base/dbus.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class GlobalMenu
{
public:
    struct Entry {
        std::string label;
        bool separator = false;
        bool enabled = true;
        bool visible = true;
        std::function<void()> action;
        std::vector<Entry> children;
    };

    GlobalMenu(Lingmo::base::dbus::Connection* bus, std::function<std::vector<Entry>()> menu);
    ~GlobalMenu();
    void update();

private:
    using Layout = std::tuple<int32_t, Lingmo::base::dbus::VariantMap, std::vector<Lingmo::base::dbus::Variant>>;
    void rebuild();
    const Entry* find(int32_t id) const;
    Lingmo::base::dbus::VariantMap properties(const Entry& entry) const;
    Layout layout(int32_t id, const std::vector<Entry>& children, int depth) const;
    void registerWindow();

    Lingmo::base::dbus::Connection* m_bus;
    std::function<std::vector<Entry>()> m_menu;
    Lingmo::base::dbus::Object m_object;
    std::vector<Entry> m_entries;
    std::map<int32_t, const Entry*> m_ids;
    std::map<const Entry*, int32_t> m_entryIds;
    uint32_t m_revision = 1;
    Lingmo::base::dbus::NameWatch m_registrar;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
