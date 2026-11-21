// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/system_tray_model.h"

#include <algorithm>
#include <filesystem>

#include "shell/statusbar/dbus_menu.h"
#include "shell/statusbar/status_notifier_watcher.h"
#include "ui/icon.h"

namespace Lingmo::Shell {

bool themeIconExists(const std::string &name, const std::string &themePath)
{
    if (name.empty())
        return false;
    if (!ui::ThemeIcon(name, 16).path().empty())
        return true;
    if (themePath.empty() || name.find('/') != std::string::npos)
        return false;
    std::error_code error;
    for (const char *suffix : {".png", ".svg", ".xpm"}) {
        if (std::filesystem::is_regular_file(themePath + '/' + name + suffix, error))
            return true;
    }
    return false;
}

SystemTrayModel::SystemTrayModel(StatusNotifierWatcher &watcher, base::dbus::Connection *bus, TaskManager *tasks,
                                 StatusNotifierItem::IconExists iconExists)
    : m_bus(bus)
    , m_tasks(tasks)
    , m_iconExists(std::move(iconExists))
{
    m_registered = watcher.itemRegistered.connect([this](const std::string &id) { add(id); });
    m_unregistered = watcher.itemUnregistered.connect([this](const std::string &id) { remove(id); });
    for (const std::string &id : watcher.items())
        add(id);
}

SystemTrayModel::~SystemTrayModel() = default;

int SystemTrayModel::indexOf(const std::string &id) const
{
    for (size_t row = 0; row < m_items.size(); ++row) {
        if (m_items[row]->id() == id)
            return int(row);
    }
    return -1;
}

StatusNotifierItem *SystemTrayModel::find(const std::string &id) const
{
    const int row = indexOf(id);
    return row < 0 ? nullptr : m_items[size_t(row)].get();
}

void SystemTrayModel::add(const std::string &id)
{
    if (indexOf(id) >= 0)
        return;
    auto item = std::make_unique<StatusNotifierItem>(m_bus, id, m_tasks, m_iconExists);
    StatusNotifierItem *raw = item.get();
    m_itemChanges.push_back(raw->changed.connect([this, raw] {
        const auto it = std::find_if(m_items.begin(), m_items.end(),
                                     [raw](const auto &item) { return item.get() == raw; });
        if (it != m_items.end()) {
            const int row = int(it - m_items.begin());
            dataChanged(row, row);
        }
    }));
    m_items.push_back(std::move(item));
    rowsInserted(count() - 1, 1);
}

void SystemTrayModel::remove(const std::string &id)
{
    const int row = indexOf(id);
    if (row < 0)
        return;
    // The subscription goes with the item: it was added at the same index.
    m_items.erase(m_items.begin() + row);
    m_itemChanges.erase(m_itemChanges.begin() + row);
    rowsRemoved(row, 1);
}

void SystemTrayModel::leftButtonClick(const std::string &id, int x, int y)
{
    if (StatusNotifierItem *item = find(id))
        item->activate(x, y);
}

void SystemTrayModel::middleButtonClick(const std::string &id, int x, int y)
{
    if (StatusNotifierItem *item = find(id))
        item->secondaryActivate(x, y);
}

DBusMenu *SystemTrayModel::contextMenu(const std::string &id, int x, int y)
{
    StatusNotifierItem *item = find(id);
    if (!item)
        return nullptr;
    if (DBusMenu *menu = item->menu()) {
        menu->prepare(0);
        return menu;
    }
    item->requestContextMenu(x, y);
    return nullptr;
}

void SystemTrayModel::move(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= count() || to >= count())
        return;
    auto item = std::move(m_items[size_t(from)]);
    auto change = std::move(m_itemChanges[size_t(from)]);
    m_items.erase(m_items.begin() + from);
    m_itemChanges.erase(m_itemChanges.begin() + from);
    m_items.insert(m_items.begin() + to, std::move(item));
    m_itemChanges.insert(m_itemChanges.begin() + to, std::move(change));
    rowMoved(from, to);
}

} // namespace Lingmo::Shell
