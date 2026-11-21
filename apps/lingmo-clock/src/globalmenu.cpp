// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "globalmenu.h"
#include <base/event_loop.h>
#include <base/i18n.h>
namespace Clock {
using namespace Lingmo::base::dbus;
using Lingmo::I18n::tr;
namespace {
constexpr auto path = "/org/lingmo/Clock/Menu";
constexpr auto iface = "com.canonical.dbusmenu";
constexpr auto registrar = "com.canonical.AppMenu.Registrar";
constexpr auto registrarPath = "/com/canonical/AppMenu/Registrar";
} // namespace
static std::string labelFor(int32_t id) {
    switch (id) {
    case 1:
        return tr("Clock");
    case 2:
        return tr("World Clock");
    case 3:
        return tr("Alarms");
    case 4:
        return tr("Stopwatch");
    case 5:
        return tr("Timer");
    case 6:
        return tr("Quit");
    case 8:
        return tr("About Clock");
    case 7:
        return tr("Help");
    default:
        return {};
    }
}
static VariantMap propertiesFor(int32_t id) {
    VariantMap properties;
    if ((id >= 1 && id <= 8)) {
        properties["label"] = Variant(labelFor(id));
        properties["enabled"] = Variant(true);
        properties["visible"] = Variant(true);
    }
    return properties;
}
GlobalMenu::GlobalMenu(Connection* bus, std::function<void(int32_t)> activate)
    : m_bus(bus), m_activate(std::move(activate)), m_object(bus, path, iface) {
    if (!bus)
        return;
    m_object.addProperty("Version", [] { return uint32_t(3); });
    m_object.addProperty("TextDirection", [] { return std::string("ltr"); });
    m_object.addProperty("Status", [] { return std::string("normal"); });
    m_object.addProperty("IconThemePath", [] { return std::vector<std::string>(); });
    m_object.addMethod("GetLayout", "iias", "u(ia{sv}av)", [this](MethodCall& call) {
        int32_t parent = 0, depth = -1;
        std::vector<std::string> props;
        call.read(parent, depth, props);
        using V = Variant;
        using M = VariantMap;
        std::vector<V> items;
        auto add = [&](int id, std::string label) {
            M p;
            p["label"] = V(label);
            p["enabled"] = V(true);
            p["visible"] = V(true);
            items.emplace_back(std::tuple<int32_t, M, std::vector<V>>(id, std::move(p), std::vector<V>{}));
        };
        auto submenu = [&](int id, std::string label) {
            M p;
            p["label"] = V(label);
            p["enabled"] = V(true);
            p["visible"] = V(true);
            p["children-display"] = V(std::string("submenu"));
            items.emplace_back(std::tuple<int32_t, M, std::vector<V>>(id, std::move(p), std::vector<V>{}));
        };
        if (parent == 0) {
            submenu(1, tr("Clock"));
            submenu(7, tr("Help"));
        } else if (parent == 1) {
            add(2, tr("World Clock"));
            add(3, tr("Alarms"));
            add(4, tr("Stopwatch"));
            add(5, tr("Timer"));
            add(6, tr("Quit"));
        } else if (parent == 7) {
            add(8, tr("About Clock"));
        }
        call.reply(uint32_t(1), std::tuple<int32_t, M, std::vector<V>>(parent, std::move(M{}), std::move(items)));
    });
    m_object.addMethod("GetGroupProperties", "aias", "a(ia{sv})", [](MethodCall& c) {
        std::vector<int32_t> ids;
        std::vector<std::string> names;
        c.read(ids, names);
        std::vector<std::tuple<int32_t, VariantMap>> values;
        for (const auto id : ids)
            if ((id >= 2 && id <= 6) || id == 8)
                values.emplace_back(id, propertiesFor(id));
        c.reply(values);
    });
    m_object.addMethod("GetProperty", "is", "v", [](MethodCall& c) {
        int32_t id = 0;
        std::string key;
        c.read(id, key);
        const auto properties = propertiesFor(id);
        auto found = properties.find(key);
        c.reply(found == properties.end() ? Variant(std::string()) : found->second);
    });
    m_object.addMethod("Event", "isvu", "", [this](MethodCall& c) {
        int32_t id = 0;
        std::string event;
        c.read(id, event);
        c.reply();
        if (event == "clicked" && id >= 2 && id <= 6) {
            const auto activate = m_activate;
            auto alive = m_alive;
            Lingmo::base::EventLoop::current()->post([activate, id, alive] {
                if (*alive && activate)
                    activate(id);
            });
        } else if (event == "clicked" && id == 8) {
            const auto activate = m_activate;
            auto alive = m_alive;
            Lingmo::base::EventLoop::current()->post([activate, alive] {
                if (*alive && activate)
                    activate(8);
            });
        }
    });
    m_object.addMethod("AboutToShow", "i", "b", [](MethodCall& c) {
        int32_t id = 0;
        c.read(id);
        c.reply(false);
    });
    m_object.publish();
    m_watch = bus->watchName(registrar, [this, alive = m_alive](const std::string& owner) {
        if (!*alive)
            return;
        ++m_generation;
        m_registration.reset();
        if (m_available) {
            m_available = false;
            availabilityChanged(false);
        }
        if (!owner.empty())
            registerMenu();
    });
}
void GlobalMenu::registerMenu() {
    if (!m_bus)
        return;
    const auto generation = m_generation;
    auto call = m_bus->newMethodCall(registrar, registrarPath, registrar, "RegisterWindow");
    call.append(uint32_t(1), std::string(path));
    auto alive = m_alive;
    m_registration = m_bus->callAsync(call, [this, alive, generation](const Reply& r) {
        if (!*alive || generation != m_generation)
            return;
        const bool available = !r.isError();
        if (m_available != available) {
            m_available = available;
            availabilityChanged(m_available);
        }
    });
}
GlobalMenu::~GlobalMenu() {
    *m_alive = false;
    m_registration.reset();
    m_watch = {};
    if (m_available && m_bus) {
        auto c = m_bus->newMethodCall(registrar, registrarPath, registrar, "UnregisterWindow");
        c.append(uint32_t(1));
        m_bus->send(c);
    }
}
} // namespace Clock
