// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/toplevel_window_event_handler.* and the
// drag-lifecycle half of ash/wm/workspace/workspace_window_resizer.*
// (revision 7acd40ddefff49c9d36144999aafdbb674dbd07a).
//
// What is taken: the drag state machine and its completion statuses, revert on
// Escape, refusing a second drag while one is running, dragging a maximized or
// snapped window out of its state by the caption, and settling the drag into
// whatever edge it ended against, and a drag that follows one touch point
// instead of the pointer. What is dropped: pinch and two-finger gestures,
// flings, window cycling, capture through aura and everything that starts a
// drag from a hit-test on a frame - the decoration is what calls
// beginMove/beginResize, and the shell surface is what validates xdg serials
// first.

#ifndef LINGMO_WM_TOPLEVEL_EVENT_HANDLER_H_
#define LINGMO_WM_TOPLEVEL_EVENT_HANDLER_H_

#include <memory>
#include <optional>

#include "base/geometry.h"
#include "base/signal.h"
#include "wm/window_snap.h"

namespace Lingmo::WM {

class SnapDragController;
class Window;
class WindowManager;
class WindowResizer;

class ToplevelEventHandler
{
public:
    enum class DragResult {
        Success,
        Revert,
        WindowDestroyed,
    };

    explicit ToplevelEventHandler(WindowManager *manager);
    ToplevelEventHandler(const ToplevelEventHandler &) = delete;
    ToplevelEventHandler &operator=(const ToplevelEventHandler &) = delete;
    ~ToplevelEventHandler();

    // `pointerPosition` is in output coordinates. Both refuse a window that is
    // minimized or fullscreen, and refuse to start while another drag is
    // running. A move also accepts a maximized or snapped window, which it
    // drags out of that state; a resize accepts a snapped one, which stays
    // snapped as long as its bounds still look snapped.
    //
    // With `touchId` the drag follows that touch point, which `pointerPosition`
    // is then the location of, and ends when it is lifted. Any client that has
    // the point is sent a cancel.
    bool beginMove(Window *window, const base::PointF &pointerPosition,
                   std::optional<int32_t> touchId = std::nullopt);
    bool beginResize(Window *window, ResizeEdge edge, const base::PointF &pointerPosition,
                     std::optional<int32_t> touchId = std::nullopt);

    bool isDragging() const { return m_resizer != nullptr; }

    bool isTouchDrag() const { return m_resizer && m_touchId; }

    Window *draggedWindow() const;
    // ResizeEdge::None while moving, or when no drag is running.
    ResizeEdge draggedEdge() const;

    SnapDragController *snapController() const { return m_snapController.get(); }

    // --- Event feed, from WindowManager's input sink ---

    // Each returns true if the event was consumed by the drag. The pointer
    // does not move or end a drag that follows a touch point, nor another
    // touch point one that follows the pointer.
    bool onPointerMotion(const base::PointF &location);
    bool onPointerButtonReleased();
    bool onTouchMotion(int32_t id, const base::PointF &location);
    bool onTouchUp(int32_t id);
    // The device cancelled its points: the drag ends where it is, as ash
    // completes a gesture drag on its gesture end.
    void onTouchCancel();
    // Escape reverts the drag.
    bool onKeyPressed(uint32_t evdevCode);

    void completeDrag(DragResult result);

    // Called by WindowManager just before `window` is destroyed.
    void onWindowRemoving(Window *window);

private:
    bool beginDrag(Window *window, ResizeEdge edge, const base::PointF &pointerPosition,
                   std::optional<int32_t> touchId);
    void drag(const base::PointF &location);
    // Applies what the finished drag settled on: an edge it ended against, or
    // the state a window dragged out of a tiled state lands in. Called with
    // the drag already detached from the handler.
    void settleDrag(WindowResizer *resizer, SnapType snapped, const base::Rect &initialRestoreBounds);
    void applyMotion(const base::PointF &location);
    void holdResize();
    void releaseHeldResize();

    WindowManager *m_manager = nullptr;
    // A resize takes one step per frame: the first move at once, the newest
    // of the rest when the next frame starts, as aura holds pointer moves
    // while a window resizes (WindowTreeHost::OnCompositingChildResizing).
    // Every step reconfigures the client and re-announces the window to the
    // shell, and a 1000 Hz mouse would do that a thousand times a second.
    bool m_holdingResize = false;
    std::optional<base::PointF> m_heldResize;
    std::optional<base::Subscription> m_frameStarted;
    std::unique_ptr<WindowResizer> m_resizer;
    std::optional<int32_t> m_touchId;
    std::unique_ptr<SnapDragController> m_snapController;
    // What the dragged window would restore to, captured before the drag
    // cleared it. Empty unless the drag started from a tiled state.
    base::Rect m_initialRestoreBounds;
    WindowStateType m_initialStateType = WindowStateType::Normal;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_TOPLEVEL_EVENT_HANDLER_H_
