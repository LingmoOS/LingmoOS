// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/shell_surface_util.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "input/hit_test.h"
#include "display/display_manager.h"
#include "input/session_gate.h"

#include <cmath>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "scene/node.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {

// The input region is an integer region, so the fractional part of a pointer
// location is dropped for the containment test only; the location handed to the
// client keeps its precision.
static base::Point flooredPoint(const base::PointF &point)
{
    return point.toFlooredPoint();
}

compositor::Surface *findTargetSurface(compositor::Surface *root, const base::PointF &location,
                                       base::PointF *surfaceLocation)
{
    if (!root || !root->isMapped())
        return nullptr;

    // Cheap rejection first: the hierarchy region covers this surface and every
    // subsurface, so a miss here cannot become a hit further down.
    if (!root->hitTestRegion().contains(flooredPoint(location)))
        return nullptr;

    // Topmost first. The entry whose surface is the parent marks where the
    // parent's own content sits among its children.
    const auto &stack = root->subSurfaceStack();
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        compositor::Surface *surface = it->first;
        if (surface == root)
            continue;
        const base::PointF childLocation = location - base::PointF(it->second);
        if (compositor::Surface *target =
                    findTargetSurface(surface, childLocation, surfaceLocation)) {
            return target;
        }
    }

    if (root->hitTest(flooredPoint(location))) {
        *surfaceLocation = location;
        return root;
    }
    return nullptr;
}

namespace {

// The topmost surface under `location` among `nodes`, which are siblings in the
// scene. Recurses through nodes that are not surfaces themselves.
compositor::Surface *findTargetSurfaceInNodes(compositor::Compositor *compositor,
                                              const std::vector<scene::Node *> &nodes,
                                              const base::PointF &location, base::PointF *surfaceLocation,
                                              bool throughCapture = false)
{
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        scene::Node *node = *it;
        if (!node->isEffectivelyVisible() || node->isEventTargetingBlocked()
            || !node->acceptsInput(location))
            continue;
        if (throughCapture && node->isExcludedFromCapture())
            continue;
        compositor::Surface *surface = compositor->surfaceForNode(node);
        if (!surface) {
            // A grouping node - a window, holding its frame and the client's
            // surface tree. Descend into it; the frame's own nodes have no
            // surface and are skipped the same way.
            if (compositor::Surface *target = findTargetSurfaceInNodes(
                        compositor, node->children(), location, surfaceLocation, throughCapture)) {
                return target;
            }
            continue;
        }
        // Subsurfaces are reached through their parent, never from the
        // container, and a surface reparented elsewhere (the cursor) is not
        // here at all.
        if (surface->parentSurface())
            continue;
        const base::PointF surfaceRelative = locationInSurface(surface, location);
        if (compositor::Surface *target =
                    findTargetSurface(surface, surfaceRelative, surfaceLocation)) {
            return target;
        }
    }
    return nullptr;
}

} // namespace

compositor::Surface *findTargetSurface(compositor::Compositor *compositor, const base::PointF &location,
                                       base::PointF *surfaceLocation)
{
    return findTargetSurfaceIn(compositor, compositor ? compositor->surfaceContainer() : nullptr,
                               location, surfaceLocation);
}

compositor::Surface *findTargetSurfaceIn(compositor::Compositor *compositor, scene::Node *container,
                                         const base::PointF &location, base::PointF *surfaceLocation)
{
    if (!compositor || !container)
        return nullptr;

    // Candidate popups stack above the session's windows; a blocked session
    // searches a container of its own and never reaches them.
    scene::Node *inputMethod = compositor->inputMethodContainer();
    if (inputMethod && container == compositor->surfaceContainer()) {
        if (compositor::Surface *target = findTargetSurfaceInNodes(
                    compositor, inputMethod->children(), location, surfaceLocation)) {
            return target;
        }
    }
    return findTargetSurfaceInNodes(compositor, container->children(), location, surfaceLocation);
}

bool isInputMethodSurface(const compositor::Compositor *compositor,
                          const compositor::Surface *surface)
{
    const scene::Node *layer = compositor ? compositor->inputMethodContainer() : nullptr;
    for (const scene::Node *node = surface ? surface->node() : nullptr; layer && node;
         node = node->parent()) {
        if (node == layer)
            return true;
    }
    return false;
}

compositor::Surface *findCapturedSurfaceIn(compositor::Compositor *compositor,
                                           scene::Node *container, const base::PointF &location)
{
    if (!compositor || !container)
        return nullptr;

    base::PointF surfaceLocation;
    return findTargetSurfaceInNodes(compositor, container->children(), location, &surfaceLocation,
                                    true);
}

scene::Node *inputRootFor(compositor::Compositor *compositor, const SessionGate *gate)
{
    if (gate && gate->restrictsInput())
        return gate->inputRoot();
    return compositor ? compositor->surfaceContainer() : nullptr;
}

base::PointF locationInSurface(const compositor::Surface *surface, const base::PointF &location)
{
    const scene::Node *node = surface ? surface->node() : nullptr;
    if (!node)
        return location;
    return display::DisplayManager::globalToSurfaceLocal(location, node->absolutePosition());
}

} // namespace Lingmo::Input
