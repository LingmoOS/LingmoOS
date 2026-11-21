// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <systemd/sd-bus.h>
#include "base/dbus.h"

namespace Lingmo::base::dbus {
class Connection;
}

namespace dbus = Lingmo::base::dbus;

namespace Lingmo::ui {

// Keeps the theme in step with the user's appearance settings, which
// lingmo-services keeps: dark mode, accent colour, font and icon theme.
class AppearanceWatcher
{
public:
    // Asks for the settings, then follows their change signals.
    explicit AppearanceWatcher(dbus::Connection& connection);
    ~AppearanceWatcher();

    AppearanceWatcher(const AppearanceWatcher&) = delete;
    AppearanceWatcher& operator=(const AppearanceWatcher&) = delete;

    // Moves the connection on without waiting, so the call goes out while
    // the caller does other work; nothing is applied yet.
    void advance();
    // Blocks until the settings asked for at construction have been applied,
    // or the call failed, so the first frame has the right colours.
    void waitForSettings();

private:
    void reload();
    static int changed(sd_bus_message* message, void* data, sd_bus_error* error);
    static int replied(sd_bus_message* reply, void* data, sd_bus_error* error);
    static void apply(sd_bus_message* reply);

    dbus::Connection& m_connection;
    dbus::NameWatch m_serviceWatch;
    sd_bus_slot* m_match = nullptr;
    sd_bus_slot* m_call = nullptr;
    // The first reply, held until waitForSettings() applies it.
    bool m_waited = false;
    sd_bus_message* m_firstReply = nullptr;
};

} // namespace Lingmo::ui
