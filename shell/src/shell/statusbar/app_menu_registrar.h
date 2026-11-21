// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "shell/common/pending_calls.h"

namespace Lingmo::Shell {

// com.canonical.AppMenu.Registrar at /com/canonical/AppMenu/Registrar.
// Wayland has no window id a client and the
// compositor agree on, and Qt hands out a per-process window counter, so a
// registration is the window id together with the exporter's unique bus
// name, and the menu a window owns is found through the registering
// process's id.
class AppMenuRegistrar
{
public:
    explicit AppMenuRegistrar(base::dbus::Connection *bus);
    ~AppMenuRegistrar();

    AppMenuRegistrar(const AppMenuRegistrar &) = delete;
    AppMenuRegistrar &operator=(const AppMenuRegistrar &) = delete;

    bool registerService();
    void setEnabled(bool enabled);

    // The newest registration of the process: an application that opened a
    // second window registers again, and the last is the one in front.
    // Window id 0 is the desktop's (the file manager registers its desktop's
    // File/Edit so); its own browser windows must not show them.
    std::string serviceForPid(uint32_t pid) const;
    std::string pathForPid(uint32_t pid) const;
    // The process's desktop menu, window id 0.
    std::string desktopServiceForPid(uint32_t pid) const;
    std::string desktopPathForPid(uint32_t pid) const;
    // The newest registration of a menu at `path`, for an exporter whose
    // windows' process cannot be told (a client on a socket pair).
    std::string serviceForPath(const std::string &path) const;

    // windowId, service, path
    base::Signal<uint32_t, const std::string &, const std::string &> windowRegistered;
    base::Signal<uint32_t> windowUnregistered;

private:
    struct Registration {
        uint32_t windowId = 0;
        std::string service;
        std::string path;
        uint32_t pid = 0;
    };

    void registerWindow(const std::string &sender, uint32_t windowId, const std::string &path);
    void unregisterWindow(const std::string &sender, uint32_t windowId);
    const Registration *newestFor(uint32_t pid, bool desktop) const;
    void removeAt(size_t index);
    void serviceUnregistered(const std::string &service);

    bool m_enabled = true;
    bool m_requested = false;
    uint64_t m_generation = 0;
    base::dbus::Connection *m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
    PendingCalls m_calls;
    // Oldest first; the newest registration of a process wins.
    std::vector<Registration> m_registrations;
    std::map<std::string, base::dbus::NameWatch> m_watches;
};

} // namespace Lingmo::Shell

