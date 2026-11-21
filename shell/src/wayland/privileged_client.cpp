// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/privileged_client.h"

#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>

#include "core/logging.h"

namespace Lingmo::Wayland {

PrivilegedClients::PrivilegedClients(wl_display *display)
    : m_display(display)
{
    // Installed before any socket is added, so no client can ever have seen a
    // private global through an unfiltered registry.
    wl_display_set_global_filter(m_display, &PrivilegedClients::globalFilter, this);
}

PrivilegedClients::~PrivilegedClients()
{
    shutdown();
}

void PrivilegedClients::shutdown()
{
    forget(m_shell);
    for (const auto &connection : m_captureConnections)
        forget(*connection);
    m_captureConnections.clear();
    if (m_display)
        wl_display_set_global_filter(m_display, nullptr, nullptr);
    m_display = nullptr;
    m_restrictedGlobals.clear();
}

bool PrivilegedClients::globalFilter(const wl_client *client, const wl_global *global, void *data)
{
    auto *self = static_cast<PrivilegedClients *>(data);
    if (auto it = self->m_restrictedGlobals.find(global); it != self->m_restrictedGlobals.end())
        return it->second && it->second(client);
    const bool isPrivate =
            std::find(self->m_privateGlobals.begin(), self->m_privateGlobals.end(), global)
            != self->m_privateGlobals.end();
    if (!isPrivate)
        return true;
    return self->isTrusted(client);
}

void PrivilegedClients::addRestrictedGlobal(wl_global *global,
                                            std::function<bool(const wl_client *)> authorize)
{
    if (global)
        m_restrictedGlobals[global] = std::move(authorize);
}

void PrivilegedClients::removeRestrictedGlobal(wl_global *global)
{
    m_restrictedGlobals.erase(global);
}

void PrivilegedClients::addPrivateGlobal(wl_global *global)
{
    if (global
        && std::find(m_privateGlobals.begin(), m_privateGlobals.end(), global)
                == m_privateGlobals.end()) {
        m_privateGlobals.push_back(global);
    }
}

void PrivilegedClients::onConnectionDestroyed(wl_listener *listener, void *)
{
    auto *connection = reinterpret_cast<DestroyListener *>(listener)->connection;

    // The listener is removed with the client; do not touch it again.
    connection->listening = false;
    connection->client = nullptr;
    LM_INFO(lingmoWayland) << "Trusted" << connection->label << "client disconnected";
}

void PrivilegedClients::forget(TrustedConnection &connection)
{
    if (connection.listening) {
        wl_list_remove(&connection.destroy.listener.link);
        connection.listening = false;
    }
    connection.client = nullptr;
}

wl_client *PrivilegedClients::adopt(TrustedConnection &connection, int fd)
{
    if (fd < 0)
        return nullptr;

    // Only one at a time. The old connection is destroyed first, which runs its
    // destroy listener and clears every piece of its state, so a late event
    // from it can never reach the new one.
    if (connection.client) {
        wl_client *previous = connection.client;
        forget(connection);
        wl_client_destroy(previous);
    }

    wl_client *client = wl_client_create(m_display, fd);
    if (!client) {
        LM_WARNING(lingmoWayland) << "Cannot create the trusted" << connection.label << "client";
        close(fd);
        return nullptr;
    }

    connection.client = client;
    connection.destroy.connection = &connection;
    connection.destroy.listener.notify = &PrivilegedClients::onConnectionDestroyed;
    wl_client_add_destroy_listener(client, &connection.destroy.listener);
    connection.listening = true;

    LM_INFO(lingmoWayland) << "Trusted" << connection.label << "client adopted";
    return client;
}

wl_client *PrivilegedClients::adoptShellConnection(int fd)
{
    return adopt(m_shell, fd);
}

int PrivilegedClients::createCaptureConnection()
{
    if (!m_display)
        return -1;
    std::erase_if(m_captureConnections, [](const std::unique_ptr<TrustedConnection> &connection) {
        return !connection->client;
    });
    int fds[2] {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds)) {
        LM_WARNING(lingmoWayland) << "Cannot create a capture connection";
        return -1;
    }
    auto connection = std::make_unique<TrustedConnection>();
    connection->label = "capture";
    if (!adopt(*connection, fds[0])) {
        close(fds[1]);
        return -1;
    }
    m_captureConnections.push_back(std::move(connection));
    return fds[1];
}

bool PrivilegedClients::isCaptureConnection(const wl_client *client) const
{
    return client
            && std::any_of(m_captureConnections.begin(), m_captureConnections.end(),
                           [client](const std::unique_ptr<TrustedConnection> &connection) {
                               return connection->client == client;
                           });
}

} // namespace Lingmo::Wayland
