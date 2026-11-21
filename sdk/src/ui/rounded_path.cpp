// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/rounded_path.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"

namespace Lingmo::ui {

static double continuousCornerExtent(double radius, double halfSize)
{
    return std::clamp(radius * 1.5, 0.0, std::max(halfSize, 0.0));
}

// Appends one corner of x^3 + y^3 = 1, scaled to rx by ry around the centre
// and turned by the angle, in two regular halves that avoid the singular
// slopes at the ends.
static void addContinuousCorner(SkPathBuilder& path, SkPoint centre, double rx, double ry,
                                double angle)
{
    SkMatrix transform = SkMatrix::Translate(centre.x(), centre.y());
    transform.preScale(SkScalar(rx), SkScalar(ry));
    transform.preRotate(SkScalar(angle));
    const auto map = [&transform](SkPoint point) { return transform.mapPoint(point); };

    const double diagonal = std::cbrt(0.5);
    const auto sample = [diagonal](double t) {
        if (t <= 1) {
            const double x = diagonal * t;
            const double y = std::cbrt(1 - x * x * x);
            return std::pair{SkPoint::Make(SkScalar(x), SkScalar(-y)),
                             SkPoint::Make(SkScalar(diagonal), SkScalar(diagonal * x * x / (y * y)))};
        }
        const double y = diagonal * (2 - t);
        const double x = std::cbrt(1 - y * y * y);
        return std::pair{SkPoint::Make(SkScalar(x), SkScalar(-y)),
                         SkPoint::Make(SkScalar(diagonal * y * y / (x * x)), SkScalar(diagonal))};
    };
    path.lineTo(map(sample(0).first));
    constexpr int segments = 8;
    constexpr double step = 2.0 / segments;
    for (int i = 0; i < segments; ++i) {
        const auto [start, startTangent] = sample(i * step);
        const auto [end, endTangent] = sample((i + 1) * step);
        SkPoint first = start + startTangent * SkScalar(step / 3);
        SkPoint second = end - endTangent * SkScalar(step / 3);
        // Collinear end controls keep zero curvature at the straight edges.
        if (i == 0)
            second.fY = -1;
        if (i == segments - 1)
            first.fX = 1;
        path.cubicTo(map(first), map(second), map(end));
    }
}

SkPath roundedRectPath(const SkRect& rect, double radius, bool continuous)
{
    if (rect.isEmpty())
        return {};
    if (!continuous) {
        const SkScalar r = SkScalar(std::clamp(radius, 0.0,
                                               double(std::min(rect.width(), rect.height())) / 2));
        return SkPath::RRect(SkRRect::MakeRectXY(rect, r, r));
    }

    const double rx = continuousCornerExtent(radius, rect.width() / 2.0);
    const double ry = continuousCornerExtent(radius, rect.height() / 2.0);
    if (rx <= 0 || ry <= 0)
        return SkPath::Rect(rect);

    const double w = rect.width();
    const double h = rect.height();
    SkPathBuilder path;
    path.moveTo(SkScalar(rx), 0);
    addContinuousCorner(path, SkPoint::Make(SkScalar(w - rx), SkScalar(ry)), rx, ry, 0);
    addContinuousCorner(path, SkPoint::Make(SkScalar(w - rx), SkScalar(h - ry)), rx, ry, 90);
    addContinuousCorner(path, SkPoint::Make(SkScalar(rx), SkScalar(h - ry)), rx, ry, 180);
    addContinuousCorner(path, SkPoint::Make(SkScalar(rx), SkScalar(ry)), rx, ry, 270);
    path.close();
    return path.detach().makeOffset(rect.left(), rect.top());
}

} // namespace Lingmo::ui
