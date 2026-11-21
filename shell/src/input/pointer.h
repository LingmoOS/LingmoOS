// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/pointer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Cursor surfaces are composed directly in the scene; drag capture belongs to Seat.

#ifndef LINGMO_INPUT_POINTER_H_
#define LINGMO_INPUT_POINTER_H_

#include <memory>
#include <optional>
#include <vector>


#include "compositor/surface_observer.h"
#include "base/geometry.h"
#include "input/cursor_shape.h"
#include "input/pointer_gesture_delegate.h"
#include "input/types.h"

namespace Lingmo::Input {

class PointerDelegate;
class RelativePointerDelegate;
struct RelativeMotion;
class Seat;

// A client pointer, representing one or more physical pointing devices. It
// owns the pointer focus and the cursor image for its client.
class Pointer : public compositor::SurfaceObserver
{
public:
    // The delegate outlives the pointer, or destroys itself from
    // onPointerDestroying(); this mirrors Exo, where the Wayland delegate is
    // owned by the wl_pointer resource.
    Pointer(PointerDelegate* delegate, Seat* seat);
    Pointer(const Pointer&) = delete;
    Pointer& operator=(const Pointer&) = delete;
    ~Pointer() override;

    PointerDelegate* delegate() const { return m_delegate; }

    // Exo keeps one; the protocol lets a client ask for several.
    void addRelativePointerDelegate(RelativePointerDelegate* delegate);
    void removeRelativePointerDelegate(RelativePointerDelegate* delegate);

    // Exo keeps one pinch delegate; here a client may ask for a swipe, a pinch
    // and a hold object, or several of each.
    void addGestureDelegate(PointerGestureDelegate* delegate);
    void removeGestureDelegate(PointerGestureDelegate* delegate);

    compositor::Surface* focusedSurface() const { return m_focusSurface; }

    compositor::Surface* cursorSurface() const { return m_cursorSurface; }

    // Includes an explicit null/hide request; cleared on focus loss or image
    // destruction. Physical positioning never changes this ownership.
    bool hasCursorRequest() const { return m_hasCursorRequest; }

    std::optional<CursorShape> cursorShape() const { return m_cursorShape; }

    void setCursorShape(CursorShape shape);

    void setCursorPosition(const base::PointF& position);

    base::Point hotspot() const { return m_hotspot; }

    // wl_pointer.set_cursor. `surface` becomes the pointer image, positioned so
    // that `hotspot` sits under the pointer location. A null surface hides the
    // cursor.
    void setCursor(compositor::Surface* surface, const base::Point& hotspot);

    // Drops focus without moving the pointer: the compositor took the pointer
    // over content of its own, a server-side window frame, and the client must
    // see a leave rather than stale motion.
    void clearFocus();

    // Called by the Seat.
    void onPointerMotion(Timestamp timestamp, const base::PointF& rootLocation,
                         const RelativeMotion* relative = nullptr);
    void onPointerButton(Timestamp timestamp, const base::PointF& rootLocation,
                         ButtonFlags changedButtonFlags, bool pressed);
    void onPointerAxis(Timestamp timestamp, const base::PointF& rootLocation, const base::PointF& offset,
                       bool discrete);
    void onPointerAxisStop(Timestamp timestamp, const base::PointF& rootLocation);
    void onPointerGestureBegin(Timestamp timestamp, const base::PointF& rootLocation,
                               GestureType type, uint32_t fingers);
    void onPointerGestureUpdate(Timestamp timestamp, GestureType type,
                                const GestureUpdate& update);
    void onPointerGestureEnd(Timestamp timestamp, GestureType type, bool cancelled);

    void traceCursor(const char* transition) const;

    // Complete a press already delivered to this pointer without retargeting
    // through compositor decorations or another client under a compositor-owned drag.
    void releasePressedButtons(Timestamp timestamp, ButtonFlags buttons);

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface* surface) override;
    void onSurfaceCommit(compositor::Surface* surface) override;

private:
    // The target for a location in output coordinates, with the location
    // converted into that surface's coordinates. Null if no surface this
    // pointer may talk to is under the location.
    compositor::Surface* effectiveTargetForLocation(const base::PointF& rootLocation,
                                                    base::PointF* locationInTarget) const;

    // Resolves the target for `rootLocation` and moves focus if it changed.
    // Returns the current focus afterwards.
    compositor::Surface* updateFocus(const base::PointF& rootLocation, base::PointF* locationInTarget);

    void setFocus(compositor::Surface* surface, const base::PointF& rootLocation,
                  const base::PointF& surfaceLocation);

    // Ends a gesture in progress as cancelled: its surface lost focus or went
    // away, and the rest of it must not reach another surface.
    void cancelGesture();

    void updatePointerSurface(compositor::Surface* surface);
    void updateCursor();

    bool shouldObserveSurface(compositor::Surface* surface) const;
    void maybeRemoveSurfaceObserver(compositor::Surface* surface);

    // Whether the pointer has effectively not moved, in which case Exo
    // suppresses the motion event.
    bool checkIfSameLocation(const base::PointF& locationInRoot, const base::PointF& locationInTarget) const;

    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    PointerDelegate* m_delegate = nullptr;
    std::vector<RelativePointerDelegate*> m_relativeDelegates;
    std::vector<PointerGestureDelegate*> m_gestureDelegates;
    // The gesture in progress, which began on the focused surface.
    std::optional<GestureType> m_gesture;
    Seat* m_seat = nullptr;

    compositor::Surface* m_focusSurface = nullptr;

    // Set once focus has been taken; a client that never calls set_cursor gets
    // no pointer image, per the protocol.
    compositor::Surface* m_cursorSurface = nullptr;
    base::Point m_hotspot;
    bool m_hasCursorRequest = false;
    std::optional<CursorShape> m_cursorShape;
    ButtonFlags m_deliveredButtons = NoButton;

    base::PointF m_cursorPosition;
    base::PointF m_locationInRoot;
    base::PointF m_locationInSurface;
    bool m_hasLocation = false;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_POINTER_H_
