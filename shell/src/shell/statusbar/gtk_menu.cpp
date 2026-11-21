// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/gtk_menu.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <utility>

namespace Lingmo::Shell {
namespace dbus = base::dbus;
namespace {

constexpr const char *kMenusInterface = "org.gtk.Menus";
constexpr const char *kActionsInterface = "org.gtk.Actions";

std::vector<std::vector<std::string>> shortcutOf(const std::string &accelerator)
{
    std::vector<std::string> keys;
    size_t at = 0;
    while (at < accelerator.size() && accelerator[at] == '<') {
        const size_t end = accelerator.find('>', at);
        if (end == std::string::npos)
            return {};
        std::string modifier = accelerator.substr(at + 1, end - at - 1);
        std::transform(modifier.begin(), modifier.end(), modifier.begin(),
                       [](unsigned char c) { return char(std::tolower(c)); });
        if (modifier == "primary" || modifier == "control" || modifier == "ctrl" || modifier == "ctl")
            keys.push_back("Control");
        else if (modifier == "shift" || modifier == "shft")
            keys.push_back("Shift");
        else if (modifier == "alt" || modifier == "mod1")
            keys.push_back("Alt");
        else if (modifier == "super" || modifier == "meta" || modifier == "hyper")
            keys.push_back("Super");
        at = end + 1;
    }
    if (at >= accelerator.size())
        return {};
    keys.push_back(accelerator.substr(at));
    return {std::move(keys)};
}

std::string stringAttribute(const dbus::VariantMap &attributes, const std::string &name)
{
    const auto it = attributes.find(name);
    return it == attributes.end() ? std::string() : it->second.value<std::string>();
}

std::string iconNameOf(const dbus::Variant &icon)
{
    if (const auto name = icon.get<std::string>())
        return name->find('/') == std::string::npos ? *name : std::string();
    if (icon.signature() != "(sv)" || icon.items()[0].value<std::string>() != "themed")
        return {};
    const auto names = icon.items()[1].get<dbus::Variant>();
    const auto list = names ? names->value<std::vector<std::string>>() : std::vector<std::string>();
    return list.empty() ? std::string() : list.front();
}

std::string namespaceUnder(const std::string &outer, const dbus::VariantMap &attributes)
{
    const std::string inner = stringAttribute(attributes, "action-namespace");
    return inner.empty() ? outer : outer.empty() ? inner : outer + '.' + inner;
}

std::string qualified(const std::string &actionNamespace, const std::string &name)
{
    return name.empty() || actionNamespace.empty() ? name : actionNamespace + '.' + name;
}

} // namespace

GtkMenu::GtkMenu(dbus::Connection *bus, std::string busName, std::string menuPath,
                 std::string appMenuPath, std::string windowPath, std::string applicationPath)
    : DBusMenu(bus, busName)
    , m_bus(bus)
    , m_busName(std::move(busName))
    , m_sender(bus, m_busName)
{
    DBusMenuItem root;
    root.hasSubmenu = true;
    m_items.emplace(0, std::move(root));
    m_rebuildTimer.setSingleShot(true);
    m_rebuildTimer.onTimeout = [this] { rebuild(); };
    m_preparedTimer.setSingleShot(true);
    m_preparedTimer.onTimeout = [this] {
        for (int32_t id : std::exchange(m_prepared, {}))
            if (m_items.count(id))
                menuUpdated(id);
    };

    const std::array<std::string, 2> paths = {menuPath, appMenuPath};
    for (int exporter = 0; exporter < 2; ++exporter) {
        if (paths[exporter].empty())
            continue;
        m_menuProxies[exporter] = dbus::Proxy(bus, m_busName, paths[exporter], kMenusInterface);
        m_matches.push_back(m_menuProxies[exporter].onSignal("Changed", [this, exporter](const dbus::Message &message) {
            onMenusChanged(exporter, message);
        }));
        start(exporter, {0});
    }
    for (const auto &[prefix, path] : {std::pair<std::string, std::string>("app", applicationPath),
                                       std::pair<std::string, std::string>("win", windowPath)}) {
        if (path.empty())
            continue;
        const auto [it, inserted] = m_actionProxies.emplace(
                prefix, dbus::Proxy(bus, m_busName, path, kActionsInterface));
        (void)inserted;
        m_matches.push_back(it->second.onSignal("Changed", [this, prefix](const dbus::Message &message) {
            onActionsChanged(prefix, message);
        }));
        describeActions(prefix);
    }
}

GtkMenu::~GtkMenu()
{
    for (int exporter = 0; exporter < 2; ++exporter) {
        if (!m_subscribed[exporter].empty() && m_menuProxies[exporter].isValid())
            m_menuProxies[exporter].send("End", std::vector<uint32_t>(m_subscribed[exporter].begin(), m_subscribed[exporter].end()));
    }
}

GtkMenu::Entry GtkMenu::parseEntry(const dbus::VariantMap &attributes)
{
    Entry entry;
    for (const auto &[key, value] : attributes) {
        if (key == ":section")
            entry.section = value.get<Link>();
        else if (key == ":submenu")
            entry.submenu = value.get<Link>();
        else if (!key.starts_with(':'))
            entry.attributes.emplace(key, value);
    }
    return entry;
}

GtkMenu::Action GtkMenu::parseAction(const ActionDescription &description)
{
    Action action;
    action.enabled = std::get<0>(description);
    action.parameterType = std::get<1>(description).signature;
    if (!std::get<2>(description).empty())
        action.state = std::get<2>(description).front();
    return action;
}

const DBusMenuItem *GtkMenu::item(int32_t id) const
{
    const auto it = m_items.find(id);
    return it == m_items.end() ? nullptr : &it->second;
}

void GtkMenu::start(int exporter, std::vector<uint32_t> requested)
{
    std::vector<uint32_t> groups;
    for (uint32_t group : requested) {
        if (m_subscribed[exporter].size() < kMaxGroups && m_subscribed[exporter].insert(group).second)
            groups.push_back(group);
    }
    if (groups.empty())
        return;
    const bool initial = !m_built;
    if (initial)
        ++m_loading;
    m_calls.track([&](std::function<void()> finished) {
        return m_menuProxies[exporter].callAsync("Start", [this, exporter, groups, initial, finished](const dbus::Reply &reply) {
            std::vector<std::tuple<uint32_t, uint32_t, std::vector<dbus::VariantMap>>> menus;
            std::vector<Entry> linked;
            if (reply.read(menus)) {
                for (const auto &[group, menu, attributes] : menus) {
                    if (!m_subscribed[exporter].count(group))
                        continue;
                    std::vector<Entry> entries;
                    for (const auto &item : attributes) {
                        if (entries.size() >= kMaxItems)
                            break;
                        entries.push_back(parseEntry(item));
                    }
                    linked.insert(linked.end(), entries.begin(), entries.end());
                    m_menus[{exporter, group, menu}] = std::move(entries);
                }
            }
            m_loaded[exporter].insert(groups.begin(), groups.end());
            subscribeLinks(exporter, linked);
            if (initial)
                --m_loading;
            scheduleRebuild();
            finished();
        }, groups);
    });
}

void GtkMenu::subscribeLinks(int exporter, const std::vector<Entry> &entries)
{
    std::vector<uint32_t> groups;
    for (const Entry &entry : entries) {
        for (const auto &link : {entry.section, entry.submenu}) {
            if (link && !m_subscribed[exporter].count(std::get<0>(*link))
                && std::find(groups.begin(), groups.end(), std::get<0>(*link)) == groups.end())
                groups.push_back(std::get<0>(*link));
        }
    }
    if (!groups.empty())
        start(exporter, std::move(groups));
}

void GtkMenu::describeActions(const std::string &prefix)
{
    const bool initial = !m_built;
    if (initial)
        ++m_loading;
    m_calls.track([&](std::function<void()> finished) {
        return m_actionProxies.at(prefix).callAsync("DescribeAll",
                [this, prefix, initial, finished](const dbus::Reply &reply) {
            std::map<std::string, ActionDescription> described;
            if (reply.read(described)) {
                auto &actions = m_actions[prefix];
                for (const auto &[name, description] : described) {
                    if (actions.size() >= kMaxItems)
                        break;
                    actions.emplace(name, parseAction(description));
                }
            }
            if (initial)
                --m_loading;
            scheduleRebuild();
            finished();
        });
    });
}

void GtkMenu::onMenusChanged(int exporter, const dbus::Message &message)
{
    using Change = std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, std::vector<dbus::VariantMap>>;
    std::vector<Change> changes;
    if (!m_sender.accepts(message) || !message.read(changes))
        return;
    std::vector<Entry> linked;
    for (const auto &[group, menu, position, removed, added] : changes) {
        auto it = m_menus.find({exporter, group, menu});
        if (it == m_menus.end()) {
            if (!m_loaded[exporter].count(group))
                continue;
            it = m_menus.emplace(MenuKey{exporter, group, menu}, std::vector<Entry>()).first;
        }
        auto &entries = it->second;
        const size_t at = std::min<size_t>(position, entries.size());
        const size_t count = std::min<size_t>(removed, entries.size() - at);
        entries.erase(entries.begin() + ptrdiff_t(at), entries.begin() + ptrdiff_t(at + count));
        std::vector<Entry> inserted;
        for (const auto &attributes : added) {
            if (entries.size() + inserted.size() >= kMaxItems)
                break;
            inserted.push_back(parseEntry(attributes));
        }
        entries.insert(entries.begin() + ptrdiff_t(at), inserted.begin(), inserted.end());
        linked.insert(linked.end(), inserted.begin(), inserted.end());
    }
    subscribeLinks(exporter, linked);
    scheduleRebuild();
}

void GtkMenu::onActionsChanged(const std::string &prefix, const dbus::Message &message)
{
    std::vector<std::string> removed;
    std::map<std::string, bool> enabled;
    dbus::VariantMap states;
    std::map<std::string, ActionDescription> added;
    if (!m_sender.accepts(message) || !message.read(removed, enabled, states, added))
        return;
    auto &actions = m_actions[prefix];
    for (const auto &name : removed)
        actions.erase(name);
    for (const auto &[name, value] : enabled)
        if (auto it = actions.find(name); it != actions.end())
            it->second.enabled = value;
    for (const auto &[name, value] : states)
        if (auto it = actions.find(name); it != actions.end())
            it->second.state = value;
    for (const auto &[name, description] : added)
        if (actions.size() < kMaxItems || actions.count(name))
            actions.insert_or_assign(name, parseAction(description));
    scheduleRebuild();
}

void GtkMenu::scheduleRebuild()
{
    if (!m_rebuildTimer.isActive())
        m_rebuildTimer.start(std::chrono::nanoseconds(0));
}

int32_t GtkMenu::idFor(const std::string &key)
{
    const auto [it, inserted] = m_ids.emplace(key, m_nextId);
    if (inserted)
        ++m_nextId;
    return it->second;
}

const GtkMenu::Action *GtkMenu::findAction(const std::string &name) const
{
    const size_t dot = name.find('.');
    if (dot == std::string::npos)
        return nullptr;
    const auto group = m_actions.find(name.substr(0, dot));
    if (group == m_actions.end())
        return nullptr;
    const auto action = group->second.find(name.substr(dot + 1));
    return action == group->second.end() ? nullptr : &action->second;
}

void GtkMenu::applyAction(DBusMenuItem &item, const Entry &entry, const Target &target) const
{
    if (target.action.empty())
        return;
    const std::string hiddenWhen = stringAttribute(entry.attributes, "hidden-when");
    const Action *action = findAction(target.action);
    if (!action) {
        item.enabled = false;
        item.visible = hiddenWhen != "action-missing" && hiddenWhen != "action-disabled";
        return;
    }
    const bool parameterMatches = target.parameter ? action->parameterType == target.parameter->signature()
                                                   : action->parameterType.empty();
    item.enabled = parameterMatches && action->enabled;
    if (hiddenWhen == "action-disabled")
        item.visible = item.enabled;
    if (!parameterMatches || !action->state)
        return;
    if (target.parameter) {
        item.toggle = DBusMenuItem::Toggle::Radio;
        item.toggleState = *action->state == *target.parameter ? 1 : 0;
    } else if (const auto state = action->state->get<bool>()) {
        item.toggle = DBusMenuItem::Toggle::Checkmark;
        item.toggleState = *state ? 1 : 0;
    }
}

void GtkMenu::flatten(int exporter, uint32_t group, uint32_t menu, const std::string &actionNamespace,
                      const std::string &key, int depth, int32_t parent,
                      std::unordered_map<int32_t, DBusMenuItem> &items,
                      std::unordered_map<int32_t, Target> &targets)
{
    if (depth > kMaxDepth)
        return;
    const auto found = m_menus.find({exporter, group, menu});
    if (found == m_menus.end())
        return;
    const auto separator = [&](const std::string &separatorKey) {
        if (parent == 0 || items.size() >= kMaxItems)
            return;
        DBusMenuItem item;
        item.id = idFor(separatorKey);
        item.parent = parent;
        item.separator = true;
        items.at(parent).children.push_back(item.id);
        items.emplace(item.id, std::move(item));
    };
    for (size_t index = 0; index < found->second.size(); ++index) {
        const Entry &entry = found->second[index];
        const std::string itemKey = key + '/' + std::to_string(index);
        if (entry.section) {
            separator(itemKey + '<');
            flatten(exporter, std::get<0>(*entry.section), std::get<1>(*entry.section),
                    namespaceUnder(actionNamespace, entry.attributes), itemKey, depth + 1, parent, items, targets);
            separator(itemKey + '>');
            continue;
        }
        if (items.size() >= kMaxItems)
            return;
        DBusMenuItem item;
        item.id = idFor(itemKey);
        item.parent = parent;
        item.label = stringAttribute(entry.attributes, "label");
        item.shortcut = shortcutOf(stringAttribute(entry.attributes, "accel"));
        if (const auto icon = entry.attributes.find("icon"); icon != entry.attributes.end())
            item.iconName = iconNameOf(icon->second);
        Target target;
        target.action = qualified(actionNamespace, stringAttribute(entry.attributes, "action"));
        if (const auto parameter = entry.attributes.find("target"); parameter != entry.attributes.end())
            target.parameter = parameter->second;
        applyAction(item, entry, target);
        const int32_t id = item.id;
        items.at(parent).children.push_back(id);
        if (entry.submenu) {
            const std::string submenuNamespace = namespaceUnder(actionNamespace, entry.attributes);
            target.submenuAction = qualified(submenuNamespace,
                                              stringAttribute(entry.attributes, "submenu-action"));
            item.hasSubmenu = true;
            item.childrenLoaded = m_loaded[exporter].count(std::get<0>(*entry.submenu)) > 0;
            items.emplace(id, std::move(item));
            flatten(exporter, std::get<0>(*entry.submenu), std::get<1>(*entry.submenu), submenuNamespace, itemKey,
                    depth + 1, id, items, targets);
        } else {
            items.emplace(id, std::move(item));
        }
        targets.emplace(id, std::move(target));
    }
}

void GtkMenu::rebuild()
{
    if (m_loading > 0)
        return;
    m_built = true;
    std::unordered_map<int32_t, DBusMenuItem> items;
    std::unordered_map<int32_t, Target> targets;
    DBusMenuItem root;
    root.hasSubmenu = true;
    root.childrenLoaded = true;
    items.emplace(0, std::move(root));
    // The application menu is inserted below a synthetic application item,
    // while the window menu remains at the root of the global bar.
    if (m_menus.count({1, 0, 0})) {
        DBusMenuItem application;
        application.id = idFor("a");
        application.parent = 0;
        application.hasSubmenu = true;
        application.childrenLoaded = true;
        const int32_t id = application.id;
        items.emplace(id, std::move(application));
        items.at(0).children.push_back(id);
        flatten(1, 0, 0, {}, "a", 1, id, items, targets);
    }
    flatten(0, 0, 0, {}, "m", 0, 0, items, targets);
    std::vector<int32_t> updated;
    std::vector<int32_t> changed;
    for (const auto &[id, item] : items) {
        const auto old = m_items.find(id);
        if (old == m_items.end())
            continue;
        if (old->second.children != item.children || old->second.childrenLoaded != item.childrenLoaded)
            updated.push_back(id);
        if (old->second.label != item.label || old->second.enabled != item.enabled
            || old->second.visible != item.visible || old->second.iconName != item.iconName
            || old->second.shortcut != item.shortcut || old->second.toggle != item.toggle
            || old->second.toggleState != item.toggleState || old->second.hasSubmenu != item.hasSubmenu
            || old->second.separator != item.separator)
            changed.push_back(id);
    }
    m_items = std::move(items);
    m_targets = std::move(targets);
    std::erase_if(m_ids, [this](const auto &entry) { return !m_items.count(entry.second); });
    std::erase_if(m_open, [this](int32_t id) { return !m_items.count(id); });
    for (int32_t id : changed)
        if (m_items.count(id))
            itemChanged(id);
    std::sort(updated.begin(), updated.end());
    for (int32_t id : updated)
        if (m_items.count(id))
            menuUpdated(id);
    menuUpdated(0);
}

void GtkMenu::refresh(int32_t id)
{
    prepare(id);
}

void GtkMenu::prepare(int32_t id)
{
    if (const auto it = m_items.find(id); it != m_items.end() && it->second.childrenLoaded) {
        m_prepared.insert(id);
        if (!m_preparedTimer.isActive())
            m_preparedTimer.start(std::chrono::nanoseconds(0));
    }
}

void GtkMenu::setActionState(const std::string &name, bool state)
{
    const size_t dot = name.find('.');
    if (dot == std::string::npos)
        return;
    const auto proxy = m_actionProxies.find(name.substr(0, dot));
    if (proxy != m_actionProxies.end())
        proxy->second.send("SetState", name.substr(dot + 1), dbus::Variant(state), dbus::VariantMap());
}

void GtkMenu::opened(int32_t id)
{
    m_open.insert(id);
    if (const auto it = m_targets.find(id); it != m_targets.end() && !it->second.submenuAction.empty())
        setActionState(it->second.submenuAction, true);
}

void GtkMenu::closed(int32_t id)
{
    m_open.erase(id);
    if (const auto it = m_targets.find(id); it != m_targets.end() && !it->second.submenuAction.empty())
        setActionState(it->second.submenuAction, false);
}

void GtkMenu::activate(int32_t id)
{
    const auto target = m_targets.find(id);
    if (target == m_targets.end() || target->second.action.empty())
        return;
    const std::string &name = target->second.action;
    const size_t dot = name.find('.');
    if (dot == std::string::npos)
        return;
    const auto proxy = m_actionProxies.find(name.substr(0, dot));
    if (proxy == m_actionProxies.end())
        return;
    std::vector<dbus::Variant> parameters;
    if (target->second.parameter)
        parameters.push_back(*target->second.parameter);
    proxy->second.send("Activate", name.substr(dot + 1), parameters, dbus::VariantMap());
}

} // namespace Lingmo::Shell
