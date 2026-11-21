// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_WINDOW_MANAGER_H_
#define LINGMO_WM_WINDOW_MANAGER_H_

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/geometry.h"
#include "base/lru_cache.h"
#include "base/signal.h"
#include "base/timer.h"
#include "base/weak_ptr.h"
#include "decoration/window_visual_style.h"
#include "display/display_types.h"
#include "input/event_sink.h"
#include "wm/activation.h"
#include "wm/placement.h"
#include "wm/stacking.h"
#include "wm/types.h"
#include "wm/window_frame.h"
#include "wm/window_modality_controller.h"
#include "wm/work_area.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {
class Compositor;
class Surface;
} // namespace Lingmo::Compositor

namespace Lingmo::Input {
class Seat;
class SessionGate;
} // namespace Lingmo::Input

namespace Lingmo::Display {
class DisplayManager;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::WM {

class DesksController;
class MruWindowTracker;
class OverviewController;
class ShowDesktopController;
class ShellSurface;
class StatusItems;
class ToplevelEventHandler;
class TransientManager;
class Window;

// What a shell surface is for, in window-manager concepts: set by the
// compositor's own shell windows, and by the trusted desktop client through
// lingmo_shell_v1. Registered against the compositor::Surface; the window
// manager applies it when the surface's role turns into a Window, before
// placement and before the window is announced.
struct ShellWindowMetadata {
    WindowRole role = WindowRole::Normal;
    FocusPolicy focusPolicy = FocusPolicy::Normal;
    PanelBehavior panelBehavior = PanelBehavior::StayAbove;
    base::Point position;
    bool hasPosition = false;
    std::string outputName;
    // The window a Sheet hangs from, by Window::id; zero for none.
    uint64_t parentWindow = 0;
    // See Window::isProtectedFromControl.
    bool protectedFromControl = false;
};

// Everything a window is announced with. The application id and the process id
// belong here rather than being set afterwards: windowAdded is what the shell's
// task list enumerates from, and a task that appears without an app id never
// gets one, because nothing changed after that.
struct WindowInit {
    WindowType type = WindowType::Normal;
    base::Point origin;
    base::Size requestedSize;
    std::string title;
    std::string applicationId;
    int64_t processId = 0;
    bool decorated = false;
    base::Size minimumSize {1, 1};
    base::Size maximumSize;
    FocusPolicy focusPolicy = FocusPolicy::Normal;
    // A transient toplevel is centered over its parent instead of placed.
    Window *transientParent = nullptr;
    // Cascades the window from the application's most recent one instead of
    // centering it. See window_positioner.h.
    bool positionManaged = false;
    // Window-modal on `transientParent`, applied before anything can activate
    // the window.
    bool modal = false;
    // The output an ordinary window is placed on, chosen when it was first
    // configured. Empty means outputForNewWindow() at creation.
    base::Rect output;
    std::function<void()> closeCallback;
};

// Owns the windows, their stacking order, and the policy that decides which of
// them is active. It is also the compositor's input sink: it sees pointer and
// key events before the seat dispatches them to clients, so that a drag can
// consume them and a click can change activation first.
class WindowManager : public input::EventSink, public ActivationObserver
{
public:
    // `seat` may be null in tests that do not exercise keyboard focus.
    WindowManager(compositor::Compositor *compositor, input::Seat *seat);
    WindowManager(const WindowManager &) = delete;
    WindowManager &operator=(const WindowManager &) = delete;
    ~WindowManager() override;

    compositor::Compositor *compositor() const { return m_compositor; }

    base::WeakPtr<WindowManager> weakPtr() { return m_weakFactory.getWeakPtr(); }
    // Runs `task` from the main loop once `delay` has passed, or on its next
    // turn for no delay, unless the window manager is gone by then.
    void runLater(std::chrono::milliseconds delay, std::function<void()> task);

    input::Seat *seat() const { return m_seat; }

    // Where server-side frames come from. Null leaves every window
    // undecorated, which is what the protocol-only tests want. Set once, before
    // any window exists.
    FrameProvider *frameProvider() const { return m_frameProvider; }

    void setFrameProvider(FrameProvider *provider) { m_frameProvider = provider; }

    const decoration::WindowVisualStyle &visualStyle() const { return m_visualStyle; }

    void setVisualStyle(const decoration::WindowVisualStyle &style);

    // The session's view of whether the desktop may be touched at all. Null
    // leaves the window manager unrestricted, which is what tests that never
    // lock the session want.
    void setSessionGate(input::SessionGate *gate) { m_sessionGate = gate; }

    input::SessionGate *sessionGate() const { return m_sessionGate; }

    // Whether any window may take activation, and the scene subtree input may
    // target. Both answer for the session, not for a single window.
    bool allowsActivation() const;
    scene::Node *inputRoot() const;

    Activation *activation() { return &m_activation; }

    // Gives `window` the keyboard without making it active, as Ash's capture
    // session filters keys while the application window keeps activation. It
    // lasts until the window is hidden or gone.
    void setKeyboardGrab(Window *window);
    Window *keyboardGrab() const { return m_keyboardGrab; }

    // Sends the paste shortcut to the application window holding the keyboard:
    // Ctrl+Shift+V to a terminal, Ctrl+V elsewhere. Keys and buttons the user
    // still holds, such as the Return that chose what to paste, are waited for
    // up to kPasteWaitMs. Nothing while the session is blocked.
    void sendPaste();
    static constexpr int kPasteWaitMs = 2000;

    TransientManager *transientManager() { return m_transientManager.get(); }

    MruWindowTracker *mruWindowTracker() const { return m_mruTracker.get(); }

    DesksController *desks() const { return m_desks.get(); }

    OverviewController *overview() const { return m_overview.get(); }

    ShowDesktopController *showDesktop() const { return m_showDesktop.get(); }

    // The items applications draw in the status bar.
    StatusItems *statusItems() const { return m_statusItems.get(); }

    ToplevelEventHandler *eventHandler() { return m_eventHandler.get(); }

    // --- Windows ---

    // The window takes its initial bounds from the placement policy, using
    // `init.requestedSize` when it is not empty and the surface's current
    // content size otherwise. Both describe the client area: the frame is added
    // around it when `init.decorated` asks for one.
    Window *createWindow(compositor::Surface *rootSurface, const WindowInit &init = {});
    void destroyWindow(Window *window);

    // Bottom to top.
    const std::vector<Window *> &windows() const { return m_stackingOrder; }

    // Decides from the client's pid whether its toplevels are system dialogs.
    // The pid comes from the socket, so it names the process, not a claim.
    void setSystemDialogPolicy(std::function<bool(int64_t pid)> policy)
    {
        m_systemDialogPolicy = std::move(policy);
    }

    // Moves `window` to the top of its own stacking layer, in the model and in
    // the scene. A raise never lifts a window past a window of a higher layer:
    // an ordinary application can no more cover the dock than a dock can bury
    // the notifications.
    void raise(Window *window);

    // To the bottom of the window's stacking layer, its transient children
    // still above it.
    void stackAtBottomOfLayer(Window *window);

    // Synchronizes live scene order, preserving outgoing snapshot slots.
    void restackScene();

    // The window owning `surface`, following the surface hierarchy up to its
    // root first. A subsurface therefore resolves to its toplevel's window.
    Window *windowForSurface(compositor::Surface *surface) const;

    // --- Applications ---
    //
    // Quit, Hide and Hide Others, for the shell's application menu and dock
    // and the Command convention's shortcuts.
    // An application is the task-list windows of one application id, as the
    // dock groups them, or of one process for windows without an id.

    // Asks the application's top-level windows to close; dialogs are left
    // alone, so a second Quit asks again rather than dismissing a prompt. Their
    // processes that have no task-list window left after the grace period are
    // terminated through setTerminateProcess().
    void quitApplication(Window *window);

    // Whether `candidate` is one of `window`'s application's windows, as Quit
    // and the dock group them.
    bool belongsToApplication(const Window *candidate, const Window *window) const;

    void hideApplication(Window *window);

    // Shows every hidden window of `window`'s application again, in place:
    // what switching to a hidden application does.
    void unhideApplication(Window *window);

    // Hides every application but `window`'s; all of them for null.
    void hideOtherApplications(Window *window);

    // How Quit ends a process that closed its windows but keeps running: the
    // session's AppRuntime in production. Unset, the process is left alone.
    void setTerminateProcess(std::function<void(int64_t pid)> terminate)
    {
        m_terminateProcess = std::move(terminate);
    }

    void setQuitGracePeriod(std::chrono::milliseconds period) { m_quitGracePeriod = period; }

    // Ends `window`'s client at once, without asking it, and the rest of the
    // application through setForceQuitProcess().
    void forceQuit(Window *window);

    // How force quit also ends the application scope the process was started
    // in: the session's AppRuntime in production. Unset, only the client ends.
    void setForceQuitProcess(std::function<void(int64_t pid)> forceQuit)
    {
        m_forceQuitProcess = std::move(forceQuit);
    }

    // How the user is offered force quit for a window that stopped answering
    // (the shell's not-responding sheets); returns whether it was offered.
    void setNotRespondingPrompt(std::function<bool(Window *)> prompt)
    {
        m_notRespondingPrompt = std::move(prompt);
    }

    // Called by Window; not part of the public API.
    bool promptNotResponding(Window *window);

    // The window with this identifier, or null. Identifiers are stable for the
    // life of the compositor and never reused.
    Window *windowById(uint64_t id) const;

    // --- Private shell metadata ---

    // Records what `surface` is for and, when the surface already has a
    // window, applies it there. Passing metadata whose role is Normal takes a
    // surface back out of the shell model.
    void setShellMetadata(compositor::Surface *surface, const ShellWindowMetadata &metadata);
    void clearShellMetadata(compositor::Surface *surface);
    const ShellWindowMetadata *shellMetadata(compositor::Surface *surface) const;

    // Re-sorts `window` into the stacking order after its role or panel
    // behavior changed. Called by Window; not part of the public API.
    void onWindowStackingChanged(Window *window);

    // Centres a Sheet on what its transient parent shows. Called by Window; not
    // part of the public API.
    void placeSheet(Window *sheet);

    // Called by Window and TransientManager; not part of the public API.
    void onWindowModalChanged(Window *window);
    void onWindowModalityInputsChanged(Window *window);
    void onWindowVisibilityChanged(Window *window);
    // The window's outer bounds or its root surface moved or resized, from
    // `oldBounds` to `newBounds` in global coordinates.
    void onWindowTargetBoundsChanged(Window *window, const base::Rect &oldBounds,
                                     const base::Rect &newBounds);

    // --- Work-area reservations ---

    // `token` identifies the reservation's owner - the protocol layer uses the
    // shell surface object - so it can be withdrawn again. Every change
    // recomputes the work area from the output bounds. A reservation that
    // `yieldsToMaximized` leaves the work area of an output while a window
    // there is maximized.
    void setExclusiveZone(const void *token, ScreenEdge edge, int thickness,
                          const std::string &outputName = {}, bool yieldsToMaximized = false);
    void clearExclusiveZone(const void *token);
    // A visible application window on the output with `outputBounds` is maximized.
    bool hasMaximizedWindow(const base::Rect &outputBounds) const;
    // Tells the work areas whether the dock `dock` is slid away: what a new
    // window is placed in leaves its reservation out while it is.
    void setDockHidden(const Window *dock, bool hidden);
    // Brings the work areas up to date after windows were maximized, restored,
    // shown, hidden or moved between outputs. Upstream: the shelf's
    // UpdateWorkAreaInsetsAndNotifyObservers, called by the dock layout.
    void updateWorkAreas();

    // --- Surfaces that are not windows ---

    // Auxiliary surfaces, such as Xwayland override-redirect surfaces, resolve
    // input and activation to their managed owner.
    void setSurfaceOwner(compositor::Surface *surface, Window *owner);
    void clearSurfaceOwner(compositor::Surface *surface);

    // Unowned auxiliary surfaces remain below the shell layers.
    void setAuxiliarySurface(compositor::Surface *surface, Window *owner);

    // --- Drag overlay ---

    // The snap preview an interactive move shows, stacked directly below
    // `below` so the dragged window stays on top of it. Passing null clears it.
    // The node is owned by whatever created it, not by the window manager.
    void setDragOverlay(scene::Node *overlay, Window *below);

    scene::Node *dragOverlay() const { return m_dragOverlay; }

    // Drawn right above the desks' windows, below everything that is on every
    // desk: the desk switch animation and the windows sent to another desk.
    void setDeskOverlays(std::vector<scene::Node *> overlays);

    // Minimize and restore pass over the panels to meet the dock's minimized
    // area; without it they go into the dock's icons, under the panels.
    void setDockTransitionsAbovePanels(bool above);

    // --- Popup grabs ---

    // The chain of popups holding an explicit grab, outermost first. A pointer
    // press outside the chain dismisses all of it and is not delivered, except
    // on the status bar the chain hangs from (see isPopupGrabMenuBar()). The
    // chain belongs to one client, or to the shell: a grab from anyone else
    // dismisses it first.
    void pushPopupGrab(ShellSurface *popup);
    void removePopupGrab(ShellSurface *popup);

    const std::vector<ShellSurface *> &popupGrabs() const { return m_popupGrabs; }

    void dismissPopupGrabs();

    // Ends a window drag where it is and lets go of a held caption button.
    // A blocked session sends their release to the lock screen instead.
    void cancelInteractions();

    // Topmost visible window whose surface hierarchy contains `location`, given
    // in output coordinates. `surfaceLocation`, when non-null, receives the
    // point in the hit surface's own coordinates.
    Window *windowAt(const base::PointF &location, compositor::Surface **hitSurface = nullptr,
                     base::PointF *surfaceLocation = nullptr) const;

    // The topmost window shown at `location` as a capture sees the screen:
    // the capture interface is looked through, and a popup answers for the
    // window it belongs to.
    Window *capturableWindowAt(const base::PointF &location) const;

    // The topmost window whose server-side frame owns `location`, and which
    // part of it. Null when the point belongs to a client, to nothing, or to a
    // window under one that already covers the point. A window drawn away from
    // its bounds (Window::presentationOffset) covers where it is drawn and owns
    // no frame actions there.
    Window *frameAt(const base::PointF &location, FrameArea *area = nullptr) const;

    // The topmost window whose server resize handles own `location`, and the
    // edge they grab; see lingmo_window_hints_v1.set_server_resize. A touch
    // gets wider handles. A window drawn away from its bounds has none.
    Window *resizeHandleAt(const base::PointF &location, bool touch, ResizeEdge *edge) const;

    // Whether this window is still managed. Callbacks are re-entrant, so a
    // pointer held across one has to be re-checked before it is used again.
    bool hasWindow(const Window *window) const;

    // --- Output and work area ---

    base::Rect outputBounds() const;
    base::Rect outputBoundsFor(const Window *window) const;
    void setWindowOutput(Window *window, const std::string &waylandName);
    void clearWindowOutput(Window *window);
    void setOutputBounds(const base::Rect &bounds);
    void setDisplayManager(display::DisplayManager *manager);

    display::DisplayManager *displayManager() const { return m_displayManager; }

    // Where normal and maximized windows are laid out: the output bounds minus
    // what the shell's panels reserved. A shell window is never confined to it
    // - the launcher covers the whole screen - so this is the output bounds for
    // those.
    base::Rect workAreaFor(const Window *window) const;
    // The same for `window` once it is at `bounds`, which may be on another output.
    base::Rect workAreaFor(const Window *window, const base::Rect &bounds) const;
    // What a normal window must stay partly inside while moved: the work area,
    // but down to the output's bottom edge so it can slide under the dock.
    base::Rect visibleAreaFor(const Window *window) const;
    base::Rect visibleAreaFor(const Window *window, const base::Rect &bounds) const;
    void setWorkArea(const base::Rect &bounds);

    // The output bounds a new ordinary window goes to: the active application
    // window's output, else the one under the pointer, else the primary one.
    base::Rect outputForNewWindow() const;
    // The work area of the output with `outputBounds`, or an empty rect when
    // there is no such output any more.
    base::Rect workAreaForOutput(const base::Rect &outputBounds) const;
    // The work area with every reservation in place, maximized windows or
    // not: what the shell's own surfaces and the desktop lay themselves out
    // in, so they stay clear of the dock and keep still as windows maximize.
    base::Rect restingWorkAreaForOutput(const base::Rect &outputBounds) const;
    // The work area an ordinary window opens in: the reservations that yield
    // are left out while the dock on the output is hidden.
    base::Rect placementWorkAreaForOutput(const base::Rect &outputBounds) const;
    // Where a new ordinary window at `bounds`, centered in its work area, goes
    // when its position is managed: next to its placement anchor, back where
    // the last window with its restore id was left, or cascaded; see
    // window_positioner.h.
    base::Rect boundsForNewWindow(const Window *window, base::Rect bounds) const;
    // Moves a normal window's client area to `position`, in global
    // coordinates, keeping it reachable; see
    // lingmo_window_placement_v1.set_position. Maximized, fullscreen,
    // snapped and dragged windows stay where they are.
    void moveWindowTo(Window *window, const base::Point &position);

    struct OutputArea {
        base::Rect bounds;
        base::Rect workArea;
        double scale = 1;
        // The wl_output name.
        std::string name;
        bool primary = false;

        bool operator==(const OutputArea &) const = default;
    };
    // Every output, or the single output bounds without a display manager,
    // with its resting work area.
    std::vector<OutputArea> outputAreas() const;

    // --- input::EventSink ---

    bool onPointerMotion(input::Timestamp timestamp, const base::PointF &location) override;
    bool onPointerButton(input::Timestamp timestamp, const base::PointF &location, uint32_t evdevButton,
                         bool pressed) override;
    bool onPointerAxis(input::Timestamp timestamp, const base::PointF &location, const base::PointF &offset,
                       bool discrete) override;
    bool onPointerAxisStop(input::Timestamp timestamp, const base::PointF &location) override;
    bool onKey(input::Timestamp timestamp, uint32_t evdevCode, bool pressed) override;
    bool onTouchDown(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchMotion(input::Timestamp timestamp, int32_t id, const base::PointF &location) override;
    bool onTouchUp(input::Timestamp timestamp, int32_t id) override;
    void onTouchCancel(input::Timestamp timestamp) override;

    // --- ActivationObserver ---

    void onWindowActivated(Window *gainedActive, Window *lostActive) override;

    base::Signal<Window *> windowAdded;
    base::Signal<Window *> windowRemoved;
    base::Signal<Window *> activeWindowChanged;
    // Every pointer motion, before any drag, grab or frame takes it: what the
    // dock's auto-hide watches, since it must see the pointer reach the screen
    // edge whatever else is going on.
    base::Signal<const base::PointF &> pointerMoved;
    // What lies under a still pointer may have changed: a window was shown,
    // hidden, removed, restacked, or moved or resized across the pointer.
    base::Signal<> pointerTargetsChanged;
    // An output came, went or changed its bounds, work area or scale; windows
    // left on no output have been brought back by now.
    base::Signal<> outputsChanged;
    // A window's bounds moved: their top left changed.
    base::Signal<Window *> windowMoved;
    // A reservation came, went or changed: the work areas may have.
    base::Signal<> exclusiveZonesChanged;
    // A popup took a grab or let it go.
    base::Signal<> popupGrabsChanged;

private:
    bool hasApplicationWindow(int64_t process) const;

    // A press on a candidate popup picks a candidate: the window typed into
    // keeps the keyboard, and its menus stay open.
    bool isInputMethodPress(const base::PointF &location) const;

    // Puts `window` at the top of its own layer in m_stackingOrder.
    void insertIntoStackingOrder(Window *window);
    void applyExclusiveZones();
    // `workArea` of the output with `outputBounds` minus the reservations.
    base::Rect zonedWorkArea(const base::Rect &workArea, const base::Rect &outputBounds,
                             const std::string &outputName, bool primary) const;
    void notifyWorkAreaChanged();
    // Puts `bounds` where the window's surface asked to map, kept reachable;
    // false when it did not ask.
    bool requestedPlacement(const Window *window, base::Rect *bounds) const;
    // Emits outputsChanged() when outputAreas() differs from the last time.
    void notifyOutputsChanged();
    // Keeps windows where they were on their displays across a layout change.
    // Runs before anything fits windows to the new layout.
    void followDisplays();
    bool isInsidePopupGrabChain(compositor::Surface *surface) const;
    // The window the outermost grabbing popup hangs from.
    Window *popupGrabRoot() const;
    // The status bar the grabbing popups hang from, unless they hang from a
    // status item in it. It switches between its panels and menus, or closes
    // them, itself, as a menu bar does.
    bool isPopupGrabMenuBar(const Window *window) const;
    // `surface` is, or is inside, the surface a grabbing popup hangs from.
    bool isPopupGrabAnchor(compositor::Surface *surface) const;
    // Whether a press outside the grab chain at `target` dismisses the chain
    // and is kept from the client, or dismisses it and reaches the window:
    // the bar a client's status item popups hang from.
    bool pressReachesPopupGrabRoot(const Window *target) const;
    // While grabbing popups hang from a window that never takes the keyboard,
    // the status bar or the dock, that window borrows it, as Ash reroutes keys
    // to a tray bubble: the client hands them to its popups, and the
    // application keeps activation.
    void updatePopupKeyboard();
    bool isCaptureWindow(const Window *window) const;
    // The window a pointer event at `location` is aimed at, frames included,
    // when a modal transient blocks it.
    Window *modalBlockedWindowAt(const base::PointF &location) const;
    bool handlePointerButton(input::Timestamp timestamp, const base::PointF &location, uint32_t code,
                             bool pressed);
    bool isPopupSurface(compositor::Surface *surface) const;
    // A popup under `location` takes client input over any window handle.
    bool isPopupAt(const base::PointF &location) const;
    // The topmost visible window drawn at `location`, frame included.
    Window *drawnWindowAt(const base::PointF &location) const;
    // The frame currently holding a pressed caption button, if any.
    Window *windowTrackingFramePress() const;
    void setFrameHover(Window *window);
    // Click to activate, which also ends a keyboard grab the press is not in.
    void activateForPress(Window *window);
    // Hands the keyboard back to the active window.
    void releaseKeyboardGrab();
    // Hangs a Sheet from the application window `parentId`, or from nothing.
    void attachSheet(Window *sheet, uint64_t parentId);
    // Where a window with a restore id is, kept as it unmaps and given back
    // to the next one with the same id; see set_restore_id.
    void rememberPlacement(const Window *window);
    bool restorePlacement(const Window *window, base::Rect *bounds) const;

    compositor::Compositor *m_compositor = nullptr;
    input::Seat *m_seat = nullptr;
    input::SessionGate *m_sessionGate = nullptr;
    decoration::WindowVisualStyle m_visualStyle;
    FrameProvider *m_frameProvider = nullptr;
    // The window whose frame last saw the pointer, so it can be told when the
    // pointer leaves it.
    Window *m_frameHover = nullptr;
    scene::Node *m_dragOverlay = nullptr;
    Window *m_dragOverlayBelow = nullptr;

    std::vector<std::unique_ptr<Window>> m_windows;
    // Bottom to top. Mirrors m_windows but is reordered by raise().
    std::vector<Window *> m_stackingOrder;
    std::function<bool(int64_t)> m_systemDialogPolicy;

    Activation m_activation;
    Window *m_keyboardGrab = nullptr;
    // The window updatePopupKeyboard() lent the keyboard to, and the surface
    // inside it that has it when the chain hangs from one.
    Window *m_popupKeyboardRoot = nullptr;
    compositor::Surface *m_popupKeyboardSurface = nullptr;
    std::unique_ptr<base::Timer> m_pasteTimer;
    std::chrono::steady_clock::time_point m_pasteDeadline;
    std::unique_ptr<TransientManager> m_transientManager;
    std::unique_ptr<MruWindowTracker> m_mruTracker;
    std::unique_ptr<DesksController> m_desks;
    std::unique_ptr<OverviewController> m_overview;
    std::unique_ptr<ShowDesktopController> m_showDesktop;
    std::unique_ptr<StatusItems> m_statusItems;
    std::vector<scene::Node *> m_deskOverlays;
    bool m_dockTransitionsAbovePanels = true;
    std::unique_ptr<ToplevelEventHandler> m_eventHandler;
    Placement m_placement;
    WindowModalityController m_modality;
    display::DisplayManager *m_displayManager = nullptr;
    base::Subscription m_displaysChanged;
    // The last layout with a display, which windows are placed relative to.
    std::vector<display::LogicalDisplay> m_previousDisplays;
    base::Rect m_outputBounds;
    base::Rect m_workArea;
    std::vector<OutputArea> m_outputAreas;
    ExclusiveZones m_exclusiveZones;
    // The outputs whose yielding reservations the work areas last left out.
    std::vector<base::Rect> m_maximizedOutputs;
    std::set<const Window *> m_hiddenDocks;
    std::map<compositor::Surface *, ShellWindowMetadata> m_shellMetadata;
    std::map<const Window *, std::string> m_windowOutputs;
    struct RememberedPlacement {
        base::Rect bounds;
        base::Rect output;
    };
    // By application id and restore id. Both come from clients, so the least
    // recently used go once there are kMaxRememberedPlacements.
    static constexpr size_t kMaxRememberedPlacements = 256;
    mutable base::LruCache<std::string, RememberedPlacement> m_rememberedPlacements {
            kMaxRememberedPlacements};

    std::map<compositor::Surface *, Window *> m_surfaceOwners;
    std::vector<compositor::Surface *> m_auxiliarySurfaces;
    // Outermost first.
    std::vector<ShellSurface *> m_popupGrabs;
    // Buttons whose press the window manager kept from clients: their release
    // is kept too, or a client would see a release without a press.
    std::set<uint32_t> m_consumedButtons;
    // Touch points that went down on a window's frame.
    std::map<int32_t, Window *> m_frameTouches;
    std::function<void(int64_t pid)> m_terminateProcess;
    std::function<void(int64_t pid)> m_forceQuitProcess;
    std::function<bool(Window *)> m_notRespondingPrompt;
    std::chrono::milliseconds m_quitGracePeriod {3000};
    base::WeakPtrFactory<WindowManager> m_weakFactory {this};
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_MANAGER_H_
