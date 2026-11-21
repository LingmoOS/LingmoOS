// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/anchored_placement.h"

#include <algorithm>

#include "wm/placement.h"

namespace Lingmo::WM {

namespace {

using compositor::PlacementEdge;

// -1 for the start of an axis, 1 for its end, 0 for its middle.
int horizontal(PlacementEdge edge)
{
    switch (edge) {
    case PlacementEdge::Left:
    case PlacementEdge::TopLeft:
    case PlacementEdge::BottomLeft:
        return -1;
    case PlacementEdge::Right:
    case PlacementEdge::TopRight:
    case PlacementEdge::BottomRight:
        return 1;
    default:
        return 0;
    }
}

int vertical(PlacementEdge edge)
{
    switch (edge) {
    case PlacementEdge::Top:
    case PlacementEdge::TopLeft:
    case PlacementEdge::TopRight:
        return -1;
    case PlacementEdge::Bottom:
    case PlacementEdge::BottomLeft:
    case PlacementEdge::BottomRight:
        return 1;
    default:
        return 0;
    }
}

// The start of a window of `length` on one axis, the anchor rectangle
// spanning `anchorStart` to `anchorEnd`.
int startOnAxis(int anchorStart, int anchorEnd, int anchor, int gravity, int length)
{
    const int point = anchor < 0 ? anchorStart : anchor > 0 ? anchorEnd : (anchorStart + anchorEnd) / 2;
    if (gravity > 0)
        return point;
    if (gravity < 0)
        return point - length;
    return point - length / 2;
}

bool fits(int start, int length, int areaStart, int areaEnd)
{
    return start >= areaStart && start + length <= areaEnd;
}

int placeOnAxis(int anchorStart, int anchorEnd, int anchor, int gravity, int length, int areaStart,
                int areaEnd)
{
    int start = startOnAxis(anchorStart, anchorEnd, anchor, gravity, length);
    if (!fits(start, length, areaStart, areaEnd)) {
        const int flipped = startOnAxis(anchorStart, anchorEnd, -anchor, -gravity, length);
        if (fits(flipped, length, areaStart, areaEnd))
            start = flipped;
    }
    return std::max(areaStart, std::min(start, areaEnd - length));
}

} // namespace

base::Rect placeAtAnchor(const base::Size &size, const compositor::PlacementAnchor &anchor,
                         const base::Rect &workArea)
{
    base::Rect bounds(base::Point(), size);
    adjustBoundsSmallerThan(workArea.size(), &bounds);
    const base::Rect &rect = anchor.rect;
    bounds.moveTo(placeOnAxis(rect.left(), rect.right(), horizontal(anchor.anchor),
                              horizontal(anchor.gravity), bounds.width(), workArea.left(),
                              workArea.right()),
                  placeOnAxis(rect.top(), rect.bottom(), vertical(anchor.anchor),
                              vertical(anchor.gravity), bounds.height(), workArea.top(),
                              workArea.bottom()));
    return bounds;
}

} // namespace Lingmo::WM
