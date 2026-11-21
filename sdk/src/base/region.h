// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <initializer_list>
#include <iosfwd>
#include <span>
#include <string>
#include <vector>

#include "base/geometry.h"

namespace Lingmo::base {

// A set of whole pixels, kept as y-x banded rectangles: sorted by top, then
// left; rectangles in a band share top and bottom and never touch; vertically
// adjacent bands with the same spans are merged. The form is canonical, so
// equal pixel sets compare equal and iterate the same rectangles.
class Region
{
public:
    Region() = default;
    // Empty when `rect` is.
    Region(const Rect& rect);
    Region(std::initializer_list<Rect> rects);
    // The union of `rects`; empty ones are skipped.
    static Region fromRects(std::span<const Rect> rects);

    bool isEmpty() const { return m_rects.empty(); }
    const Rect& boundingRect() const { return m_bounds; }
    size_t rectCount() const { return m_rects.size(); }
    std::span<const Rect> rects() const { return m_rects; }
    std::vector<Rect>::const_iterator begin() const { return m_rects.begin(); }
    std::vector<Rect>::const_iterator end() const { return m_rects.end(); }

    // True when the pixel at `point` is in the region.
    bool contains(const Point& point) const;
    // True when every pixel of a non-empty `rect` is in the region.
    bool contains(const Rect& rect) const;
    // True when they share a pixel.
    bool intersects(const Rect& rect) const;
    bool intersects(const Region& region) const;

    Region united(const Region& region) const;
    Region intersected(const Region& region) const;
    Region subtracted(const Region& region) const;
    Region xored(const Region& region) const;

    void translate(int dx, int dy);
    void translate(const Point& offset) { translate(offset.x(), offset.y()); }
    Region translated(int dx, int dy) const { Region r = *this; r.translate(dx, dy); return r; }
    Region translated(const Point& offset) const { return translated(offset.x(), offset.y()); }

    void clear();

    Region& operator|=(const Region& region) { return *this = united(region); }
    Region& operator+=(const Region& region) { return *this = united(region); }
    Region& operator&=(const Region& region) { return *this = intersected(region); }
    Region& operator-=(const Region& region) { return *this = subtracted(region); }
    Region& operator^=(const Region& region) { return *this = xored(region); }
    friend Region operator|(const Region& a, const Region& b) { return a.united(b); }
    friend Region operator+(const Region& a, const Region& b) { return a.united(b); }
    friend Region operator&(const Region& a, const Region& b) { return a.intersected(b); }
    friend Region operator-(const Region& a, const Region& b) { return a.subtracted(b); }
    friend Region operator^(const Region& a, const Region& b) { return a.xored(b); }

    friend bool operator==(const Region& a, const Region& b) { return a.m_rects == b.m_rects; }

    std::string toString() const;

private:
    enum class Op { Union, Intersect, Subtract, Xor };
    static Region combine(const Region& a, const Region& b, Op op);
    void updateBounds();

    std::vector<Rect> m_rects;
    Rect m_bounds;
};

std::ostream& operator<<(std::ostream& stream, const Region& region);

} // namespace Lingmo::base
