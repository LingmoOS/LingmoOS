// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/client_tracker.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_CLIENT_TRACKER_H_
#define LINGMO_WAYLAND_CLIENT_TRACKER_H_

#include <wayland-server-core.h>

#include <memory>
#include <unordered_map>

namespace Lingmo::Wayland {

// Utility class to keep track of the lifetime of wl_client. This is necessary
// as resources owned by wl_client are freed in the process of destruction, and
// accessing client resources should not happen after this time (see
// crbug.com/1433187). All resources and associated user data are destroyed and
// freed before the wl_client is itself deleted.
class ClientTracker
{
public:
    explicit ClientTracker(wl_display* display);
    ClientTracker(const ClientTracker&) = delete;
    ClientTracker& operator=(const ClientTracker&) = delete;
    ~ClientTracker();

    // Returns true if `client` has begun destruction. Client resources should
    // not be queried after this point.
    bool isClientDestroyed(wl_client* client) const;

    int trackedClientCount() const;

private:
    // Listener called when a wl_client has been successfully created for
    // `m_display`, and immediately when a client begins destruction (before
    // associated resources have been freed).
    struct ClientListener {
        ClientListener(ClientTracker* tracker, wl_notify_func_t notify);
        wl_listener listener;
        ClientTracker* tracker;
    };

    static void onClientCreated(wl_listener* listener, void* data);
    static void onClientDestroyed(wl_listener* listener, void* data);

    void handleClientCreated(wl_client* client);
    void handleClientDestroyed(wl_client* client);

    wl_display* m_display;

    ClientListener m_clientCreatedListener;

    // Presence in `m_clientDestroyedListeners` indicates the client has been
    // created and has yet to be destroyed.
    std::unordered_map<wl_client*, std::unique_ptr<ClientListener>> m_clientDestroyedListeners;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_CLIENT_TRACKER_H_
