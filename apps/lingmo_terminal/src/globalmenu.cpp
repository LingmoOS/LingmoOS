// SPDX-License-Identifier: GPL-3.0-or-later
#include "globalmenu.h"

#include "base/event_loop.h"

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::Variant;
using base::dbus::VariantMap;

namespace {
constexpr uint32_t kWindowId = 1;
constexpr const char* kPath = "/org/lingmo/Terminal/Menu";
constexpr const char* kInterface = "com.canonical.dbusmenu";
constexpr const char* kRegistrar = "com.canonical.AppMenu.Registrar";
constexpr const char* kRegistrarPath = "/com/canonical/AppMenu/Registrar";

std::string dbusLabel(const std::string& text)
{
    std::string result;
    for (char c : text) result += c == '_' ? "__" : std::string(1, c);
    return result;
}
}

GlobalMenu::GlobalMenu(base::dbus::Connection* bus, std::function<std::vector<Entry>()> menu)
    : m_bus(bus), m_menu(std::move(menu)), m_object(bus, kPath, kInterface)
{
    rebuild();
    if (!bus)
        return;
    m_object.addProperty("Version", [] { return uint32_t(3); });
    m_object.addProperty("TextDirection", [] { return std::string("ltr"); });
    m_object.addProperty("Status", [] { return std::string("normal"); });
    m_object.addProperty("IconThemePath", [] { return std::vector<std::string>(); });
    m_object.addMethod("GetLayout", "iias", "u(ia{sv}av)", [this](MethodCall& call) {
        int32_t parent = 0, depth = -1; std::vector<std::string> names; call.read(parent, depth, names);
        if (parent == 0) { call.reply(m_revision, layout(0, m_entries, depth)); return; }
        const Entry* entry = find(parent);
        if (!entry) { call.replyError(kInterface + std::string(".Error"), "No such item"); return; }
        call.reply(m_revision, layout(parent, entry->children, depth));
    });
    m_object.addMethod("GetProperty", "is", "v", [this](MethodCall& call) {
        int32_t id = 0; std::string name; call.read(id, name);
        const Entry* entry = find(id);
        if (!entry) { call.replyError(kInterface + std::string(".Error"), "No such item"); return; }
        const auto values = properties(*entry); auto it = values.find(name);
        call.reply(it == values.end() ? Variant(std::string()) : it->second);
    });
    m_object.addMethod("GetGroupProperties", "aias", "a(ia{sv})", [this](MethodCall& call) {
        std::vector<int32_t> ids; std::vector<std::string> names; call.read(ids, names);
        std::vector<std::tuple<int32_t, VariantMap>> result;
        for (int32_t id : ids) if (const Entry* entry = find(id)) result.emplace_back(id, properties(*entry));
        call.reply(result);
    });
    m_object.addMethod("Event", "isvu", "", [this](MethodCall& call) {
        int32_t id = 0; std::string event; call.read(id, event); call.reply();
        if (event != "clicked") return;
        const Entry* entry = find(id);
        if (!entry || !entry->action || !entry->enabled || !entry->visible) return;
        base::EventLoop::current()->post([action = entry->action, alive = m_alive] { if (*alive) action(); });
    });
    m_object.addMethod("AboutToShow", "i", "b", [this](MethodCall& call) {
        int32_t id = 0; call.read(id); if (id != 0 && !find(id)) { call.replyError(kInterface + std::string(".Error"), "No such item"); return; }
        update(); call.reply(true);
    });
    m_object.addSignal("LayoutUpdated", "ui");
    m_object.addSignal("ItemsPropertiesUpdated", "a(ia{sv})a(ias)");
    if (!m_object.publish()) return;
    m_registrar = bus->watchName(kRegistrar, [this, alive = m_alive](const std::string& owner) {
        if (*alive && !owner.empty()) registerWindow();
    });
}

GlobalMenu::~GlobalMenu()
{
    *m_alive = false;
    if (m_bus) base::dbus::Proxy(m_bus, kRegistrar, kRegistrarPath, kRegistrar).send("UnregisterWindow", kWindowId);
}

void GlobalMenu::update()
{
    const auto entries = m_menu();
    rebuild();
    ++m_revision;
    m_object.emitSignal("LayoutUpdated", m_revision, int32_t(0));
}

void GlobalMenu::registerWindow()
{
    base::dbus::Proxy proxy(m_bus, kRegistrar, kRegistrarPath, kRegistrar);
    proxy.send("RegisterWindow", kWindowId, base::dbus::ObjectPath {kPath});
}

void GlobalMenu::rebuild()
{
    m_entries = m_menu ? m_menu() : std::vector<Entry> {};
    m_ids.clear(); m_entryIds.clear(); int32_t next = 1;
    std::function<void(const std::vector<Entry>&)> number = [&](const auto& entries) {
        for (const Entry& entry : entries) { m_ids[next] = &entry; m_entryIds[&entry] = next++; number(entry.children); }
    };
    number(m_entries);
}

const GlobalMenu::Entry* GlobalMenu::find(int32_t id) const { auto it = m_ids.find(id); return it == m_ids.end() ? nullptr : it->second; }

VariantMap GlobalMenu::properties(const Entry& entry) const
{
    VariantMap result;
    if (entry.separator) result["type"] = Variant(std::string("separator")); else result["label"] = Variant(dbusLabel(entry.label));
    if (!entry.enabled) result["enabled"] = Variant(false);
    if (!entry.visible) result["visible"] = Variant(false);
    if (!entry.children.empty()) result["children-display"] = Variant(std::string("submenu"));
    return result;
}

GlobalMenu::Layout GlobalMenu::layout(int32_t id, const std::vector<Entry>& children, int depth) const
{
    VariantMap props; if (id == 0) props["children-display"] = Variant(std::string("submenu")); else if (const Entry* e = find(id)) props = properties(*e);
    std::vector<Variant> items;
    if (depth != 0) for (const Entry& child : children) if (auto it = m_entryIds.find(&child); it != m_entryIds.end()) items.push_back(Variant(layout(it->second, child.children, depth < 0 ? -1 : depth - 1)));
    return {id, props, items};
}
