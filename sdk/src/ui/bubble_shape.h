// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/geometry.h"
#include "ui/path.h"

namespace Lingmo::ui {

// A bubble: a rounded body with an arrow standing out of one
// edge, pointing at what opened it. The same outline is what a popup paints
// and what it hands to the compositor as its blur shape, so blur, outline
// and shadow match. Logical pixels, the shape's own coordinates.
struct BubbleShape
{
    enum class Edge {
        None,
        Top,
        Bottom,
        Left,
        Right,
    };

    double width = 0;
    double height = 0;
    double radius = 0;
    bool continuousCorners = true;
    Edge arrowEdge = Edge::None;
    // Where the tip points, along the edge: x for top and bottom, y otherwise.
    double arrowPosition = 0;
    double arrowWidth = 12;
    double arrowLength = 6;
    double arrowTipRadius = 1.8;
    double arrowBaseRadius = 4;

    // The part that is not arrow; content belongs in here. The arrow's
    // length is reserved on its edge even when the edge is too short to show
    // it, so the content does not jump.
    RectF bodyRect() const;
    bool arrowVisible() const;
    // The narrowest edge that still shows the arrow.
    double minimumEdgeLength() const;
    Path path() const;
};

} // namespace Lingmo::ui
