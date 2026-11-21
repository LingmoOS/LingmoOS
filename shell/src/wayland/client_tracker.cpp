// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/client_tracker.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "client_tracker.h"

namespace Lingmo::Wayland {

ClientTracker::ClientTracker(wl_display* display)
    : m_display(display)
    , m_clientCreatedListener(this, &ClientTracker::onClientCreated)
{
    wl_display_add_client_created_listener(display, &m_clientCreatedListener.listener);
}

ClientTracker::~ClientTracker()
{
    while (!m_clientDestroyedListeners.empty()) {
        auto it = m_clientDestroyedListeners.begin();
        wl_list_remove(&it->second->listener.link);
        m_clientDestroyedListeners.erase(it);
    }
    // Remove the listener from the display's client-created signal.
    wl_list_remove(&m_clientCreatedListener.listener.link);
}

bool ClientTracker::isClientDestroyed(wl_client* client) const
{
    return m_clientDestroyedListeners.find(client) == m_clientDestroyedListeners.end();
}

int ClientTracker::trackedClientCount() const
{
    return static_cast<int>(m_clientDestroyedListeners.size());
}

ClientTracker::ClientListener::ClientListener(ClientTracker* tracker, wl_notify_func_t notify)
    : tracker(tracker)
{
    listener.notify = notify;
}

void ClientTracker::onClientCreated(wl_listener* listener, void* data)
{
    ClientListener* clientCreatedListener =
        wl_container_of(listener, clientCreatedListener, listener);
    clientCreatedListener->tracker->handleClientCreated(static_cast<wl_client*>(data));
}

void ClientTracker::onClientDestroyed(wl_listener* listener, void* data)
{
    ClientListener* clientDestroyedListener =
        wl_container_of(listener, clientDestroyedListener, listener);
    clientDestroyedListener->tracker->handleClientDestroyed(static_cast<wl_client*>(data));
}

void ClientTracker::handleClientCreated(wl_client* client)
{
    // Set up the destruction listener for the newly created client.
    auto [it, inserted] = m_clientDestroyedListeners.emplace(
        client, std::make_unique<ClientListener>(this, &ClientTracker::onClientDestroyed));
    wl_client_add_destroy_listener(client, &it->second->listener);
}

void ClientTracker::handleClientDestroyed(wl_client* client)
{
    auto it = m_clientDestroyedListeners.find(client);
    if (it == m_clientDestroyedListeners.end())
        return;

    // Remove the listener from the client's destroy signal.
    wl_list_remove(&it->second->listener.link);
    m_clientDestroyedListeners.erase(it);
}

} // namespace Lingmo::Wayland
