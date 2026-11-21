// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/dbus_export.h"
#include "core/logging.h"


namespace Lingmo::Input {

std::unique_ptr<DBusExport> DBusExport::create(std::string service, std::string path,
                                               std::string interface)
{
    base::dbus::Connection *connection = base::dbus::Connection::session();
    if (!connection)
        return nullptr;
    return std::unique_ptr<DBusExport>(
            new DBusExport(connection, std::move(service), std::move(path), std::move(interface)));
}

DBusExport::DBusExport(base::dbus::Connection *connection, std::string service, std::string path,
                       std::string interface)
    : m_connection(connection)
    , m_service(std::move(service))
    , m_object(connection, std::move(path), std::move(interface))
{
}

DBusExport::~DBusExport()
{
    if (m_named)
        m_connection->releaseName(m_service);
}

bool DBusExport::publish(const char *what)
{
    if (!m_object.publish()) {
        LM_WARNING(lingmoInput).printf("Cannot export %s", what);
        return false;
    }
    if (!m_connection->requestName(m_service)) {
        LM_WARNING(lingmoInput).printf("Cannot register %s: %s is taken", what, m_service.c_str());
        return false;
    }
    m_named = true;
    return true;
}

void DBusExport::emitSignal(const char *member) const
{
    m_object.emitSignal(member);
}

} // namespace Lingmo::Input
