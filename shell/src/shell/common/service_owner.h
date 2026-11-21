// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <string>
#include <utility>

#include "base/dbus.h"

namespace Lingmo::Shell {

// The session's own services (org.lingmo.Services and its Battery,
// Brightness, Language, Appearance objects) start after the compositor and
// are not D-Bus activatable: a client's first read fails and nothing tells it
// to read again. `appeared` runs each time `name` gets an owner, so the
// client reads then. System services (UPower, power profiles, accounts,
// BlueZ, Privacy) are activatable and need none of this.
inline base::dbus::NameWatch followServiceOwner(base::dbus::Connection *bus, const std::string &name,
                                                std::function<void()> appeared)
{
    if (!bus)
        return {};
    return bus->watchName(name, [appeared = std::move(appeared)](const std::string &owner) {
        if (!owner.empty())
            appeared();
    });
}

} // namespace Lingmo::Shell

