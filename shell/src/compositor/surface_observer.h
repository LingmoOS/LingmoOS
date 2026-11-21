// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/surface_observer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Reduced to the one notification input needs: focus cleanup.

#ifndef LINGMO_COMPOSITOR_SURFACE_OBSERVER_H_
#define LINGMO_COMPOSITOR_SURFACE_OBSERVER_H_

#include <functional>

namespace Lingmo::Compositor {

class Surface;

class SurfaceObserver
{
public:
    virtual ~SurfaceObserver() = default;

    // Called at the top of the surface's destructor, to give observers a
    // chance to remove themselves.
    virtual void onSurfaceDestroying(Surface* surface) = 0;

    // Called after wl_surface.commit has applied whatever state the commit
    // made current. Role owners - xdg-shell in particular - hang their own
    // commit handling off this.
    virtual void onSurfaceCommit(Surface* surface) { }

    // Called when the client sends wl_surface.commit, which may wait for its
    // buffer before it applies. A role owner takes its own double-buffered
    // state here and returns what installs it, run right before this
    // observer's onSurfaceCommit() for that commit.
    virtual std::function<void()> onSurfaceCommitRequested(Surface* surface) { return {}; }

    virtual void onScaleFactorChanged(Surface* surface) { }

    virtual void onSurfaceMenuChanged(Surface* surface) { }

    virtual void onSurfaceWindowHintsChanged(Surface* surface) { }

    // Validate cached state immediately before hierarchy application.
    virtual bool onSurfaceStateApplying(Surface* surface) { return true; }
};

} // namespace Lingmo::Compositor

#endif // LINGMO_COMPOSITOR_SURFACE_OBSERVER_H_
