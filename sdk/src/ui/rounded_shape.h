// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"

class SkCanvas;

namespace Lingmo::ui {

// A rounded rectangle in device pixels: the quad it covers, the
// shape `inset` inside it (outside when negative), a border inside the
// shape's outline, and its shader's coverage. Straight edges fall on whole
// pixels, so only the corners are computed, as its fragment shader does;
// keep them in step with FishUI's roundedrectangle.frag.
struct RoundedShapeSpec
{
    SkIRect quad;
    float radius = 0;
    float inset = 0;
    float borderWidth = 0;
    bool continuous = false;
    // Premultiplied, the opacity applied.
    SkColor4f fill;
    SkColor4f border;
};

void drawRoundedShape(SkCanvas* canvas, const RoundedShapeSpec& spec);

} // namespace Lingmo::ui
