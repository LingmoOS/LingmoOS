// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "base/dbus.h"

namespace Lingmo::Input {

// An object exported on the session bus under a well-known name of its own,
// which it gives up when it goes. Add the members to object(), then publish().
class DBusExport
{
public:
    // Null when there is no session bus on the calling thread's loop.
    static std::unique_ptr<DBusExport> create(std::string service, std::string path,
                                              std::string interface);
    ~DBusExport();

    base::dbus::Object &object() { return m_object; }
    const base::dbus::Object &object() const { return m_object; }
    base::dbus::Connection *connection() const { return m_connection; }

    // Exports the object, then takes the name without queueing for it.
    // `what` names the service in the warnings.
    bool publish(const char *what);

    // Sends a signal without arguments.
    void emitSignal(const char *member) const;

private:
    DBusExport(base::dbus::Connection *connection, std::string service, std::string path,
               std::string interface);

    base::dbus::Connection *m_connection;
    std::string m_service;
    base::dbus::Object m_object;
    bool m_named = false;
};

} // namespace Lingmo::Input
