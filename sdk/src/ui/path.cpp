namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/path.h"

#include <algorithm>
#include <cmath>

#include "base/transform.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathUtils.h"
#include "include/pathops/SkPathOps.h"
#include "ui/path_private.h"

namespace Lingmo::ui {

// The control point distance that makes a cubic follow a quarter circle.
static constexpr double kKappa = 0.5522847498;

template<typename Map>
SkPath PathAccess::convert(const Path& path, Map map)
{
    SkPathBuilder builder;
    builder.setFillType(path.m_fillRule == Path::FillRule::EvenOdd ? SkPathFillType::kEvenOdd
                                                                   : SkPathFillType::kWinding);
    const auto point = [&map](PointF p) {
        const PointF mapped = map(p);
        return SkPoint::Make(SkScalar(mapped.x), SkScalar(mapped.y));
    };
    size_t index = 0;
    for (const Path::Verb verb : path.m_verbs) {
        switch (verb) {
        case Path::Verb::Move:
            builder.moveTo(point(path.m_points[index++]));
            break;
        case Path::Verb::Line:
            builder.lineTo(point(path.m_points[index++]));
            break;
        case Path::Verb::Quad:
            builder.quadTo(point(path.m_points[index]), point(path.m_points[index + 1]));
            index += 2;
            break;
        case Path::Verb::Cubic:
            builder.cubicTo(point(path.m_points[index]), point(path.m_points[index + 1]),
                            point(path.m_points[index + 2]));
            index += 3;
            break;
        case Path::Verb::Close:
            builder.close();
            break;
        }
    }
    return builder.detach();
}

SkPath PathAccess::toSkPath(const Path& path)
{
    return convert(path, [](PointF p) { return p; });
}

SkPath PathAccess::toSkPath(const Path& path, double scale, PointF offset)
{
    return convert(path, [scale, offset](PointF p) {
        return PointF{(offset.x + p.x) * scale, (offset.y + p.y) * scale};
    });
}

SkPath PathAccess::toSkPath(const Path& path, const base::AffineTransform& transform)
{
    return convert(path, [&transform](PointF p) {
        double x = 0;
        double y = 0;
        transform.map(p.x, p.y, &x, &y);
        return PointF{x, y};
    });
}

Path PathAccess::fromSkPath(const SkPath& source)
{
    Path path;
    path.m_fillRule = source.getFillType() == SkPathFillType::kEvenOdd
                    || source.getFillType() == SkPathFillType::kInverseEvenOdd
            ? Path::FillRule::EvenOdd
            : Path::FillRule::Winding;
    SkPath::RawIter iterator(source);
    SkPoint points[4];
    const auto at = [](const SkPoint& p) { return PointF{p.x(), p.y()}; };
    while (true) {
        const SkPath::Verb verb = iterator.next(points);
        switch (verb) {
        case SkPath::kMove_Verb:
            path.moveTo(at(points[0]));
            break;
        case SkPath::kLine_Verb:
            path.lineTo(at(points[1]));
            break;
        case SkPath::kQuad_Verb:
            path.quadTo(at(points[1]), at(points[2]));
            break;
        case SkPath::kConic_Verb: {
            // Path ops and strokes only make conics for round joins and caps.
            SkPoint quads[1 + 2 * 4];
            const int count = SkPath::ConvertConicToQuads(points[0], points[1], points[2],
                                                          iterator.conicWeight(), quads, 2);
            for (int i = 0; i < count; ++i)
                path.quadTo(at(quads[1 + 2 * i]), at(quads[2 + 2 * i]));
            break;
        }
        case SkPath::kCubic_Verb:
            path.cubicTo(at(points[1]), at(points[2]), at(points[3]));
            break;
        case SkPath::kClose_Verb:
            path.close();
            break;
        case SkPath::kDone_Verb:
            return path;
        }
    }
}

void Path::moveTo(PointF point)
{
    m_verbs.push_back(Verb::Move);
    m_points.push_back(point);
}

void Path::lineTo(PointF point)
{
    m_verbs.push_back(Verb::Line);
    m_points.push_back(point);
}

void Path::quadTo(PointF control, PointF end)
{
    m_verbs.push_back(Verb::Quad);
    m_points.push_back(control);
    m_points.push_back(end);
}

void Path::cubicTo(PointF control1, PointF control2, PointF end)
{
    m_verbs.push_back(Verb::Cubic);
    m_points.push_back(control1);
    m_points.push_back(control2);
    m_points.push_back(end);
}

void Path::close()
{
    m_verbs.push_back(Verb::Close);
}

// Cubic segments of at most a quarter turn each, as the common path class
// splits arcs.
void Path::arcTo(const RectF& rect, double startAngle, double sweepLength)
{
    if (rect.width <= 0 || rect.height <= 0)
        return;
    const double rx = rect.width / 2;
    const double ry = rect.height / 2;
    const double cx = rect.x + rx;
    const double cy = rect.y + ry;
    const auto radians = [](double degrees) { return degrees * M_PI / 180; };
    const auto point = [&](double a) { return PointF{cx + rx * std::cos(a), cy - ry * std::sin(a)}; };
    const double start = radians(startAngle);
    const PointF first = point(start);
    if (m_verbs.empty() || m_verbs.back() == Verb::Close)
        moveTo(first);
    else if (!(m_points.back() == first))
        lineTo(first);
    const int segments = std::max(1, int(std::ceil(std::abs(sweepLength) / 90 - 1e-9)));
    const double step = radians(sweepLength) / segments;
    const double k = 4.0 / 3.0 * std::tan(step / 4);
    for (int i = 0; i < segments; ++i) {
        const double a0 = start + step * i;
        const double a1 = a0 + step;
        const PointF p0 = point(a0);
        const PointF p1 = point(a1);
        // Tangents of (cos a, sin a), mapped with y down.
        const PointF c1 = {p0.x - k * rx * std::sin(a0), p0.y - k * ry * std::cos(a0)};
        const PointF c2 = {p1.x + k * rx * std::sin(a1), p1.y + k * ry * std::cos(a1)};
        cubicTo(c1, c2, p1);
    }
}

void Path::clear()
{
    m_verbs.clear();
    m_points.clear();
}

bool Path::isEmpty() const
{
    return m_verbs.empty() || (m_verbs.size() == 1 && m_verbs.front() == Verb::Move);
}

void Path::addRect(const RectF& rect)
{
    moveTo(rect.x, rect.y);
    lineTo(rect.right(), rect.y);
    lineTo(rect.right(), rect.bottom());
    lineTo(rect.x, rect.bottom());
    lineTo(rect.x, rect.y);
    close();
}

void Path::addRoundedRect(const RectF& rect, double rx, double ry)
{
    rx = std::clamp(rx, 0.0, rect.width / 2);
    ry = std::clamp(ry, 0.0, rect.height / 2);
    if (rx <= 0 || ry <= 0) {
        addRect(rect);
        return;
    }
    const double kx = rx * kKappa;
    const double ky = ry * kKappa;
    const double left = rect.x;
    const double top = rect.y;
    const double right = rect.right();
    const double bottom = rect.bottom();
    moveTo(right, top + ry);
    lineTo(right, bottom - ry);
    cubicTo(right, bottom - ry + ky, right - rx + kx, bottom, right - rx, bottom);
    lineTo(left + rx, bottom);
    cubicTo(left + rx - kx, bottom, left, bottom - ry + ky, left, bottom - ry);
    lineTo(left, top + ry);
    cubicTo(left, top + ry - ky, left + rx - kx, top, left + rx, top);
    lineTo(right - rx, top);
    cubicTo(right - rx + kx, top, right, top + ry - ky, right, top + ry);
    close();
}

void Path::addEllipse(const RectF& rect)
{
    const double rx = rect.width / 2;
    const double ry = rect.height / 2;
    const double cx = rect.x + rx;
    const double cy = rect.y + ry;
    const double kx = rx * kKappa;
    const double ky = ry * kKappa;
    moveTo(cx + rx, cy);
    cubicTo(cx + rx, cy + ky, cx + kx, cy + ry, cx, cy + ry);
    cubicTo(cx - kx, cy + ry, cx - rx, cy + ky, cx - rx, cy);
    cubicTo(cx - rx, cy - ky, cx - kx, cy - ry, cx, cy - ry);
    cubicTo(cx + kx, cy - ry, cx + rx, cy - ky, cx + rx, cy);
    close();
}

void Path::addPolygon(const std::vector<PointF>& points, bool closed)
{
    if (points.empty())
        return;
    moveTo(points.front());
    for (size_t i = 1; i < points.size(); ++i)
        lineTo(points[i]);
    if (closed)
        close();
}

void Path::addPath(const Path& path)
{
    m_verbs.insert(m_verbs.end(), path.m_verbs.begin(), path.m_verbs.end());
    m_points.insert(m_points.end(), path.m_points.begin(), path.m_points.end());
}

RectF Path::controlPointRect() const
{
    if (m_points.empty())
        return {};
    double left = m_points.front().x;
    double right = left;
    double top = m_points.front().y;
    double bottom = top;
    for (const PointF& point : m_points) {
        left = std::min(left, point.x);
        right = std::max(right, point.x);
        top = std::min(top, point.y);
        bottom = std::max(bottom, point.y);
    }
    return {left, top, right - left, bottom - top};
}

// Where a cubic's coordinate has its extremes, besides the ends.
static void cubicExtrema(double p0, double p1, double p2, double p3, double* low, double* high)
{
    const auto value = [&](double t) {
        const double u = 1 - t;
        return u * u * u * p0 + 3 * u * u * t * p1 + 3 * u * t * t * p2 + t * t * t * p3;
    };
    const auto consider = [&](double t) {
        if (t > 0 && t < 1) {
            const double v = value(t);
            *low = std::min(*low, v);
            *high = std::max(*high, v);
        }
    };
    // The derivative's coefficients, divided by 3.
    const double a = -p0 + 3 * p1 - 3 * p2 + p3;
    const double b = 2 * (p0 - 2 * p1 + p2);
    const double c = p1 - p0;
    if (std::fabs(a) < 1e-12) {
        if (std::fabs(b) > 1e-12)
            consider(-c / b);
        return;
    }
    const double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
        return;
    const double root = std::sqrt(discriminant);
    consider((-b + root) / (2 * a));
    consider((-b - root) / (2 * a));
}

RectF Path::boundingRect() const
{
    if (m_points.empty())
        return {};
    double left = m_points.front().x;
    double right = left;
    double top = m_points.front().y;
    double bottom = top;
    const auto include = [&](PointF point) {
        left = std::min(left, point.x);
        right = std::max(right, point.x);
        top = std::min(top, point.y);
        bottom = std::max(bottom, point.y);
    };
    PointF current;
    PointF start;
    size_t index = 0;
    for (const Verb verb : m_verbs) {
        switch (verb) {
        case Verb::Move:
            current = start = m_points[index++];
            include(current);
            break;
        case Verb::Line:
            current = m_points[index++];
            include(current);
            break;
        case Verb::Quad: {
            const PointF control = m_points[index];
            const PointF end = m_points[index + 1];
            index += 2;
            const PointF c1{current.x + 2.0 / 3 * (control.x - current.x), current.y + 2.0 / 3 * (control.y - current.y)};
            const PointF c2{end.x + 2.0 / 3 * (control.x - end.x), end.y + 2.0 / 3 * (control.y - end.y)};
            cubicExtrema(current.x, c1.x, c2.x, end.x, &left, &right);
            cubicExtrema(current.y, c1.y, c2.y, end.y, &top, &bottom);
            current = end;
            include(current);
            break;
        }
        case Verb::Cubic: {
            const PointF c1 = m_points[index];
            const PointF c2 = m_points[index + 1];
            const PointF end = m_points[index + 2];
            index += 3;
            cubicExtrema(current.x, c1.x, c2.x, end.x, &left, &right);
            cubicExtrema(current.y, c1.y, c2.y, end.y, &top, &bottom);
            current = end;
            include(current);
            break;
        }
        case Verb::Close:
            current = start;
            break;
        }
    }
    return {left, top, right - left, bottom - top};
}

bool Path::contains(PointF point) const
{
    return PathAccess::toSkPath(*this).contains(SkPoint::Make(SkScalar(point.x), SkScalar(point.y)));
}

std::vector<Path::Element> Path::elements() const
{
    std::vector<Element> result;
    PointF start;
    PointF current;
    bool needMove = true;
    const auto ensureMove = [&] {
        if (!needMove)
            return;
        result.push_back({Element::Type::MoveTo, start.x, start.y});
        current = start;
        needMove = false;
    };
    size_t index = 0;
    for (const Verb verb : m_verbs) {
        switch (verb) {
        case Verb::Move: {
            const PointF point = m_points[index++];
            // A move right after a move replaces it.
            if (!result.empty() && result.back().type == Element::Type::MoveTo)
                result.pop_back();
            result.push_back({Element::Type::MoveTo, point.x, point.y});
            start = current = point;
            needMove = false;
            break;
        }
        case Verb::Line:
            ensureMove();
            current = m_points[index++];
            result.push_back({Element::Type::LineTo, current.x, current.y});
            break;
        case Verb::Quad: {
            ensureMove();
            const PointF control = m_points[index];
            const PointF end = m_points[index + 1];
            index += 2;
            result.push_back({Element::Type::CurveTo, current.x + 2.0 / 3 * (control.x - current.x),
                              current.y + 2.0 / 3 * (control.y - current.y)});
            result.push_back({Element::Type::CurveToData, end.x + 2.0 / 3 * (control.x - end.x),
                              end.y + 2.0 / 3 * (control.y - end.y)});
            result.push_back({Element::Type::CurveToData, end.x, end.y});
            current = end;
            break;
        }
        case Verb::Cubic:
            ensureMove();
            for (int i = 0; i < 3; ++i) {
                const PointF point = m_points[index++];
                result.push_back({i == 0 ? Element::Type::CurveTo : Element::Type::CurveToData, point.x, point.y});
            }
            current = m_points[index - 1];
            break;
        case Verb::Close:
            if (needMove)
                break;
            if (current != start)
                result.push_back({Element::Type::LineTo, start.x, start.y});
            current = start;
            needMove = true;
            break;
        }
    }
    return result;
}

void Path::translate(double dx, double dy)
{
    for (PointF& point : m_points) {
        point.x += dx;
        point.y += dy;
    }
}

Path Path::translated(double dx, double dy) const
{
    Path path = *this;
    path.translate(dx, dy);
    return path;
}

Path Path::transformed(const base::AffineTransform& transform) const
{
    Path path = *this;
    for (PointF& point : path.m_points) {
        double x = 0;
        double y = 0;
        transform.map(point.x, point.y, &x, &y);
        point = {x, y};
    }
    return path;
}

static double distance(PointF a, PointF b)
{
    return std::hypot(b.x - a.x, b.y - a.y);
}

std::vector<std::vector<PointF>> Path::toSubpathPolygons(double tolerance) const
{
    std::vector<std::vector<PointF>> polygons;
    std::vector<PointF> polygon;
    const auto finish = [&] {
        if (polygon.size() > 1)
            polygons.push_back(std::move(polygon));
        polygon.clear();
    };
    const auto cubic = [&](PointF p0, PointF p1, PointF p2, PointF p3) {
        const double length = distance(p0, p1) + distance(p1, p2) + distance(p2, p3);
        const int steps = std::clamp(int(std::ceil(std::sqrt(length / std::max(tolerance, 1e-3)))), 1, 1000);
        for (int i = 1; i <= steps; ++i) {
            const double t = double(i) / steps;
            const double u = 1 - t;
            polygon.push_back({u * u * u * p0.x + 3 * u * u * t * p1.x + 3 * u * t * t * p2.x + t * t * t * p3.x,
                               u * u * u * p0.y + 3 * u * u * t * p1.y + 3 * u * t * t * p2.y + t * t * t * p3.y});
        }
    };
    // The element list has quadratics as cubics and closes as lines already.
    const std::vector<Element> list = elements();
    for (size_t i = 0; i < list.size(); ++i) {
        const Element& element = list[i];
        switch (element.type) {
        case Element::Type::MoveTo:
            finish();
            polygon.push_back({element.x, element.y});
            break;
        case Element::Type::LineTo:
            polygon.push_back({element.x, element.y});
            break;
        case Element::Type::CurveTo:
            cubic(polygon.back(), {element.x, element.y}, {list[i + 1].x, list[i + 1].y},
                  {list[i + 2].x, list[i + 2].y});
            i += 2;
            break;
        case Element::Type::CurveToData:
            break;
        }
    }
    finish();
    return polygons;
}

static Path operate(const Path& one, const Path& two, SkPathOp op)
{
    const std::optional<SkPath> result = Op(PathAccess::toSkPath(one), PathAccess::toSkPath(two), op);
    return result ? PathAccess::fromSkPath(*result) : Path();
}

Path Path::united(const Path& other) const
{
    return operate(*this, other, kUnion_SkPathOp);
}

Path Path::intersected(const Path& other) const
{
    return operate(*this, other, kIntersect_SkPathOp);
}

Path Path::subtracted(const Path& other) const
{
    return operate(*this, other, kDifference_SkPathOp);
}

Path Path::stroked(double width, LineCap cap, LineJoin join, double miterLimit) const
{
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkScalar(width));
    paint.setStrokeMiter(SkScalar(miterLimit));
    paint.setStrokeCap(cap == LineCap::Butt ? SkPaint::kButt_Cap
                               : cap == LineCap::Round ? SkPaint::kRound_Cap
                                                       : SkPaint::kSquare_Cap);
    paint.setStrokeJoin(join == LineJoin::Miter ? SkPaint::kMiter_Join
                                : join == LineJoin::Round ? SkPaint::kRound_Join
                                                          : SkPaint::kBevel_Join);
    Path result = PathAccess::fromSkPath(skpathutils::FillPathWithPaint(PathAccess::toSkPath(*this), paint));
    result.m_fillRule = FillRule::Winding;
    return result;
}

} // namespace Lingmo::ui
