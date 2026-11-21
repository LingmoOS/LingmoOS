// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <string>

#include "base/geometry.h"
#include "base/region.h"

namespace Lingmo::base {

// A 2D affine transform with row vectors, laid out and composed as the
// common 2D graphics transform: a point maps to
//   x' = m11 x + m21 y + dx,   y' = m12 x + m22 y + dy,
// `a * b` applies a first, then b, and translate(), scale() and rotate()
// apply their step before the transform's own. Like the common
// implementation it remembers its kind lazily (a later translate or scale
// keeps a rotation's kind even when the shear has become negligible) and maps
// through the same fast paths with the same 1e-12 tolerance, so results
// match bit for bit.
class AffineTransform
{
public:
    enum class Type { Identity, Translate, Scale, Rotate, Shear };

    constexpr AffineTransform() = default;
    constexpr AffineTransform(double m11, double m12, double m21, double m22, double dx, double dy)
        : m_11(m11), m_12(m12), m_21(m21), m_22(m22), m_dx(dx), m_dy(dy), m_dirty(Type::Shear) {}

    static AffineTransform fromTranslate(double dx, double dy);
    static AffineTransform fromScale(double sx, double sy);
    // Exact for multiples of 90 degrees; positive turns clockwise on a
    // y-down screen.
    static AffineTransform fromRotation(double degrees);

    constexpr double m11() const { return m_11; }
    constexpr double m12() const { return m_12; }
    constexpr double m21() const { return m_21; }
    constexpr double m22() const { return m_22; }
    constexpr double dx() const { return m_dx; }
    constexpr double dy() const { return m_dy; }

    // The kind, ignoring differences up to 1e-12 where it was last worked
    // out.
    Type type() const;
    bool isIdentity() const { return type() == Type::Identity; }
    bool isTranslating() const { return type() >= Type::Translate; }
    bool isScaling() const { return type() >= Type::Scale; }
    bool isRotating() const { return type() >= Type::Rotate; }
    double determinant() const { return m_11 * m_22 - m_12 * m_21; }

    // Identity, with `invertible` false, when the determinant (or a scale)
    // is within 1e-12 of zero.
    AffineTransform inverted(bool* invertible = nullptr) const;

    AffineTransform& translate(double dx, double dy);
    AffineTransform& scale(double sx, double sy);
    AffineTransform& rotate(double degrees);

    AffineTransform& operator*=(const AffineTransform& other);
    friend AffineTransform operator*(const AffineTransform& a, const AffineTransform& b) { return a.multiplied(b); }

    void map(double x, double y, double* mappedX, double* mappedY) const;
    PointF map(const PointF& point) const;
    // Rounded to the nearest pixel.
    Point map(const Point& point) const;
    // The bounding rectangle of the mapped corners.
    RectF mapRect(const RectF& rect) const;
    // As mapRect(RectF) for whole pixels: translations and scales round the
    // mapped origin and size, other transforms take toNearestRect() of the
    // mapped bounds.
    Rect mapRect(const Rect& rect) const;
    // Each rectangle mapped and enclosed in whole pixels.
    Region mapToEnclosingRegion(const Region& region) const;

    // Compares the matrices only.
    friend constexpr bool operator==(const AffineTransform& a, const AffineTransform& b)
    {
        return a.m_11 == b.m_11 && a.m_12 == b.m_12 && a.m_21 == b.m_21 && a.m_22 == b.m_22 && a.m_dx == b.m_dx
                && a.m_dy == b.m_dy;
    }

    std::string toString() const;

private:
    AffineTransform multiplied(const AffineTransform& other) const;

    double m_11 = 1;
    double m_12 = 0;
    double m_21 = 0;
    double m_22 = 1;
    double m_dx = 0;
    double m_dy = 0;
    // The kind last worked out, and the highest kind of change since.
    mutable Type m_type = Type::Identity;
    mutable Type m_dirty = Type::Identity;
};

std::ostream& operator<<(std::ostream& stream, const AffineTransform& transform);

} // namespace Lingmo::base
