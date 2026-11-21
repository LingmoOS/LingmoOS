// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/pointer_constraint_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Exo only locks. Confinement, the activation region and the lock's cursor
// hint are added here for desktop clients and Xwayland.

#ifndef LINGMO_INPUT_POINTER_CONSTRAINT_DELEGATE_H_
#define LINGMO_INPUT_POINTER_CONSTRAINT_DELEGATE_H_

#include <optional>


#include "base/geometry.h"
#include "base/region.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {

class PointerConstraintDelegate
{
public:
    enum class Kind {
        Lock,
        Confine,
    };

    virtual Kind kind() const = 0;

    // Called when the constraint is activated by the compositor.
    //
    // For non-persistent ("one-shot") constraints, this may be called 0 or 1
    // times. For persistent ones, it may be called again after
    // onConstraintBroken().
    virtual void onConstraintActivated() = 0;

    // Called when the constraint is not activated because one was already
    // requested on this surface.
    virtual void onAlreadyConstrained() = 0;

    // Called when this constraint is broken for any reason: a user action, the
    // pointer or the window focus moving elsewhere, a drag, a locked session.
    virtual void onConstraintBroken() = 0;

    // Called once the constraint will never be activated again.
    virtual void onDefunct() = 0;

    // Whether the constraint may be reactivated after being broken.
    virtual bool isPersistent() const = 0;

    // The surface this constraint is for; null once it is destroyed.
    virtual compositor::Surface* constrainedSurface() const = 0;

    // The committed region, in logical surface coordinates, where the pointer
    // must be for the constraint to activate and where a confinement keeps it.
    // Unset means the whole input region; either way it is clipped to it.
    virtual std::optional<base::Region> region() const = 0;

    // The committed position, in logical surface coordinates, a lock wants the
    // pointer at when it ends.
    virtual std::optional<base::PointF> cursorPositionHint() const = 0;

protected:
    virtual ~PointerConstraintDelegate() = default;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_POINTER_CONSTRAINT_DELEGATE_H_
