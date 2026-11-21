// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/window_state.{h,cc}, base_state.* and
// default_state.* (revision 7acd40ddefff49c9d36144999aafdbb674dbd07a).
//
// What is taken from Ash: the event-driven state object per window, the
// save-on-leaving-normal restore bounds, the restore-history stack with its
// layer ordering, and the two snapped states. What is dropped: floated, pip and
// pinned, tablet mode, desks and displays.

#ifndef LINGMO_WM_WINDOW_STATE_H_
#define LINGMO_WM_WINDOW_STATE_H_

#include <optional>
#include <vector>

#include "base/geometry.h"
#include "wm/types.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class WindowAnimation;
}

namespace Lingmo::WM {

class Window;
class WMEvent;

class WindowState
{
public:
    explicit WindowState(Window* window);
    WindowState(const WindowState&) = delete;
    WindowState& operator=(const WindowState&) = delete;
    ~WindowState();

    Window* window() const { return m_window; }

    WindowStateType stateType() const { return m_type; }

    bool isNormal() const { return m_type == WindowStateType::Normal; }
    bool isMaximized() const { return m_type == WindowStateType::Maximized; }
    bool isFullscreen() const { return m_type == WindowStateType::Fullscreen; }
    bool isMinimized() const { return m_type == WindowStateType::Minimized; }

    bool isSnapped() const { return isSnappedStateType(m_type); }

    // Whether this window could take a snapped state at all: a window that is
    // not resizable, or whose minimum size does not fit in half the work area,
    // cannot.
    bool canSnap() const;

    // The single entry point for state change.
    void onEvent(const WMEvent& event);

    // --- Restore bounds ---

    bool hasRestoreBounds() const { return m_restoreBounds.has_value(); }
    base::Rect restoreBounds() const { return m_restoreBounds.value_or(base::Rect()); }
    void setRestoreBounds(const base::Rect& bounds) { m_restoreBounds = bounds; }
    void clearRestoreBounds() { m_restoreBounds.reset(); }
    void saveCurrentBoundsForRestore();

    // The state a Restore event would land in. Normal when there is no history.
    WindowStateType restoreStateType() const;

    // The dock slot moved: a running minimize lands on the new one.
    void updateMinimizeTarget();

private:
    // Hands the window's last frame to Window::minimizedSnapshot once rendered.
    void requestMinimizedSnapshot(scene::WindowAnimation* animation);
    void enterState(WindowStateType next);
    void updateBoundsFromState(WindowStateType previous);
    void updateVisibility(WindowStateType previous);
    void updateRestoreHistory(WindowStateType previous);
    void setBoundsDirect(const base::Rect& bounds);

    Window* m_window = nullptr;
    WindowStateType m_type = WindowStateType::Normal;
    std::optional<base::Rect> m_restoreBounds;

    // Bottom to top. Only states strictly below the current one in the layer
    // order can be on it, so it never grows past the number of states.
    std::vector<WindowStateType> m_restoreHistory;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WINDOW_STATE_H_
