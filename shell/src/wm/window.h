// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_WINDOW_H_
#define LINGMO_WM_WINDOW_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>


#include "base/geometry.h"
#include "compositor/surface_observer.h"
#include "ui/bitmap.h"
#include "ui/path.h"
#include "wm/types.h"
#include "wm/window_frame.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::WM {

class WindowManager;
class WindowObserver;
class WindowState;
class WindowVisualController;

// A user-visible window. This is a window-manager concept: it is not a
// wl_surface and holds no protocol resource. It refers to the root Surface of
// the client's surface tree and owns that surface's placement in the scene.
//
// Geometry note: bounds() is the *outer* managed window bounds - everything the
// window manager owns on screen, including any server-side frame. The client's
// area is contentBounds(), which is bounds() minus the frame extents, and that
// is what an xdg configure describes. For a client-decorated or fullscreen
// window the two are the same rectangle.
//
//   bounds()                       contentBounds()
//   +---------------------------+
//   | frame                     |
//   +---------------------------+  +---------------------------+
//   |                           |  |                           |
//   |  client                   |  |  client                   |
//   +---------------------------+  +---------------------------+
//
// The origin is authoritative and drives the scene node position directly. The
// size is a request; the surface node keeps whatever size the committed buffer
// has until the client resizes to match, which the xdg configure closes.
class Window : public compositor::SurfaceObserver
{
public:
    Window(WindowManager *manager, compositor::Surface *rootSurface, const std::string &title = {},
           WindowType type = WindowType::Normal);
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    ~Window() override;

    WindowType type() const { return m_type; }

    bool isPopup() const { return m_type == WindowType::Popup; }

    uint64_t id() const { return m_id; }

    WindowManager *manager() const { return m_manager; }

    // The root of the client's surface tree. Never a subsurface.
    compositor::Surface *rootSurface() const { return m_rootSurface; }

    // The node grouping everything this window draws: the frame, and the
    // client's surface tree under it. Null when there is no scene at all
    // (protocol-only tests). The window manager stacks this node.
    scene::Node *node() const { return m_node; }

    scene::Node *visualNode() const;

    // The root of the client's surface tree, inside visualNode().
    scene::Node *surfaceNode() const;

    const std::string &title() const { return m_title; }

    void setTitle(const std::string &title);

    // xdg_toplevel.set_app_id. Metadata only; nothing in the window manager
    // keys off it yet.
    const std::string &applicationId() const { return m_applicationId; }

    void setApplicationId(const std::string &id);

    bool skipTaskbar() const;

    // The process behind the Wayland connection the window's shell surface was
    // created on, or 0 when unknown. Metadata for the task list only: it is
    // never a privilege check.
    int64_t processId() const { return m_processId; }

    void setProcessId(int64_t pid) { m_processId = pid; }

    // --- Shell role ---
    //
    // Everything but Normal is one of the compositor's shell windows, or the
    // trusted desktop client's surface (lingmo_shell_v1), adapted into these
    // plain WM concepts before the window is announced. Set once, before the
    // window manager places the window or emits windowAdded.

    WindowRole role() const { return m_role; }

    void setRole(WindowRole role);

    bool isShellWindow() const { return m_role != WindowRole::Normal; }

    FocusPolicy focusPolicy() const { return m_focusPolicy; }

    void setFocusPolicy(FocusPolicy policy) { m_focusPolicy = policy; }

    PanelBehavior panelBehavior() const { return m_panelBehavior; }

    void setPanelBehavior(PanelBehavior behavior);

    // Where the client asked to be put, in output coordinates. Authoritative
    // for a shell window; ordinary applications never set it.
    void setClientPosition(const base::Point &position);

    // Where a minimize animation should end: the task-list entry the shell
    // reported, in output coordinates. Null when the shell has not said.
    // Stored for the animation to use; nothing here depends on it.
    base::Rect minimizedGeometry() const;
    void setMinimizedGeometry(const base::Rect &geometry);
    void setMinimizedGeometry(Window *panel, const base::Rect &geometry);

    // The window as it last looked when it was minimized, for the dock to show
    // in its place. Null while the window is not minimized.
    const ui::Bitmap &minimizedSnapshot() const { return m_minimizedSnapshot; }

    void setMinimizedSnapshot(const ui::Bitmap &snapshot);

    // The minimize animation holds the window on its dock slot until the shell
    // shows the thumbnail there, or kMinimizeHandoffTimeoutMs pass.
    void beginMinimizeHandoff();
    void endMinimizeHandoff();

    static constexpr int kMinimizeHandoffTimeoutMs = 250;

    WindowState *state() { return m_state.get(); }

    const WindowState *state() const { return m_state.get(); }

    WindowStateType stateType() const;

    // What decoration, visuals and xdg configures follow. It differs from
    // stateType() only while a drag pulls a tiled window out of its state: the
    // window manager keeps the tiled state until the drag ends, but everything
    // the user sees has to be the restored look already.
    // Upstream: ash/public/cpp/window_properties.h kFrameRestoreLookKey.
    WindowStateType presentedStateType() const;

    // What the compositor draws *around* the window follows - its shadow, its
    // rounded corners, its outline and its frame. It lags presentedStateType()
    // by the frame or two a drag out of a tiled state takes to reach the
    // client: the client is told it has been restored straight away, but until
    // its smaller buffer arrives what is on screen is still the tiled window,
    // and exterior decoration drawn around that hugs an edge the window has
    // already left.
    WindowStateType decoratedStateType() const;

    bool isRestoringFromDrag() const { return m_restoringFromDrag; }

    // --- Geometry ---

    base::Rect bounds() const { return m_bounds; }

    // The client area inside the frame. Equal to bounds() when undecorated.
    base::Rect contentBounds() const { return frameInsets().inset(m_bounds); }

    // What the frame occupies of bounds(). Null while fullscreen: a fullscreen
    // window has no server decoration.
    Insets frameInsets() const;

    // Sets the logical geometry directly, bypassing the state machine. Use a
    // SetBoundsWMEvent for anything a user or client requested; this is for the
    // state machine, the resizer and initial placement.
    void setBounds(const base::Rect &bounds);

    // Offset from contentBounds().topLeft() to the root surface's origin. The
    // shell surface sets it from the client's window geometry - a CSD client
    // whose buffer carries a shadow has a non-zero one - and from the
    // compensation applied while an asynchronous resize from a top or left edge
    // is in flight. The frame extents are added on top of it here, so the shell
    // surface never has to know about them.
    base::Point surfaceOrigin() const { return m_surfaceOrigin; }

    void setSurfaceOrigin(const base::Point &origin);

    // What the window actually has on screen right now, in node() coordinates.
    // bounds() is a *request*: during an interactive resize the window manager
    // moves it on every pointer motion, while the client's buffer only catches
    // up a round trip later. Anything the compositor draws around the window -
    // its frame, outline and shadow - follows this instead of an edge the client
    // has not reached yet.
    //
    // Includes real SSD insets around the committed client size. Configure
    // acknowledgements alone do not replace the presented buffer.
    base::Rect presentedBounds() const;

    // Both constrain the client area, which is what xdg_toplevel.set_min_size /
    // set_max_size talk about. An empty maximum size means unbounded.
    base::Size minimumSize() const { return m_minimumSize; }

    void setMinimumSize(const base::Size &size);

    base::Size maximumSize() const { return m_maximumSize; }

    void setMaximumSize(const base::Size &size);

    // The same two limits applied to the outer bounds, which is what the
    // resizer and the state machine work in.
    base::Size minimumOuterSize() const;
    base::Size maximumOuterSize() const;

    bool isResizable() const { return !isPopup() && m_resizable; }

    void setResizable(bool resizable) { m_resizable = resizable; }

    // --- Presentation offset ---

    // How far from bounds() the window is drawn, in output coordinates. The
    // dock slides off its screen edge by this much while it auto-hides; input
    // follows what is drawn, so a dock parked off screen takes no clicks.
    // Nothing else reads it: bounds() stays where the client put the window,
    // and that is what its popups, its minimize targets and its work-area
    // reservation are measured from.
    base::Point presentationOffset() const { return m_presentationOffset; }
    void setPresentationOffset(const base::Point &offset);

    // A dock parked off screen is not drawn at all, so it cannot land on an
    // output that happens to lie beyond the edge it left through. Distinct
    // from isVisible(), which is the minimized state.
    bool isPresentationHidden() const { return m_presentationHidden; }
    void setPresentationHidden(bool hidden);

    // --- Client-side decoration ---

    // What the client's own buffer occupies outside its declared xdg_surface
    // window geometry. A CSD client that paints its own shadow or an invisible
    // resize border has a non-zero one; a frameless client whose geometry is
    // its whole buffer has none. Set by the shell surface from committed
    // protocol geometry: nothing infers it from the application id, the
    // toolkit or the buffer's pixels.
    Insets clientExteriorInsets() const { return m_clientExteriorInsets; }

    void setClientExteriorInsets(const Insets &insets);

    // Leading margins distinguish surrounding decoration from trailing resize slack.
    // Visual ownership does not depend on this classification.
    bool hasClientExteriorDecoration() const
    {
        return m_clientExteriorInsets.left > 0 && m_clientExteriorInsets.top > 0;
    }

    // How far the committed input region stays inside the surface on each
    // edge. A client that declares its whole surface as window geometry but
    // leaves a band on every edge out of its input region paints its own
    // exterior there, the way a menu with a self-drawn shadow does.
    Insets clientInputInsets() const { return m_clientInputInsets; }

    void setClientInputInsets(const Insets &insets);

    bool hasClientInputMargin() const
    {
        return m_clientInputInsets.left > 0 && m_clientInputInsets.top > 0
                && m_clientInputInsets.right > 0 && m_clientInputInsets.bottom > 0;
    }

    // --- Server-side decoration ---

    // Gives the window a frame, or takes it away, from the window manager's
    // frame provider. The client area keeps its size and its position; the
    // outer bounds absorb the difference. Does nothing without a provider,
    // which is the case in protocol-only tests.
    void setDecorated(bool decorated);

    bool isDecorated() const { return m_decorated; }

    // Null unless the window is decorated and a provider was installed.
    WindowFrame *frame() const { return m_frame.get(); }

    // --- Visibility and activation ---

    // False while minimized, hidden or on a desk that is not active. An
    // invisible window is not rendered and is not hit tested; its surface and
    // state stay alive. Upstream: aura::Window::IsVisible.
    bool isVisible() const { return m_visible && !m_deskHidden; }

    // The window's own visibility, whatever its desk: a window on an inactive
    // desk can still be activated, which brings its desk up.
    // Upstream: aura::Window::TargetVisibility.
    bool targetVisibility() const { return m_visible; }

    void setVisible(bool visible);

    // Set by the window's desk while that desk is not the active one, as Ash
    // hides an inactive desk's container. Orthogonal to minimized and hidden.
    bool isDeskHidden() const { return m_deskHidden; }

    void setDeskHidden(bool hidden);

    // Hidden by the shell: off screen at once, with no animation and no
    // thumbnail, and orthogonal to the state type. A minimized window that is
    // hidden stays minimized and returns to the dock when it is unhidden.
    bool isHidden() const { return m_hidden; }

    void setHidden(bool hidden);

    // The window manager's opacity for the whole window, frame included,
    // times the client's own (set_opacity). At zero it is still shown for
    // everything but the eye: frames, input, stacking.
    void setOpacity(float opacity);

    // Drawn at `scale` of its size about `anchor`, a point in bounds()
    // coordinates, animated there from what is drawn now unless `animate` is
    // false. Presentation only: bounds, input and the client are unaffected.
    void setPresentationScale(double scale, const base::PointF &anchor, bool animate);

    bool isActive() const;
    void activate();

    // True between beginMove/beginResize and the end of the drag. ShellSurface
    // maps this onto the xdg_toplevel resizing state.
    bool isResizing() const { return m_resizing; }

    // True for an interactive move as well, which the xdg_toplevel resizing
    // state deliberately does not cover.
    bool isDragged() const { return m_dragged; }

    // A window pulled out of another by a drag and not yet let go: it follows
    // the pointer as a preview, and cannot be activated until it is dropped.
    bool isDragPreview() const { return m_dragPreview; }

    void setDragPreview(bool preview) { m_dragPreview = preview; }

    // --- Convenience state requests, each one a WMEvent ---

    void maximize();
    void snapPrimary();
    void snapSecondary();
    void minimize();
    void restore();
    void setFullscreen(bool fullscreen);
    void requestBounds(const base::Rect &bounds);

    // --- Closing ---

    // How a close request reaches the client. The shell surface installs it;
    // for xdg-shell it sends xdg_toplevel.close.
    using CloseCallback = std::function<void()>;
    void setCloseCallback(CloseCallback callback);

    bool canClose() const { return static_cast<bool>(m_closeCallback); }

    // Runs the close callback, which is client-visible and may destroy this
    // window. An unresponsive window offers force quit instead, or is force
    // quit when nobody can offer it.
    void requestClose();

    // --- Responsiveness ---

    // How the window manager asks the client to show it still handles events.
    // The shell surface installs it; for xdg-shell it sends xdg_wm_base.ping,
    // and the answer, or its absence, comes back through setResponsive().
    using PingCallback = std::function<void()>;
    void setPingCallback(PingCallback callback);

    // Does nothing for shell windows, which the session supervises. Pinging a
    // window that already stopped answering means the user is waiting on it,
    // and offers force quit.
    void ping();

    // False while a ping goes unanswered past its timeout. The window is dimmed,
    // and closing it offers force quit instead of asking the client.
    bool isResponsive() const { return m_responsive; }

    void setResponsive(bool responsive);

    // The application asked for the user's attention: WM_HINTS' urgency bit,
    // or _NET_WM_STATE_DEMANDS_ATTENTION under XWayland. The dock bounces the
    // application's icon for it, and activating the window clears it.
    bool demandsAttention() const { return m_demandsAttention; }
    void setDemandsAttention(bool demandsAttention);

    // Ends the client process behind the window: the only place the window
    // manager terminates a client. See WindowManager::forceQuit().
    using KillCallback = std::function<void()>;
    void setKillCallback(KillCallback callback);
    void kill();

    // --- Transients ---

    // Null unsets the relationship. Refused, returning false, if it would form
    // a cycle or if `parent` is this window.
    bool setTransientParent(Window *parent);

    Window *transientParent() const { return m_transientParent; }

    const std::vector<Window *> &transientChildren() const { return m_transientChildren; }

    // Window-modal: while this window is visible, its transient parent gets no
    // pointer input and cannot be activated. Without a parent it blocks nothing.
    bool isModal() const { return m_modal; }

    void setModal(bool modal);

    // A dialog the session asks on an application's behalf. Only its stacking
    // layer differs from an ordinary window; see StackingLayer::SystemDialog.
    bool isSystemDialog() const { return m_systemDialog; }

    void setSystemDialog(bool systemDialog);

    // A shell prompt the user answers in person, such as a password or a
    // pairing confirmation: computer control neither clicks nor types into it
    // or anything transient to it.
    bool isProtectedFromControl() const { return m_protectedFromControl; }

    void setProtectedFromControl(bool protectedFromControl)
    {
        m_protectedFromControl = protectedFromControl;
    }

    // Set by policy for floating clients, or asked for by the client itself
    // through lingmo_window_hints_v1.
    bool isAlwaysOnTop() const;

    void setAlwaysOnTop(bool alwaysOnTop);

    // --- Observers ---

    // compositor::SurfaceObserver: the client destroyed the surface, so the
    // window goes with it. This deletes the window.
    void onSurfaceDestroying(compositor::Surface *surface) override;
    // The client committed; whatever it now has on screen is presentedBounds().
    void onSurfaceCommit(compositor::Surface *surface) override;
    void onSurfaceWindowHintsChanged(compositor::Surface *surface) override;

    void addObserver(WindowObserver *observer);
    void removeObserver(WindowObserver *observer);
    bool hasObserver(const WindowObserver *observer) const;

private:
    friend class Activation;
    friend class TransientManager;
    friend class WindowState;
    friend class ToplevelEventHandler;

    void setActiveInternal(bool active);
    void setResizingInternal(bool resizing);
    void setDraggedInternal(bool dragged);
    void setRestoringFromDragInternal(bool restoring);
    void updateSurfacePlacement();
    // Reports a move or resize of the root surface, in global coordinates, to
    // the window manager.
    void updateSurfaceTargetBounds();
    void updateNodeVisibility();
    // Applies a change of isVisible() from `wasVisible`.
    void onEffectiveVisibilityChanged(bool wasVisible);
    // Recentres the sheets hanging from this window on what it shows.
    void placeSheets();
    void updateFrame();
    // Returns the client's surface tree to the compositor's surface container,
    // so it survives this window's scene node.
    void detachSurfaceNode();

    const WindowType m_type;
    const uint64_t m_id;
    WindowManager *m_manager = nullptr;
    compositor::Surface *m_rootSurface = nullptr;
    std::string m_title;
    std::string m_applicationId;
    int64_t m_processId = 0;
    WindowRole m_role = WindowRole::Normal;
    FocusPolicy m_focusPolicy = FocusPolicy::Normal;
    PanelBehavior m_panelBehavior = PanelBehavior::StayAbove;
    base::Point m_clientPosition;
    base::Rect m_minimizedGeometry;
    uint64_t m_minimizedGeometryPanel = 0;
    ui::Bitmap m_minimizedSnapshot;
    uint64_t m_minimizeHandoff = 0;
    CloseCallback m_closeCallback;
    PingCallback m_pingCallback;
    KillCallback m_killCallback;
    std::unique_ptr<WindowState> m_state;

    scene::Node *m_node = nullptr;
    std::unique_ptr<WindowVisualController> m_visual;
    // The root surface's body shape the visuals were last made for.
    ui::Path m_bodyShape;
    bool m_castsShadow = true;
    std::unique_ptr<WindowFrame> m_frame;
    bool m_decorated = false;

    base::Rect m_bounds;
    base::Point m_surfaceOrigin;
    base::Rect m_surfaceTargetBounds;
    base::Point m_presentationOffset;
    bool m_presentationHidden = false;
    Insets m_clientExteriorInsets;
    Insets m_clientInputInsets;
    base::Size m_minimumSize {1, 1};
    base::Size m_maximumSize;
    bool m_resizable = true;
    bool m_visible = true;
    bool m_hidden = false;
    bool m_deskHidden = false;
    bool m_active = false;
    bool m_resizing = false;
    bool m_dragged = false;
    bool m_dragPreview = false;
    bool m_restoringFromDrag = false;
    bool m_modal = false;
    bool m_responsive = true;
    bool m_demandsAttention = false;
    bool m_systemDialog = false;
    bool m_protectedFromControl = false;
    bool m_alwaysOnTop = false;
    bool m_surfaceAlwaysOnTop = false;
    float m_opacity = 1;
    float m_surfaceOpacity = 1;

    Window *m_transientParent = nullptr;
    std::vector<Window *> m_transientChildren;

    std::vector<WindowObserver *> m_observers;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_H_
