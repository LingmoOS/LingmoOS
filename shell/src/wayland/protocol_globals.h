// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PROTOCOL_GLOBALS_H_
#define LINGMO_WAYLAND_PROTOCOL_GLOBALS_H_

#include <memory>
#include <string>

#include "base/signal.h"

#include "base/signal.h"

namespace Lingmo {

namespace Compositor {
class Compositor;
}

namespace ComputerControl {
class Controller;
}

namespace Input {
class InputMethodSettings;
class Seat;
class InputManager;
} // namespace input

namespace Scene {
class Node;
}

namespace WM {
class ShadowController;
class WindowManager;
}

namespace Wayland {

class DataControlManager;
class ForeignToplevelList;
class IdleInhibition;
class ImageCaptureManager;
class InputMethodLauncher;
class InputMethodManager;
class PrivilegedClients;
class SecurityDelegate;
class Server;
class WaylandComputerControl;
class WaylandWindowPlacement;
class WaylandLingmoShell;
class WaylandXdgActivation;
class WaylandXdgShell;
struct WaylandSeat;

// Every device-independent Wayland global this compositor registers, together
// with the state each global binds to.
//
// Upstream: chromium/src/components/exo/wayland/server.cc, Server::Initialize().
// Exo keeps this inside Server; here it is a separate type because
// lingmo-wayland is the base library the protocol implementations are built
// on, so the registration has to sit above them rather than inside it.
//
// Globals that depend on the graphics device - linux-dmabuf, drm-syncobj,
// presentation - are registered by whoever owns that device, not here.
class ProtocolGlobals
{
public:
    struct Dependencies {
        Server *server = nullptr;
        compositor::Compositor *compositor = nullptr;
        input::Seat *seat = nullptr;
        input::InputManager *inputManager = nullptr;
        wm::WindowManager *windowManager = nullptr;
        // Null leaves shadowed window captures at the window bounds.
        wm::ShadowController *shadowController = nullptr;
        // Candidate popups stack here, above every window but below the lock
        // screen.
        scene::Node *inputMethodPopupLayer = nullptr;
        SecurityDelegate *security = nullptr;
        // Null leaves computer control out.
        computer_control::Controller *computerControl = nullptr;
        // The greeter and development instances must not take over the user's
        // input method.
        bool launchInputMethod = false;
        // Development override; empty means use the saved selection.
        std::string inputMethodOverride;
    };

    explicit ProtocolGlobals(const Dependencies &dependencies);
    ProtocolGlobals(const ProtocolGlobals &) = delete;
    ProtocolGlobals &operator=(const ProtocolGlobals &) = delete;
    ~ProtocolGlobals();

    // Registers every global; call once the server is listening. False means the
    // compositor cannot serve clients, and the reason has already been logged.
    // The privileged registry filter is in place from construction, before the
    // socket exists.
    bool initialize();

    // Releases the globals. Must run after the clients are gone and before the
    // display is destroyed.
    void shutdown();

    // Spawns the input method once, without an X11 display. Later calls are
    // ignored.
    void startInputMethod();
    // Restarts a started input method with this X11 display: Xwayland started
    // on demand after it, or restarted after a crash.
    void setInputMethodX11Display(const std::string &x11Display,
                                  const std::string &x11Authority);

    // Keeps input-method popups on the text cursor when windows move.
    void sceneChanged();

    PrivilegedClients *privileged() { return m_privileged.get(); }

    IdleInhibition *idleInhibition() { return m_idleInhibition.get(); }

    ImageCaptureManager *imageCapture() { return m_imageCapture.get(); }

    ForeignToplevelList *toplevelList() { return m_toplevelList.get(); }

    InputMethodLauncher *inputMethodLauncher() { return m_inputMethodLauncher.get(); }

    InputMethodManager *inputMethod() { return m_inputMethod.get(); }

    WaylandXdgActivation *xdgActivation() { return m_xdgActivation.get(); }

private:
    bool registerCoreGlobals();
    bool registerInputMethod();
    bool registerClipboard();
    bool registerCapture();
    bool registerComputerControl();
    bool registerWindowPlacement();
    bool registerPrivateShell();

    const Dependencies m_deps;
    bool m_inputMethodStarted = false;

    // Declaration order is destruction order reversed, and it matters: this
    // mirrors the order these objects had as locals of main().
    std::unique_ptr<PrivilegedClients> m_privileged;
    std::unique_ptr<WaylandSeat> m_seat;
    std::unique_ptr<IdleInhibition> m_idleInhibition;
    std::unique_ptr<WaylandXdgActivation> m_xdgActivation;
    std::unique_ptr<WaylandXdgShell> m_xdgShell;
    std::unique_ptr<InputMethodManager> m_inputMethod;
    std::unique_ptr<InputMethodLauncher> m_inputMethodLauncher;
    std::unique_ptr<input::InputMethodSettings> m_inputMethodSettings;
    base::Subscription m_launcherRunning;
    base::Subscription m_inputMethodCommand;
    std::unique_ptr<DataControlManager> m_dataControl;
    std::unique_ptr<ImageCaptureManager> m_imageCapture;
    std::unique_ptr<ForeignToplevelList> m_toplevelList;
    std::unique_ptr<WaylandComputerControl> m_computerControl;
    std::unique_ptr<WaylandWindowPlacement> m_windowPlacement;
    std::unique_ptr<WaylandLingmoShell> m_lingmoShell;
};

} // namespace wayland
} // namespace Lingmo

#endif // LINGMO_WAYLAND_PROTOCOL_GLOBALS_H_
