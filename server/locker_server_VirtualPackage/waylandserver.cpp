/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "waylandserver.h"
// Qt
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
// ksld
#include "kscreenlocker_logging.h"
#include <config-kscreenlocker.h>
// Wayland
#include <wayland-ksld-server-protocol.h>
// system
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace ScreenLocker
{

WaylandServer::WaylandServer(QObject *parent)
    : QObject(parent)
{
    m_listener.server = this;
    m_listener.listener.notify = [](wl_listener *listener, void *data) {
        Q_UNUSED(data)

        WaylandServer *server = reinterpret_cast<Listener *>(listener)->server;
        server->m_greeter = nullptr;
    };
}

WaylandServer::~WaylandServer()
{
    stop();
}

int WaylandServer::start()
{
    stop();

    m_display = wl_display_create();

    wl_event_loop *eventLoop = wl_display_get_event_loop(m_display);
    const int fd = wl_event_loop_get_fd(eventLoop);
    if (fd == -1) {
        stop();
        return -1;
    }

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read);
    connect(m_notifier, &QSocketNotifier::activated, this, &WaylandServer::dispatchEvents);

    QAbstractEventDispatcher *eventDispatcher = QCoreApplication::eventDispatcher();
    connect(eventDispatcher, &QAbstractEventDispatcher::aboutToBlock, this, &WaylandServer::flush);

    int socketPair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair) == -1) {
        // failed creating socket
        stop();
        return -1;
    }
    fcntl(socketPair[0], F_SETFD, FD_CLOEXEC);

    m_greeter = wl_client_create(m_display, socketPair[0]);
    if (!m_greeter) {
        // failed creating the Wayland client
        stop();
        close(socketPair[0]);
        close(socketPair[1]);
        return -1;
    }
    wl_client_add_destroy_listener(m_greeter, &m_listener.listener);

    m_interface = wl_global_create(m_display, &org_kde_ksld_interface, 3, this, bind);
    return socketPair[1];
}

void WaylandServer::stop()
{
    QAbstractEventDispatcher *eventDispatcher = QCoreApplication::eventDispatcher();
    disconnect(eventDispatcher, &QAbstractEventDispatcher::aboutToBlock, this, &WaylandServer::flush);

    delete m_notifier;
    m_notifier = nullptr;

    if (m_interface) {
        wl_global_destroy(m_interface);
        m_interface = nullptr;
    }
    if (m_greeter) {
        wl_client_destroy(m_greeter);
        m_greeter = nullptr;
    }
    if (m_display) {
        wl_display_destroy(m_display);
        m_display = nullptr;
    }
}

void WaylandServer::flush()
{
    wl_display_flush_clients(m_display);
}

void WaylandServer::dispatchEvents()
{
    if (wl_event_loop_dispatch(wl_display_get_event_loop(m_display), 0) != 0) {
        qCWarning(KSCREENLOCKER) << "Error on dispatching WaylandServer event loop";
    }
}

void WaylandServer::bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto server = reinterpret_cast<WaylandServer *>(data);
    if (client != server->m_greeter) {
        // a proper error would be better
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource *resource = wl_resource_create(server->m_greeter, &org_kde_ksld_interface, qMin(version, 1u), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    static const struct org_kde_ksld_interface s_interface = {
        .x11window =
            [](wl_client *client, wl_resource *resource, uint32_t id) {
                auto s = reinterpret_cast<WaylandServer *>(wl_resource_get_user_data(resource));
                if (s->m_greeter == client) {
                    Q_EMIT s->x11WindowAdded(id);
                }
            },
    };

    wl_resource_set_implementation(resource, &s_interface, server, nullptr);
}

}
