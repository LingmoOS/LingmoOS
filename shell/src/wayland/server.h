// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/server.h
//           chromium/src/components/exo/wayland/wayland_watcher.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Trimmed for Lingmo: no globals are registered yet, and the fd is watched by
// the compositor's own main loop instead of base::MessagePumpForUI.

#ifndef LINGMO_WAYLAND_SERVER_H_
#define LINGMO_WAYLAND_SERVER_H_

#include <chrono>
#include <memory>
#include <map>
#include <set>
#include <string>

#include "client_tracker.h"
#include "scoped_wl.h"
#include "serial_tracker.h"

namespace Lingmo::Wayland {

// A thin wrapper around a libwayland display server, driven by the main
// thread's event loop.
class Server
{
public:
    Server();
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    ~Server();

    // Returns the Server instance owning `display`, or nullptr.
    static Server* serverForDisplay(wl_display* display);

    // Creates the listening socket. With an empty `name` a free `wayland-N`
    // name is picked automatically. Must be called before start().
    bool addSocket(const std::string& name = {});

    // Attaches the event loop fd to the thread's event loop. Clients are
    // flushed after every dispatch and before that loop blocks.
    bool start();

    // Detaches from the event loop; the display stays alive.
    void stop();

    // The socket name clients should use as WAYLAND_DISPLAY. Empty until
    // addSocket() succeeds.
    const std::string& socketName() const { return m_socketName; }

    wl_display* display() const { return m_display.get(); }

    // The event loop fd, or -1 if there is no display.
    int fileDescriptor() const;

    // Dispatches pending events, blocking for at most `timeout`. A zero
    // timeout polls. Returns false on a dispatch error.
    bool dispatch(std::chrono::milliseconds timeout);

    // Flushes queued events to every connected client.
    void flush();

    // Whether a client of this server has started destruction.
    bool isClientDestroyed(wl_client* client) const;

    // Compatibility frontends publish their surfaces only after role association.
    void setClientSurfacesInitiallyHidden(wl_client* client, bool hidden)
    {
        if (hidden)
            m_hiddenSurfaceClients.insert(client);
        else
            m_hiddenSurfaceClients.erase(client);
    }
    bool clientSurfacesInitiallyHidden(wl_client* client) const
    {
        return m_hiddenSurfaceClients.contains(client);
    }

    // The client's surface units per logical unit; see Surface::State::clientScale.
    // Surface coordinates the client sends are divided by it, and surface and
    // output coordinates it receives are multiplied by it.
    void setClientScale(wl_client* client, double scale)
    {
        if (scale == 1)
            m_clientScales.erase(client);
        else
            m_clientScales[client] = scale;
    }
    double clientScale(wl_client* client) const
    {
        const auto it = m_clientScales.find(client);
        return it == m_clientScales.end() ? 1 : it->second;
    }

    SerialTracker* serialTracker() { return m_serialTracker.get(); }
    int trackedClientCount() const { return m_clientTracker->trackedClientCount(); }

private:
    struct LoopHook;

    void onSocketActivated();

    // Declaration order is destruction order reversed, and both matter here:
    // ClientTracker must unregister its wl_listeners while the display is still
    // alive, while destroying the display tears down clients and so must still
    // see a live SerialTracker.
    std::unique_ptr<SerialTracker> m_serialTracker;
    ScopedWlDisplay m_display;
    std::unique_ptr<ClientTracker> m_clientTracker;

    std::set<wl_client*> m_hiddenSurfaceClients;
    std::map<wl_client*, double> m_clientScales;
    std::unique_ptr<LoopHook> m_loopHook;
    std::string m_socketName;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_SERVER_H_
