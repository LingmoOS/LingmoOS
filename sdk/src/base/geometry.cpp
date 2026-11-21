// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/geometry.h"

#include <format>
#include <ostream>

namespace Lingmo::base {

static int clampToInt(int64_t value)
{
    return int(std::clamp<int64_t>(value, INT_MIN, INT_MAX));
}

std::string Point::toString() const
{
    return std::format("{},{}", m_x, m_y);
}

std::string PointF::toString() const
{
    return std::format("{},{}", m_x, m_y);
}

std::string Size::toString() const
{
    return std::format("{}x{}", m_width, m_height);
}

std::string SizeF::toString() const
{
    return std::format("{}x{}", m_width, m_height);
}

std::string Insets::toString() const
{
    return std::format("l={} t={} r={} b={}", m_left, m_top, m_right, m_bottom);
}

std::string InsetsF::toString() const
{
    return std::format("l={} t={} r={} b={}", m_left, m_top, m_right, m_bottom);
}

void Rect::setSize(int width, int height)
{
    m_width = int(std::clamp<int64_t>(width, 0, int64_t(INT_MAX) - m_x));
    m_height = int(std::clamp<int64_t>(height, 0, int64_t(INT_MAX) - m_y));
}

Rect Rect::fromEdges(int left, int top, int right, int bottom)
{
    return Rect(left, top, clampToInt(int64_t(right) - left), clampToInt(int64_t(bottom) - top));
}

Rect Rect::boundingRect(const Point& a, const Point& b)
{
    return fromEdges(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::max(a.x(), b.x()),
                     std::max(a.y(), b.y()));
}

bool Rect::intersects(const Rect& rect) const
{
    return !isEmpty() && !rect.isEmpty() && rect.x() < right() && rect.right() > m_x
            && rect.y() < bottom() && rect.bottom() > m_y;
}

Rect Rect::intersected(const Rect& rect) const
{
    if (!intersects(rect))
        return {};
    return fromEdges(std::max(m_x, rect.x()), std::max(m_y, rect.y()), std::min(right(), rect.right()),
                     std::min(bottom(), rect.bottom()));
}

Rect Rect::united(const Rect& rect) const
{
    if (rect.isEmpty())
        return *this;
    if (isEmpty())
        return rect;
    return fromEdges(std::min(m_x, rect.x()), std::min(m_y, rect.y()), std::max(right(), rect.right()),
                     std::max(bottom(), rect.bottom()));
}

std::string Rect::toString() const
{
    return std::format("{},{} {}x{}", m_x, m_y, m_width, m_height);
}

void RectF::setSize(double width, double height)
{
    m_width = width > 0 ? width : 0;
    m_height = height > 0 ? height : 0;
}

RectF RectF::fromEdges(double left, double top, double right, double bottom)
{
    return RectF(left, top, right - left, bottom - top);
}

RectF RectF::boundingRect(const PointF& a, const PointF& b)
{
    return fromEdges(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::max(a.x(), b.x()),
                     std::max(a.y(), b.y()));
}

bool RectF::intersects(const RectF& rect) const
{
    return !isEmpty() && !rect.isEmpty() && rect.x() < right() && rect.right() > m_x
            && rect.y() < bottom() && rect.bottom() > m_y;
}

RectF RectF::intersected(const RectF& rect) const
{
    if (!intersects(rect))
        return {};
    return fromEdges(std::max(m_x, rect.x()), std::max(m_y, rect.y()), std::min(right(), rect.right()),
                     std::min(bottom(), rect.bottom()));
}

RectF RectF::united(const RectF& rect) const
{
    if (rect.isEmpty())
        return *this;
    if (isEmpty())
        return rect;
    return fromEdges(std::min(m_x, rect.x()), std::min(m_y, rect.y()), std::max(right(), rect.right()),
                     std::max(bottom(), rect.bottom()));
}

Rect RectF::toEnclosingRect() const
{
    return Rect::fromEdges(floorToInt(left()), floorToInt(top()), ceilToInt(right()), ceilToInt(bottom()));
}

Rect RectF::toEnclosedRect() const
{
    return Rect::fromEdges(ceilToInt(left()), ceilToInt(top()), floorToInt(right()), floorToInt(bottom()));
}

Rect RectF::toRoundedRect() const
{
    return Rect::fromEdges(roundToInt(left()), roundToInt(top()), roundToInt(right()), roundToInt(bottom()));
}

Rect RectF::toNearestRect() const
{
    const int x = roundToInt(m_x);
    const int y = roundToInt(m_y);
    return Rect(x, y, roundToInt(m_width + (m_x - x) / 2), roundToInt(m_height + (m_y - y) / 2));
}

std::string RectF::toString() const
{
    return std::format("{},{} {}x{}", m_x, m_y, m_width, m_height);
}

Rect scaleToEnclosingRect(const Rect& rect, double scale)
{
    return RectF(rect).scaled(scale).toEnclosingRect();
}

Rect scaleToEnclosedRect(const Rect& rect, double scale)
{
    return RectF(rect).scaled(scale).toEnclosedRect();
}

Rect scaleToRoundedRect(const Rect& rect, double scale)
{
    return RectF(rect).scaled(scale).toRoundedRect();
}

std::ostream& operator<<(std::ostream& stream, const Point& point)
{
    return stream << point.toString();
}

std::ostream& operator<<(std::ostream& stream, const PointF& point)
{
    return stream << point.toString();
}

std::ostream& operator<<(std::ostream& stream, const Size& size)
{
    return stream << size.toString();
}

std::ostream& operator<<(std::ostream& stream, const SizeF& size)
{
    return stream << size.toString();
}

std::ostream& operator<<(std::ostream& stream, const Insets& insets)
{
    return stream << insets.toString();
}

std::ostream& operator<<(std::ostream& stream, const InsetsF& insets)
{
    return stream << insets.toString();
}

std::ostream& operator<<(std::ostream& stream, const Rect& rect)
{
    return stream << rect.toString();
}

std::ostream& operator<<(std::ostream& stream, const RectF& rect)
{
    return stream << rect.toString();
}

} // namespace Lingmo::base
