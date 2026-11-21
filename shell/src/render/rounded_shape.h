// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <cmath>
#include "base/geometry.h"


namespace Lingmo::Render {

// The session's corner style. Every window silhouette follows it, so it is
// one flag consulted where shapes are drawn or hit rather than a field on
// each shape; decoration::ThemePreferences keeps it in step with the
// Appearance setting.
bool continuousCorners();
void setContinuousCorners(bool continuous);

// Cubic superellipse corners meet straight edges with zero curvature.
// The 1.5 extent preserves roughly the circular radius's diagonal inset.
inline double continuousCornerExtent(double radius, double halfSize)
{
    // Keep the extent and exponent in sync with shaders/rounded_clip.glsl.
    return std::clamp(radius * 1.5, double(0), std::max(halfSize, double(0)));
}

// The corner's reach along one axis in the session's style.
inline double cornerExtent(double radius, double halfSize)
{
    return continuousCorners() ? continuousCornerExtent(radius, halfSize)
                               : std::clamp(radius, double(0), std::max(halfSize, double(0)));
}

// One output-coordinate shape shared by every surface in a window tree.
struct RoundedShape {
    base::RectF bounds;
    double radius = 0;
    base::SizeF radiusScale {1, 1};

    bool contains(const base::PointF& point) const
    {
        // Inclusive of the right and bottom edges, unlike RectF::contains().
        if (bounds.isEmpty() || point.x() < bounds.left() || point.x() > bounds.right() || point.y() < bounds.top()
            || point.y() > bounds.bottom())
            return false;
        const double rx = cornerExtent(radius * radiusScale.width(), bounds.width() / 2);
        const double ry = cornerExtent(radius * radiusScale.height(), bounds.height() / 2);
        if (rx <= 0 || ry <= 0)
            return true;
        const base::PointF d = point - bounds.center();
        const double x = std::max(std::abs(d.x()) - bounds.width() / 2 + rx, double(0)) / rx;
        const double y = std::max(std::abs(d.y()) - bounds.height() / 2 + ry, double(0)) / ry;
        if (!continuousCorners())
            return x * x + y * y <= 1;
        return x * x * x + y * y * y <= 1;
    }

    // Signed distance to the outline, negative inside; roundedDistance() in
    // shaders/rounded_clip.glsl.
    double distance(const base::PointF &point) const
    {
        const base::PointF half(bounds.width() / 2, bounds.height() / 2);
        const double rx = cornerExtent(radius * radiusScale.width(), half.x());
        const double ry = cornerExtent(radius * radiusScale.height(), half.y());
        const base::PointF d = point - bounds.center();
        const double ex = std::abs(d.x()) - half.x();
        const double ey = std::abs(d.y()) - half.y();
        if (rx <= 0 || ry <= 0 || ex + rx <= 0 || ey + ry <= 0)
            return std::max(ex, ey);
        const double qx = (ex + rx) / rx;
        const double qy = (ey + ry) / ry;
        if (!continuousCorners()) {
            const double l = std::hypot(qx, qy);
            return (l - 1) * l / std::hypot(qx / rx, qy / ry);
        }
        const double l = std::cbrt(qx * qx * qx + qy * qy * qy);
        return (l - 1) * l * l / std::hypot(qx * qx / rx, qy * qy / ry);
    }
};

} // namespace Lingmo::Render
