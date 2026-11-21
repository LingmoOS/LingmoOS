// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <string>

// Points, sizes and rectangles in whole and fractional pixels. Rectangles are
// half-open: right() is x() + width(), and a rectangle never has a negative
// width or height. ui/geometry.h keeps its own plain structs for views.
namespace Lingmo::base {

// Half away from zero, as truncate(value ± 0.5): 0.49999999999999994 gives
// 1, as the x86 build of the common rounding helper does. Saturates at the
// int range; NaN gives 0.
inline int roundToInt(double value)
{
    if (std::isnan(value))
        return 0;
    const double shifted = value + std::copysign(0.5, value);
    if (shifted >= 2147483648.0)
        return INT_MAX;
    if (shifted <= -2147483649.0)
        return INT_MIN;
    return int(shifted);
}

inline int floorToInt(double value)
{
    return roundToInt(std::floor(value));
}

inline int ceilToInt(double value)
{
    return roundToInt(std::ceil(value));
}

// True when the two differ by at most one part in 10^12 of the smaller.
inline bool fuzzyCompare(double a, double b)
{
    return std::fabs(a - b) * 1000000000000. <= std::min(std::fabs(a), std::fabs(b));
}

inline bool fuzzyIsNull(double value)
{
    return std::fabs(value) <= 0.000000000001;
}

class Point
{
public:
    constexpr Point() = default;
    constexpr Point(int x, int y) : m_x(x), m_y(y) {}

    constexpr int x() const { return m_x; }
    constexpr int y() const { return m_y; }
    void setX(int x) { m_x = x; }
    void setY(int y) { m_y = y; }

    constexpr bool isOrigin() const { return m_x == 0 && m_y == 0; }
    int manhattanLength() const { return std::abs(m_x) + std::abs(m_y); }
    Point transposed() const { return {m_y, m_x}; }

    Point& operator+=(const Point& other) { m_x += other.m_x, m_y += other.m_y; return *this; }
    Point& operator-=(const Point& other) { m_x -= other.m_x, m_y -= other.m_y; return *this; }
    friend constexpr Point operator+(const Point& a, const Point& b) { return {a.m_x + b.m_x, a.m_y + b.m_y}; }
    friend constexpr Point operator-(const Point& a, const Point& b) { return {a.m_x - b.m_x, a.m_y - b.m_y}; }
    friend constexpr Point operator-(const Point& point) { return {-point.m_x, -point.m_y}; }

    friend constexpr bool operator==(const Point&, const Point&) = default;

    std::string toString() const;

private:
    int m_x = 0;
    int m_y = 0;
};

class PointF
{
public:
    constexpr PointF() = default;
    constexpr PointF(double x, double y) : m_x(x), m_y(y) {}
    constexpr PointF(const Point& point) : m_x(point.x()), m_y(point.y()) {}

    constexpr double x() const { return m_x; }
    constexpr double y() const { return m_y; }
    void setX(double x) { m_x = x; }
    void setY(double y) { m_y = y; }

    constexpr bool isOrigin() const { return m_x == 0 && m_y == 0; }
    double manhattanLength() const { return std::fabs(m_x) + std::fabs(m_y); }
    double length() const { return std::hypot(m_x, m_y); }
    PointF transposed() const { return {m_y, m_x}; }
    PointF scaled(double scale) const { return {m_x * scale, m_y * scale}; }
    PointF scaled(double sx, double sy) const { return {m_x * sx, m_y * sy}; }

    Point toPoint() const { return {roundToInt(m_x), roundToInt(m_y)}; }
    Point toFlooredPoint() const { return {floorToInt(m_x), floorToInt(m_y)}; }
    Point toCeiledPoint() const { return {ceilToInt(m_x), ceilToInt(m_y)}; }

    PointF& operator+=(const PointF& other) { m_x += other.m_x, m_y += other.m_y; return *this; }
    PointF& operator-=(const PointF& other) { m_x -= other.m_x, m_y -= other.m_y; return *this; }
    PointF& operator*=(double factor) { m_x *= factor, m_y *= factor; return *this; }
    PointF& operator/=(double divisor) { m_x /= divisor, m_y /= divisor; return *this; }
    friend constexpr PointF operator+(const PointF& a, const PointF& b) { return {a.m_x + b.m_x, a.m_y + b.m_y}; }
    friend constexpr PointF operator-(const PointF& a, const PointF& b) { return {a.m_x - b.m_x, a.m_y - b.m_y}; }
    friend constexpr PointF operator-(const PointF& point) { return {-point.m_x, -point.m_y}; }
    friend constexpr PointF operator*(const PointF& point, double factor) { return {point.m_x * factor, point.m_y * factor}; }
    friend constexpr PointF operator*(double factor, const PointF& point) { return point * factor; }
    friend constexpr PointF operator/(const PointF& point, double divisor) { return {point.m_x / divisor, point.m_y / divisor}; }

    friend constexpr bool operator==(const PointF&, const PointF&) = default;

    std::string toString() const;

private:
    double m_x = 0;
    double m_y = 0;
};

// A pair of extents; unlike Rect it keeps negative values, so it doubles as
// a difference of sizes.
class Size
{
public:
    constexpr Size() = default;
    constexpr Size(int width, int height) : m_width(width), m_height(height) {}

    constexpr int width() const { return m_width; }
    constexpr int height() const { return m_height; }
    void setWidth(int width) { m_width = width; }
    void setHeight(int height) { m_height = height; }

    // True when either extent is zero or negative.
    constexpr bool isEmpty() const { return m_width <= 0 || m_height <= 0; }
    int64_t area() const { return int64_t(m_width) * m_height; }
    Size transposed() const { return {m_height, m_width}; }
    Size expandedTo(const Size& other) const
    {
        return {std::max(m_width, other.m_width), std::max(m_height, other.m_height)};
    }
    Size boundedTo(const Size& other) const
    {
        return {std::min(m_width, other.m_width), std::min(m_height, other.m_height)};
    }

    Size& operator+=(const Size& other) { m_width += other.m_width, m_height += other.m_height; return *this; }
    Size& operator-=(const Size& other) { m_width -= other.m_width, m_height -= other.m_height; return *this; }
    friend constexpr Size operator+(const Size& a, const Size& b) { return {a.m_width + b.m_width, a.m_height + b.m_height}; }
    friend constexpr Size operator-(const Size& a, const Size& b) { return {a.m_width - b.m_width, a.m_height - b.m_height}; }

    friend constexpr bool operator==(const Size&, const Size&) = default;

    std::string toString() const;

private:
    int m_width = 0;
    int m_height = 0;
};

class SizeF
{
public:
    constexpr SizeF() = default;
    constexpr SizeF(double width, double height) : m_width(width), m_height(height) {}
    constexpr SizeF(const Size& size) : m_width(size.width()), m_height(size.height()) {}

    constexpr double width() const { return m_width; }
    constexpr double height() const { return m_height; }
    void setWidth(double width) { m_width = width; }
    void setHeight(double height) { m_height = height; }

    constexpr bool isEmpty() const { return !(m_width > 0) || !(m_height > 0); }
    SizeF transposed() const { return {m_height, m_width}; }
    SizeF scaled(double scale) const { return {m_width * scale, m_height * scale}; }
    SizeF scaled(double sx, double sy) const { return {m_width * sx, m_height * sy}; }
    SizeF expandedTo(const SizeF& other) const
    {
        return {std::max(m_width, other.m_width), std::max(m_height, other.m_height)};
    }
    SizeF boundedTo(const SizeF& other) const
    {
        return {std::min(m_width, other.m_width), std::min(m_height, other.m_height)};
    }

    Size toSize() const { return {roundToInt(m_width), roundToInt(m_height)}; }
    Size toFlooredSize() const { return {floorToInt(m_width), floorToInt(m_height)}; }
    Size toCeiledSize() const { return {ceilToInt(m_width), ceilToInt(m_height)}; }

    SizeF& operator+=(const SizeF& other) { m_width += other.m_width, m_height += other.m_height; return *this; }
    SizeF& operator-=(const SizeF& other) { m_width -= other.m_width, m_height -= other.m_height; return *this; }
    friend constexpr SizeF operator+(const SizeF& a, const SizeF& b) { return {a.m_width + b.m_width, a.m_height + b.m_height}; }
    friend constexpr SizeF operator-(const SizeF& a, const SizeF& b) { return {a.m_width - b.m_width, a.m_height - b.m_height}; }
    friend constexpr SizeF operator*(const SizeF& size, double factor) { return {size.m_width * factor, size.m_height * factor}; }
    friend constexpr SizeF operator*(double factor, const SizeF& size) { return size * factor; }
    friend constexpr SizeF operator/(const SizeF& size, double divisor) { return {size.m_width / divisor, size.m_height / divisor}; }

    friend constexpr bool operator==(const SizeF&, const SizeF&) = default;

    std::string toString() const;

private:
    double m_width = 0;
    double m_height = 0;
};

// Edge distances in the order left, top, right, bottom.
class Insets
{
public:
    constexpr Insets() = default;
    constexpr Insets(int left, int top, int right, int bottom)
        : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}
    static constexpr Insets uniform(int value) { return {value, value, value, value}; }

    constexpr int left() const { return m_left; }
    constexpr int top() const { return m_top; }
    constexpr int right() const { return m_right; }
    constexpr int bottom() const { return m_bottom; }
    void setLeft(int left) { m_left = left; }
    void setTop(int top) { m_top = top; }
    void setRight(int right) { m_right = right; }
    void setBottom(int bottom) { m_bottom = bottom; }

    constexpr int horizontal() const { return m_left + m_right; }
    constexpr int vertical() const { return m_top + m_bottom; }
    constexpr bool isEmpty() const { return !m_left && !m_top && !m_right && !m_bottom; }

    friend constexpr Insets operator+(const Insets& a, const Insets& b)
    {
        return {a.m_left + b.m_left, a.m_top + b.m_top, a.m_right + b.m_right, a.m_bottom + b.m_bottom};
    }
    friend constexpr Insets operator-(const Insets& a, const Insets& b)
    {
        return {a.m_left - b.m_left, a.m_top - b.m_top, a.m_right - b.m_right, a.m_bottom - b.m_bottom};
    }
    friend constexpr Insets operator-(const Insets& insets)
    {
        return {-insets.m_left, -insets.m_top, -insets.m_right, -insets.m_bottom};
    }

    friend constexpr bool operator==(const Insets&, const Insets&) = default;

    std::string toString() const;

private:
    int m_left = 0;
    int m_top = 0;
    int m_right = 0;
    int m_bottom = 0;
};

class InsetsF
{
public:
    constexpr InsetsF() = default;
    constexpr InsetsF(double left, double top, double right, double bottom)
        : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}
    constexpr InsetsF(const Insets& insets)
        : m_left(insets.left()), m_top(insets.top()), m_right(insets.right()), m_bottom(insets.bottom()) {}
    static constexpr InsetsF uniform(double value) { return {value, value, value, value}; }

    constexpr double left() const { return m_left; }
    constexpr double top() const { return m_top; }
    constexpr double right() const { return m_right; }
    constexpr double bottom() const { return m_bottom; }
    void setLeft(double left) { m_left = left; }
    void setTop(double top) { m_top = top; }
    void setRight(double right) { m_right = right; }
    void setBottom(double bottom) { m_bottom = bottom; }

    constexpr double horizontal() const { return m_left + m_right; }
    constexpr double vertical() const { return m_top + m_bottom; }
    constexpr bool isEmpty() const { return !m_left && !m_top && !m_right && !m_bottom; }

    InsetsF scaled(double scale) const { return {m_left * scale, m_top * scale, m_right * scale, m_bottom * scale}; }

    friend constexpr bool operator==(const InsetsF&, const InsetsF&) = default;

    std::string toString() const;

private:
    double m_left = 0;
    double m_top = 0;
    double m_right = 0;
    double m_bottom = 0;
};

class RectF;

// Half-open whole-pixel rectangle. A negative width or height becomes zero,
// and the size shrinks so that right() and bottom() stay within int.
class Rect
{
public:
    constexpr Rect() = default;
    Rect(int x, int y, int width, int height) : m_x(x), m_y(y) { setSize(width, height); }
    Rect(const Point& origin, const Size& size) : Rect(origin.x(), origin.y(), size.width(), size.height()) {}
    explicit Rect(const Size& size) : Rect(0, 0, size.width(), size.height()) {}

    // From edges; empty when right < left or bottom < top.
    static Rect fromEdges(int left, int top, int right, int bottom);
    // The smallest rectangle holding both points, as edges in either order.
    static Rect boundingRect(const Point& a, const Point& b);

    constexpr int x() const { return m_x; }
    constexpr int y() const { return m_y; }
    constexpr int width() const { return m_width; }
    constexpr int height() const { return m_height; }
    constexpr int left() const { return m_x; }
    constexpr int top() const { return m_y; }
    constexpr int right() const { return m_x + m_width; }
    constexpr int bottom() const { return m_y + m_height; }

    constexpr Point origin() const { return {m_x, m_y}; }
    constexpr Point topLeft() const { return {m_x, m_y}; }
    constexpr Point topRight() const { return {right(), m_y}; }
    constexpr Point bottomLeft() const { return {m_x, bottom()}; }
    constexpr Point bottomRight() const { return {right(), bottom()}; }
    // x + width / 2, rounded towards zero.
    constexpr Point center() const { return {m_x + m_width / 2, m_y + m_height / 2}; }
    PointF centerF() const { return {m_x + m_width / 2.0, m_y + m_height / 2.0}; }
    constexpr Size size() const { return {m_width, m_height}; }

    void setX(int x) { m_x = x; setSize(m_width, m_height); }
    void setY(int y) { m_y = y; setSize(m_width, m_height); }
    void setWidth(int width) { setSize(width, m_height); }
    void setHeight(int height) { setSize(m_width, height); }
    void setSize(const Size& size) { setSize(size.width(), size.height()); }
    void setSize(int width, int height);
    void setRect(int x, int y, int width, int height) { m_x = x, m_y = y; setSize(width, height); }

    // Move one edge, keeping the opposite edge in place.
    void setLeft(int left) { *this = fromEdges(left, top(), right(), bottom()); }
    void setTop(int top) { *this = fromEdges(left(), top, right(), bottom()); }
    void setRight(int right) { setWidth(right - m_x); }
    void setBottom(int bottom) { setHeight(bottom - m_y); }

    // Move the rectangle, keeping its size.
    void moveTo(int x, int y) { setRect(x, y, m_width, m_height); }
    void moveTo(const Point& point) { moveTo(point.x(), point.y()); }
    void moveTopLeft(const Point& point) { moveTo(point); }
    void moveLeft(int left) { moveTo(left, m_y); }
    void moveTop(int top) { moveTo(m_x, top); }
    void moveRight(int right) { moveTo(right - m_width, m_y); }
    void moveBottom(int bottom) { moveTo(m_x, bottom - m_height); }
    void moveCenter(const Point& point) { moveTo(point.x() - m_width / 2, point.y() - m_height / 2); }

    void translate(int dx, int dy) { moveTo(m_x + dx, m_y + dy); }
    void translate(const Point& offset) { translate(offset.x(), offset.y()); }
    Rect translated(int dx, int dy) const { Rect r = *this; r.translate(dx, dy); return r; }
    Rect translated(const Point& offset) const { return translated(offset.x(), offset.y()); }
    Rect transposed() const { return {m_x, m_y, m_height, m_width}; }

    // Adds the deltas to the left, top, right and bottom edges.
    Rect adjusted(int dx1, int dy1, int dx2, int dy2) const
    {
        return fromEdges(left() + dx1, top() + dy1, right() + dx2, bottom() + dy2);
    }
    Rect marginsAdded(const Insets& insets) const
    {
        return adjusted(-insets.left(), -insets.top(), insets.right(), insets.bottom());
    }
    Rect marginsRemoved(const Insets& insets) const
    {
        return adjusted(insets.left(), insets.top(), -insets.right(), -insets.bottom());
    }

    // True when either extent is zero.
    constexpr bool isEmpty() const { return m_width == 0 || m_height == 0; }
    int64_t area() const { return int64_t(m_width) * m_height; }

    bool contains(int x, int y) const { return x >= m_x && x < right() && y >= m_y && y < bottom(); }
    bool contains(const Point& point) const { return contains(point.x(), point.y()); }
    // True when `rect` lies wholly inside; an empty `rect` counts when its
    // origin and far corner fall within the edges.
    bool contains(const Rect& rect) const
    {
        return rect.x() >= m_x && rect.right() <= right() && rect.y() >= m_y && rect.bottom() <= bottom();
    }
    // False when either is empty.
    bool intersects(const Rect& rect) const;
    // Empty, at the origin, when they do not overlap.
    Rect intersected(const Rect& rect) const;
    // The bounding rectangle; an empty operand is ignored.
    Rect united(const Rect& rect) const;

    Rect& operator&=(const Rect& rect) { return *this = intersected(rect); }
    Rect& operator|=(const Rect& rect) { return *this = united(rect); }
    friend Rect operator&(const Rect& a, const Rect& b) { return a.intersected(b); }
    friend Rect operator|(const Rect& a, const Rect& b) { return a.united(b); }

    friend constexpr bool operator==(const Rect&, const Rect&) = default;

    std::string toString() const;

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
};

// Half-open fractional rectangle; a negative or NaN width or height becomes
// zero.
class RectF
{
public:
    constexpr RectF() = default;
    RectF(double x, double y, double width, double height) : m_x(x), m_y(y) { setSize(width, height); }
    RectF(const PointF& origin, const SizeF& size) : RectF(origin.x(), origin.y(), size.width(), size.height()) {}
    explicit RectF(const SizeF& size) : RectF(0, 0, size.width(), size.height()) {}
    constexpr RectF(const Rect& rect) : m_x(rect.x()), m_y(rect.y()), m_width(rect.width()), m_height(rect.height()) {}

    static RectF fromEdges(double left, double top, double right, double bottom);
    static RectF boundingRect(const PointF& a, const PointF& b);

    constexpr double x() const { return m_x; }
    constexpr double y() const { return m_y; }
    constexpr double width() const { return m_width; }
    constexpr double height() const { return m_height; }
    constexpr double left() const { return m_x; }
    constexpr double top() const { return m_y; }
    constexpr double right() const { return m_x + m_width; }
    constexpr double bottom() const { return m_y + m_height; }

    constexpr PointF origin() const { return {m_x, m_y}; }
    constexpr PointF topLeft() const { return {m_x, m_y}; }
    constexpr PointF topRight() const { return {right(), m_y}; }
    constexpr PointF bottomLeft() const { return {m_x, bottom()}; }
    constexpr PointF bottomRight() const { return {right(), bottom()}; }
    constexpr PointF center() const { return {m_x + m_width / 2, m_y + m_height / 2}; }
    constexpr SizeF size() const { return {m_width, m_height}; }

    void setX(double x) { m_x = x; }
    void setY(double y) { m_y = y; }
    void setWidth(double width) { setSize(width, m_height); }
    void setHeight(double height) { setSize(m_width, height); }
    void setSize(const SizeF& size) { setSize(size.width(), size.height()); }
    void setSize(double width, double height);
    void setRect(double x, double y, double width, double height) { m_x = x, m_y = y; setSize(width, height); }

    void setLeft(double left) { *this = fromEdges(left, top(), right(), bottom()); }
    void setTop(double top) { *this = fromEdges(left(), top, right(), bottom()); }
    void setRight(double right) { setWidth(right - m_x); }
    void setBottom(double bottom) { setHeight(bottom - m_y); }

    void moveTo(double x, double y) { m_x = x, m_y = y; }
    void moveTo(const PointF& point) { moveTo(point.x(), point.y()); }
    void moveTopLeft(const PointF& point) { moveTo(point); }
    void moveLeft(double left) { m_x = left; }
    void moveTop(double top) { m_y = top; }
    void moveRight(double right) { m_x = right - m_width; }
    void moveBottom(double bottom) { m_y = bottom - m_height; }
    void moveCenter(const PointF& point) { moveTo(point.x() - m_width / 2, point.y() - m_height / 2); }

    void translate(double dx, double dy) { m_x += dx, m_y += dy; }
    void translate(const PointF& offset) { translate(offset.x(), offset.y()); }
    RectF translated(double dx, double dy) const { RectF r = *this; r.translate(dx, dy); return r; }
    RectF translated(const PointF& offset) const { return translated(offset.x(), offset.y()); }
    RectF transposed() const { return {m_x, m_y, m_height, m_width}; }
    // Origin and size both multiplied.
    RectF scaled(double scale) const { return scaled(scale, scale); }
    RectF scaled(double sx, double sy) const { return {m_x * sx, m_y * sy, m_width * sx, m_height * sy}; }

    RectF adjusted(double dx1, double dy1, double dx2, double dy2) const
    {
        return fromEdges(left() + dx1, top() + dy1, right() + dx2, bottom() + dy2);
    }
    RectF marginsAdded(const InsetsF& insets) const
    {
        return adjusted(-insets.left(), -insets.top(), insets.right(), insets.bottom());
    }
    RectF marginsRemoved(const InsetsF& insets) const
    {
        return adjusted(insets.left(), insets.top(), -insets.right(), -insets.bottom());
    }

    constexpr bool isEmpty() const { return m_width == 0 || m_height == 0; }

    // Half-open: a point on the right or bottom edge is outside.
    bool contains(double x, double y) const { return x >= m_x && x < right() && y >= m_y && y < bottom(); }
    bool contains(const PointF& point) const { return contains(point.x(), point.y()); }
    bool contains(const RectF& rect) const
    {
        return rect.x() >= m_x && rect.right() <= right() && rect.y() >= m_y && rect.bottom() <= bottom();
    }
    bool intersects(const RectF& rect) const;
    RectF intersected(const RectF& rect) const;
    RectF united(const RectF& rect) const;

    RectF& operator&=(const RectF& rect) { return *this = intersected(rect); }
    RectF& operator|=(const RectF& rect) { return *this = united(rect); }
    friend RectF operator&(const RectF& a, const RectF& b) { return a.intersected(b); }
    friend RectF operator|(const RectF& a, const RectF& b) { return a.united(b); }

    // Floors the near edges and ceils the far ones: every touched pixel.
    Rect toEnclosingRect() const;
    // Ceils the near edges and floors the far ones: only whole pixels.
    Rect toEnclosedRect() const;
    // Rounds each edge.
    Rect toRoundedRect() const;
    // Rounds the origin, and the size with half the origin's rounding error
    // added, so the size is off by at most 0.75 and the origin by 0.5.
    Rect toNearestRect() const;

    friend constexpr bool operator==(const RectF&, const RectF&) = default;

    std::string toString() const;

private:
    double m_x = 0;
    double m_y = 0;
    double m_width = 0;
    double m_height = 0;
};

// Equality up to rounding error, as the common 2D types compare with ==:
// each coordinate by fuzzyCompare, or by fuzzyIsNull of the difference when
// either is 0. A rectangle compares its origin and its size.
inline bool fuzzyCompareCoordinate(double a, double b)
{
    return (a == 0 || b == 0) ? fuzzyIsNull(a - b) : fuzzyCompare(a, b);
}

inline bool fuzzyCompare(const PointF& a, const PointF& b)
{
    return fuzzyCompareCoordinate(a.x(), b.x()) && fuzzyCompareCoordinate(a.y(), b.y());
}

inline bool fuzzyCompare(const SizeF& a, const SizeF& b)
{
    return fuzzyCompareCoordinate(a.width(), b.width()) && fuzzyCompareCoordinate(a.height(), b.height());
}

inline bool fuzzyCompare(const RectF& a, const RectF& b)
{
    return fuzzyCompare(a.topLeft(), b.topLeft()) && fuzzyCompare(a.size(), b.size());
}

// The rectangle form of the fuzzy compare function rather than ==: origin
// and far corner.
inline bool fuzzyCompareCorners(const RectF& a, const RectF& b)
{
    return fuzzyCompare(a.topLeft(), b.topLeft()) && fuzzyCompare(a.bottomRight(), b.bottomRight());
}

// Scaling a whole-pixel rectangle into another whole-pixel one.
Rect scaleToEnclosingRect(const Rect& rect, double scale);
Rect scaleToEnclosedRect(const Rect& rect, double scale);
Rect scaleToRoundedRect(const Rect& rect, double scale);

std::ostream& operator<<(std::ostream& stream, const Point& point);
std::ostream& operator<<(std::ostream& stream, const PointF& point);
std::ostream& operator<<(std::ostream& stream, const Size& size);
std::ostream& operator<<(std::ostream& stream, const SizeF& size);
std::ostream& operator<<(std::ostream& stream, const Insets& insets);
std::ostream& operator<<(std::ostream& stream, const InsetsF& insets);
std::ostream& operator<<(std::ostream& stream, const Rect& rect);
std::ostream& operator<<(std::ostream& stream, const RectF& rect);

} // namespace Lingmo::base
