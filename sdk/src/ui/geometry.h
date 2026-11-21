// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace Lingmo::ui {

struct PointF
{
    double x = 0;
    double y = 0;

    bool isNull() const { return x == 0 && y == 0; }

    PointF operator+(const PointF& other) const { return {x + other.x, y + other.y}; }
    PointF operator-(const PointF& other) const { return {x - other.x, y - other.y}; }
    PointF operator*(double factor) const { return {x * factor, y * factor}; }
    PointF operator/(double divisor) const { return {x / divisor, y / divisor}; }
    PointF& operator+=(const PointF& other) { return *this = *this + other; }
    PointF& operator-=(const PointF& other) { return *this = *this - other; }

    bool operator==(const PointF&) const = default;
};

struct SizeF
{
    double width = 0;
    double height = 0;

    bool operator==(const SizeF&) const = default;
};

struct Margins
{
    double left = 0;
    double top = 0;
    double right = 0;
    double bottom = 0;

    static constexpr Margins all(double margin) { return {margin, margin, margin, margin}; }

    double horizontal() const { return left + right; }
    double vertical() const { return top + bottom; }

    bool operator==(const Margins&) const = default;
};

struct RectF
{
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;

    double right() const { return x + width; }
    double bottom() const { return y + height; }
    PointF topLeft() const { return {x, y}; }
    SizeF size() const { return {width, height}; }

    bool contains(PointF point) const
    {
        return point.x >= x && point.x < right() && point.y >= y && point.y < bottom();
    }

    RectF adjusted(double left, double top, double right, double bottom) const
    {
        return {x + left, y + top, width - left + right, height - top + bottom};
    }

    RectF shrunk(const Margins& margins) const
    {
        return adjusted(margins.left, margins.top, -margins.right, -margins.bottom);
    }

    bool operator==(const RectF&) const = default;
};

// A move and a uniform scale about a view's centre, in logical pixels.
struct Transform
{
    PointF translation;
    double scale = 1;

    bool isIdentity() const { return translation.isNull() && scale == 1; }
    bool operator==(const Transform&) const = default;
};

// Whole device pixels.
struct Rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool operator==(const Rect&) const = default;
};

struct Color
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    static constexpr Color rgb(uint32_t value)
    {
        return {uint8_t(value >> 16), uint8_t(value >> 8), uint8_t(value), 255};
    }

    static constexpr Color fromArgb(uint32_t value)
    {
        return {uint8_t(value >> 16), uint8_t(value >> 8), uint8_t(value), uint8_t(value >> 24)};
    }

    constexpr Color withAlpha(double alpha) const
    {
        return {r, g, b, uint8_t(alpha * 255 + 0.5)};
    }

    constexpr uint32_t argb() const
    {
        return uint32_t(a) << 24 | uint32_t(r) << 16 | uint32_t(g) << 8 | b;
    }

    // The HSV value divided or multiplied by the factor.
    Color darker(double factor) const;
    Color lighter(double factor) const;

    bool operator==(const Color&) const = default;
};

} // namespace Lingmo::ui
