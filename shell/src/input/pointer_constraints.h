// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/pointer.h (pointer constraints)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: Exo keeps constraints on each client Pointer and
// captures through aura. Here every client pointer drives the one physical
// pointer, so the constraints belong to the Seat and the InputManager applies
// the active one to motion.

#ifndef LINGMO_INPUT_POINTER_CONSTRAINTS_H_
#define LINGMO_INPUT_POINTER_CONSTRAINTS_H_

#include <functional>
#include <map>
#include <optional>


#include "compositor/surface_observer.h"
#include "base/geometry.h"
#include "base/region.h"

namespace Lingmo::Input {

class PointerConstraintDelegate;
class Seat;

// At most one constraint per surface, and at most one active: the one on the
// surface with pointer focus, in the active window, once the pointer is inside
// its region.
class PointerConstraints : public compositor::SurfaceObserver
{
public:
    explicit PointerConstraints(Seat *seat);
    PointerConstraints(const PointerConstraints &) = delete;
    PointerConstraints &operator=(const PointerConstraints &) = delete;
    ~PointerConstraints() override;

    // Exo's Pointer::ConstrainPointer. A surface that already has a constraint
    // refuses the new one, which is then told so and is defunct.
    bool add(PointerConstraintDelegate *delegate);

    // The delegate is being destroyed.
    void remove(PointerConstraintDelegate *delegate);

    PointerConstraintDelegate *active() const { return m_active; }

    bool isLocked() const;

    // Re-evaluates the active constraint after anything it depends on changed:
    // pointer focus, the active window, the pointer location, a committed
    // region, a drag or the session lock.
    void update();

    // Where motion from `from` to `to`, both in output coordinates, ends under
    // the active confinement.
    base::PointF confine(const base::PointF &from, const base::PointF &to) const;

    // Exo's UnconstrainPointerByUserAction: breaks the active constraint and
    // refuses any until a click permits them again, so an application cannot
    // take the pointer straight back. Returns whether one was active.
    bool breakByUserAction();

    // A press on `surface`, which permits capture again if it is constrained.
    void onPointerPressed(compositor::Surface *surface);

    // Moves the pointer, with a motion event, where a lock's cursor hint puts
    // it once the lock ends. Set by the input manager.
    std::function<void(const base::PointF &)> warp;

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface *surface) override;

private:
    bool canActivate(PointerConstraintDelegate *delegate) const;

    // The delegate's region clipped to its surface's input region, in surface
    // coordinates.
    base::Region effectiveRegion(PointerConstraintDelegate *delegate) const;

    // Where the active lock's cursor hint puts the pointer, in output
    // coordinates.
    std::optional<base::PointF> hintPosition() const;

    // Exo's UnconstrainPointer and RemoveConstraintDelegate.
    void deactivate();
    void erase(PointerConstraintDelegate *delegate);

    Seat *m_seat;
    std::map<compositor::Surface *, PointerConstraintDelegate *> m_constraints;
    PointerConstraintDelegate *m_active = nullptr;
    bool m_capturePermitted = true;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_POINTER_CONSTRAINTS_H_
