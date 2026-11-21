// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/overview_window_frame.h"

#include <algorithm>
#include <cmath>

#include "decoration/decoration_theme.h"
#include "decoration/shadow.h"
#include "decoration/shadow_node.h"
#include "decoration/window_visual_style.h"
#include "render/renderer.h"
#include "scene/window_animation.h"
#include "scene/window_visual_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {

scene::OverviewWindowFrame overviewWindowFrame(const Theme *theme)
{
    scene::OverviewWindowFrame frame;
    frame.radius = theme->visualStyle().radius;
    frame.shadow = [theme](render::Renderer &renderer, const scene::WindowAnimation &window,
                           const render::RoundedShape &body, double uniform, float opacity,
                           bool moving) {
        // Inside the body's edges on whole logical pixels: a sliver of shadow
        // under the window rather than a gap beside it.
        const int left = int(std::ceil(body.bounds.left()));
        const int top = int(std::ceil(body.bounds.top()));
        const base::Rect content(left, top, int(std::floor(body.bounds.right())) - left,
                                 int(std::floor(body.bounds.bottom())) - top);
        if (content.isEmpty())
            return;
        // Each shadow keeps the corners of its window at full size, not the
        // body's as it shrinks: a new radius is a new shadow texture every
        // frame, and the few pixels more it curves lie under the body.
        const auto paint = [&](wm::ShadowElevation elevation, int radius, float alpha) {
            if (elevation == wm::ShadowElevation::None || alpha <= 0)
                return;
            radius = std::min(radius, ShadowGeometry::kMaxRadius);
            const ShadowStyle style = theme->metrics().shadowFor(elevation);
            const int rasterScale = int(std::ceil(renderer.rasterScale()));
            // Below the nine-patch's least size every size is a texture of its
            // own. In motion, a size a step of a shared ladder up stands in.
            base::Size size = content.size();
            if (moving) {
                const base::Size least =
                        ShadowGeometry(base::Size(4096, 4096), radius, style, rasterScale)
                                .canonicalSize();
                const auto step = [](int value, int least) {
                    if (value >= least || value <= 0)
                        return value;
                    constexpr double kRatio = 0.9;
                    const int k = int(std::floor(std::log(double(value) / least)
                                                 / std::log(kRatio)));
                    return std::max(value, int(std::ceil(least * std::pow(kRatio, k))));
                };
                size = base::Size(step(size.width(), least.width()),
                                  step(size.height(), least.height()));
            }
            const ShadowGeometry geometry(size, radius, style, rasterScale);
            if (size == content.size())
                paintShadow(renderer, geometry, content.origin(), alpha);
            else
                paintStretchedShadow(renderer, geometry, content, alpha);
        };
        auto own = wm::ShadowElevation::None;
        int ownRadius = 0;
        for (const scene::Node *child : window.children()) {
            const auto *shadow = dynamic_cast<const ShadowNode *>(child);
            if (shadow && shadow->isVisible()) {
                own = shadow->elevation();
                ownRadius = shadow->radius();
            }
        }
        const int radius = theme->visualStyle().radius;
        // The active window's own shadow is the overview's: drawn once, not
        // two of them blended over each other all the way.
        if (own == wm::ShadowElevation::Active && ownRadius == radius) {
            paint(own, ownRadius, opacity);
            return;
        }
        paint(own, ownRadius, opacity * float(1 - uniform));
        paint(wm::ShadowElevation::Active, radius, opacity * float(uniform));
    };
    frame.outline = [theme](render::Renderer &renderer, const render::RoundedShape &body,
                            float opacity) {
        const WindowVisualStyle style = theme->visualStyle();
        scene::WindowVisualState state;
        state.shape = body;
        state.drawOutline = true;
        state.outlineWidthPx = style.outlineWidthPx;
        state.outlineColor = style.activeOutline;
        state.highlightColor = style.highlight;
        scene::paintWindowOutline(renderer, body, state, opacity);
    };
    return frame;
}

} // namespace Lingmo::Decoration
