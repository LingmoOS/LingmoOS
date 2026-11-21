namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/bubble_shape.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "base/transform.h"

namespace Lingmo::ui {

// Keeps a fillet off the corner rounding, so the edge next to the arrow is straight.
static constexpr double kCornerClearance = 2;

static double continuousCornerExtent(double radius, double halfSize)
{
    return std::clamp(radius * 1.5, 0.0, std::max(halfSize, 0.0));
}

// One corner of x^3 + y^3 = 1, scaled to rx by ry around the centre and
// turned by the angle; in step with rounded_path.cpp and FishUI's
// roundedpath.cpp.
static void addContinuousCorner(Path& path, PointF centre, double rx, double ry, double angle)
{
    base::AffineTransform transform;
    transform.translate(centre.x, centre.y);
    transform.scale(rx, ry);
    transform.rotate(angle);
    const auto map = [&transform](PointF point) {
        return PointF{transform.m11() * point.x + transform.m21() * point.y + transform.dx(),
                      transform.m12() * point.x + transform.m22() * point.y + transform.dy()};
    };

    const double diagonal = std::cbrt(0.5);
    const auto sample = [diagonal](double t) {
        if (t <= 1) {
            const double x = diagonal * t;
            const double y = std::cbrt(1 - x * x * x);
            return std::pair{PointF{x, -y}, PointF{diagonal, diagonal * x * x / (y * y)}};
        }
        const double y = diagonal * (2 - t);
        const double x = std::cbrt(1 - y * y * y);
        return std::pair{PointF{x, -y}, PointF{diagonal * y * y / (x * x), diagonal}};
    };
    path.lineTo(map(sample(0).first));
    constexpr int segments = 8;
    constexpr double step = 2.0 / segments;
    for (int i = 0; i < segments; ++i) {
        const auto [start, startTangent] = sample(i * step);
        const auto [end, endTangent] = sample((i + 1) * step);
        PointF first = {start.x + startTangent.x * step / 3, start.y + startTangent.y * step / 3};
        PointF second = {end.x - endTangent.x * step / 3, end.y - endTangent.y * step / 3};
        // Collinear end controls keep zero curvature at the straight edges.
        if (i == 0)
            second.y = -1;
        if (i == segments - 1)
            first.x = 1;
        path.cubicTo(map(first), map(second), map(end));
    }
}

namespace {

// The arrow as built under the bottom edge, its fillets clamped to what its
// sides leave room for; all zero for an arrow without width or length.
struct Arrow
{
    double tipRadius = 0;
    // How far below the edge the tip fillet's centre lies.
    double tipCentre = 0;
    double baseRadius = 0;
    // Half the edge the arrow takes, out to where its fillets meet the edge.
    double footprint = 0;
};

} // namespace

static Arrow arrowGeometry(const BubbleShape& shape)
{
    Arrow arrow;
    const double hw = shape.arrowWidth / 2;
    const double h = shape.arrowLength;
    if (hw <= 0 || h <= 0)
        return arrow;
    const double slant = std::hypot(hw, h);
    arrow.tipRadius = std::clamp(shape.arrowTipRadius, 0.0, h * hw / slant);
    arrow.tipCentre = h - arrow.tipRadius * slant / hw;
    const double tipStart = (hw * hw + arrow.tipCentre * h) / slant;
    arrow.baseRadius = std::clamp(shape.arrowBaseRadius, 0.0, tipStart * h / (slant - hw));
    arrow.footprint = hw + arrow.baseRadius * (slant - hw) / h;
    return arrow;
}

RectF BubbleShape::bodyRect() const
{
    const double length = arrowEdge != Edge::None ? std::max(arrowLength, 0.0) : 0;
    switch (arrowEdge) {
    case Edge::Top:
        return {0, length, width, height - length};
    case Edge::Bottom:
        return {0, 0, width, height - length};
    case Edge::Left:
        return {length, 0, width - length, height};
    case Edge::Right:
        return {0, 0, width - length, height};
    case Edge::None:
        break;
    }
    return {0, 0, width, height};
}

double BubbleShape::minimumEdgeLength() const
{
    return 2 * (std::max(radius, 0.0) * (continuousCorners ? 1.5 : 1.0) + arrowGeometry(*this).footprint
                + kCornerClearance);
}

bool BubbleShape::arrowVisible() const
{
    if (arrowEdge == Edge::None || arrowWidth <= 0 || arrowLength <= 0)
        return false;
    const RectF body = bodyRect();
    const bool horizontal = arrowEdge == Edge::Top || arrowEdge == Edge::Bottom;
    const double edge = horizontal ? body.width : body.height;
    if (body.width <= 0 || body.height <= 0)
        return false;
    const double corner = continuousCorners ? continuousCornerExtent(radius, edge / 2)
                                            : std::clamp(radius, 0.0, std::min(body.width, body.height) / 2);
    return edge >= 2 * (corner + arrowGeometry(*this).footprint + kCornerClearance);
}

Path BubbleShape::path() const
{
    const RectF body = bodyRect();
    if (body.width <= 0 || body.height <= 0)
        return {};

    // Built with the arrow under the bottom edge, then turned onto its edge.
    const bool horizontal = arrowEdge != Edge::Left && arrowEdge != Edge::Right;
    const double w = horizontal ? body.width : body.height;
    const double b = horizontal ? body.height : body.width;
    const double r = std::clamp(radius, 0.0, std::min(w, b) / 2);
    const double rx = continuousCorners ? continuousCornerExtent(radius, w / 2) : r;
    const double ry = continuousCorners ? continuousCornerExtent(radius, b / 2) : r;

    Path path;
    path.moveTo(rx, 0);
    if (continuousCorners) {
        addContinuousCorner(path, {w - rx, ry}, rx, ry, 0);
        addContinuousCorner(path, {w - rx, b - ry}, rx, ry, 90);
    } else {
        path.arcTo({w - 2 * r, 0, 2 * r, 2 * r}, 90, -90);
        path.arcTo({w - 2 * r, b - 2 * r, 2 * r, 2 * r}, 0, -90);
    }
    if (arrowVisible()) {
        const Arrow arrow = arrowGeometry(*this);
        const double reach = arrow.footprint;
        const double reserved = rx + reach + kCornerClearance;
        const double c = std::clamp(arrowPosition, reserved, w - reserved);
        // The side's angle from the edge normal, in the degrees arcTo() takes.
        const double side = std::atan2(arrowWidth / 2, arrowLength) * 180 / M_PI;
        const auto circle = [](double x, double y, double radius) {
            return RectF{x - radius, y - radius, 2 * radius, 2 * radius};
        };
        path.arcTo(circle(c + reach, b + arrow.baseRadius, arrow.baseRadius), 90, 90 - side);
        // arcTo() adds nothing for an empty rect, which would cut the tip off.
        if (arrow.tipRadius > 0)
            path.arcTo(circle(c, b + arrow.tipCentre, arrow.tipRadius), -side, 2 * side - 180);
        else
            path.lineTo(c, b + arrowLength);
        path.arcTo(circle(c - reach, b + arrow.baseRadius, arrow.baseRadius), side, 90 - side);
    }
    if (continuousCorners) {
        addContinuousCorner(path, {rx, b - ry}, rx, ry, 180);
        addContinuousCorner(path, {rx, ry}, rx, ry, 270);
    } else {
        path.arcTo({0, b - 2 * r, 2 * r, 2 * r}, 270, -90);
        path.arcTo({0, 0, 2 * r, 2 * r}, 180, -90);
    }
    path.close();

    switch (arrowEdge) {
    case Edge::Top:
        return path.transformed(base::AffineTransform(1, 0, 0, -1, 0, height));
    case Edge::Left:
        return path.transformed(base::AffineTransform(0, 1, -1, 0, width, 0));
    case Edge::Right:
        return path.transformed(base::AffineTransform(0, 1, 1, 0, 0, 0));
    case Edge::Bottom:
    case Edge::None:
        break;
    }
    return path;
}

} // namespace Lingmo::ui
