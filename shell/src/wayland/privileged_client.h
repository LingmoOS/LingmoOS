// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PRIVILEGED_CLIENT_H_
#define LINGMO_WAYLAND_PRIVILEGED_CLIENT_H_

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <wayland-server-core.h>

namespace Lingmo::Wayland {

// The privilege boundary for the private shell protocols.
//
// Exactly one wl_client at a time is trusted: the file manager's desktop, called
// the shell here. It is not recognised by uid, by pid or by anything else it
// could claim about itself: the session hands the compositor one end of a
// socketpair it created, the compositor turns that fd into a wl_client with
// wl_client_create(), and *that* client object is the trusted one. Nothing that
// connects to the public socket can become it.
//
// A global registered here is invisible to every other client - the filter runs
// before wl_registry.global is sent - and its bind handler must check
// isTrusted() as well, so a stale or forged bind cannot slip through.
class PrivilegedClients
{
public:
    explicit PrivilegedClients(wl_display *display);
    PrivilegedClients(const PrivilegedClients &) = delete;
    PrivilegedClients &operator=(const PrivilegedClients &) = delete;
    ~PrivilegedClients();
    void shutdown();

    // Takes ownership of `fd` and makes the resulting connection the trusted
    // shell, replacing any previous one. Returns null and closes `fd` on
    // failure.
    wl_client *adoptShellConnection(int fd);

    wl_client *shellClient() const { return m_shell.client; }

    bool isTrusted(const wl_client *client) const { return client && client == m_shell.client; }

    // A connection for a capture helper the compositor starts, such as the
    // screen recorder: the client on one end of a new socketpair may capture,
    // as long as it lasts, whatever executable runs on the other. Returns that
    // other end for the caller to hand to the helper and close, or -1.
    int createCaptureConnection();
    bool isCaptureConnection(const wl_client *client) const;

    // Globals only the trusted shell may see or bind.
    void addPrivateGlobal(wl_global *global);
    void addRestrictedGlobal(wl_global *global, std::function<bool(const wl_client *)> authorize);
    void removeRestrictedGlobal(wl_global *global);

private:
    struct TrustedConnection;

    // Standard layout, so libwayland's wl_listener* can be cast back to it.
    struct DestroyListener {
        wl_listener listener;
        TrustedConnection *connection;
    };

    // One adopted connection: the client and its destroy listener.
    struct TrustedConnection {
        wl_client *client = nullptr;
        const char *label = "";
        DestroyListener destroy {};
        bool listening = false;
    };

    static bool globalFilter(const wl_client *client, const wl_global *global, void *data);
    static void onConnectionDestroyed(wl_listener *listener, void *data);

    wl_client *adopt(TrustedConnection &connection, int fd);
    static void forget(TrustedConnection &connection);

    wl_display *m_display = nullptr;
    TrustedConnection m_shell {nullptr, "shell", {}, false};
    std::vector<std::unique_ptr<TrustedConnection>> m_captureConnections;
    std::vector<wl_global *> m_privateGlobals;
    std::map<const wl_global *, std::function<bool(const wl_client *)>> m_restrictedGlobals;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PRIVILEGED_CLIENT_H_
