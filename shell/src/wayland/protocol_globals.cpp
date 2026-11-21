// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocol_globals.h"


#include "compositor/compositor.h"
#include "computer_control/controller.h"
#include "core/logging.h"
#include "core/process_environment.h"
#include "display/display_manager.h"
#include "input/input_method_settings.h"
#include "wayland/idle_inhibition.h"
#include "wayland/input_method_launcher.h"
#include "wayland/privileged_client.h"
#include "wayland/protocols/application_menu.h"
#include "wayland/protocols/gtk_shell.h"
#include "wayland/protocols/cursor_shape.h"
#include "wayland/protocols/lingmo_computer_control.h"
#include "wayland/protocols/lingmo_shell.h"
#include "wayland/protocols/data_control.h"
#include "wayland/protocols/idle_notify.h"
#include "wayland/protocols/foreign_toplevel_list.h"
#include "wayland/protocols/image_capture.h"
#include "wayland/protocols/input_method.h"
#include "wayland/protocols/primary_selection.h"
#include "wayland/protocols/status_item.h"
#include "wayland/protocols/surface_blur.h"
#include "wayland/protocols/window_hints.h"
#include "wayland/protocols/window_placement.h"
#include "wayland/protocols/wl_compositor.h"
#include "wayland/protocols/wl_data_device_manager.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/wp_fractional_scale.h"
#include "wayland/protocols/wp_viewporter.h"
#include "wayland/protocols/xdg_activation.h"
#include "wm/shadow_controller.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_shadow.h"
#include "wayland/protocols/xdg_decoration.h"
#include "wayland/protocols/xdg_dialog.h"
#include "wayland/protocols/xdg_foreign.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/protocols/xdg_toplevel_drag.h"
#include "wayland/protocols/zwp_idle_inhibit_manager.h"
#include "wayland/protocols/zwp_keyboard_shortcuts_inhibit_manager.h"
#include "wayland/protocols/zwp_pointer_constraints.h"
#include "wayland/protocols/zwp_pointer_gestures.h"
#include "wayland/protocols/zwp_relative_pointer_manager.h"
#include "wayland/security_delegate.h"
#include "wayland/server.h"

namespace Lingmo::Wayland {

ProtocolGlobals::ProtocolGlobals(const Dependencies &dependencies)
    : m_deps(dependencies)
    // Built before the socket exists, so no client can ever reach a registry
    // that has not been filtered.
    , m_privileged(std::make_unique<PrivilegedClients>(dependencies.server->display()))
{
}

ProtocolGlobals::~ProtocolGlobals()
{
    // The launcher outlives the settings it reports to.
    m_launcherRunning.reset();
}

bool ProtocolGlobals::initialize()
{
    return registerCoreGlobals() && registerInputMethod() && registerClipboard()
            && registerCapture() && registerComputerControl() && registerWindowPlacement()
            && registerPrivateShell();
}

bool ProtocolGlobals::registerCoreGlobals()
{
    wl_display *display = m_deps.server->display();
    m_idleInhibition = std::make_unique<IdleInhibition>();
    m_idleInhibition->input = m_deps.inputManager;

    m_seat = std::make_unique<WaylandSeat>(m_deps.seat, m_deps.server->serialTracker());
    m_xdgActivation = std::make_unique<WaylandXdgActivation>(m_deps.windowManager, m_seat.get());
    m_xdgShell = std::make_unique<WaylandXdgShell>(m_deps.compositor, m_deps.windowManager,
                                                   m_deps.seat, m_deps.server->serialTracker());

    if (!createCompositorGlobal(display, m_deps.compositor) || !createSubcompositorGlobal(display)
        || !createShmGlobal(display, m_deps.compositor)
        || !createIdleNotifierGlobal(display, m_idleInhibition.get())
        || !createIdleInhibitManagerGlobal(display, m_idleInhibition.get())
        || !createSeatGlobal(display, m_seat.get()) || !createCursorShapeGlobal(display)
        || !createPointerConstraintsGlobal(display, m_seat.get())
        || !createRelativePointerManagerGlobal(display)
        || !createPointerGesturesGlobal(display, m_seat.get())
        || !createKeyboardShortcutsInhibitManagerGlobal(display, m_seat.get())
        || !createPrimarySelectionGlobal(display, m_seat.get())
        || !createDataDeviceManagerGlobal(display, m_seat.get())
        || !createXdgActivationGlobal(display, m_xdgActivation.get())
        || !createXdgShellGlobal(display, m_xdgShell.get()) || !createXdgDecorationGlobal(display)
        || !createXdgDialogGlobal(display) || !createXdgExporterV2Global(display)
        || (m_deps.windowManager
            && !createXdgToplevelDragGlobal(display, m_deps.windowManager))
        || !createXdgImporterV2Global(display) || !createSurfaceBlurGlobal(display)
        || !createWindowHintsGlobal(display, m_deps.windowManager) || !createAppMenuGlobal(display)
        || !createGtkShellGlobal(display) || !createFractionalScaleGlobal(display)
        || !createXdgOutputManagerGlobal(display) || !createViewporterGlobal(display)
        || (m_deps.windowManager
            && !createStatusItemGlobal(display, m_deps.windowManager->statusItems()))) {
        LM_CRITICAL(lingmoWayland) << "Failed to register the globals";
        return false;
    }
    return true;
}

bool ProtocolGlobals::registerInputMethod()
{
    wl_display *display = m_deps.server->display();

    m_inputMethod = std::make_unique<InputMethodManager>(display, m_seat.get(),
                                                         m_deps.inputMethodPopupLayer);
    if (!m_inputMethod->textInputGlobal() || !m_inputMethod->textInputV1Global()
        || !m_inputMethod->inputMethodGlobal() || !m_inputMethod->virtualKeyboardGlobal())
        return false;

    m_inputMethodLauncher = std::make_unique<InputMethodLauncher>(display);
    // Identity, not policy: the only client allowed to drive the input method is
    // the process the compositor spawned for it.
    const auto authorize = [launcher = m_inputMethodLauncher.get()](const wl_client *client) {
        return launcher->isInputMethod(client);
    };
    m_inputMethod->setAuthorizer(authorize);
    if (wm::WindowManager *windows = m_deps.windowManager) {
        m_inputMethod->setPlacementArea([windows](const base::Rect &caret) {
            const auto *displays = windows->displayManager();
            const auto *display = displays ? displays->displayForRect(caret) : nullptr;
            return display ? windows->workAreaForOutput(display->bounds)
                           : windows->workAreaFor(nullptr);
        });
    }
    m_privileged->addRestrictedGlobal(m_inputMethod->inputMethodGlobal(), authorize);
    m_privileged->addRestrictedGlobal(m_inputMethod->virtualKeyboardGlobal(), authorize);

    m_inputMethodSettings = std::make_unique<input::InputMethodSettings>();
    m_launcherRunning = m_inputMethodLauncher->runningChanged.connect([this] {
        m_inputMethodSettings->setRunning(m_inputMethodLauncher->isRunning());
    });
    return true;
}

bool ProtocolGlobals::registerClipboard()
{
    // The shell puts screenshots on the clipboard without holding the keyboard.
    const auto authorize = [security = m_deps.security,
                            privileged = m_privileged.get()](const wl_client *client) {
        return privileged->isTrusted(client) || security->canManageClipboard(client);
    };
    m_dataControl =
            std::make_unique<DataControlManager>(m_deps.server->display(), m_seat.get(), authorize);
    if (!m_dataControl->extGlobal() || !m_dataControl->wlrGlobal())
        return false;
    m_privileged->addRestrictedGlobal(m_dataControl->extGlobal(), authorize);
    m_privileged->addRestrictedGlobal(m_dataControl->wlrGlobal(), authorize);
    return true;
}

bool ProtocolGlobals::registerCapture()
{
    // The shell takes screenshots itself; recordings run in a helper it starts on
    // a capture connection.
    // The computer-use service reads the screen only while the user lets it
    // control the computer, but binds the output managers beforehand: it cannot
    // bind once it has dropped its /proc identity. It never sees window lists.
    const auto authorize = [security = m_deps.security, privileged = m_privileged.get(),
                            control = m_deps.computerControl](const wl_client *client) {
        return privileged->isTrusted(client) || privileged->isCaptureConnection(client)
                || security->canCaptureOutputs(client) || (control && control->allowsCapture(client));
    };
    const auto visible = [security = m_deps.security, authorize](const wl_client *client) {
        return authorize(client) || security->canControlComputer(client);
    };
    m_imageCapture = std::make_unique<ImageCaptureManager>(
            m_deps.server->display(), authorize,
            [compositor = m_deps.compositor] {
                compositor->scheduleRepaint();
            },
            visible);
    if (!m_imageCapture->sourceGlobal() || !m_imageCapture->toplevelSourceGlobal()
        || !m_imageCapture->copyGlobal())
        return false;
    m_privileged->addRestrictedGlobal(m_imageCapture->sourceGlobal(), visible);
    m_privileged->addRestrictedGlobal(m_imageCapture->toplevelSourceGlobal(), authorize);
    m_privileged->addRestrictedGlobal(m_imageCapture->copyGlobal(), visible);
    // Window sources need the toplevel list, which the same clients may bind.
    m_toplevelList = std::make_unique<ForeignToplevelList>(
            m_deps.server->display(), m_deps.windowManager, m_deps.compositor, authorize);
    if (!m_toplevelList->global())
        return false;
    m_privileged->addRestrictedGlobal(m_toplevelList->global(), authorize);
    m_imageCapture->setToplevelSourceResolver([list = m_toplevelList.get()](wl_resource *handle) {
        return list->sourceFor(handle);
    });
    // The margins are fixed per shadow role, so a shadow not yet moved to the
    // window's new bounds still gives the right ones.
    // What the shadow paints, placed from the current bounds: the shadow
    // follows a move through an observer of its own, in no fixed order with
    // the list's. A dock's shadow hugs its rounded body, so the room of the
    // surface around that body stays out of the picture.
    m_toplevelList->setShadowBounds([shadows = m_deps.shadowController](const wm::Window *window) {
        const base::Rect bounds = window->bounds();
        const wm::WindowShadow *shadow = shadows ? shadows->shadowForWindow(window) : nullptr;
        if (!shadow || shadow->shadowBounds().isEmpty() || shadow->contentBounds().isEmpty())
            return bounds;
        const base::Rect content = shadow->contentBounds();
        return shadow->shadowBounds().translated(bounds.topLeft() - content.topLeft());
    });
    return true;
}

bool ProtocolGlobals::registerComputerControl()
{
    if (!m_deps.computerControl)
        return true;
    const auto authorize = [security = m_deps.security](const wl_client *client) {
        return security->canControlComputer(client);
    };
    m_computerControl = std::make_unique<WaylandComputerControl>(
            m_deps.server->display(), m_deps.computerControl, authorize);
    if (!m_computerControl->global())
        return false;
    m_privileged->addRestrictedGlobal(m_computerControl->global(), authorize);
    return true;
}

bool ProtocolGlobals::registerWindowPlacement()
{
    if (!m_deps.windowManager)
        return true;
    const auto authorize = [security = m_deps.security](const wl_client *client) {
        return security->canPlaceWindows(client);
    };
    m_windowPlacement = std::make_unique<WaylandWindowPlacement>(
            m_deps.server->display(), m_deps.windowManager, authorize);
    if (!m_windowPlacement->global())
        return false;
    m_privileged->addRestrictedGlobal(m_windowPlacement->global(), authorize);
    return true;
}

bool ProtocolGlobals::registerPrivateShell()
{
    // Registered with the privileged filter, so an ordinary client never sees
    // it in its registry.
    m_lingmoShell =
            std::make_unique<WaylandLingmoShell>(m_deps.windowManager, m_privileged.get());
    if (!createLingmoShellGlobal(m_deps.server->display(), m_lingmoShell.get())) {
        LM_CRITICAL(lingmoWayland) << "Failed to register the private shell global";
        return false;
    }
    return true;
}

static std::vector<std::string> inputMethodEnvironment(const std::string &waylandDisplay,
                                                       const std::string &x11Display,
                                                       const std::string &x11Authority)
{
    auto environment = childEnvironment();
    std::erase_if(environment, [](const std::string &entry) {
        return entry.starts_with("WAYLAND_DISPLAY=") || entry.starts_with("DISPLAY=")
                || entry.starts_with("XAUTHORITY=");
    });
    environment.push_back("WAYLAND_DISPLAY=" + waylandDisplay);
    if (!x11Display.empty()) {
        environment.push_back("DISPLAY=" + x11Display);
        environment.push_back("XAUTHORITY=" + x11Authority);
    }
    return environment;
}

void ProtocolGlobals::setInputMethodX11Display(const std::string &x11Display,
                                               const std::string &x11Authority)
{
    if (!m_deps.launchInputMethod || !m_inputMethodStarted)
        return;
    // An input method connects to X11 when it starts, and only then.
    m_inputMethodLauncher->setEnvironment(
            inputMethodEnvironment(m_deps.server->socketName(), x11Display, x11Authority));
    m_inputMethodLauncher->restart();
}

void ProtocolGlobals::startInputMethod()
{
    if (!m_deps.launchInputMethod || m_inputMethodStarted)
        return;
    m_inputMethodStarted = true;

    m_inputMethodLauncher->setEnvironment(
            inputMethodEnvironment(m_deps.server->socketName(), {}, {}));
    if (!m_deps.inputMethodOverride.empty())
        m_inputMethodSettings->overrideInputMethod(m_deps.inputMethodOverride);
    else
        m_inputMethodSettings->registerService();
    const auto applyCommand = [this] {
        m_inputMethodLauncher->setCommand(m_inputMethodSettings->command());
    };
    m_inputMethodCommand = m_inputMethodSettings->commandChanged.connect(applyCommand);
    applyCommand();
}

void ProtocolGlobals::sceneChanged()
{
    if (m_idleInhibition)
        m_idleInhibition->sceneChanged();
    if (m_inputMethod)
        m_inputMethod->sceneChanged();
}

void ProtocolGlobals::shutdown()
{
    if (m_privileged && m_dataControl) {
        m_privileged->removeRestrictedGlobal(m_dataControl->extGlobal());
        m_privileged->removeRestrictedGlobal(m_dataControl->wlrGlobal());
    }
    if (m_privileged && m_inputMethod) {
        m_privileged->removeRestrictedGlobal(m_inputMethod->inputMethodGlobal());
        m_privileged->removeRestrictedGlobal(m_inputMethod->virtualKeyboardGlobal());
    }
    if (m_dataControl)
        m_dataControl->shutdown();
    if (m_inputMethod)
        m_inputMethod->shutdown();
    if (m_privileged && m_imageCapture) {
        m_privileged->removeRestrictedGlobal(m_imageCapture->sourceGlobal());
        m_privileged->removeRestrictedGlobal(m_imageCapture->toplevelSourceGlobal());
        m_privileged->removeRestrictedGlobal(m_imageCapture->copyGlobal());
    }
    if (m_privileged && m_toplevelList)
        m_privileged->removeRestrictedGlobal(m_toplevelList->global());
    if (m_privileged && m_computerControl)
        m_privileged->removeRestrictedGlobal(m_computerControl->global());
    m_computerControl.reset();
    if (m_privileged && m_windowPlacement)
        m_privileged->removeRestrictedGlobal(m_windowPlacement->global());
    m_windowPlacement.reset();
    if (m_imageCapture)
        m_imageCapture->shutdown();
    if (m_toplevelList)
        m_toplevelList->shutdown();
    if (m_privileged)
        m_privileged->shutdown();
}

} // namespace Lingmo::Wayland
