// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_overlay_plane.h
//           chromium/src/components/viz/service/display/
//               overlay_strategy_fullscreen.cc
//               overlay_strategy_single_on_top.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: there is no quad list and no DisplayResourceProvider,
// so the strategies run over the scene tree directly. gfx::Rect becomes base::Rect,
// gfx::GpuFence becomes render::SyncFence, and everything ChromeOS-specific -
// rounded display masks, colour spaces, temporal promotion tracking - is
// omitted.

#ifndef LINGMO_SCENE_OVERLAY_CANDIDATE_H_
#define LINGMO_SCENE_OVERLAY_CANDIDATE_H_

#include <memory>
#include <vector>


#include "render/presentation.h"
#include "render/scanout.h"
#include "render/sync_fence.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

class Node;
void removeOccludedPresentation(Node& root, const base::Size& logicalSize,
                                render::PresentationSubmission& submission);

class SurfaceNode;

// One piece of scene content a display controller could put on a hardware
// plane instead of compositing it.
struct OverlayCandidate {
    OverlayCandidate() = default;
    OverlayCandidate(OverlayCandidate&&) noexcept = default;
    OverlayCandidate& operator=(OverlayCandidate&&) noexcept = default;

    bool isValid() const { return buffer != nullptr; }

    // The client storage to scan out. Holding it keeps the whole wl_buffer
    // alive for as long as the plane references it.
    std::shared_ptr<render::ScanoutBuffer> buffer;

    // Signalled when the client has finished writing `buffer`. Handed to KMS as
    // the plane's IN_FENCE_FD rather than waited on.
    render::SyncFence acquireFence;

    // Where the content lands on the output, in output pixels.
    base::Rect displayRect;

    // The region of the buffer to show, in buffer pixels. Scanout requires it
    // to match displayRect's size: no plane scaling is attempted.
    base::Rect sourceRect;

    // No blending needed. Only opaque content may take the primary plane.
    bool opaque = false;

    // 0 is the primary plane; overlays are above it in ascending order.
    int zOrder = 0;

    // The scene node this came from, so the display controller can mark it as
    // promoted and keep composition from drawing it twice. Never dereferenced
    // after the frame it was proposed for.
    SurfaceNode* node = nullptr;
};

// What the planner proposes for one scene on one output.
struct OverlayPlan {
    OverlayPlan() = default;
    OverlayPlan(OverlayPlan&&) noexcept = default;
    OverlayPlan& operator=(OverlayPlan&&) noexcept = default;

    // A client buffer that replaces the composited frame on the primary plane.
    // Invalid when the scene has to be composited as usual, which is the
    // ordinary case and never an error.
    OverlayCandidate primary;

    // Content for overlay planes, z ascending, all above the primary plane.
    std::vector<OverlayCandidate> overlays;

    bool hasDirectScanout() const { return primary.isValid(); }
    bool isEmpty() const { return !primary.isValid() && overlays.empty(); }
};

// Proposes hardware planes for the scene rooted at `root` on an output of
// `outputSize`, offering at most `maxOverlays` overlay planes.
//
// This decides only what the scene permits. Whether the hardware can actually
// do it - format, modifier, plane capabilities, an atomic TEST_ONLY - is the
// display controller's answer, and a rejection there simply means the content
// is composited instead.
OverlayPlan proposeOverlays(Node& root, const base::Size& outputSize, size_t maxOverlays,
                            double outputScale = 1);

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_OVERLAY_CANDIDATE_H_
