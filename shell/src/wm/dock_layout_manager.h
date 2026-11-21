// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/shelf/shelf_layout_manager.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_WM_DOCK_LAYOUT_MANAGER_H_
#define LINGMO_WM_DOCK_LAYOUT_MANAGER_H_

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "core/main_loop.h"
#include "wm/types.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {

class Window;
class WindowManager;

// When the dock gives way to windows. The shell's dock setting, adapted.
// Upstream: ShelfAutoHideBehavior.
enum class DockAutoHideBehavior {
    Never,
    Always,
    // A maximized or fullscreen window on the dock's output pushes it away.
    WhenObstructed,
};

// Whether the dock is parked on screen for good, or comes and goes with the
// pointer and the workspace. Upstream: ShelfVisibilityState.
enum class DockVisibilityState {
    Visible,
    AutoHide,
};

// Where an auto-hiding dock is right now. Upstream: ShelfAutoHideState.
enum class DockAutoHideState {
    Shown,
    Hidden,
};

// What the windows on an output ask of the dock. Upstream: WorkspaceWindowState.
enum class WorkspaceState {
    Default,
    Maximized,
    Fullscreen,
};

// Decides, for every dock and panel the shell maps, whether it is on
// screen right now, and slides it off its screen edge when it is not. The dock
// itself only draws: it never unmaps, and it learns nothing about the pointer.
// A panel such as the status bar follows the same rules, minus the setting: it
// only gives way to a fullscreen window.
//
// Two decisions, as upstream: the visibility state comes from the setting and
// from the windows on the dock's output; the auto-hide state, only meaningful
// under AutoHide, from what would be interrupted by hiding - the pointer on the
// dock or at its edge, a menu of the dock's, the launcher - and from whether
// there is a window to make room for at all. Showing, and hiding because the
// pointer left, wait kAutoHideDelayMs; anything else hides at once.
// A dock or panel that maps slides in from its edge, as the session starts.
// The window overview pushes the panels off their edge as it comes in,
// following its progress, and lets them back as it leaves.
//
// Must be destroyed before the WindowManager it observes.
class DockLayoutManager : public WindowObserver
{
public:
    // The strip along the dock's screen edge that brings a hidden dock back.
    static constexpr int kRevealStripPx = 5;
    // Upstream: kAutoHideDelayMS.
    static constexpr int kAutoHideDelayMs = 200;
    static constexpr int kSlideDurationMs = 200;

    explicit DockLayoutManager(WindowManager *manager);
    ~DockLayoutManager() override;

    DockAutoHideBehavior behavior() const { return m_behavior; }
    void setBehavior(DockAutoHideBehavior behavior);

    // While the session starts behind the startup cover every dock and panel
    // waits past its edge, and slides in once it is released.
    void setSessionStarting(bool starting);

    // While the session ends every dock and panel slides away and the desktop's
    // icons go, leaving the wallpaper; they come back if it carries on.
    void setSessionEnding(bool ending);

    // Both answer for a window this manager treats as a dock; anything else is
    // Visible and Shown.
    DockVisibilityState visibilityState(const Window *dock) const;
    DockAutoHideState autoHideState(const Window *dock) const;

    WorkspaceState workspaceStateFor(const base::Rect &output) const;

    // Re-evaluates every dock now. Every observer below funnels into this.
    void update();

    // Moves every running slide, and the panels with the overview, to
    // `frameTimeMs`, on the clock output frames are timed with. Called before
    // each frame is drawn; the animation asks for the next frame itself while
    // it runs.
    void advance(double frameTimeMs);
    bool isSliding(const Window *dock) const;

    // --- WindowObserver ---

    void onWindowBoundsChanged(Window *window, const base::Rect &oldBounds) override;
    void onWindowStateTypeChanged(Window *window, WindowStateType oldType) override;
    void onWindowVisibilityChanged(Window *window, bool visible) override;
    void onWindowTransientParentChanged(Window *window, Window *oldParent) override;

private:
    void onWindowAdded(Lingmo::WM::Window *window);
    void onWindowRemoved(Lingmo::WM::Window *window);
    void onPointerMoved(const base::PointF &location);

private:
    struct Dock {
        DockVisibilityState visibility = DockVisibilityState::Visible;
        DockAutoHideState autoHide = DockAutoHideState::Shown;
        ScreenEdge edge = ScreenEdge::Bottom;
        base::Timer autoHideTimer {core::mainLoop()};
        // Upstream: last_seen_mouse_position_was_over_shelf_.
        bool pointerOver = false;
        bool sliding = false;
        double slideStarted = -1;
        base::Point slideFrom;
        base::Point slideTo;
        // Where its own state puts it, before the overview: drawn this far
        // from its bounds, and not drawn at all when parked.
        base::Point offset;
        bool parked = false;
    };

    Dock *dockFor(Window *window);
    const Dock *dockFor(const Window *window) const;
    // A dock or panel just mapped starts past its screen edge and slides in,
    // without the reveal delay; one its state hides stays parked there.
    void enter(Window *dock, Dock &entry);
    void updateDock(Window *dock, Dock &entry);
    void updateAutoHideStateNow(Window *dock);
    DockVisibilityState computeVisibility(const Window *dock, const base::Rect &output) const;
    // `revealing`: the dock is shown or on its way, so the pointer anywhere on
    // it keeps it, not only at the edge strip.
    DockAutoHideState computeAutoHideState(const Window *dock, const Dock &entry,
                                           const base::Rect &output, bool revealing) const;
    bool isAwayForSession() const { return m_sessionStarting || m_sessionEnding; }
    bool hasVisibleWindow(const base::Rect &output) const;
    bool isLauncherShowing(const base::Rect &output) const;
    void applyAutoHideState(Window *dock, Dock &entry, DockAutoHideState state);
    // Aims the dock at where its current state puts it: on screen, or just
    // past its edge. Animated unless the dock is already parked there.
    void slideTo(Window *dock, Dock &entry, const base::Point &offset);
    // Draws the dock where its own offset and the overview put it.
    void present(Window *dock, const Dock &entry);
    void requestFrame();

    WindowManager *m_manager = nullptr;
    DockAutoHideBehavior m_behavior = DockAutoHideBehavior::Never;
    bool m_sessionStarting = false;
    bool m_sessionEnding = false;
    std::map<Window *, std::unique_ptr<Dock>> m_docks;
    std::optional<base::PointF> m_pointer;
    // How far in the overview was at the last frame, 0 to 1.
    double m_overviewProgress = 0;
    std::vector<base::Subscription> m_managerSignals;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_DOCK_LAYOUT_MANAGER_H_
