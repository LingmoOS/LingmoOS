// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/window_snap.h"

namespace Lingmo::WM {

namespace {

// Ash's kScreenEdgeInsetForSnappingSides / kScreenEdgeInsetForSnappingTop. The
// top is less aggressive: the caption is what gets dragged there, and a wide
// band would snap windows the user only wanted to align.
constexpr int kSideTolerance = 32;
constexpr int kTopTolerance = 8;

} // namespace

base::Rect nonSnapArea(const base::Rect &workArea, const base::Rect &outputBounds)
{
    base::Rect area = workArea;
    if (area.isEmpty())
        return area;

    int top = 0;
    int left = 0;
    int right = 0;
    if (outputBounds.isEmpty() || area.top() == outputBounds.top())
        top = kTopTolerance;
    if (outputBounds.isEmpty() || area.left() == outputBounds.left())
        left = kSideTolerance;
    if (outputBounds.isEmpty() || area.right() == outputBounds.right())
        right = kSideTolerance;
    return area.adjusted(left, top, -right, 0);
}

SnapType snapTypeAt(const base::PointF &location, const base::Rect &workArea, const base::Rect &outputBounds)
{
    const base::Rect area = nonSnapArea(workArea, outputBounds);
    if (area.isEmpty())
        return SnapType::None;

    if (location.x() <= area.left())
        return SnapType::Primary;
    if (location.x() >= area.right() - 1)
        return SnapType::Secondary;
    return location.y() <= area.top() ? SnapType::Maximize : SnapType::None;
}

base::Rect snapBounds(SnapType type, const base::Rect &workArea)
{
    switch (type) {
    case SnapType::Primary:
        return base::Rect(workArea.x(), workArea.y(), workArea.width() / 2, workArea.height());
    case SnapType::Secondary: {
        const int width = workArea.width() / 2;
        return base::Rect(workArea.right() - width, workArea.y(), width, workArea.height());
    }
    case SnapType::Maximize:
        return workArea;
    case SnapType::None:
        break;
    }
    return base::Rect();
}

WindowStateType snapStateType(SnapType type)
{
    switch (type) {
    case SnapType::Primary:
        return WindowStateType::PrimarySnapped;
    case SnapType::Secondary:
        return WindowStateType::SecondarySnapped;
    case SnapType::Maximize:
        return WindowStateType::Maximized;
    case SnapType::None:
        break;
    }
    return WindowStateType::Normal;
}

bool canSnapSize(const base::Size &minimumOuterSize, const base::Rect &workArea)
{
    return !workArea.isEmpty() && minimumOuterSize.width() <= workArea.width() / 2
            && minimumOuterSize.height() <= workArea.height();
}

bool areBoundsValidSnappedBounds(WindowStateType type, const base::Rect &bounds, const base::Rect &workArea)
{
    if (!isSnappedStateType(type) || workArea.isEmpty())
        return false;
    if (bounds.y() != workArea.y() || bounds.height() != workArea.height())
        return false;
    return type == WindowStateType::PrimarySnapped ? bounds.x() == workArea.x()
                                                   : bounds.right() == workArea.right();
}

const char *toString(SnapType type)
{
    switch (type) {
    case SnapType::None:
        return "none";
    case SnapType::Primary:
        return "primary";
    case SnapType::Secondary:
        return "secondary";
    case SnapType::Maximize:
        return "maximize";
    }
    return "unknown";
}

} // namespace Lingmo::WM
