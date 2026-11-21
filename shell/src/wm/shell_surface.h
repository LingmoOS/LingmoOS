// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/shell_surface_base.{h,cc}
//           chromium/src/components/exo/shell_surface.{h,cc}
//           chromium/src/components/exo/xdg_shell_surface.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// What is taken from Exo: the configure serial queue and its ack semantics, the
// geometry / bounds / surface-origin math including the resize-from-left-or-top
// compensation, the min/max size to resizability rule, and the deferred
// mapping. What is dropped: views::Widget, aura::Window, ash::WindowState and
// every ChromeOS-only state.
//
// This class is deliberately free of Wayland types. The protocol layer hands it
// a configure callback that returns a serial, exactly as Exo's wayland/
// xdg_shell.cc does.

#ifndef LINGMO_WM_SHELL_SURFACE_H_
#define LINGMO_WM_SHELL_SURFACE_H_

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "compositor/surface_observer.h"
#include "wm/types.h"
#include "wm/window_observer.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::WM {

class Window;
class WindowManager;

class ShellSurface : public compositor::SurfaceObserver, public WindowObserver
{
public:
    enum class Role {
        None,
        Toplevel,
        Popup,
    };

    // xdg-decoration's two modes. Lingmo prefers ServerSide, but only for a
    // client that asked to negotiate at all: one that never creates a
    // decoration object is decorating itself and must be left alone.
    enum class DecorationMode {
        ClientSide,
        ServerSide,
    };

    static constexpr DecorationMode kPreferredDecorationMode = DecorationMode::ServerSide;

    // What a toplevel configure reports. `restoreStateType` is what a restore
    // would land in, which xdg-shell clients use to keep the maximized state
    // visible while fullscreen.
    struct ConfigureState {
        base::Size size;
        WindowStateType stateType = WindowStateType::Normal;
        WindowStateType restoreStateType = WindowStateType::Normal;
        bool resizing = false;
        bool activated = false;
        // xdg_toplevel.configure_bounds: the largest window the work area of
        // its output holds, in client-area terms. 0x0 when unknown.
        base::Size bounds {0, 0};
        // xdg_toplevel.wm_capabilities: maximize is only offered to a window
        // that can be resized; minimize and fullscreen always are.
        bool maximizable = true;
        // Minimized or hidden by the shell: nothing of the window is on screen.
        bool suspended = false;
    };

    // Asks the client to configure itself. Returns the serial the client must
    // acknowledge, or 0 when no configure was sent.
    using ConfigureCallback = std::function<uint32_t(const ConfigureState &state)>;
    // xdg_popup.configure, in coordinates relative to the parent's window
    // geometry, followed by xdg_surface.configure.
    using PopupConfigureCallback = std::function<uint32_t(const base::Rect &bounds)>;
    // xdg_toplevel.close / xdg_popup.popup_done.
    using CloseCallback = std::function<void()>;
    // xdg_wm_base.ping, and ending the client process; see Window.
    using PingCallback = std::function<void()>;
    using KillCallback = std::function<void()>;

    ShellSurface(WindowManager *manager, compositor::Surface *surface);
    ShellSurface(const ShellSurface &) = delete;
    ShellSurface &operator=(const ShellSurface &) = delete;
    ~ShellSurface() override;

    compositor::Surface *rootSurface() const { return m_surface; }

    WindowManager *manager() const { return m_manager; }

    // Null until the surface has a role, has acknowledged its initial configure
    // and has committed contents, including popup roles.
    Window *window() const { return m_window; }

    Role role() const { return m_role; }

    bool hasRole() const { return m_role != Role::None; }

    void setConfigureCallback(ConfigureCallback callback);
    void setPopupConfigureCallback(PopupConfigureCallback callback);
    void setCloseCallback(CloseCallback callback);
    void setPingCallback(PingCallback callback);
    void setKillCallback(KillCallback callback);

    // --- Roles ---

    // Returns false if a role has already been assigned.
    bool setToplevel();
    // `bounds` is the positioner result, relative to `parent`'s window
    // geometry.
    bool setPopup(ShellSurface *parent, const base::Rect &bounds);

    // Destroying an xdg_toplevel or xdg_popup unmaps the surface and gives up
    // the role. The xdg_surface stays, and may be given a new one.
    void clearRole();

    ShellSurface *popupParent() const { return m_parentShell; }
    // The popups opened from this surface, one level down.
    const std::vector<ShellSurface *> &childPopups() const { return m_childShells; }
    // Whether `root` is the surface of a popup opened from this one, at any depth.
    bool hasPopupDescendant(const compositor::Surface *root) const;

    // The toplevel window a popup chain hangs off, or the window itself.
    Window *ownerWindow() const;

    // --- xdg_surface ---

    // Pending until the next commit. An empty rect clears it, so the surface's
    // own content bounds are used again.
    void setGeometry(const base::Rect &geometry);
    // The committed window geometry, or the content rect when none was set.
    base::Rect visibleBounds() const;

    // False when the serial was never sent or has already been superseded; the
    // caller posts the protocol error.
    bool acknowledgeConfigure(uint32_t serial);

    bool hasPendingConfigures() const { return !m_pendingConfigs.empty(); }

    size_t pendingConfigureCount() const { return m_pendingConfigs.size(); }

    // --- xdg-decoration ---

    // A zxdg_toplevel_decoration_v1 exists for this toplevel. Without one the
    // effective mode is ClientSide.
    bool hasDecorationObject() const { return m_hasDecorationObject; }

    void setHasDecorationObject(bool has);

    // The mode the client asked for, or nothing after unset_mode.
    void setRequestedDecorationMode(std::optional<DecorationMode> mode);

    // What the compositor decided, which is what the decoration configure
    // event must carry.
    DecorationMode decorationMode() const;

    // The same decision for a mode the client has not asked for yet, so the
    // protocol layer can send the decoration configure before the toplevel
    // configure that follows from it.
    DecorationMode decorationModeFor(std::optional<DecorationMode> requested) const;

    // --- xdg-dialog ---

    // An xdg_dialog_v1 exists for this toplevel.
    bool hasDialogObject() const { return m_hasDialogObject; }

    void setHasDialogObject(bool has) { m_hasDialogObject = has; }

    // Kept across a remap; applied to the window whenever there is one.
    void setModal(bool modal);

    bool isModal() const { return m_modal; }

    // --- xdg_toplevel ---

    void setTitle(const std::string &title);

    const std::string &title() const { return m_title; }

    void setApplicationId(const std::string &id);

    const std::string &applicationId() const { return m_applicationId; }

    // The process behind the client connection this surface came in on. Set by
    // the protocol layer, which is the only place that can ask libwayland; the
    // window carries it on to the task list.
    void setProcessId(int64_t pid) { m_processId = pid; }

    int64_t processId() const { return m_processId; }

    // Pending until the next commit, like the geometry.
    void setMinimumSize(const base::Size &size);
    void setMaximumSize(const base::Size &size);

    // Refused, returning false, when it would form a cycle. A parent that is
    // not a mapped toplevel counts as no parent, as xdg-shell requires.
    bool setTransientParent(ShellSurface *parent);

    ShellSurface *transientParent() const { return m_transientParentShell; }

    void maximize();
    void unmaximize();
    void minimize();
    void setFullscreen(bool fullscreen);
    void setFullscreenOutput(const std::string &waylandName);

    // A toplevel attached to a drag before it maps follows the pointer, so it
    // maps in the normal state instead of one it would inherit from another
    // window of its application. The client is configured again if it was
    // told that state.
    void dropInheritedState();

    // Both validate nothing: the protocol layer checks the serial first.
    // `byTouch` drags with the newest touch point still down on the window,
    // and fails if there is none; otherwise the pointer drags.
    bool startMove(bool byTouch = false);
    bool startResize(ResizeEdge edge, bool byTouch = false);

    // Runs the close callback. The client decides what to do about it.
    void close();

    // --- xdg_popup ---

    // `client` identifies the client taking the grab and is only compared;
    // null is the shell's own menus.
    void grab(const void *client = nullptr);

    const void *grabClient() const { return m_grabClient; }

    // The surface inside the parent's window that a popup hangs from, such as
    // a status item in the bar: a press on it reaches its client and keeps
    // the popup, and it borrows the keyboard for the chain. Not owned; the
    // caller clears it before the surface goes.
    void setPopupAnchorSurface(compositor::Surface *surface) { m_popupAnchorSurface = surface; }

    compositor::Surface *popupAnchorSurface() const { return m_popupAnchorSurface; }

    // Sends xdg_popup.configure for the current bounds, followed by
    // xdg_surface.configure, and records the serial so ack_configure can find
    // it. Popups have no state of their own to report, so unlike a toplevel
    // they are configured only when the compositor is asked to place them.
    void configurePopup();
    // xdg_popup.reposition, with an already-solved positioner result. A popup
    // with a client moves once it has acknowledged and committed the new bounds.
    void reposition(const base::Rect &bounds);

    base::Rect popupBounds() const { return m_popupBounds; }

    // The popup origin in output coordinates.
    base::Point absoluteOrigin() const;

    // --- compositor::SurfaceObserver ---

    void onSurfaceDestroying(compositor::Surface *surface) override;
    void onSurfaceCommit(compositor::Surface *surface) override;
    std::function<void()> onSurfaceCommitRequested(compositor::Surface *surface) override;

    // --- WindowObserver ---

    void onWindowBoundsChanged(Window *window, const base::Rect &oldBounds) override;
    void onWindowStateTypeChanged(Window *window, WindowStateType oldType) override;
    void onWindowActivationChanged(Window *window, bool active) override;
    void onWindowResizingChanged(Window *window, bool resizing) override;
    void onWindowFrameChanged(Window *window) override;
    void onWindowHiddenChanged(Window *window) override;
    void onWindowDeskHiddenChanged(Window *window) override;
    void onWindowDestroying(Window *window) override;

private:
    // Coalesces the configures a single commit or state change would otherwise
    // produce into one. Upstream: ShellSurface::ScopedConfigure.
    class ScopedConfigure
    {
    public:
        ScopedConfigure(ShellSurface *shellSurface, bool forceConfigure);
        ScopedConfigure(const ScopedConfigure &) = delete;
        ScopedConfigure &operator=(const ScopedConfigure &) = delete;
        ~ScopedConfigure();

        void setNeedsConfigure() { m_needsConfigure = true; }

    private:
        ShellSurface *m_shellSurface;
        // A configure callback runs client-visible code, which can destroy the
        // shell surface this instance lives inside. See m_self.
        std::shared_ptr<ShellSurface *> m_alive;
        const bool m_forceConfigure;
        bool m_needsConfigure = false;
    };

    // Surface state associated with each outstanding configure.
    struct Config {
        uint32_t serial;
        base::Point originOffset;
        ResizeEdge resizeEdge;
        ConfigureState state;
        std::optional<base::Rect> popupBounds;
    };

    // Double-buffered state, taken when the client commits and installed when
    // that commit applies, which may be later.
    struct CommittedState {
        base::Rect geometry;
        base::Size minimumSize;
        base::Size maximumSize;
        // The accumulated offset of every configure acknowledged before the
        // commit, and the resize direction that came with the newest of them.
        base::Point originOffset;
        ResizeEdge resizeEdge = ResizeEdge::None;
        std::optional<ConfigureState> acknowledgedConfigure;
        bool decorationRemoval = false;
        std::optional<base::Rect> popupBounds;
    };

    void configure();
    void sendPopupConfigure(const base::Rect &bounds, bool reposition);
    void applyCommittedState(const CommittedState &state);
    void mapToplevel();
    void unmapToplevel();
    void mapPopup();
    void unmapPopup();
    void unmapChildPopups();
    void updatePopupPosition();
    void updateChildPopupPositions();
    // Both derive from the committed xdg_surface window geometry, so they are
    // refreshed together.
    void updateFromWindowGeometry();
    void updateSurfaceOrigin();
    void updateClientExteriorInsets();
    // What the buffer covers outside the declared window geometry.
    Insets clientExteriorInsets() const;
    // What the surface covers outside its committed input region.
    Insets clientInputInsets() const;
    void updateWindowBoundsFromClient();
    void updateSizeConstraints();
    base::Size configureBounds() const;
    // The frame a normal window would have, whatever the current state.
    Insets normalFrameInsets() const;
    bool computeResizable() const;
    // An ordinary top-level application window: no transient parent, not a
    // shell window.
    bool isPositionManaged() const;
    base::Size initialSizeForState(WindowStateType stateType) const;
    base::Point surfaceOriginForWindow() const;
    void resetConfigureState();
    void applyDecorationMode();
    void addChildShell(ShellSurface *child);
    void removeChildShell(ShellSurface *child);
    void linkTransientParent(ShellSurface *parent);
    // xdg-shell: the children of an unmapped toplevel pass to its own parent,
    // or lose theirs. A remap does not bring them back.
    void releaseTransientChildren();
    void detachTransientRelationships();

    // Cleared at the top of the destructor. Every callback out of this class
    // reaches the client, and a client-facing callback may destroy the protocol
    // object that owns this one, so anything that runs code after such a call
    // re-checks this first.
    std::shared_ptr<ShellSurface *> m_self;

    WindowManager *m_manager = nullptr;
    compositor::Surface *m_surface = nullptr;
    Window *m_window = nullptr;
    Role m_role = Role::None;

    ConfigureCallback m_configureCallback;
    PopupConfigureCallback m_popupConfigureCallback;
    CloseCallback m_closeCallback;
    PingCallback m_pingCallback;
    KillCallback m_killCallback;

    std::string m_title;
    std::string m_applicationId;
    int64_t m_processId = 0;

    base::Rect m_geometry;
    base::Rect m_pendingGeometry;
    base::Size m_minimumSize;
    base::Size m_pendingMinimumSize;
    base::Size m_maximumSize;
    base::Size m_pendingMaximumSize;

    // The state the client asked for before it was mapped. Applied when the
    // window is created, and reported in the initial configure.
    WindowStateType m_initialStateType = WindowStateType::Normal;
    // Set when the window positioner, not the client, chose m_initialStateType.
    bool m_initialStateInherited = false;
    // The output chosen for an ordinary window at its initial configure, so the
    // inherited state, the initial size and the placement all agree on it.
    base::Rect m_initialOutput;
    std::string m_fullscreenOutputName;
    bool m_initialConfigureSent = false;

    ScopedConfigure *m_scopedConfigure = nullptr;
    std::deque<Config> m_pendingConfigs;
    std::optional<ConfigureState> m_acknowledgedConfigure;
    std::optional<ConfigureState> m_boundsTransitionTarget;

    // Resizing is asynchronous: the window manager has already moved and
    // resized the window while the client is still painting the old size.
    // `m_originOffset` holds the surface where it was until the client catches
    // up. Upstream: ShellSurface::origin_offset_ and friends.
    base::Point m_originOffset;
    base::Point m_pendingOriginOffset;
    base::Point m_pendingOriginOffsetAccumulator;

    // The acknowledged resize direction, which decides which corner of the
    // window the surface is anchored to.
    ResizeEdge m_resizeEdge = ResizeEdge::None;
    ResizeEdge m_pendingResizeEdge = ResizeEdge::None;

    bool m_hasDecorationObject = false;
    bool m_decorationRemovalPending = false;
    // Installed by a commit about to apply; unset for a commit sent before
    // the role existed.
    std::optional<CommittedState> m_committed;
    std::optional<DecorationMode> m_requestedMode;
    bool m_hasDialogObject = false;
    bool m_modal = false;
    // The client area the last configure described, so a bounds change can be
    // compared against what the client was actually told. The outer bounds also
    // move when the frame comes and goes, which is not something the client
    // needs to hear about.
    base::Rect m_configuredContentBounds;
    base::Size m_boundsTransitionOldContentSize;

    // Set while the window bounds are being changed to follow the client's own
    // geometry, so that change does not bounce a configure back at it.
    bool m_applyingClientBounds = false;

    ShellSurface *m_parentShell = nullptr;
    ShellSurface *m_transientParentShell = nullptr;
    std::vector<ShellSurface *> m_transientChildShells;
    std::vector<ShellSurface *> m_childShells;
    base::Rect m_popupBounds;
    // Repositioned bounds the client has acknowledged but not yet committed.
    std::optional<base::Rect> m_pendingPopupBounds;
    bool m_hasGrab = false;
    const void *m_grabClient = nullptr;
    compositor::Surface *m_popupAnchorSurface = nullptr;
    bool m_popupMapped = false;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_SHELL_SURFACE_H_
