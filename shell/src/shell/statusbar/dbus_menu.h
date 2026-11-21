// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "shell/common/pending_calls.h"
#include "shell/common/sender_filter.h"
#include "ui/bitmap.h"

namespace Lingmo::Shell {

// One entry of a com.canonical.dbusmenu menu.
struct DBusMenuItem {
    enum class Toggle {
        None,
        Checkmark,
        Radio,
    };

    int32_t id = 0;
    int32_t parent = -1;
    bool separator = false;
    // With the application's "_" mnemonic markers, "__" for a literal one.
    std::string label;
    bool enabled = true;
    bool visible = true;
    std::string iconName;
    // "icon-data", decoded on a worker; null while it is not or there is none.
    std::shared_ptr<const ui::Bitmap> icon;
    // Each combination a list of keys, as "Control", "Shift", "q".
    std::vector<std::vector<std::string>> shortcut;
    Toggle toggle = Toggle::None;
    // 0 off, 1 on, anything else indeterminate.
    int32_t toggleState = -1;
    // children-display is "submenu": its entries come when it opens.
    bool hasSubmenu = false;
    std::vector<int32_t> children;
    // A layout of its children arrived at least once.
    bool childrenLoaded = false;
};

// A client of a com.canonical.dbusmenu exporter, written from the protocol:
// the layout fetched one level at a time as menus open, kept current from
// LayoutUpdated and ItemsPropertiesUpdated, and the events a menu sends back.
// Nothing blocks; untrusted sizes are capped.
//
// An entry keeps the kind it had when it
// first arrived (type, toggle-type and children-display are not changed by a
// later layout), a submenu that opens is asked AboutToShow and refetched when
// the application says so or it has no entries yet, and "opened", "closed"
// and "clicked" events go along.
class DBusMenu
{
public:
    // Past these, the rest of a layout is dropped.
    static constexpr size_t kMaxItems = 4096;
    static constexpr size_t kMaxLabelBytes = 4096;
    static constexpr size_t kMaxIconBytes = 1 << 20;
    static constexpr int kMaxIconSide = 512;

    DBusMenu(base::dbus::Connection *bus, std::string service, std::string path);
    virtual ~DBusMenu();

    DBusMenu(const DBusMenu &) = delete;
    DBusMenu &operator=(const DBusMenu &) = delete;

    const std::string &service() const { return m_service; }
    const std::string &path() const { return m_path; }

    // The root is 0. Null for an id not (or no longer) in the menu.
    virtual const DBusMenuItem *item(int32_t id) const;
    const DBusMenuItem &root() const { return *item(0); }

    // Fetches the entries of `id` again.
    virtual void refresh(int32_t id);
    // The submenu `id` opens: prepare() and opened() at once.
    virtual void aboutToShow(int32_t id);
    // The submenu `id` is about to open: AboutToShow; its entries are
    // refetched when needed, and menuUpdated follows either way.
    virtual void prepare(int32_t id);
    // The submenu `id` is on screen: the "opened" event.
    virtual void opened(int32_t id);
    // The submenu `id` closed.
    virtual void closed(int32_t id);
    // The entry was chosen.
    virtual void activate(int32_t id);

    // The entries of `id` arrived, or a fetch failed; either way the menu
    // shows what it has.
    base::Signal<int32_t> menuUpdated;
    // Properties of an entry changed in place.
    base::Signal<int32_t> itemChanged;
    // The application asks for the entry to be shown, e.g. for its shortcut.
    base::Signal<int32_t> activationRequested;

protected:
    DBusMenu(base::dbus::Connection *bus, std::string service);

private:
    void applyLayout(int32_t parentId, const base::dbus::Variant &layout);
    void applyProperties(DBusMenuItem &item, const base::dbus::VariantMap &properties, bool created);
    void resetProperty(DBusMenuItem &item, const std::string &key);
    void removeItem(int32_t id);
    void decodeIcon(int32_t id, const std::string &bytes);
    void sendEvent(int32_t id, const std::string &event);

    base::dbus::Connection *m_bus;
    std::string m_service;
    std::string m_path;
    base::dbus::Proxy m_proxy;
    SenderFilter m_sender;
    std::unordered_map<int32_t, DBusMenuItem> m_items;
    std::vector<base::dbus::Match> m_matches;
    PendingCalls m_calls;
    // A LayoutUpdated for one of these is the refetch AboutToShow asked for.
    std::set<int32_t> m_refreshedByAboutToShow;
    std::set<int32_t> m_pendingLayoutUpdates;
    base::Timer m_layoutTimer;
    std::unordered_map<int32_t, base::Job> m_iconJobs;
};

} // namespace Lingmo::Shell

