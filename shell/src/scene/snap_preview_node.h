// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>

#include "render/renderer.h"
#include "render/rounded_shape.h"
#include "scene/node.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

// The phantom of where a dragged window would land if the pointer were
// released now: an accent-tinted rounded plate inset from the target bounds,
// which grows out of its centre and fades in each time the target changes.
// Upstream: ash/wm/workspace/phantom_window_controller.* (revision
// 7acd40ddefff49c9d36144999aafdbb674dbd07a); the maximize cue belongs to
// Ash's portrait top snap and is left out.
class SnapPreviewNode : public Node
{
public:
    // Upstream kPhantomWindowInsets and kPhantomWindowCornerRadius.
    static constexpr int kInset = 8;
    static constexpr double kRadius = 12;
    // Upstream kScrimEntranceSizeAnimationDurationMs and
    // kScrimEntranceOpacityAnimationDurationMs.
    static constexpr int kSizeDurationMs = 150;
    static constexpr int kOpacityDurationMs = 50;

    // `clock` stands in for the frame time outside an output frame, in
    // milliseconds of the steady clock.
    explicit SnapPreviewNode(std::function<double()> clock = {});

    // `windowBounds` are where the window will land, in the parent's
    // coordinates. A change restarts the entrance, as Ash builds a new phantom
    // for every new snap type.
    void show(const base::Rect &windowBounds);

    void setColors(const render::Color &fill, const render::Color &outline,
                   const render::Color &highlight);

    // The plate as the entrance draws it `elapsedMs` in, and its opacity.
    static base::RectF entranceBounds(const base::RectF &target, double elapsedMs);
    static float entranceOpacity(double elapsedMs);

    Content content() const override { return Content::Translucent; }

    // The rounded fill and the outline are one composite; the planner must not
    // look inside it.
    bool paintsSubtree() const override { return true; }

    bool requiresFullRepaint() const override { return false; }

    // A preview is a hint, never a target: input goes to whatever is under it.
    bool acceptsInput(const base::PointF &) const override { return false; }

protected:
    void paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const override;

private:
    std::function<double()> m_now;
    // Set by the first frame that draws the plate, so a slow first frame does
    // not eat into the entrance.
    mutable double m_started = -1;
    render::Color m_fill {0.2f, 0.52f, 1, 0.25f};
    render::Color m_outline {0, 0, 0, 0.08f};
    render::Color m_highlight {1, 1, 1, 0.16f};
};

} // namespace Lingmo::Scene
