// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/window_resizer.h"

#include <algorithm>
#include <cmath>

#include "wm/placement.h"
#include "wm/window.h"

namespace Lingmo::WM {

namespace {

// A tiled window must be dragged this far before it leaves its state, so that a
// click that wobbles does not unmaximize it.
// Upstream: ash/wm/workspace/workspace_window_resizer.cc
// kResizeRestoreDragThresholdDp.
constexpr int kRestoreDragThreshold = 5;

} // namespace

WindowResizer::WindowResizer(Window *window, const base::PointF &startLocation, ResizeEdge edge,
                             const base::Rect &workArea, const base::Rect &restoreBounds,
                             const base::Rect &visibleArea)
    : m_window(window)
    , m_edge(window && window->isResizable() ? edge : ResizeEdge::None)
    , m_initialLocation(startLocation)
    , m_initialBounds(window ? window->bounds() : base::Rect())
    , m_restoreBounds(edge == ResizeEdge::None ? restoreBounds : base::Rect())
    , m_workArea(workArea)
    , m_visibleArea(visibleArea.isEmpty() ? workArea : visibleArea)
{
}

void WindowResizer::setAreas(const base::Rect &workArea, const base::Rect &visibleArea)
{
    m_workArea = workArea;
    m_visibleArea = visibleArea.isEmpty() ? workArea : visibleArea;
}

int WindowResizer::widthForDrag(int *deltaX) const
{
    int width = m_initialBounds.width();
    if (!changesWidth(m_edge))
        return width;

    // Along the right edge a positive delta grows the window.
    const int multiplier = isRightEdge(m_edge) ? 1 : -1;
    width += multiplier * (*deltaX);

    const int minWidth = std::max(1, m_window->minimumOuterSize().width());
    if (width < minWidth) {
        width = minWidth;
        // Clamping the delta too is what keeps the opposite edge fixed once
        // the limit is hit; the origin is computed from this same delta.
        *deltaX = -multiplier * (m_initialBounds.width() - minWidth);
    }

    int maxWidth = m_workArea.isEmpty() ? width : m_workArea.width();
    if (m_window->maximumOuterSize().width() > 0)
        maxWidth = std::min(maxWidth, m_window->maximumOuterSize().width());
    maxWidth = std::max(maxWidth, minWidth);
    if (width > maxWidth) {
        width = maxWidth;
        *deltaX = -multiplier * (m_initialBounds.width() - maxWidth);
    }
    return width;
}

int WindowResizer::heightForDrag(int *deltaY) const
{
    int height = m_initialBounds.height();
    if (!changesHeight(m_edge))
        return height;

    const int multiplier = isBottomEdge(m_edge) ? 1 : -1;
    height += multiplier * (*deltaY);

    const int minHeight = std::max(1, m_window->minimumOuterSize().height());
    if (height < minHeight) {
        height = minHeight;
        *deltaY = -multiplier * (m_initialBounds.height() - minHeight);
    }

    int maxHeight = m_workArea.isEmpty() ? height : m_workArea.height();
    if (m_window->maximumOuterSize().height() > 0)
        maxHeight = std::min(maxHeight, m_window->maximumOuterSize().height());
    maxHeight = std::max(maxHeight, minHeight);
    if (height > maxHeight) {
        height = maxHeight;
        *deltaY = -multiplier * (m_initialBounds.height() - maxHeight);
    }
    return height;
}

base::Rect WindowResizer::calculateBoundsForDrag(const base::PointF &location) const
{
    if (!m_window)
        return m_initialBounds;

    int deltaX = static_cast<int>(std::lround(location.x() - m_initialLocation.x()));
    int deltaY = static_cast<int>(std::lround(location.y() - m_initialLocation.y()));

    base::Size size(widthForDrag(&deltaX), heightForDrag(&deltaY));

    base::Point origin = m_initialBounds.topLeft();
    if (!isResize()) {
        // Unlike ash, a move leaves the size alone: a client may resize while
        // it is dragged, as a pulled-out tab's window leaving an inherited state does.
        size = m_window->bounds().size();
        origin += base::Point(deltaX, deltaY);
        if (!m_restoreBounds.isEmpty() && !m_initialBounds.isEmpty()) {
            // The window shrinks back to its restored size under the pointer.
            // Keeping the grab at the same fraction of the title bar is what
            // stops it jumping out from under the cursor.
            size = m_restoreBounds.size();
            const double ratio = (m_initialLocation.x() - m_initialBounds.x())
                    / static_cast<double>(m_initialBounds.width());
            origin.setX(static_cast<int>(std::lround(location.x() - ratio * size.width())));
        }
    } else {
        // Only the edges that move the origin consume the (possibly clamped)
        // delta; the opposite edges stay where they were.
        if (isLeftEdge(m_edge))
            origin.setX(origin.x() + deltaX);
        if (isTopEdge(m_edge))
            origin.setY(origin.y() + deltaY);
    }

    base::Rect bounds(origin, size);
    adjustBoundsToEnsureMinimumVisibility(m_visibleArea, &bounds, m_workArea);
    return bounds;
}

void WindowResizer::drag(const base::PointF &location)
{
    if (!m_window)
        return;
    if (!m_movedOrResized && !m_restoreBounds.isEmpty()
        && (location - m_initialLocation).length() < kRestoreDragThreshold) {
        return;
    }
    const base::Rect bounds = calculateBoundsForDrag(location);
    if (bounds == m_window->bounds())
        return;
    m_movedOrResized = true;
    m_window->setBounds(bounds);
}

void WindowResizer::completeDrag()
{
}

void WindowResizer::revertDrag()
{
    if (m_window && m_movedOrResized)
        m_window->setBounds(m_initialBounds);
}

} // namespace Lingmo::WM
