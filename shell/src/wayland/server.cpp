// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/server.cc
//           chromium/src/components/exo/wayland/wayland_watcher.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "server.h"

#include <sys/epoll.h>

#include <cstdarg>
#include <cstdio>
#include <map>

#include "base/event_loop.h"
#include "core/logging.h"

namespace Lingmo::Wayland {

namespace {

std::map<wl_display*, Server*>& displayServerMap()
{
    static std::map<wl_display*, Server*> map;
    return map;
}

void waylandLog(const char* fmt, va_list argp)
{
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), fmt, argp);
    LM_WARNING(lingmoWayland).printf("libwayland: %s", buffer);
}

} // namespace

// The display's descriptor and the flush on the thread's loop.
struct Server::LoopHook {
    ~LoopHook()
    {
        loop->removePrepareHook(flush);
        loop->unwatch(watch);
    }

    base::EventLoop* loop = nullptr;
    int watch = 0;
    int flush = 0;
};

Server::Server()
{
    wl_log_set_handler_server(waylandLog);

    m_display.reset(wl_display_create());
    if (!m_display) {
        LM_CRITICAL(lingmoWayland) << "Failed to create the Wayland display";
        return;
    }

    // A client that stops reading while it works through a burst of input
    // (GTK does) otherwise overflows the default 4 KiB and is disconnected.
    wl_display_set_default_max_buffer_size(m_display.get(), 1024 * 1024);

    displayServerMap().emplace(m_display.get(), this);
    m_serialTracker = std::make_unique<SerialTracker>(m_display.get());
    m_clientTracker = std::make_unique<ClientTracker>(m_display.get());
}

Server::~Server()
{
    stop();

    if (m_display) {
        displayServerMap().erase(m_display.get());
        m_serialTracker->shutdown();
    }
}

Server* Server::serverForDisplay(wl_display* display)
{
    auto& map = displayServerMap();
    auto it = map.find(display);
    return it == map.end() ? nullptr : it->second;
}

bool Server::addSocket(const std::string& name)
{
    if (!m_display)
        return false;

    if (name.empty()) {
        const char* socket = wl_display_add_socket_auto(m_display.get());
        if (!socket) {
            LM_CRITICAL(lingmoWayland) << "Failed to create a Wayland socket";
            return false;
        }
        m_socketName = socket;
    } else {
        if (wl_display_add_socket(m_display.get(), name.c_str()) != 0) {
            LM_CRITICAL(lingmoWayland) << "Failed to create the Wayland socket" << name.c_str();
            return false;
        }
        m_socketName = name;
    }

    LM_INFO(lingmoWayland) << "Listening on Wayland socket" << m_socketName.c_str();
    return true;
}

bool Server::start()
{
    if (m_loopHook)
        return true;

    const int fd = fileDescriptor();
    if (fd < 0)
        return false;

    base::EventLoop* loop = base::EventLoop::current();
    if (!loop)
        return false;

    auto hook = std::make_unique<LoopHook>();
    hook->loop = loop;
    hook->watch = loop->watch(fd, EPOLLIN, [this](uint32_t) { onSocketActivated(); });
    if (!hook->watch)
        return false;
    // Nothing wakes the event loop when the server itself queues events, so
    // flush whatever is pending just before the loop waits.
    hook->flush = loop->addPrepareHook([this] { flush(); });
    m_loopHook = std::move(hook);
    return true;
}

void Server::stop()
{
    if (!m_loopHook)
        return;

    m_loopHook.reset();
}

int Server::fileDescriptor() const
{
    if (!m_display)
        return -1;
    wl_event_loop* eventLoop = wl_display_get_event_loop(m_display.get());
    return eventLoop ? wl_event_loop_get_fd(eventLoop) : -1;
}

bool Server::dispatch(std::chrono::milliseconds timeout)
{
    if (!m_display)
        return false;

    wl_event_loop* eventLoop = wl_display_get_event_loop(m_display.get());
    if (wl_event_loop_dispatch(eventLoop, static_cast<int>(timeout.count())) < 0) {
        LM_WARNING(lingmoWayland) << "Wayland event loop dispatch failed";
        return false;
    }
    return true;
}

void Server::flush()
{
    if (!m_display)
        return;

    // wl_display_flush_clients() is avoided for the same reason as upstream
    // Exo: it can crash while a client is disconnecting.
    wl_client* client = nullptr;
    wl_list* allClients = wl_display_get_client_list(m_display.get());
    wl_client_for_each(client, allClients)
    {
        if (!isClientDestroyed(client))
            wl_client_flush(client);
    }
}

bool Server::isClientDestroyed(wl_client* client) const
{
    return m_clientTracker ? m_clientTracker->isClientDestroyed(client) : true;
}

void Server::onSocketActivated()
{
    dispatch(std::chrono::milliseconds(0));
    flush();
}

} // namespace Lingmo::Wayland
