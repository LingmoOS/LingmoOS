// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <cstdint>
#include <vector>

#include "ui/geometry.h"

namespace Lingmo::base {
class AffineTransform;
}

namespace Lingmo::ui {

enum class LineCap {
    Butt,
    Round,
    Square,
};

enum class LineJoin {
    Miter,
    Round,
    Bevel,
};

// An outline of lines and curves, in double precision. Views draw it in
// logical coordinates, relative to the view, antialiased and not snapped to
// the device grid; BitmapPainter draws it in its own coordinates.
class Path
{
public:
    // Winding by default; the common 2D path class defaults to EvenOdd.
    enum class FillRule {
        Winding,
        EvenOdd,
    };

    // The common element list: a cubic is one CurveTo (the first control
    // point) and two CurveToData (the second control point, the end); a
    // quadratic becomes the equal cubic, and a close a line back to the
    // subpath's start unless it is there already.
    struct Element
    {
        enum class Type {
            MoveTo,
            LineTo,
            CurveTo,
            CurveToData,
        };
        Type type;
        double x;
        double y;

        bool operator==(const Element&) const = default;
    };

    void moveTo(PointF point);
    void lineTo(PointF point);
    void quadTo(PointF control, PointF end);
    void cubicTo(PointF control1, PointF control2, PointF end);
    void close();
    // The common 2D path class's arcTo: a line from the current point (a
    // move on an empty path) to the arc's start, then the arc of the
    // ellipse inside the rect from startAngle over sweepLength degrees,
    // counter-clockwise on screen for positive angles, 0 at three o'clock.
    // Nothing for an empty rect.
    void arcTo(const RectF& rect, double startAngle, double sweepLength);

    void moveTo(double x, double y) { moveTo(PointF{x, y}); }
    void lineTo(double x, double y) { lineTo(PointF{x, y}); }
    void quadTo(double cx, double cy, double x, double y) { quadTo(PointF{cx, cy}, PointF{x, y}); }
    void cubicTo(double c1x, double c1y, double c2x, double c2y, double x, double y)
    {
        cubicTo(PointF{c1x, c1y}, PointF{c2x, c2y}, PointF{x, y});
    }

    // Closed subpaths of their own.
    void addRect(const RectF& rect);
    // Corners of radii rx and ry, clamped to half the size.
    void addRoundedRect(const RectF& rect, double rx, double ry);
    void addEllipse(const RectF& rect);
    void addPolygon(const std::vector<PointF>& points, bool close = true);
    void addPath(const Path& path);

    FillRule fillRule() const { return m_fillRule; }
    void setFillRule(FillRule rule) { m_fillRule = rule; }

    // True without elements, or with nothing but one move.
    bool isEmpty() const;
    void clear();

    // The exact bounds of the outline, curves included.
    RectF boundingRect() const;
    // The bounds of every point, control points included.
    RectF controlPointRect() const;
    // Whether the fill covers the point, by the fill rule.
    bool contains(PointF point) const;

    std::vector<Element> elements() const;
    size_t elementCount() const { return elements().size(); }

    void translate(double dx, double dy);
    Path translated(double dx, double dy) const;
    Path transformed(const base::AffineTransform& transform) const;

    // Each subpath flattened into a polygon: curves become lines no further
    // than `tolerance` from them.
    std::vector<std::vector<PointF>> toSubpathPolygons(double tolerance = 0.25) const;

    // Boolean operations on the filled areas; the results have curves
    // flattened to single precision.
    Path united(const Path& other) const;
    Path intersected(const Path& other) const;
    Path subtracted(const Path& other) const;
    // The area a pen of `width` covers along the outline. The defaults are
    // the common stroker's.
    Path stroked(double width, LineCap cap = LineCap::Square, LineJoin join = LineJoin::Bevel,
                 double miterLimit = 2) const;

    bool operator==(const Path&) const = default;

private:
    friend class Painter;
    friend class BitmapPainter;
    friend struct PathAccess;

    enum class Verb : uint8_t {
        Move,
        Line,
        Quad,
        Cubic,
        Close,
    };

    std::vector<Verb> m_verbs;
    std::vector<PointF> m_points;
    FillRule m_fillRule = FillRule::Winding;
};

struct GradientStop
{
    // 0 at the start, 1 at the end.
    double position = 0;
    Color color;
};

// Colours along a line, in the same logical coordinates as what is filled.
struct LinearGradient
{
    PointF start;
    PointF end;
    std::vector<GradientStop> stops;

    // From the top colour at the rect's top to the bottom one at its bottom.
    static LinearGradient vertical(const RectF& rect, Color top, Color bottom)
    {
        return {{rect.x, rect.y}, {rect.x, rect.bottom()}, {{0, top}, {1, bottom}}};
    }
};

} // namespace Lingmo::ui
