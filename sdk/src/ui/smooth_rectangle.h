// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/geometry.h"

class SkCanvas;

namespace Lingmo::ui {

// An antialiased rectangle with a radius, drawn as vertex geometry: the
// corners as ceil(r·π/6) chords (3 to 18), the edges ramped across
// about a device pixel by vertices the vertex shader pushes apart, and
// triangles filled where they cover pixel centres. rect is in logical
// pixels, already offset; the border lies inside the outline.
void drawSmoothRoundedRectangle(SkCanvas* canvas, const RectF& rect, double radius, double scale, Color fill,
                               double borderWidth, Color border, float opacity);

} // namespace Lingmo::ui
