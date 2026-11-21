// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "shell/common/pending_calls.h"
#include "shell/common/sender_filter.h"
#include "ui/bitmap.h"

namespace Lingmo::Shell {

class DBusMenu;
class TaskManager;

// One tray item: an org.kde.StatusNotifierItem read with one GetAll after
// each of its change signals, its pictures decoded on a worker.
class StatusNotifierItem
{
public:
    // Past these, a picture is not decoded.
    static constexpr int kMaxPixmapSide = 4096;
    static constexpr size_t kMaxPixmaps = 16;

    // Whether the icon theme, or the item's own IconThemePath, has an icon of
    // that name, the path searched last as a fallback.
    using IconExists = std::function<bool(const std::string &name, const std::string &themePath)>;

    // `id` is "service/path". `tasks` may be null: then an item without an
    // icon of its own has none.
    StatusNotifierItem(base::dbus::Connection *bus, std::string id, TaskManager *tasks, IconExists iconExists);
    ~StatusNotifierItem();

    StatusNotifierItem(const StatusNotifierItem &) = delete;
    StatusNotifierItem &operator=(const StatusNotifierItem &) = delete;

    const std::string &id() const { return m_id; }
    const std::string &title() const { return m_title; }
    // The tooltip's title.
    const std::string &toolTip() const { return m_toolTip; }
    // A theme icon that exists: IconName, else Id, each as given or as its
    // -symbolic variant. Kept as a name, so a new icon theme applies.
    const std::string &iconName() const { return m_iconName; }
    const std::string &iconThemePath() const { return m_iconThemePath; }
    // IconPixmap, one per size; empty while decoding or when there are none.
    const std::vector<ui::Bitmap> &pixmaps() const { return m_pixmaps; }
    // The icon of the application's desktop entry, for an item that shows no
    // usable icon of its own: a theme name or an absolute path.
    const std::string &applicationIcon() const { return m_applicationIcon; }

    // ItemIsMenu: the item only has a menu, which a left click shows too.
    bool itemIsMenu() const { return m_itemIsMenu; }

    // The exported menu, or null when the item has none; then
    // requestContextMenu() asks the item to show its own.
    DBusMenu *menu() const { return m_menu.get(); }

    // The position is the item's, in logical global coordinates: only a hint
    // for where the application puts its window.
    void activate(int x, int y);
    void secondaryActivate(int x, int y);
    void scroll(int delta, const std::string &orientation);
    void requestContextMenu(int x, int y);

    // Reads the item again, shortly.
    void refresh();

    base::Signal<> changed;

private:
    void performRefresh();
    void apply(const base::dbus::VariantMap &properties);
    void decodePixmaps(const base::dbus::Variant &pixmaps);
    void lookUpApplicationIcon();

    base::dbus::Connection *m_bus;
    std::string m_id;
    std::string m_service;
    std::string m_path;
    TaskManager *m_tasks;
    IconExists m_iconExists;
    base::dbus::Proxy m_proxy;
    std::unique_ptr<SenderFilter> m_sender;
    std::vector<base::dbus::Match> m_matches;
    PendingCalls m_calls;
    base::Timer m_refreshTimer;
    bool m_refreshing = false;
    bool m_needsRefresh = false;

    std::string m_appId;
    std::string m_title;
    std::string m_toolTip;
    std::string m_iconName;
    std::string m_iconThemePath;
    std::vector<ui::Bitmap> m_pixmaps;
    std::string m_applicationIcon;
    bool m_itemIsMenu = false;
    std::unique_ptr<DBusMenu> m_menu;
    base::Job m_decode;
    base::Subscription m_applicationsChanged;
};

} // namespace Lingmo::Shell

