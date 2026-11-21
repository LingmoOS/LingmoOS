// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/resize_handles.h"

#include <algorithm>
#include <cmath>

#include "compositor/surface.h"
#include "wm/window.h"
#include "wm/window_modality_controller.h"
#include "wm/window_state.h"
#include "wm/window_visual_controller.h"

namespace Lingmo::WM {

namespace {

ResizeEdge edgeFor(bool left, bool right, bool top, bool bottom)
{
    if (top && left)
        return ResizeEdge::TopLeft;
    if (top && right)
        return ResizeEdge::TopRight;
    if (bottom && left)
        return ResizeEdge::BottomLeft;
    if (bottom && right)
        return ResizeEdge::BottomRight;
    if (top)
        return ResizeEdge::Top;
    if (bottom)
        return ResizeEdge::Bottom;
    if (left)
        return ResizeEdge::Left;
    if (right)
        return ResizeEdge::Right;
    return ResizeEdge::None;
}

} // namespace

ResizeEdge resizeEdgeFor(const render::RoundedShape &shape, const base::Rect &frame,
                         const base::Point &point, int inside, int corner)
{
    // The border follows the arc along a rounded corner; everything outside
    // the outline, the corners' empty pixels included, is border.
    if (shape.distance(base::PointF(point) + base::PointF(0.5, 0.5)) <= -inside)
        return ResizeEdge::None;

    const auto reach = [&](double radius, double halfSize) {
        const double arc = render::cornerExtent(radius, halfSize);
        return std::max(corner, int(std::ceil(arc)) + inside);
    };
    const int cornerX = reach(shape.radius * shape.radiusScale.width(), shape.bounds.width() / 2);
    const int cornerY = reach(shape.radius * shape.radiusScale.height(), shape.bounds.height() / 2);
    const bool nearLeft = point.x() < frame.left() + cornerX;
    const bool nearRight = point.x() >= frame.right() - cornerX;
    const bool nearTop = point.y() < frame.top() + cornerY;
    const bool nearBottom = point.y() >= frame.bottom() - cornerY;
    if ((nearLeft || nearRight) && (nearTop || nearBottom)) {
        const base::Point center = centerPixel(frame);
        const bool left = point.x() <= center.x();
        const bool top = point.y() <= center.y();
        return edgeFor(left, !left, top, !top);
    }

    return edgeFor(point.x() < frame.left() + inside, point.x() >= frame.right() - inside,
                   point.y() < frame.top() + inside, point.y() >= frame.bottom() - inside);
}

ResizeEdge serverResizeEdgeAt(const Window &window, const decoration::WindowVisualStyle &style,
                              const base::PointF &location, bool touch)
{
    const compositor::Surface *surface = window.rootSurface();
    if (!surface || !surface->serverResize() || window.isDecorated() || window.isPopup()
        || window.role() != WindowRole::Normal || !window.isVisible() || !window.isResizable()
        || (!window.state()->isNormal() && !window.state()->isSnapped())
        || modalTransient(&window))
        return ResizeEdge::None;

    // Window coordinates: the shape is relative to the window's origin.
    const base::Point point = location.toPoint() - window.bounds().topLeft();
    const auto visual = WindowVisualController::evaluate(window, style);
    const base::Rect body = visual.shape.bounds.toEnclosingRect();
    const int outside = touch ? kResizeOutsideBoundsSize * kResizeOutsideBoundsScaleForTouch
                              : kResizeOutsideBoundsSize;
    if (!body.adjusted(-outside, -outside, outside, outside).contains(point))
        return ResizeEdge::None;
    return resizeEdgeFor(visual.shape, body, point, kResizeInsideBoundsSize,
                         kResizeAreaCornerSize);
}

} // namespace Lingmo::WM
