// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>

#include "base/dbus.h"
#include "common/detached_call.h"

namespace Lingmo::Services {

// Calls without keeping the pending call: `done` runs once the answer came,
// however long the caller lives. Capture only what outlives the call.
template<typename... Args>
void callDetached(const Lingmo::Base::DBus::Proxy &proxy, const std::string &method,
                  std::function<void(const Lingmo::Base::DBus::Reply &)> done, const Args &...args)
{
    Lingmo::Base::DBus::Connection *connection = proxy.connection();
    Lingmo::Base::DBus::Message call;
    if (connection) {
        call = connection->newMethodCall(proxy.service(), proxy.path(), proxy.interface(), method);
        call.append(args...);
    }
    Lingmo::Core::callDetached(connection, call, std::move(done));
}

} // namespace Lingmo::Services

