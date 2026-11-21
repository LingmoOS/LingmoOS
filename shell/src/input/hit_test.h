// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/shell_surface_util.cc
//           (GetTargetSurfaceForLocatedEvent)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Adaptation: Exo delegates the descent to aura::WindowTargeter, which walks
// the window tree and consults Surface::HitTest at each level. Lingmo has no
// window tree yet, so the same descent is done directly over the subsurface
// stacking order.

#ifndef LINGMO_INPUT_HIT_TEST_H_
#define LINGMO_INPUT_HIT_TEST_H_

#include "base/geometry.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {
class Compositor;
class Surface;
} // namespace Lingmo::Compositor

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::Input {

class SessionGate;

// The deepest surface in `root`'s hierarchy whose own input region contains
// `location` (given in `root`-local coordinates), searched top of the stacking
// order first. `surfaceLocation` receives the point in the returned surface's
// coordinates.
compositor::Surface *findTargetSurface(compositor::Surface *root, const base::PointF &location,
                                       base::PointF *surfaceLocation);

// The same search across every root surface in the scene, topmost first.
// `location` is in output coordinates.
compositor::Surface *findTargetSurface(compositor::Compositor *compositor, const base::PointF &location,
                                       base::PointF *surfaceLocation);

// The same search confined to `container`'s subtree. A blocked session searches
// only the secure container, which is how a locked screen can be clicked while
// nothing behind it can. The session's surface container comes after the input
// method's candidate popups. Null `container` finds nothing.
compositor::Surface *findTargetSurfaceIn(compositor::Compositor *compositor, scene::Node *container,
                                         const base::PointF &location, base::PointF *surfaceLocation);

// The same search looking through what captures leave out, so the screen
// capture interface can pick what is under it. Upstream: ash's
// capture_mode_util::GetTopMostCapturableWindowAtPoint.
compositor::Surface *findCapturedSurfaceIn(compositor::Compositor *compositor,
                                           scene::Node *container, const base::PointF &location);

// Whether `surface` is, or is part of, an input method candidate popup.
bool isInputMethodSurface(const compositor::Compositor *compositor,
                          const compositor::Surface *surface);

// The scene subtree input may target, honouring `gate`. Null when the gate
// leaves nothing targetable.
scene::Node *inputRootFor(compositor::Compositor *compositor, const SessionGate *gate);

// `location`, given in output coordinates, in `surface`'s own coordinates.
// Unlike the hit test this ignores the input region and the stacking order: it
// is how an implicit grab keeps reporting locations outside the grabbed
// surface. Counterpart of aura::Window::ConvertPointToTarget.
base::PointF locationInSurface(const compositor::Surface *surface, const base::PointF &location);

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_HIT_TEST_H_
