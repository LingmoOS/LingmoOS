// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on the snap and maximize half of
// chromium/src/ash/wm/workspace/workspace_window_resizer.cc (revision
// 7acd40ddefff49c9d36144999aafdbb674dbd07a): the edge detection, the dwell
// countdown that turns a drag against the top of the screen into a maximize,
// and the phantom window that previews where the window will land.
//
// What is dropped: portrait orientation, multi-display, touch, haptics, the
// window splitter and the per-window snap ratio.

#ifndef LINGMO_WM_SNAP_DRAG_CONTROLLER_H_
#define LINGMO_WM_SNAP_DRAG_CONTROLLER_H_

#include <optional>

#include "base/geometry.h"
#include "base/timer.h"
#include "wm/window_snap.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class SnapPreviewNode;
}

namespace Lingmo::WM {

class Window;
class WindowManager;

class SnapDragController
{
public:
    explicit SnapDragController(WindowManager *manager);
    SnapDragController(const SnapDragController &) = delete;
    SnapDragController &operator=(const SnapDragController &) = delete;
    ~SnapDragController();

    // `start` is the pointer position the move began at, in output coordinates.
    void begin(Window *window, const base::PointF &start);

    // Only call while the window is actually being moved: a drag that has not
    // passed its threshold yet, and a resize, never snap.
    void update(const base::PointF &location);

    // Hides the preview and forgets the drag. Returns what the drag had
    // settled on, which is what the caller then applies.
    SnapType end();

    SnapType snapType() const { return m_snapType; }

    // Runs the dwell countdown to completion now, instead of waiting for the
    // timer. For tests, which have no event loop to spin.
    void triggerDwellForTesting() { fireDwell(); }

private:
    void startDwell();
    void stopDwell();
    void fireDwell();
    void setSnapType(SnapType type);
    void updatePreview();
    void clearPreview();
    SnapType constrained(SnapType type) const;
    bool isBelowTopEdge(const base::PointF &location) const;

    WindowManager *m_manager = nullptr;
    Window *m_window = nullptr;
    SnapType m_snapType = SnapType::None;
    // Ash's can_snap_to_maximize_, but gated on having been below the top edge
    // rather than on 64px of vertical travel, which a window already sitting
    // under the status bar can never make.
    bool m_canSnapToMaximize = false;
    std::optional<base::PointF> m_dwellLocation;
    // The countdown's state, not the timer's: a test with no event loop to run
    // the timer in still has to see the same state machine.
    bool m_dwellPending = false;
    base::Timer m_dwellTimer;
    scene::SnapPreviewNode *m_preview = nullptr;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_SNAP_DRAG_CONTROLLER_H_
