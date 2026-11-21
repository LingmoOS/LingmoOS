// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>

#include "base/dbus.h"

namespace Lingmo::Shell {

// Whether a signal comes from a service. sd-bus cannot check a match rule's
// well-known sender name itself, so once the bus delivers a signal for one
// rule of a connection, every rule with the same path and interface sees it:
// two players or menus exporting the same path would hear each other. This
// follows the name's owner and lets through only what it sent.
class SenderFilter
{
public:
    SenderFilter(base::dbus::Connection *bus, const std::string &service)
    {
        // A unique name is its own owner.
        if (service.starts_with(':')) {
            m_owner = service;
            return;
        }
        if (bus)
            m_watch = bus->watchName(service, [this](const std::string &owner) { m_owner = owner; });
    }

    SenderFilter(const SenderFilter &) = delete;
    SenderFilter &operator=(const SenderFilter &) = delete;

    bool accepts(const base::dbus::Message &message) const
    {
        return !m_owner.empty() && message.sender() == m_owner;
    }

private:
    std::string m_owner;
    base::dbus::NameWatch m_watch;
};

} // namespace Lingmo::Shell

