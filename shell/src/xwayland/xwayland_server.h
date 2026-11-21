// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>
#include <wayland-server-core.h>

#include "base/timer.h"
#include "surface_association.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::Wayland {
class PrivilegedClients;
}

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::XWayland {

class XwaylandKeyboardGrab;
class XwaylandShell;
class Xwm;

class XwaylandServer
{
public:
    struct Options {
        std::string executable = "/usr/bin/Xwayland";
        std::string readinessFile;
        bool software = false;
        int timeoutMs = 10000;
        // Restarts after a crash allowed within one minute; beyond that
        // Xwayland is given up for the session.
        int restartLimit = 3;
        // How long an X11 client has to answer _NET_WM_PING; xdg_wm_base's
        // timeout by default.
        int pingTimeoutMs = 5000;
        // Top-level X11 windows tracked at once; the client holding the most
        // is disconnected beyond that.
        size_t windowLimit = SurfaceAssociation::limit;
        // Publish the display at once, but start Xwayland only when the first
        // X11 client connects to it.
        bool lazy = false;
    };

    XwaylandServer(wl_display*, wayland::PrivilegedClients*, wm::WindowManager*);
    XwaylandServer(const XwaylandServer&) = delete;
    XwaylandServer& operator=(const XwaylandServer&) = delete;
    ~XwaylandServer();
    bool start(const Options& options);
    void stop();

    // The output scale Xwayland renders at. X11 clients get device-resolution
    // coordinates and Xft.dpi, so they draw sharp instead of being upscaled.
    void setScale(double scale);

    // Xwayland is running and managed. A lazy server has published its display
    // before this.
    bool isReady() const { return m_ready; }

    std::string display() const { return m_displayName; }

    std::string authority() const { return m_authority; }

    std::string error() const { return m_error; }

    // Zero while Xwayland is not running.
    pid_t processId() const { return m_pid > 0 ? m_pid : 0; }

    // The display is published to the session. For a lazy server that is
    // before Xwayland runs; started follows once it does.
    std::function<void()> ready;
    std::function<void()> started;
    std::function<void(const std::string&)> failure;

private:
    bool launch();
    bool openSockets();
    void closeSockets();
    void arm();
    bool writeReadiness();
    void readDisplay();
    void fail(const std::string& error);
    // Once the current callback is done, as one crash reports itself through
    // several channels: the first report of an epoch counts.
    void deferFailure(const std::string& error);
    // Starts Xwayland with the descriptors it inherits; false when it cannot.
    bool spawn(const std::vector<std::string>& arguments, const std::vector<int>& inherited,
               int waylandSocket);
    void reap();
    void stopProcess();
    void closeListenWatches();
    void publish();
    // Ends the running server. `keepSession` keeps what the session was given
    // - display, Xauthority and readiness file - for a restart to reuse.
    void teardown(bool keepSession);
    bool mayRestart();
    void applyScale();
    wl_display* m_display;
    wayland::PrivilegedClients* m_privileged;
    wm::WindowManager* m_manager;
    wl_client* m_client = nullptr;

    struct Listener {
        wl_listener listener {};
        XwaylandServer* owner;
    } m_destroy;

    base::EventLoop& m_loop;
    pid_t m_pid = -1;
    int m_pidfd = -1;
    int m_pidWatch = 0;
    base::Timer m_timeout;
    base::Timer m_deferredFailure;
    std::string m_deferredError;
    Options m_options;
    std::deque<int64_t> m_restarts;
    // A lazy server's listening sockets and the lock file that reserves them.
    std::vector<int> m_listenFds;
    std::vector<int> m_listenWatches;
    std::string m_lockFile, m_socketFile;
    double m_scale = 1;
    // The private runtime directory, removed with what is in it.
    std::string m_runtime;
    std::unique_ptr<SurfaceAssociation> m_association;
    std::unique_ptr<XwaylandShell> m_shell;
    std::unique_ptr<XwaylandKeyboardGrab> m_keyboardGrab;
    std::unique_ptr<Xwm> m_xwm;
    int m_readWatch = 0;
    int m_readFd = -1;
    std::string m_readiness;
    std::string m_displayName, m_authority, m_error, m_readinessFile;
    bool m_ready = false, m_stopping = false, m_xwmReady = false, m_published = false;
};

} // namespace Lingmo::XWayland
