// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"

#include <functional>
#include <memory>
#include <string>

// Exports an org.freedesktop.impl.portal.Request at a request handle, from
// the method that creates the request and before it returns, so a Close the
// frontend sends right behind that method finds it. Null when the path is
// not a request handle or is taken. `close` may reply itself; otherwise Close
// gets an empty reply after it. Destroying the object, from `close` too,
// unexports it.
std::unique_ptr<Lingmo::base::dbus::Object>
exportRequest(Lingmo::base::dbus::Connection *bus, const std::string &path,
              std::function<void(Lingmo::base::dbus::MethodCall &)> close);

// Runs `gone` when the service has no owner, at once when it has none already.
// It can run more than once, so it must ignore a repeat.
Lingmo::base::dbus::NameWatch watchRequester(Lingmo::base::dbus::Connection *bus, const std::string &service,
                                               std::function<void()> gone);

// Runs `task` on the next loop turn while `alive` is still set.
void later(const std::shared_ptr<bool> &alive, std::function<void()> task);
