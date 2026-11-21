// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <algorithm>
#include <cmath>

namespace Lingmo::Shell {

// The dock's geometry in one place. For the
// default icon size of 54 and a bottom dock:
//
//     +--------------------------------------------------+  <- far side
//     |                    outerPadding 16               |
//     |  16 |  icon 54  | 16 |  icon 54  | 16 |          |  <- lengthPadding
//     |                      dotGap 8                    |
//     |                      dot 4                       |
//     |                      edgePadding 4               |
//     +--------------------------------------------------+  <- screen edge
//
// A left or right dock is the same block, rotated: the running dot always
// stays on the side that faces the screen edge.
struct DockMetrics {
    static constexpr int kBaseIconSize = 54;

    int iconSize = 54;
    int iconGap = 16;
    int lengthPadding = 16;
    int outerPadding = 16;
    int dotGap = 8;
    int dotSize = 4;
    int edgePadding = 4;

    static DockMetrics forIconSize(int iconSize)
    {
        const double scale = std::max(1, iconSize) / double(kBaseIconSize);
        // qRound: halves away from zero.
        const auto scaled = [scale](int value) {
            return std::max(1, int(std::floor(value * scale + 0.5)));
        };
        DockMetrics metrics;
        metrics.iconSize = std::max(1, iconSize);
        metrics.iconGap = scaled(16);
        metrics.lengthPadding = scaled(16);
        metrics.outerPadding = scaled(16);
        metrics.dotGap = scaled(8);
        metrics.dotSize = scaled(4);
        metrics.edgePadding = scaled(4);
        return metrics;
    }

    // From the screen edge to the far side of the dock.
    int thickness() const { return edgePadding + dotSize + dotGap + iconSize + outerPadding; }
    // The gap a divider stands in, between the pinned and the running applications.
    int sectionExtent() const { return int(std::floor(iconGap * 1.5 + 0.5)); }
    int pitch() const { return iconSize + iconGap; }
    int dotAreaSize() const { return edgePadding + dotSize + dotGap; }
    // Along the dock: the icons plus the padding at both ends.
    int length(int itemCount) const
    {
        return lengthPadding * 2 + itemCount * iconSize + std::max(0, itemCount - 1) * iconGap;
    }

    bool operator==(const DockMetrics &) const = default;
};

// DockMetrics as drawn between two of them while the dock resizes.
struct DrawnDockMetrics {
    double iconSize = 54;
    double iconGap = 16;
    double lengthPadding = 16;
    double outerPadding = 16;
    double dotGap = 8;
    double dotSize = 4;
    double edgePadding = 4;
    double separatorExtent = 24;
    double thickness = 86;

    static DrawnDockMetrics of(const DockMetrics &metrics)
    {
        return {double(metrics.iconSize),      double(metrics.iconGap),
                double(metrics.lengthPadding), double(metrics.outerPadding),
                double(metrics.dotGap),        double(metrics.dotSize),
                double(metrics.edgePadding),   double(metrics.sectionExtent()),
                double(metrics.thickness())};
    }

    static DrawnDockMetrics blend(const DrawnDockMetrics &from, const DrawnDockMetrics &to,
                                  double progress)
    {
        const auto mix = [progress](double a, double b) { return a + (b - a) * progress; };
        return {mix(from.iconSize, to.iconSize),
                mix(from.iconGap, to.iconGap),
                mix(from.lengthPadding, to.lengthPadding),
                mix(from.outerPadding, to.outerPadding),
                mix(from.dotGap, to.dotGap),
                mix(from.dotSize, to.dotSize),
                mix(from.edgePadding, to.edgePadding),
                mix(from.separatorExtent, to.separatorExtent),
                mix(from.thickness, to.thickness)};
    }
};

} // namespace Lingmo::Shell

