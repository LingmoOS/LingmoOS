// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/window_positioning_utils.cc
//           chromium/src/ui/views/widget/native_widget_aura.cc (CenterWindow)
//           chromium/src/ui/gfx/geometry/rect.cc (AdjustToFit)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gfx::Rect becomes base::Rect.

#include "wm/placement.h"

#include <algorithm>

namespace Lingmo::WM {

namespace {

void adjustAlongAxis(int areaOrigin, int areaSize, int *origin, int *size)
{
    *size = std::min(areaSize, *size);
    if (*origin < areaOrigin)
        *origin = areaOrigin;
    else
        *origin = std::min(areaOrigin + areaSize, *origin + *size) - *size;
}

} // namespace

void adjustBoundsSmallerThan(const base::Size &maxSize, base::Rect *bounds)
{
    bounds->setWidth(std::min(bounds->width(), maxSize.width()));
    bounds->setHeight(std::min(bounds->height(), maxSize.height()));
}

void adjustBoundsToEnsureMinimumVisibility(const base::Rect &visibleArea, base::Rect *bounds,
                                           const base::Rect &grabArea)
{
    if (visibleArea.isEmpty())
        return;

    // A fixed strip rather than Ash's 30% of the window, so large windows can
    // still be pushed to the screen edge.
    int minWidth = kMinimumOnScreenArea;
    int minHeight = kMinimumOnScreenArea;

    adjustBoundsSmallerThan(visibleArea.size(), bounds);

    minWidth = std::min(minWidth, visibleArea.width());
    minHeight = std::min(minHeight, visibleArea.height());

    const int areaLeft = visibleArea.x();
    const int areaTop = visibleArea.y();
    const int areaRight = visibleArea.right();
    const int areaBottom = visibleArea.bottom();
    // The dock covers the bottom of the visible area; a strip pushed under it
    // has nothing left to grab, so the lower limit is the work area's.
    const int grabBottom = grabArea.isEmpty()
            ? areaBottom
            : std::min(areaBottom, grabArea.y() + grabArea.height());

    if (bounds->x() + bounds->width() < areaLeft + minWidth)
        bounds->moveLeft(areaLeft + std::min(bounds->width(), minWidth) - bounds->width());
    else if (bounds->x() > areaRight - minWidth)
        bounds->moveLeft(areaRight - std::min(bounds->width(), minWidth));

    if (bounds->y() + bounds->height() < areaTop + minHeight)
        bounds->moveTop(areaTop + std::min(bounds->height(), minHeight) - bounds->height());
    else if (bounds->y() > grabBottom - minHeight)
        bounds->moveTop(grabBottom - std::min(bounds->height(), minHeight));

    if (bounds->y() < areaTop)
        bounds->moveTop(areaTop);
}

void adjustBoundsToFit(const base::Rect &area, base::Rect *bounds)
{
    int x = bounds->x();
    int y = bounds->y();
    int width = bounds->width();
    int height = bounds->height();
    adjustAlongAxis(area.x(), area.width(), &x, &width);
    adjustAlongAxis(area.y(), area.height(), &y, &height);
    *bounds = base::Rect(x, y, width, height);
}

base::Rect Placement::place(const base::Size &requestedSize, const base::Rect &workArea)
{
    base::Rect bounds(workArea.topLeft(), requestedSize);
    if (bounds.width() <= 0)
        bounds.setWidth(1);
    if (bounds.height() <= 0)
        bounds.setHeight(1);

    if (!workArea.isEmpty()) {
        adjustBoundsSmallerThan(workArea.size(), &bounds);

        bounds.moveTopLeft(workArea.topLeft()
                           + base::Point((workArea.width() - bounds.width()) / 2,
                                    (workArea.height() - bounds.height()) / 2));
    }

    return bounds;
}

base::Rect Placement::placeTransient(const base::Size &requestedSize, const base::Rect &parentBounds,
                                int parentTopInset, const base::Rect &workArea)
{
    const base::Size size = requestedSize.expandedTo(base::Size(1, 1));

    base::Rect area = workArea;
    if (parentBounds.height() >= size.height() || parentBounds.width() >= size.width())
        area = parentBounds.adjusted(0, parentTopInset, 0, 0);

    base::Rect bounds(area.x() + (area.width() - size.width()) / 2,
                 area.y() + (area.height() - size.height()) / 2, size.width(), size.height());

    // Never larger than the work area, or its caption could end up out of reach.
    if (!workArea.isEmpty())
        adjustBoundsToFit(workArea, &bounds);
    return bounds;
}

} // namespace Lingmo::WM
