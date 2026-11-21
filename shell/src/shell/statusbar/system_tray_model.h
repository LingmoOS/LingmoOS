// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "shell/common/list_model.h"
#include "shell/statusbar/status_notifier_item.h"

namespace Lingmo::Shell {

class StatusNotifierWatcher;
class TaskManager;

// A theme icon lookup with the item's IconThemePath as a fallback search
// path: the icon theme, its parents and hicolor, then name.png, .svg or .xpm
// directly in the path.
bool themeIconExists(const std::string &name, const std::string &themePath);

// The tray's items in the order they registered, moved as the user drags
// them; a row changes whenever its item read itself again.
class SystemTrayModel : public ListModelSignals
{
public:
    SystemTrayModel(StatusNotifierWatcher &watcher, base::dbus::Connection *bus, TaskManager *tasks,
                    StatusNotifierItem::IconExists iconExists = themeIconExists);
    ~SystemTrayModel();

    SystemTrayModel(const SystemTrayModel &) = delete;
    SystemTrayModel &operator=(const SystemTrayModel &) = delete;

    int count() const { return int(m_items.size()); }
    StatusNotifierItem *at(int row) const { return m_items[size_t(row)].get(); }
    int indexOf(const std::string &id) const;
    StatusNotifierItem *find(const std::string &id) const;

    void leftButtonClick(const std::string &id, int x, int y);
    void middleButtonClick(const std::string &id, int x, int y);
    // The item's menu, asked AboutToShow; null for an item that exports
    // none, which is then asked to show its own.
    DBusMenu *contextMenu(const std::string &id, int x, int y);
    void move(int from, int to);

private:
    void add(const std::string &id);
    void remove(const std::string &id);

    base::dbus::Connection *m_bus;
    TaskManager *m_tasks;
    StatusNotifierItem::IconExists m_iconExists;
    std::vector<std::unique_ptr<StatusNotifierItem>> m_items;
    std::vector<base::Subscription> m_itemChanges;
    base::Subscription m_registered;
    base::Subscription m_unregistered;
};

} // namespace Lingmo::Shell

