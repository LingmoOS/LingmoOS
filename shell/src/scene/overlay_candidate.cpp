// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_overlay_plane.cc
//           chromium/src/components/viz/service/display/
//               overlay_strategy_fullscreen.cc
//               overlay_strategy_single_on_top.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "scene/overlay_candidate.h"

#include <algorithm>
#include <cmath>


#include "scene/node.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "base/region.h"

namespace Lingmo::Scene {

namespace {

// Animations step when they are drawn, and a minimize or close reports its
// end from there, so one anywhere in the scene keeps composition going even
// under a fullscreen surface that hides it.
bool hasAnimation(const Node &node)
{
    if (!node.isVisible() && !node.paintsWhenHidden())
        return false;
    if (const auto *animation = dynamic_cast<const WindowAnimation *>(&node);
        animation && animation->paintsSubtree())
        return true;
    return std::any_of(node.children().begin(), node.children().end(), [](const Node *child) {
        return hasAnimation(*child);
    });
}

// A blur reads the buffers under it from the composition target, so no
// overlay may take one of them away. Under an opaque fullscreen surface it
// reads nothing anyone sees.
bool hasBackgroundBlur(const Node &node)
{
    if (!node.isVisible())
        return false;
    if (const auto *surface = dynamic_cast<const SurfaceNode *>(&node);
        surface && surface->hasBackgroundBlur())
        return true;
    return std::any_of(node.children().begin(), node.children().end(), [](const Node *child) {
        return hasBackgroundBlur(*child);
    });
}

// One painted thing, in paint order. Upstream this is a viz::DrawQuad in an
// AggregatedRenderPass; here the scene tree is walked directly.
struct SceneItem {
    base::Rect bounds;
    bool opaque = false;
    // Non-null when the item is one client buffer a plane could show directly.
    SurfaceNode *surface = nullptr;
    std::shared_ptr<render::PresentationContent> presentation;
};

void flatten(Node &node, const base::Rect &outputRect, std::vector<SceneItem> &items)
{
    if (!node.isVisible() && !node.paintsWhenHidden())
        return;

    const base::Rect bounds = node.absolutePaintBounds().intersected(outputRect);

    // An effect draws its whole subtree itself, so it is one opaque-unknown
    // blob and nothing inside it can reach a plane.
    if (node.paintsSubtree()) {
        if (!bounds.isEmpty())
            items.push_back(SceneItem {bounds, false, nullptr});
        return;
    }

    const auto appendSelf = [&] {
        const Node::Content content = node.content();
        if (content == Node::Content::None || bounds.isEmpty())
            return;
        SceneItem item;
        item.bounds = bounds;
        item.opaque = content == Node::Content::Opaque;
        if (auto *surface = dynamic_cast<SurfaceNode *>(&node))
            item.presentation = surface->presentationContent();
        // A partially clipped surface cannot go on a plane as-is, and neither
        // can one the compositor is fading: both would need a source crop or
        // blending the planner does not attempt.
        if (bounds == node.absoluteGeometry())
            item.surface = dynamic_cast<SurfaceNode *>(&node);
        if (item.surface && !item.surface->scanoutBuffer())
            item.surface = nullptr;
        items.push_back(std::move(item));
    };

    // Node::render draws the children marked "below parent", then itself, then
    // the rest. The flattened order has to match exactly.
    const auto &children = node.children();
    const size_t split = std::min(node.childrenBelowParent(), children.size());

    for (size_t i = 0; i < split; ++i)
        flatten(*children[i], outputRect, items);
    appendSelf();
    for (size_t i = split; i < children.size(); ++i)
        flatten(*children[i], outputRect, items);
}

// The buffer maps one-to-one onto `pixels` of the plane: no plane scaling and
// no source crop, which is all the planner is willing to ask a plane for.
bool isUnscaled(const SurfaceNode &surface, const base::Size &pixels)
{
    const render::ScanoutBuffer *buffer = surface.scanoutBuffer().get();
    return buffer && !surface.hasContentShape() && surface.alpha() == 1.0f
            && surface.textureMapping().isIdentity() && !buffer->scanoutAttributes().yInvert
            && buffer->scanoutAttributes().size == pixels;
}

bool isUnscaled(const SurfaceNode &surface)
{
    return isUnscaled(surface, surface.geometry().size());
}

OverlayCandidate makeCandidate(SurfaceNode &surface, const base::Rect &bounds, bool opaque, int zOrder)
{
    OverlayCandidate candidate;
    candidate.buffer = surface.scanoutBuffer();
    candidate.acquireFence = surface.acquireFence().duplicate();
    if (surface.acquireFence().isValid() && !candidate.acquireFence.isValid())
        return {};
    candidate.displayRect = bounds;
    candidate.sourceRect = base::Rect(base::Point(0, 0), candidate.buffer->scanoutAttributes().size);
    candidate.opaque = opaque;
    candidate.zOrder = zOrder;
    candidate.node = &surface;
    return candidate;
}

// Port of OverlayStrategyFullscreen::Propose: the topmost visible item may take
// the primary plane when it covers the whole output opaquely. Everything below
// it is then invisible by construction, which is the same conclusion upstream
// reaches through its BlackFullscreenOptimization. `outputRect` is the output
// in logical coordinates and `outputPixels` its mode: a buffer of the mode's
// size goes on the plane at any scale, as viz promotes in device space.
bool proposeFullscreen(std::vector<SceneItem> &items, const base::Rect &outputRect,
                       const base::Size &outputPixels, OverlayPlan &plan)
{
    if (items.empty())
        return false;

    SceneItem &top = items.back();
    if (!top.surface || !top.opaque || top.bounds != outputRect)
        return false;
    if (!isUnscaled(*top.surface, outputPixels))
        return false;

    plan.primary = makeCandidate(*top.surface, base::Rect(base::Point(), outputPixels), true, 0);
    return plan.primary.isValid();
}

// Port of OverlayStrategySingleOnTop::Propose: an item nothing above it touches
// can be lifted onto an overlay plane, because composition below it stays
// correct and nothing has to blend over it.
void proposeSingleOnTop(std::vector<SceneItem> &items, size_t maxOverlays, OverlayPlan &plan)
{
    if (maxOverlays == 0)
        return;

    std::vector<SceneItem *> promoted;
    for (size_t i = items.size(); i-- > 0 && promoted.size() < maxOverlays;) {
        SceneItem &item = items[i];
        if (!item.surface || !item.opaque || !isUnscaled(*item.surface))
            continue;

        bool occluded = false;
        for (size_t above = i + 1; above < items.size() && !occluded; ++above)
            occluded = items[above].bounds.intersects(item.bounds);
        if (occluded)
            continue;

        promoted.push_back(&item);
    }

    // Ascending z: the lowest promoted item gets the plane nearest the primary.
    std::reverse(promoted.begin(), promoted.end());
    int zOrder = 1;
    for (SceneItem *item : promoted) {
        auto candidate = makeCandidate(*item->surface, item->bounds, item->opaque, zOrder++);
        if (candidate.isValid())
            plan.overlays.push_back(std::move(candidate));
    }
}

} // namespace

void removeOccludedPresentation(Node &root, const base::Size &logicalSize,
                                render::PresentationSubmission &submission)
{
    std::vector<SceneItem> items;
    flatten(root, base::Rect(base::Point(), logicalSize), items);
    base::Region covered;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        if ((base::Region(it->bounds) - covered).isEmpty())
            submission.remove(it->presentation);
        if (it->opaque)
            covered += it->bounds;
    }
}

OverlayPlan proposeOverlays(Node &root, const base::Size &outputSize, size_t maxOverlays,
                            double outputScale)
{
    OverlayPlan plan;
    if (outputSize.isEmpty() || outputScale <= 0 || hasAnimation(root))
        return plan;

    // The logical extent the scene sees, as OutputScale::logicalSize rounds it.
    const base::Rect outputRect(base::Point(0, 0),
                           base::Size(int(std::ceil(outputSize.width() / outputScale)),
                                 int(std::ceil(outputSize.height() / outputScale))));
    std::vector<SceneItem> items;
    flatten(root, outputRect, items);

    // Fullscreen wins outright: with the primary plane showing a client buffer
    // there is nothing left for an overlay to sit on top of.
    if (proposeFullscreen(items, outputRect, outputSize, plan))
        return plan;

    // Overlays are placed at their logical bounds, which only at scale one
    // are the plane's device pixels.
    if (outputScale != 1 || hasBackgroundBlur(root))
        return plan;
    proposeSingleOnTop(items, maxOverlays, plan);
    return plan;
}

} // namespace Lingmo::Scene
