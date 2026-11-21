// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalmenu.h"

#include "base/event_loop.h"

namespace Calculator {

using base::dbus::MethodCall;
using base::dbus::Variant;
using base::dbus::VariantMap;

// Zero is reserved for the desktop menu. Lingmo associates this nonzero
// application registration with the D-Bus sender PID, not an X11 window.
static constexpr uint32_t kApplicationWindowId = 1;

static constexpr const char* kInterface = "com.canonical.dbusmenu";
static constexpr const char* kRegistrar = "com.canonical.AppMenu.Registrar";
static constexpr const char* kRegistrarPath = "/com/canonical/AppMenu/Registrar";

// "Save & Exit" in dbusmenu's spelling, with a literal underscore doubled;
// the labels here have no mnemonics.
static std::string dbusLabel(const std::string& text)
{
    std::string label;
    for (const char c : text) {
        if (c == '_')
            label += "__";
        else
            label += c;
    }
    return label;
}

static bool sameLayout(const std::vector<GlobalMenu::Entry>& a, const std::vector<GlobalMenu::Entry>& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].label != b[i].label || a[i].separator != b[i].separator || a[i].enabled != b[i].enabled
            || a[i].visible != b[i].visible || a[i].checkable != b[i].checkable || a[i].checked != b[i].checked || a[i].shortcut != b[i].shortcut || !sameLayout(a[i].children, b[i].children))
            return false;
    }
    return true;
}

// The same entries and submenus in the same places: only labels and states differ, and the ids stay.
static bool sameShape(const std::vector<GlobalMenu::Entry>& a, const std::vector<GlobalMenu::Entry>& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].separator != b[i].separator || a[i].checkable != b[i].checkable || !sameShape(a[i].children, b[i].children))
            return false;
    }
    return true;
}

// The entries of `now` that differ from `before`, numbered as rebuild() numbers them.
static void changedEntries(const std::vector<GlobalMenu::Entry>& before,
                           const std::vector<GlobalMenu::Entry>& now, int32_t& next,
                           std::vector<std::tuple<int32_t, VariantMap>>& changed)
{
    for (size_t i = 0; i < now.size(); ++i) {
        const int32_t id = next++;
        const GlobalMenu::Entry& a = before[i];
        const GlobalMenu::Entry& b = now[i];
        if (a.label != b.label || a.enabled != b.enabled || a.visible != b.visible || a.shortcut != b.shortcut || a.checked != b.checked) {
            VariantMap properties;
            if (!b.separator)
                properties["label"] = Variant(dbusLabel(b.label));
            properties["enabled"] = Variant(b.enabled);
            properties["visible"] = Variant(b.visible);
            if (b.checkable) properties["toggle-state"] = Variant(int32_t(b.checked ? 1 : 0));
            properties["shortcut"] = Variant(std::vector<std::vector<std::string>>{b.shortcut});
            changed.emplace_back(id, std::move(properties));
        }
        changedEntries(a.children, b.children, next, changed);
    }
}

GlobalMenu::GlobalMenu(base::dbus::Connection* bus, std::function<std::vector<Entry>()> menu,
                                         std::function<void()> refresh)
    : m_bus(bus)
    , m_menu(std::move(menu))
    , m_refresh(std::move(refresh))
    , m_object(bus, kPath, kInterface)
{
    rebuild();
    if (!bus) return;
    m_object.addProperty("Version", [] { return uint32_t(3); });
    m_object.addProperty("TextDirection", [] { return std::string("ltr"); });
    m_object.addProperty("Status", [] { return std::string("normal"); });
    m_object.addProperty("IconThemePath", [] { return std::vector<std::string>(); });

    m_object.addMethod("GetLayout", "iias", "u(ia{sv}av)", [this](MethodCall& call) {
        int32_t parent = 0;
        int32_t depth = -1;
        std::vector<std::string> names;
        call.read(parent, depth, names);
        if (parent == 0) {
            call.reply(m_revision, layout(0, m_entries, depth));
            return;
        }
        const Entry* entry = find(parent);
        if (!entry) {
            call.replyError("com.canonical.dbusmenu.Error", "No such item");
            return;
        }
        call.reply(m_revision, layout(parent, entry->children, depth));
    });
    m_object.addMethod("GetGroupProperties", "aias", "a(ia{sv})", [this](MethodCall& call) {
        std::vector<int32_t> ids;
        std::vector<std::string> names;
        call.read(ids, names);
        std::vector<std::tuple<int32_t, VariantMap>> result;
        for (const int32_t id : ids) {
            if (const Entry* entry = find(id))
                result.emplace_back(id, properties(*entry));
        }
        call.reply(result);
    });
    m_object.addMethod("GetProperty", "is", "v", [this](MethodCall& call) {
        int32_t id = 0;
        std::string name;
        call.read(id, name);
        const Entry* entry = find(id);
        if (!entry) {
            call.replyError("com.canonical.dbusmenu.Error", "No such item");
            return;
        }
        const VariantMap values = properties(*entry);
        const auto found = values.find(name);
        call.reply(found != values.end() ? found->second : Variant(std::string()));
    });
    m_object.addMethod("Event", "isvu", "", [this](MethodCall& call) {
        int32_t id = 0;
        std::string event;
        call.read(id, event);
        call.reply();
        if (event != "clicked")
            return;
        const Entry* entry = find(id);
        if (!entry || !entry->action || !entry->enabled || !entry->visible)
            return;
        // After the reply, as a click on the menu would.
        base::EventLoop::current()->post([action = entry->action, alive = m_alive] {
            if (*alive)
                action();
        });
    });
    m_object.addMethod("AboutToShow", "i", "b", [this](MethodCall& call) {
        int32_t id = 0;
        call.read(id);
        if (id != 0 && !find(id)) {
            call.replyError("com.canonical.dbusmenu.Error", "No such item");
            return;
        }
        // The reply always asks for a refetch: a bar may hold submenus from before an update.
        update();
        call.reply(true);
    });
    m_object.addSignal("LayoutUpdated", "ui");
    m_object.addSignal("ItemsPropertiesUpdated", "a(ia{sv})a(ias)");
    m_object.addSignal("ItemActivationRequested", "iu");
    if (!m_object.publish()) return;

    // A registrar that comes (back) later has not heard of the menu.
    m_registrar = bus->watchName(kRegistrar, [this, alive = m_alive](const std::string& owner) {
        if (!*alive) return;
        m_registration = {};
        setAvailable(false);
        if (!owner.empty()) registerWindow();
    });
}

GlobalMenu::~GlobalMenu()
{
    *m_alive = false;
    base::dbus::Proxy registrar(m_bus, kRegistrar, kRegistrarPath, kRegistrar);
    registrar.send("UnregisterWindow", kApplicationWindowId);
}

void GlobalMenu::update()
{
    if (m_refresh)
        m_refresh();
    std::vector<Entry> entries = m_menu();
    if (sameLayout(entries, m_entries))
        return;
    // A bar shows the rows it holds as a menu opens, so changes are pushed, not left for its refetch.
    if (sameShape(entries, m_entries)) {
        std::vector<std::tuple<int32_t, VariantMap>> changed;
        int32_t next = 1;
        changedEntries(m_entries, entries, next, changed);
        rebuild();
        ++m_revision;
        const std::vector<std::tuple<int32_t, std::vector<std::string>>> removed;
        m_object.emitSignal("ItemsPropertiesUpdated", changed, removed);
        return;
    }
    rebuild();
    ++m_revision;
    // A bar refetches one level per update: the root and each submenu.
    m_object.emitSignal("LayoutUpdated", m_revision, int32_t(0));
    for (const auto& [id, entry] : m_ids) {
        if (!entry->children.empty())
            m_object.emitSignal("LayoutUpdated", m_revision, id);
    }
}

void GlobalMenu::registerWindow()
{
    base::dbus::Proxy registrar(m_bus, kRegistrar, kRegistrarPath, kRegistrar);
    m_registration = registrar.callAsync("RegisterWindow", [this, alive = m_alive](const base::dbus::Reply& reply) {
        if (*alive) setAvailable(!reply.isError());
    }, kApplicationWindowId, base::dbus::ObjectPath {kPath});
}

void GlobalMenu::setAvailable(bool available)
{
    if (m_available == available) return;
    m_available = available;
    availabilityChanged.emit(available);
}

void GlobalMenu::rebuild()
{
    m_entries = m_menu();
    m_ids.clear();
    m_entryIds.clear();
    int32_t next = 1;
    std::function<void(const std::vector<Entry>&)> number = [&](const std::vector<Entry>& entries) {
        for (const Entry& entry : entries) {
            m_ids[next] = &entry;
            m_entryIds[&entry] = next;
            ++next;
            number(entry.children);
        }
    };
    number(m_entries);
}

const GlobalMenu::Entry* GlobalMenu::find(int32_t id) const
{
    const auto found = m_ids.find(id);
    return found == m_ids.end() ? nullptr : found->second;
}

VariantMap GlobalMenu::properties(const Entry& entry) const
{
    VariantMap properties;
    if (entry.separator)
        properties["type"] = Variant(std::string("separator"));
    else
        properties["label"] = Variant(dbusLabel(entry.label));
    if (!entry.enabled)
        properties["enabled"] = Variant(false);
    if (!entry.visible)
        properties["visible"] = Variant(false);
    if (entry.checkable) {
        properties["toggle-type"] = Variant(std::string("radio"));
        properties["toggle-state"] = Variant(int32_t(entry.checked ? 1 : 0));
    }
    if (!entry.shortcut.empty())
        properties["shortcut"] = Variant(std::vector<std::vector<std::string>>{entry.shortcut});
    if (!entry.children.empty())
        properties["children-display"] = Variant(std::string("submenu"));
    return properties;
}

GlobalMenu::Layout GlobalMenu::layout(int32_t id, const std::vector<Entry>& children,
                                                        int depth) const
{
    VariantMap itemProperties;
    if (id == 0)
        itemProperties["children-display"] = Variant(std::string("submenu"));
    else if (const Entry* entry = find(id))
        itemProperties = properties(*entry);
    std::vector<Variant> items;
    if (depth != 0) {
        for (const Entry& child : children) {
            const auto childId = m_entryIds.find(&child);
            if (childId == m_entryIds.end())
                continue;
            items.push_back(Variant(layout(childId->second, child.children, depth < 0 ? -1 : depth - 1)));
        }
    }
    return {id, itemProperties, items};
}

} // namespace Calculator
