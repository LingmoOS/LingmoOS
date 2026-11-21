// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_metrics.h"

#include <algorithm>
#include <cmath>

namespace Lingmo::Shell {

// JavaScript's Math.round: halves go up.
static int jsRound(double value)
{
    return int(std::floor(value + 0.5));
}

static int clampRound(int low, int high, double value)
{
    return std::max(low, std::min(high, jsRound(value)));
}

LauncherMetrics LauncherMetrics::compute(const base::Size &surface, const Margins &margins)
{
    LauncherMetrics m;
    m.width = surface.width();
    m.height = surface.height();
    m.margins = margins;
    const double w = m.width;
    const double h = m.height;

    m.outerMarginH = clampRound(16, 96, w * 0.035);
    m.outerMarginV = clampRound(16, 64, h * 0.035);
    m.sectionSpacing = clampRound(10, 48, h * 0.025);
    m.searchPixelSize = clampRound(12, 22, h * 0.017);
    m.searchFieldWidth = jsRound(std::max(240.0, std::min(w * 0.22, h * 0.42)));
    m.searchFieldHeight = jsRound(m.searchPixelSize * 2.3);
    m.searchIconSize = jsRound(m.searchFieldHeight * 0.44);
    m.indicatorDotSize = clampRound(9, 14, h * 0.0115);
    m.notFoundPixelSize = clampRound(20, 48, h * 0.035);

    const int left = m.outerMarginH + margins.left;
    const int top = m.outerMarginV + margins.top;
    const int right = m.outerMarginH + margins.right;
    const int bottom = m.outerMarginV + margins.bottom;
    m.content = base::Rect(left, top, std::max(0, m.width - left - right),
                           std::max(0, m.height - top - bottom));
    m.searchRow = base::Rect(m.content.x(), m.content.y(), m.content.width(), m.searchFieldHeight);
    m.indicatorRow = base::Rect(m.content.x(), m.content.y() + m.content.height() - m.indicatorDotSize,
                                m.content.width(), m.indicatorDotSize);
    const int gridTop = m.searchRow.y() + m.searchRow.height() + m.sectionSpacing;
    const int gridBottom = m.indicatorRow.y() - m.sectionSpacing;
    m.gridArea = base::Rect(m.content.x(), gridTop, m.content.width(), std::max(0, gridBottom - gridTop));

    const int gw = m.gridArea.width();
    const int gh = m.gridArea.height();
    m.columns = gw < 720 ? 4 : gw < 1100 ? 5 : gw < 1500 ? 6 : gw < 2000 ? 7 : gw < 2800 ? 8 : 9;
    m.rows = gh < 420 ? 2 : gh < 620 ? 3 : gh < 880 ? 4 : gh < 1440 ? 5 : 6;
    m.pageCount = std::max(1, m.columns * m.rows);

    m.cellWidth = std::max(64, int(std::floor(gw * kContentWidthRatio / m.columns)));
    m.cellHeight = std::max(64, int(std::floor(double(gh) / m.rows)));
    m.labelPixelSize = clampRound(11, 18, m.cellWidth * 0.073);
    m.labelLineHeight = jsRound(m.labelPixelSize * 1.3);
    m.iconSize = std::max(32, std::min({192, jsRound(m.cellWidth * 0.48),
                                        int(std::floor((m.cellHeight - m.labelLineHeight) / 1.5))}));
    m.cellSpacing = std::max(6, jsRound(m.iconSize * 0.1));
    m.cellPaddingV = int(std::floor(
            (m.cellHeight - m.iconSize - m.cellSpacing - m.labelLineHeight) / 2.0));
    m.selectionWidth = jsRound(m.iconSize * 1.44);
    m.selectionPadding = jsRound((m.selectionWidth - m.iconSize) / 2.0);
    return m;
}

base::Rect LauncherMetrics::pageGrid() const
{
    const int w = cellWidth * columns;
    const int h = cellHeight * rows;
    return base::Rect(jsRound(gridArea.x() + (gridArea.width() - w) / 2.0),
                      jsRound(gridArea.y() + (gridArea.height() - h) / 2.0), w, h);
}

base::Rect LauncherMetrics::cellRect(int indexInPage) const
{
    const base::Rect grid = pageGrid();
    return base::Rect(grid.x() + (indexInPage % columns) * cellWidth,
                      grid.y() + (indexInPage / columns) * cellHeight, cellWidth, cellHeight);
}

int moveLauncherSelection(int selected, int page, int total, int columns, int pageCount,
                          LauncherDirection direction)
{
    if (total <= 0)
        return -1;
    page = std::max(0, page);
    const int pageStart = page * pageCount;
    const auto pageOf = [pageCount](int index) { return index / pageCount; };

    // A selection paged out of view with the mouse restarts on the visible page.
    if (selected < 0 || pageOf(selected) != page)
        return std::min(pageStart, total - 1);

    const int i = selected;
    int next = i;
    const int column = (i - pageStart) % columns;
    const int row = (i - pageStart) / columns;

    if (direction == LauncherDirection::Left || direction == LauncherDirection::Right) {
        const bool forward = direction == LauncherDirection::Right;
        const bool atEdge = forward ? column == columns - 1 || i == total - 1 : column == 0;
        if (!atEdge) {
            next = forward ? i + 1 : i - 1;
        } else {
            // The edge turns the page, keeping the row.
            const int targetPage = forward ? page + 1 : page - 1;
            if (targetPage < 0 || targetPage * pageCount >= total)
                return -1;
            const int targetStart = targetPage * pageCount;
            // A shorter page clamps the row first, so the column still lands on its edge.
            const int lastRow = (std::min(total, targetStart + pageCount) - 1 - targetStart) / columns;
            next = std::min(total - 1,
                            targetStart + std::min(row, lastRow) * columns + (forward ? 0 : columns - 1));
        }
    } else {
        next = direction == LauncherDirection::Up ? i - columns : i + columns;
        if (next < pageStart || next >= pageStart + pageCount)
            return -1;
        // The last row can be short: land on its last icon.
        const auto rowOf = [pageStart, columns](int n) { return (n - pageStart) / columns; };
        if (next >= total && rowOf(total - 1) > rowOf(i))
            next = total - 1;
    }
    if (next < 0 || next >= total)
        return -1;
    return next;
}

} // namespace Lingmo::Shell
