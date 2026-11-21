// Copyright 2020 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desks_animations.cc (WindowMoveToDeskAnimation)
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "scene/slide_out_node.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "scene/motion.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

static double slideDuration()
{
    return std::max(1.0, SlideOutNode::kDurationMs * motion::durationScale());
}

SlideOutNode::SlideOutNode(std::function<int64_t()> clock)
    : Node("window-move-to-desk")
    , m_now(clock ? std::move(clock) : [] {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
    })
{
}

void SlideOutNode::slideOut(DeskSwitchNode::Frames frames, const base::Rect &window,
                            const base::Rect &output, bool goingLeft)
{
    if (frames.empty())
        return;
    // Upstream GetWindowEndTransform: just past the edge on the target's side.
    const int distance = goingLeft ? -(window.right() - output.left())
                                   : output.right() - window.left();
    const int64_t now = m_now();
    // Slides end where they are drawn; with no output drawing, they end here.
    std::erase_if(m_slides, [now](const Slide &slide) {
        return now - slide.start >= slideDuration();
    });
    m_slides.push_back({std::move(frames), distance, now});
    notifyChanged();
}

void SlideOutNode::render(render::Renderer &renderer) const
{
    if (m_slides.empty() || !isVisible())
        return;
    const int64_t now = m_now();
    const auto &context = renderer.outputContext();
    const base::Rect viewport =
            context.logicalViewport.isEmpty() ? absoluteGeometry() : context.logicalViewport;
    const double duration = slideDuration();
    for (const Slide &slide : m_slides) {
        const auto frame = slide.frames.find(context.outputId);
        if (frame == slide.frames.end() || !frame->second)
            continue;
        const double t = std::clamp(double(now - slide.start) / duration, 0.0, 1.0);
        // gfx::Tween::EASE_IN.
        const double offset = slide.distance * t * t;
        renderer.drawTextureF(frame->second.get(),
                              base::RectF(viewport.x() + offset, viewport.y(), viewport.width(),
                                     viewport.height()));
    }
    if (renderer.capturing)
        return;
    std::erase_if(m_slides, [now, duration](const Slide &slide) {
        return now - slide.start >= duration;
    });
    // Once more after the last slide ends, to repaint without it.
    notifyChanged();
}

void SlideOutNode::finishRetiring()
{
    m_slides.clear();
    Node::finishRetiring();
}

void SlideOutNode::releaseRenderResources()
{
    m_slides.clear();
    Node::releaseRenderResources();
}

} // namespace Lingmo::Scene
