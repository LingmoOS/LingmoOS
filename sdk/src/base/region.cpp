// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/region.h"

#include <ostream>

namespace Lingmo::base {

// The end of the band starting at `begin`.
static size_t bandEnd(const std::vector<Rect>& rects, size_t begin)
{
    size_t end = begin + 1;
    while (end < rects.size() && rects[end].y() == rects[begin].y())
        ++end;
    return end;
}

// Combines two bands' sorted, non-touching spans; `spans` gets left and right
// edges in pairs, touching results merged.
template<typename Predicate>
static void combineSpans(const Rect* a, const Rect* aEnd, const Rect* b, const Rect* bEnd,
                         Predicate predicate, std::vector<int>& spans)
{
    spans.clear();
    bool inA = false;
    bool inB = false;
    bool inside = false;
    int start = 0;
    while (a != aEnd || b != bEnd) {
        int x = INT_MAX;
        if (a != aEnd)
            x = std::min(x, inA ? a->right() : a->x());
        if (b != bEnd)
            x = std::min(x, inB ? b->right() : b->x());
        if (a != aEnd && (inA ? a->right() : a->x()) == x) {
            inA = !inA;
            if (!inA)
                ++a;
        }
        if (b != bEnd && (inB ? b->right() : b->x()) == x) {
            inB = !inB;
            if (!inB)
                ++b;
        }
        const bool now = predicate(inA, inB);
        if (now == inside)
            continue;
        if (now) {
            start = x;
        } else {
            spans.push_back(start);
            spans.push_back(x);
        }
        inside = now;
    }
}

// Appends a band, or grows the previous band down when it ends at `top` with
// the same spans.
static void appendBand(std::vector<Rect>& rects, size_t& previousBand, int top, int bottom,
                       const std::vector<int>& spans)
{
    if (spans.empty())
        return;
    const size_t count = spans.size() / 2;
    if (previousBand < rects.size() && rects[previousBand].bottom() == top
        && rects.size() - previousBand == count) {
        bool same = true;
        for (size_t i = 0; i < count && same; ++i) {
            const Rect& rect = rects[previousBand + i];
            same = rect.x() == spans[2 * i] && rect.right() == spans[2 * i + 1];
        }
        if (same) {
            for (size_t i = previousBand; i < rects.size(); ++i)
                rects[i].setBottom(bottom);
            return;
        }
    }
    previousBand = rects.size();
    for (size_t i = 0; i < count; ++i)
        rects.push_back(Rect::fromEdges(spans[2 * i], top, spans[2 * i + 1], bottom));
}

Region::Region(const Rect& rect)
{
    if (rect.isEmpty())
        return;
    m_rects.push_back(rect);
    m_bounds = rect;
}

Region::Region(std::initializer_list<Rect> rects)
    : Region(fromRects(std::span<const Rect>(rects.begin(), rects.size())))
{
}

Region Region::fromRects(std::span<const Rect> rects)
{
    if (rects.empty())
        return {};
    if (rects.size() == 1)
        return Region(rects.front());
    const size_t half = rects.size() / 2;
    return fromRects(rects.first(half)).united(fromRects(rects.subspan(half)));
}

bool Region::contains(const Point& point) const
{
    if (!m_bounds.contains(point))
        return false;
    for (const Rect& rect : m_rects) {
        if (rect.y() > point.y())
            break;
        if (rect.contains(point))
            return true;
    }
    return false;
}

bool Region::contains(const Rect& rect) const
{
    if (rect.isEmpty() || !m_bounds.contains(rect))
        return false;
    if (m_rects.size() == 1)
        return true;
    return Region(rect).subtracted(*this).isEmpty();
}

bool Region::intersects(const Rect& rect) const
{
    if (!m_bounds.intersects(rect))
        return false;
    for (const Rect& r : m_rects) {
        if (r.y() >= rect.bottom())
            break;
        if (r.intersects(rect))
            return true;
    }
    return false;
}

bool Region::intersects(const Region& region) const
{
    if (!m_bounds.intersects(region.m_bounds))
        return false;
    if (region.m_rects.size() == 1)
        return intersects(region.m_rects.front());
    if (m_rects.size() == 1)
        return region.intersects(m_rects.front());
    return !intersected(region).isEmpty();
}

Region Region::united(const Region& region) const
{
    return combine(*this, region, Op::Union);
}

Region Region::intersected(const Region& region) const
{
    return combine(*this, region, Op::Intersect);
}

Region Region::subtracted(const Region& region) const
{
    return combine(*this, region, Op::Subtract);
}

Region Region::xored(const Region& region) const
{
    return combine(*this, region, Op::Xor);
}

Region Region::combine(const Region& a, const Region& b, Op op)
{
    switch (op) {
    case Op::Union:
        if (b.isEmpty() || (a.m_rects.size() == 1 && a.m_bounds.contains(b.m_bounds)))
            return a;
        if (a.isEmpty() || (b.m_rects.size() == 1 && b.m_bounds.contains(a.m_bounds)))
            return b;
        break;
    case Op::Intersect:
        if (!a.m_bounds.intersects(b.m_bounds))
            return {};
        if (a.m_rects.size() == 1 && b.m_rects.size() == 1)
            return Region(a.m_bounds.intersected(b.m_bounds));
        if (a.m_rects.size() == 1 && a.m_bounds.contains(b.m_bounds))
            return b;
        if (b.m_rects.size() == 1 && b.m_bounds.contains(a.m_bounds))
            return a;
        break;
    case Op::Subtract:
        if (!a.m_bounds.intersects(b.m_bounds))
            return a;
        if (b.m_rects.size() == 1 && b.m_bounds.contains(a.m_bounds))
            return {};
        break;
    case Op::Xor:
        if (a.isEmpty())
            return b;
        if (b.isEmpty())
            return a;
        break;
    }

    const auto predicate = [op](bool inA, bool inB) {
        switch (op) {
        case Op::Union:
            return inA || inB;
        case Op::Intersect:
            return inA && inB;
        case Op::Subtract:
            return inA && !inB;
        case Op::Xor:
            return inA != inB;
        }
        return false;
    };

    const std::vector<Rect>& ra = a.m_rects;
    const std::vector<Rect>& rb = b.m_rects;
    Region result;
    result.m_rects.reserve(ra.size() + rb.size());
    std::vector<int> spans;
    size_t previousBand = SIZE_MAX;
    size_t ia = 0;
    size_t ib = 0;
    int y = std::min(ra.empty() ? INT_MAX : ra.front().y(), rb.empty() ? INT_MAX : rb.front().y());
    while (ia < ra.size() || ib < rb.size()) {
        const size_t aEnd = ia < ra.size() ? bandEnd(ra, ia) : ia;
        const size_t bEnd = ib < rb.size() ? bandEnd(rb, ib) : ib;
        const bool aActive = ia < ra.size() && ra[ia].y() <= y;
        const bool bActive = ib < rb.size() && rb[ib].y() <= y;
        int next = INT_MAX;
        if (ia < ra.size())
            next = std::min(next, aActive ? ra[ia].bottom() : ra[ia].y());
        if (ib < rb.size())
            next = std::min(next, bActive ? rb[ib].bottom() : rb[ib].y());

        if (aActive || bActive) {
            const Rect* aBegin = ra.data() + ia;
            const Rect* bBegin = rb.data() + ib;
            combineSpans(aBegin, aActive ? ra.data() + aEnd : aBegin, bBegin,
                         bActive ? rb.data() + bEnd : bBegin, predicate, spans);
            appendBand(result.m_rects, previousBand, y, next, spans);
        }

        y = next;
        if (aActive && ra[ia].bottom() == y)
            ia = aEnd;
        if (bActive && rb[ib].bottom() == y)
            ib = bEnd;
        if (op == Op::Intersect && (ia == ra.size() || ib == rb.size()))
            break;
        if (op == Op::Subtract && ia == ra.size())
            break;
    }
    result.updateBounds();
    return result;
}

void Region::translate(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;
    for (Rect& rect : m_rects)
        rect.translate(dx, dy);
    m_bounds.translate(dx, dy);
}

void Region::clear()
{
    m_rects.clear();
    m_bounds = {};
}

void Region::updateBounds()
{
    if (m_rects.empty()) {
        m_bounds = {};
        return;
    }
    int left = INT_MAX;
    int right = INT_MIN;
    for (const Rect& rect : m_rects) {
        left = std::min(left, rect.x());
        right = std::max(right, rect.right());
    }
    m_bounds = Rect::fromEdges(left, m_rects.front().y(), right, m_rects.back().bottom());
}

std::string Region::toString() const
{
    std::string text = "{";
    for (size_t i = 0; i < m_rects.size(); ++i) {
        if (i)
            text += "; ";
        text += m_rects[i].toString();
    }
    return text + "}";
}

std::ostream& operator<<(std::ostream& stream, const Region& region)
{
    return stream << region.toString();
}

} // namespace Lingmo::base
