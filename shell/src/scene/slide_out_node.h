// Copyright 2020 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desks_animations.cc (WindowMoveToDeskAnimation)
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#ifndef LINGMO_SCENE_SLIDE_OUT_NODE_H_
#define LINGMO_SCENE_SLIDE_OUT_NODE_H_

#include <functional>
#include <vector>

#include "scene/desk_switch_node.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

// Copies of windows sent to another desk, each sliding off its output towards
// that desk and then dropped. Upstream recreates the window's layers and
// animates the old ones; here a frame of the window per output stands in.
class SlideOutNode : public Node
{
public:
    static constexpr int kDurationMs = 200;

    explicit SlideOutNode(std::function<int64_t()> clock = {});

    // `frames` show the window alone, over each output's whole viewport. It
    // leaves `output` past its left edge, or past its right edge.
    void slideOut(DeskSwitchNode::Frames frames, const base::Rect &window, const base::Rect &output,
                  bool goingLeft);

    bool isActive() const { return !m_slides.empty(); }

    void render(render::Renderer &renderer) const override;
    Content content() const override { return isActive() ? Content::Translucent : Content::None; }
    bool paintsSubtree() const override { return isActive(); }
    bool acceptsInput(const base::PointF &) const override { return false; }
    void finishRetiring() override;
    void releaseRenderResources() override;

private:
    struct Slide {
        DeskSwitchNode::Frames frames;
        int distance = 0;
        int64_t start = 0;
    };

    std::function<int64_t()> m_now;
    mutable std::vector<Slide> m_slides;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_SLIDE_OUT_NODE_H_
