// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/trusted_connection.h"

#include <unistd.h>

#include "gpu/egl_context.h"
#include "ui/application.h"
#include "wayland/connection.h"

namespace Lingmo::ui {

std::unique_ptr<TrustedConnection> TrustedConnection::fromFd(int fd)
{
    Application* application = Application::instance();
    if (!application || application->platform() != Application::Platform::Wayland) {
        close(fd);
        return nullptr;
    }
    std::unique_ptr<TrustedConnection> connection(new TrustedConnection);
    connection->m_connection = wayland::Connection::openFd(application->loop(), fd);
    if (!connection->m_connection)
        return nullptr;
    connection->m_gpu = gpu::EglContext::create(connection->m_connection->display());
    if (!connection->m_gpu)
        return nullptr;
    TrustedConnection* self = connection.get();
    connection->m_connection->onOutputsChanged = [self] {
        if (self->onOutputsChanged)
            self->onOutputsChanged();
    };
    return connection;
}

TrustedConnection::~TrustedConnection()
{
    // The GPU context goes before the display it was made on.
    m_gpu.reset();
    m_connection.reset();
}

std::vector<TrustedConnection::Output> TrustedConnection::outputs() const
{
    std::vector<Output> list;
    for (const wayland::Connection::OutputInfo& output : m_connection->outputs())
        list.push_back({output.name, output.position, output.logicalSize});
    return list;
}

bool TrustedConnection::hasShell() const
{
    return m_connection->shell() != nullptr;
}

} // namespace Lingmo::ui
