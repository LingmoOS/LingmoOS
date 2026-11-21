// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/notifications.h"
#include "core/common.h"

#include "base/dbus.h"
#include "base/event_loop.h"

#include <memory>

namespace fm::notifications {

void show(const std::string& desktopEntry, const std::string& icon, const std::string& body, int timeoutMs,
          std::function<void()> failed)
{
    base::dbus::Connection* bus = base::dbus::Connection::session();
    if (!bus) {
        if (failed)
            failed();
        return;
    }

    base::dbus::Proxy proxy(bus, "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                            "org.freedesktop.Notifications");
    const base::dbus::VariantMap hints {{"desktop-entry", base::dbus::Variant(desktopEntry)}};
    // The pending call keeps itself until the answer came.
    auto pending = std::make_shared<base::dbus::PendingCall>();
    *pending = proxy.callAsync(
            "Notify",
            [pending, failed](const base::dbus::Reply& reply) mutable {
                if (reply.isError() && failed)
                    failed();
                // Released on the next turn, not inside its own callback.
                base::EventLoop::current()->post([keep = std::move(pending)] {});
            },
            std::string(), uint32_t(0), icon, std::string(), body, std::vector<std::string>(), hints,
            int32_t(timeoutMs));
}

} // namespace fm::notifications
