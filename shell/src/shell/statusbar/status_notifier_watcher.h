// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::Shell {

// org.kde.StatusNotifierWatcher at /StatusNotifierWatcher: the tray items
// applications register, each "service/path", dropped when its service
// leaves the bus. The status bar's tray is its host in this process.
class StatusNotifierWatcher
{
public:
    explicit StatusNotifierWatcher(base::dbus::Connection *bus);
    ~StatusNotifierWatcher();

    StatusNotifierWatcher(const StatusNotifierWatcher &) = delete;
    StatusNotifierWatcher &operator=(const StatusNotifierWatcher &) = delete;

    // Exports the object and takes the name; the tray in this process then
    // counts as a host, and also takes org.kde.StatusNotifierHost-<pid>, the
    // name the specification gives a host.
    bool registerService();

    // In the order they came.
    const std::vector<std::string> &items() const { return m_items; }
    bool isHostRegistered() const { return m_internalHost || !m_hosts.empty(); }

    base::Signal<const std::string &> itemRegistered;
    base::Signal<const std::string &> itemUnregistered;

private:
    void registerItem(const std::string &sender, const std::string &serviceOrPath);
    void registerHost(const std::string &service);
    void watch(const std::string &service);
    void ownerChanged(const std::string &service, const std::string &owner);
    void serviceUnregistered(const std::string &service);

    base::dbus::Connection *m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
    std::string m_hostName;
    bool m_internalHost = false;
    std::vector<std::string> m_items;
    // Asked for, waiting to hear whether their service is on the bus.
    std::map<std::string, std::vector<std::string>> m_waitingItems;
    std::set<std::string> m_waitingHosts;
    std::set<std::string> m_hosts;
    std::map<std::string, base::dbus::NameWatch> m_watches;
    // Services whose owner has been heard once.
    std::set<std::string> m_known;
};

} // namespace Lingmo::Shell

