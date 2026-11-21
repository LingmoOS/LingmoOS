// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>

#include "base/dbus.h"

namespace Lingmo::Core {

// Calls without keeping the pending call: `done` runs once the answer came,
// however long the caller lives, or at once when the call cannot be sent. A
// connection that closes first drops it unanswered. Capture only what
// outlives the call.
void callDetached(base::dbus::Connection* connection, const base::dbus::Message& call,
                  std::function<void(const base::dbus::Reply&)> done, std::chrono::microseconds timeout = {});

} // namespace Lingmo::Core
