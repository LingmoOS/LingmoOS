// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/transform.h"

#include <format>
#include <numbers>
#include <ostream>
#include <vector>

namespace Lingmo::base {

AffineTransform AffineTransform::fromRotation(double degrees)
{
    AffineTransform transform;
    transform.rotate(degrees);
    return transform;
}

AffineTransform AffineTransform::fromTranslate(double dx, double dy)
{
    AffineTransform transform;
    transform.m_dx = dx;
    transform.m_dy = dy;
    transform.m_type = dx == 0 && dy == 0 ? Type::Identity : Type::Translate;
    return transform;
}

AffineTransform AffineTransform::fromScale(double sx, double sy)
{
    AffineTransform transform;
    transform.m_11 = sx;
    transform.m_22 = sy;
    transform.m_type = sx == 1 && sy == 1 ? Type::Identity : Type::Scale;
    return transform;
}

AffineTransform::Type AffineTransform::type() const
{
    if (m_dirty == Type::Identity || m_dirty < m_type)
        return m_type;
    // Only kinds up to the highest change are worked out again.
    switch (m_dirty) {
    case Type::Shear:
    case Type::Rotate:
        if (!fuzzyIsNull(m_12) || !fuzzyIsNull(m_21)) {
            const double dot = m_11 * m_21 + m_12 * m_22;
            m_type = fuzzyIsNull(dot) ? Type::Rotate : Type::Shear;
            break;
        }
        [[fallthrough]];
    case Type::Scale:
        if (!fuzzyIsNull(m_11 - 1) || !fuzzyIsNull(m_22 - 1)) {
            m_type = Type::Scale;
            break;
        }
        [[fallthrough]];
    case Type::Translate:
        if (!fuzzyIsNull(m_dx) || !fuzzyIsNull(m_dy)) {
            m_type = Type::Translate;
            break;
        }
        [[fallthrough]];
    case Type::Identity:
        m_type = Type::Identity;
        break;
    }
    m_dirty = Type::Identity;
    return m_type;
}

AffineTransform AffineTransform::inverted(bool* invertible) const
{
    AffineTransform result;
    bool ok = true;
    switch (type()) {
    case Type::Identity:
        break;
    case Type::Translate:
        result.m_dx = -m_dx;
        result.m_dy = -m_dy;
        break;
    case Type::Scale:
        ok = !fuzzyIsNull(m_11) && !fuzzyIsNull(m_22);
        if (ok) {
            result.m_11 = 1. / m_11;
            result.m_22 = 1. / m_22;
            result.m_dx = -m_dx * result.m_11;
            result.m_dy = -m_dy * result.m_22;
        }
        break;
    case Type::Rotate:
    case Type::Shear: {
        const double det = determinant();
        ok = !fuzzyIsNull(det);
        if (ok) {
            // The adjugate times the reciprocal, not divided: the last bits
            // follow the common implementation.
            const double reciprocal = 1 / det;
            result = AffineTransform(m_22 * reciprocal, -m_12 * reciprocal, -m_21 * reciprocal,
                                     m_11 * reciprocal, (m_21 * m_dy - m_22 * m_dx) * reciprocal,
                                     (m_12 * m_dx - m_11 * m_dy) * reciprocal);
        }
        break;
    }
    }
    if (invertible)
        *invertible = ok;
    if (ok) {
        result.m_type = m_type;
        result.m_dirty = m_dirty;
    }
    return result;
}

AffineTransform& AffineTransform::translate(double dx, double dy)
{
    if (dx == 0 && dy == 0)
        return *this;
    switch (type()) {
    case Type::Identity:
        m_dx = dx;
        m_dy = dy;
        break;
    case Type::Translate:
        m_dx += dx;
        m_dy += dy;
        break;
    case Type::Scale:
        m_dx += dx * m_11;
        m_dy += dy * m_22;
        break;
    case Type::Rotate:
    case Type::Shear:
        m_dx += dx * m_11 + dy * m_21;
        m_dy += dy * m_22 + dx * m_12;
        break;
    }
    m_dirty = std::max(m_dirty, Type::Translate);
    return *this;
}

AffineTransform& AffineTransform::scale(double sx, double sy)
{
    if (sx == 1 && sy == 1)
        return *this;
    switch (type()) {
    case Type::Identity:
    case Type::Translate:
        m_11 = sx;
        m_22 = sy;
        break;
    case Type::Rotate:
    case Type::Shear:
        m_12 *= sx;
        m_21 *= sy;
        [[fallthrough]];
    case Type::Scale:
        m_11 *= sx;
        m_22 *= sy;
        break;
    }
    m_dirty = std::max(m_dirty, Type::Scale);
    return *this;
}

AffineTransform& AffineTransform::rotate(double degrees)
{
    if (degrees == 0)
        return *this;
    double sina = 0;
    double cosa = 0;
    if (degrees == 90. || degrees == -270.) {
        sina = 1.;
    } else if (degrees == 270. || degrees == -90.) {
        sina = -1.;
    } else if (degrees == 180.) {
        cosa = -1.;
    } else {
        const double radians = degrees * (std::numbers::pi / 180);
        sina = std::sin(radians);
        cosa = std::cos(radians);
    }
    switch (type()) {
    case Type::Identity:
    case Type::Translate:
        m_11 = cosa;
        m_12 = sina;
        m_21 = -sina;
        m_22 = cosa;
        break;
    case Type::Scale: {
        const double t11 = cosa * m_11;
        const double t12 = sina * m_22;
        const double t21 = -sina * m_11;
        const double t22 = cosa * m_22;
        m_11 = t11;
        m_12 = t12;
        m_21 = t21;
        m_22 = t22;
        break;
    }
    case Type::Rotate:
    case Type::Shear: {
        const double t11 = cosa * m_11 + sina * m_21;
        const double t12 = cosa * m_12 + sina * m_22;
        const double t21 = -sina * m_11 + cosa * m_21;
        const double t22 = -sina * m_12 + cosa * m_22;
        m_11 = t11;
        m_12 = t12;
        m_21 = t21;
        m_22 = t22;
        break;
    }
    }
    m_dirty = std::max(m_dirty, Type::Rotate);
    return *this;
}

AffineTransform& AffineTransform::operator*=(const AffineTransform& o)
{
    const Type otherType = o.type();
    if (otherType == Type::Identity)
        return *this;
    const Type thisType = type();
    if (thisType == Type::Identity)
        return *this = o;

    const Type t = std::max(thisType, otherType);
    switch (t) {
    case Type::Identity:
        break;
    case Type::Translate:
        m_dx += o.m_dx;
        m_dy += o.m_dy;
        break;
    case Type::Scale: {
        const double m11 = m_11 * o.m_11;
        const double m22 = m_22 * o.m_22;
        const double dx = m_dx * o.m_11 + o.m_dx;
        const double dy = m_dy * o.m_22 + o.m_dy;
        m_11 = m11;
        m_22 = m22;
        m_dx = dx;
        m_dy = dy;
        break;
    }
    case Type::Rotate:
    case Type::Shear: {
        const double m11 = m_11 * o.m_11 + m_12 * o.m_21;
        const double m12 = m_11 * o.m_12 + m_12 * o.m_22;
        const double m21 = m_21 * o.m_11 + m_22 * o.m_21;
        const double m22 = m_21 * o.m_12 + m_22 * o.m_22;
        const double dx = m_dx * o.m_11 + m_dy * o.m_21 + o.m_dx;
        const double dy = m_dx * o.m_12 + m_dy * o.m_22 + o.m_dy;
        m_11 = m11;
        m_12 = m12;
        m_21 = m21;
        m_22 = m22;
        m_dx = dx;
        m_dy = dy;
        break;
    }
    }
    m_type = t;
    m_dirty = t;
    return *this;
}

// As *=, but into a fresh identity: entries the kind leaves out stay exact.
AffineTransform AffineTransform::multiplied(const AffineTransform& o) const
{
    const Type otherType = o.type();
    if (otherType == Type::Identity)
        return *this;
    const Type thisType = type();
    if (thisType == Type::Identity)
        return o;

    const Type t = std::max(thisType, otherType);
    AffineTransform result;
    switch (t) {
    case Type::Identity:
        break;
    case Type::Translate:
        result.m_dx = m_dx + o.m_dx;
        result.m_dy = m_dy + o.m_dy;
        break;
    case Type::Scale:
        result.m_11 = m_11 * o.m_11;
        result.m_22 = m_22 * o.m_22;
        result.m_dx = m_dx * o.m_11 + o.m_dx;
        result.m_dy = m_dy * o.m_22 + o.m_dy;
        break;
    case Type::Rotate:
    case Type::Shear:
        result.m_11 = m_11 * o.m_11 + m_12 * o.m_21;
        result.m_12 = m_11 * o.m_12 + m_12 * o.m_22;
        result.m_21 = m_21 * o.m_11 + m_22 * o.m_21;
        result.m_22 = m_21 * o.m_12 + m_22 * o.m_22;
        result.m_dx = m_dx * o.m_11 + m_dy * o.m_21 + o.m_dx;
        result.m_dy = m_dx * o.m_12 + m_dy * o.m_22 + o.m_dy;
        break;
    }
    result.m_type = t;
    result.m_dirty = t;
    return result;
}

void AffineTransform::map(double x, double y, double* mappedX, double* mappedY) const
{
    switch (type()) {
    case Type::Identity:
        *mappedX = x;
        *mappedY = y;
        return;
    case Type::Translate:
        *mappedX = x + m_dx;
        *mappedY = y + m_dy;
        return;
    case Type::Scale:
        *mappedX = m_11 * x + m_dx;
        *mappedY = m_22 * y + m_dy;
        return;
    case Type::Rotate:
    case Type::Shear:
        *mappedX = m_11 * x + m_21 * y + m_dx;
        *mappedY = m_12 * x + m_22 * y + m_dy;
        return;
    }
}

PointF AffineTransform::map(const PointF& point) const
{
    double x = 0;
    double y = 0;
    map(point.x(), point.y(), &x, &y);
    return {x, y};
}

Point AffineTransform::map(const Point& point) const
{
    return map(PointF(point)).toPoint();
}

RectF AffineTransform::mapRect(const RectF& rect) const
{
    const Type t = type();
    if (t <= Type::Translate)
        return rect.translated(m_dx, m_dy);
    if (t == Type::Scale) {
        double x = m_11 * rect.x() + m_dx;
        double y = m_22 * rect.y() + m_dy;
        double w = m_11 * rect.width();
        double h = m_22 * rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        return RectF(x, y, w, h);
    }
    double x = 0;
    double y = 0;
    map(rect.x(), rect.y(), &x, &y);
    double xmin = x, ymin = y, xmax = x, ymax = y;
    const auto extend = [&](double px, double py) {
        map(px, py, &x, &y);
        xmin = std::min(xmin, x);
        ymin = std::min(ymin, y);
        xmax = std::max(xmax, x);
        ymax = std::max(ymax, y);
    };
    extend(rect.x() + rect.width(), rect.y());
    extend(rect.x() + rect.width(), rect.y() + rect.height());
    extend(rect.x(), rect.y() + rect.height());
    return RectF(xmin, ymin, xmax - xmin, ymax - ymin);
}

Rect AffineTransform::mapRect(const Rect& rect) const
{
    const Type t = type();
    if (t <= Type::Translate)
        return rect.translated(roundToInt(m_dx), roundToInt(m_dy));
    if (t == Type::Scale) {
        int x = roundToInt(m_11 * rect.x() + m_dx);
        int y = roundToInt(m_22 * rect.y() + m_dy);
        int w = roundToInt(m_11 * rect.width());
        int h = roundToInt(m_22 * rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        return Rect(x, y, w, h);
    }
    return mapRect(RectF(rect)).toNearestRect();
}

Region AffineTransform::mapToEnclosingRegion(const Region& region) const
{
    if (type() == Type::Identity)
        return region;
    std::vector<Rect> rects;
    rects.reserve(region.rectCount());
    for (const Rect& rect : region)
        rects.push_back(mapRect(RectF(rect)).toEnclosingRect());
    return Region::fromRects(rects);
}

std::string AffineTransform::toString() const
{
    return std::format("[{} {} {} {} {} {}]", m_11, m_12, m_21, m_22, m_dx, m_dy);
}

std::ostream& operator<<(std::ostream& stream, const AffineTransform& transform)
{
    return stream << transform.toString();
}

} // namespace Lingmo::base
