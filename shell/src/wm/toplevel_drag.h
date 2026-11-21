// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/components/exo/extended_drag_source.* (revision
// 7acd40ddefff49c9d36144999aafdbb674dbd07a), for xdg_toplevel_drag_v1 instead
// of zcr_extended_drag_v1.
//
// What is taken: a window attached before it maps is put under the pointer at
// its offset when it does, without an open animation; the window moves through
// the same interactive move as a caption drag, fed by the data-device drag
// that holds the pointer; it takes no part in picking the drop target; a drop
// ends the move where it is and a cancel reverts it; unmapping the window
// detaches it. What is dropped: cursor locking, the pending start raced
// against the attach, and the tab-dragging window property.
//
// What is added: a window that maps during the drag is a preview until it is
// let go. It is drawn as a translucent thumbnail around the pointer, and grows
// to its size when dropped; it cannot be activated, so the window it was
// pulled out of keeps the keyboard; dropped where nothing takes it, it becomes
// an ordinary window and is activated. Over a window of its own process that
// takes the drop it is not drawn at all, from the frame that window next
// commits, which is the one where the client shows where the drop would go. A
// window dropped on another window of its own process, and unmapped by its
// client for it, hands the activation to that window. Chrome activates the tab
// strip it merges into itself, but Qt spends the pointer press serial on
// showing the dragged window, so the client cannot.

#ifndef LINGMO_WM_TOPLEVEL_DRAG_H_
#define LINGMO_WM_TOPLEVEL_DRAG_H_

#include <cstdint>
#include <memory>
#include <optional>

#include "base/geometry.h"
#include "base/signal.h"
#include "compositor/surface_observer.h"
#include "input/drag.h"
#include "scene/window_animation.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Clipboard {
class DataSource;
}

namespace Lingmo::WM {

class Window;
class WindowManager;

class ToplevelDrag : public compositor::SurfaceObserver,
                     public input::DragWindowDelegate
{
public:
    ToplevelDrag(WindowManager *manager, std::shared_ptr<clipboard::DataSource> source);
    ~ToplevelDrag() override;

    // Whether a toplevel is attached that has not been unmapped since.
    bool hasToplevel() const { return m_surface != nullptr; }

    // Whether the drag this source started is still running.
    bool isDragging() const { return m_dragging; }

    // `offset` is the pointer's place in the window geometry of `surface`,
    // which is a toplevel's root surface, mapped or not.
    void attach(compositor::Surface *surface, const base::Point &offset);

    // --- input::DragWindowDelegate ---

    void dragMoved(const base::PointF &location) override;
    void dragTargetChanged(compositor::Surface *target, bool accepted) override;
    void dragEnded(bool released) override;

    // --- compositor::SurfaceObserver ---

    void onSurfaceDestroying(compositor::Surface *surface) override;
    void onSurfaceCommit(compositor::Surface *surface) override;

    // How opaque a preview is drawn, and how wide at most.
    static constexpr float kPreviewOpacity = 0.85f;
    static constexpr double kPreviewWidth = 320;
    static constexpr double kPreviewMaxScale = 0.5;
    // How long a client has after the drag to take its preview back before
    // it is shown as an ordinary window anyway.
    static constexpr int kSettleDelayMs = 500;

private:
    void onDragStarted();
    void onWindowAdded(Window *window);
    void onWindowRemoved(Window *window);
    Window *windowForSurface() const;
    void startMove(Window *window);
    void stopMove();
    void detach();
    void unwatchDockTarget();

    WindowManager *m_manager = nullptr;
    std::shared_ptr<clipboard::DataSource> m_source;
    compositor::Surface *m_surface = nullptr;
    base::Point m_offset;
    bool m_dragging = false;
    Window *m_moving = nullptr;
    // The window a released drag was dropped on, while the dragged one is
    // still mapped. Zero otherwise.
    uint64_t m_dropWindow = 0;
    // The preview, while the drag runs and a little after.
    uint64_t m_preview = 0;
    // The width its thumbnail scale was worked out for.
    int m_previewWidth = 0;
    // The window of the same process under the drag that takes the drop, or
    // zero. The preview is not drawn over it from its next commit, which the
    // root surface below is watched for.
    uint64_t m_docked = 0;
    compositor::Surface *m_dockTarget = nullptr;
    std::optional<scene::WindowAnimation::Style> m_animationStyle;
    base::Subscription m_dragStarted;
    base::Subscription m_windowAdded;
    base::Subscription m_windowRemoved;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_TOPLEVEL_DRAG_H_
