// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "base/dbus.h"
#include "base/timer.h"
#include "shell/common/pending_calls.h"
#include "shell/common/sender_filter.h"
#include "shell/statusbar/dbus_menu.h"

namespace Lingmo::Shell {

class GtkMenu final : public DBusMenu
{
public:
    static constexpr size_t kMaxItems = 4096;
    static constexpr size_t kMaxGroups = 512;
    static constexpr int kMaxDepth = 16;

    GtkMenu(base::dbus::Connection *bus, std::string busName, std::string menuPath,
            std::string appMenuPath,
            std::string windowPath, std::string applicationPath);
    ~GtkMenu() override;

    const DBusMenuItem *item(int32_t id) const override;
    void refresh(int32_t id) override;
    void prepare(int32_t id) override;
    void opened(int32_t id) override;
    void closed(int32_t id) override;
    void activate(int32_t id) override;

private:
    using Link = std::tuple<uint32_t, uint32_t>;
    using MenuKey = std::tuple<int, uint32_t, uint32_t>;
    using ActionDescription = std::tuple<bool, base::dbus::Signature, std::vector<base::dbus::Variant>>;

    struct Entry {
        base::dbus::VariantMap attributes;
        std::optional<Link> section;
        std::optional<Link> submenu;
    };
    struct Action {
        bool enabled = true;
        std::string parameterType;
        std::optional<base::dbus::Variant> state;
    };
    struct Target {
        std::string action;
        std::optional<base::dbus::Variant> parameter;
        std::string submenuAction;
    };

    static Entry parseEntry(const base::dbus::VariantMap &attributes);
    static Action parseAction(const ActionDescription &description);
    void start(int exporter, std::vector<uint32_t> groups);
    void subscribeLinks(int exporter, const std::vector<Entry> &entries);
    void describeActions(const std::string &prefix);
    void onMenusChanged(int exporter, const base::dbus::Message &message);
    void onActionsChanged(const std::string &prefix, const base::dbus::Message &message);
    void scheduleRebuild();
    void rebuild();
    void flatten(int exporter, uint32_t group, uint32_t menu, const std::string &actionNamespace,
                 const std::string &key, int depth, int32_t parent,
                 std::unordered_map<int32_t, DBusMenuItem> &items,
                 std::unordered_map<int32_t, Target> &targets);
    int32_t idFor(const std::string &key);
    void applyAction(DBusMenuItem &item, const Entry &entry, const Target &target) const;
    const Action *findAction(const std::string &name) const;
    void setActionState(const std::string &name, bool state);

    base::dbus::Connection *m_bus;
    std::string m_busName;
    std::array<base::dbus::Proxy, 2> m_menuProxies;
    SenderFilter m_sender;
    std::map<std::string, base::dbus::Proxy> m_actionProxies;
    std::vector<base::dbus::Match> m_matches;
    PendingCalls m_calls;
    std::map<MenuKey, std::vector<Entry>> m_menus;
    std::array<std::set<uint32_t>, 2> m_subscribed;
    std::array<std::set<uint32_t>, 2> m_loaded;
    std::map<std::string, std::map<std::string, Action>> m_actions;
    int m_loading = 0;
    bool m_built = false;
    std::unordered_map<int32_t, DBusMenuItem> m_items;
    std::unordered_map<int32_t, Target> m_targets;
    std::unordered_map<std::string, int32_t> m_ids;
    int32_t m_nextId = 1;
    std::set<int32_t> m_open;
    std::set<int32_t> m_prepared;
    base::Timer m_rebuildTimer;
    base::Timer m_preparedTimer;
};

} // namespace Lingmo::Shell
