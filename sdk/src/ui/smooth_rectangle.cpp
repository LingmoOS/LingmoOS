// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/smooth_rectangle.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkVertices.h"

namespace Lingmo::ui {

// A vertex of the antialiased rectangle's geometry: a point in
// the item, its colour and the offset its vertex shader moves it by.
struct SmoothVertex
{
    float x;
    float y;
    Color color;
    float dx;
    float dy;
};

// smoothcolor.vert along one axis: an offset of d device pixels moves the
// vertex by d·min(1, 0.5 / (d − 0.5)), all of it when d is under half a pixel.
static float shaderOffset(float offset, double scale)
{
    if (offset == 0)
        return 0;
    const double d = std::abs(offset) * scale;
    const double numerator = (d - 0.5) * 0.5;
    const double factor = numerator < 0 ? 1 : (d == 0.5 ? 0 : std::min(1.0, numerator / ((d - 0.5) * (d - 0.5))));
    return float(std::copysign(d * factor, offset));
}

// The ramp needs premultiplied vertex colours interpolated; Skia does not. Each
// colour of the strip goes as its own pass, its alpha ramped and nothing
// else, and the passes add up in a layer to the premultiplied blend.
static void drawStrip(SkCanvas* canvas, const std::vector<SmoothVertex>& strip, double originX, double originY,
                      double scale, float opacity)
{
    if (strip.size() < 3)
        return;
    std::vector<SkPoint> points;
    std::vector<uint32_t> rgbs;
    for (const SmoothVertex& vertex : strip) {
        points.push_back(SkPoint::Make(float((originX + vertex.x) * scale) + shaderOffset(vertex.dx, scale),
                                       float((originY + vertex.y) * scale) + shaderOffset(vertex.dy, scale)));
        const uint32_t rgb = vertex.color.argb() & 0xFFFFFF;
        if (vertex.color.a > 0 && std::find(rgbs.begin(), rgbs.end(), rgb) == rgbs.end())
            rgbs.push_back(rgb);
    }
    if (rgbs.empty())
        return;

    const auto pass = [&](uint32_t rgb, SkBlendMode mode, float alpha) {
        std::vector<SkColor> colors;
        for (const SmoothVertex& vertex : strip) {
            const bool mine = (vertex.color.argb() & 0xFFFFFF) == rgb;
            colors.push_back(SkColor(uint32_t(mine ? vertex.color.a : 0) << 24 | rgb));
        }
        const sk_sp<SkVertices> vertices = SkVertices::MakeCopy(
            SkVertices::kTriangleStrip_VertexMode, int(points.size()), points.data(), nullptr, colors.data());
        SkPaint paint;
        paint.setAlphaf(alpha);
        paint.setBlendMode(mode);
        canvas->drawVertices(vertices, SkBlendMode::kDst, paint);
    };
    if (rgbs.size() == 1) {
        pass(rgbs.front(), SkBlendMode::kSrcOver, opacity);
        return;
    }
    SkRect bounds = SkRect::BoundsOrEmpty(SkSpan(points.data(), points.size()));
    bounds.outset(1, 1);
    SkPaint layer;
    layer.setAlphaf(opacity);
    canvas->saveLayer(&bounds, &layer);
    for (size_t i = 0; i < rgbs.size(); ++i)
        pass(rgbs[i], i == 0 ? SkBlendMode::kSrc : SkBlendMode::kPlus, 1);
    canvas->restore();
}

void drawSmoothRoundedRectangle(SkCanvas* canvas, const RectF& rect, double radius, double scale, Color fill,
                               double borderWidth, Color border, float opacity)
{
    const float width = float(rect.width);
    const float height = float(rect.height);
    const float penWidth = std::min(std::min(width, height) * 0.5f, float(borderWidth));
    // Never more than half the shorter side.
    const float outerRadius = std::min(std::min(width, height) * 0.4999f, float(radius));
    const float innerRadius = std::max(outerRadius - penWidth, 0.01f);
    const float delta = std::min(width, height) * 0.5f;
    const int segments = std::clamp(int(std::ceil(outerRadius * (M_PI / 6))), 3, 18);
    // The transparent edge takes the colour beside it, so interpolating it
    // only fades that colour.
    const Color fillColor = fill;
    const Color borderColor = border;
    const Color outerColor = (penWidth > 0 ? border : fill).withAlpha(0);

    // Rows from top to bottom, the gradient being vertical: y is the primary
    // coordinate, x the secondary one, left and right at each row.
    struct Row
    {
        SmoothVertex fillRight, fillLeft;
        SmoothVertex innerRight, innerLeft;
        SmoothVertex outerRight, outerLeft;
        SmoothVertex edgeRight, edgeLeft;
    };
    std::vector<Row> rows;
    for (int part = 0; part < 2; ++part) {
        const float innerCentreY = part == 0 ? innerRadius + penWidth : height - innerRadius - penWidth;
        const float outerCentreY = part == 0 ? outerRadius : height - outerRadius;
        const float innerLeft = innerRadius + penWidth;
        const float innerRight = width - innerRadius - penWidth;
        const float outerLeft = outerRadius;
        const float outerRight = width - outerRadius;
        for (int i = 0; i <= segments; ++i) {
            // Part 0 sweeps from straight up to the side, part 1 on to straight down.
            const double angle = (part + double(i) / segments) * M_PI / 2;
            const float cosAngle = float(std::cos(angle));
            const float sinAngle = float(std::sin(angle));
            const float y = innerCentreY - innerRadius * cosAngle;
            const float xLeft = innerLeft - innerRadius * sinAngle;
            const float xRight = innerRight + innerRadius * sinAngle;
            const float outerY = outerCentreY - outerRadius * cosAngle;
            const float outerXLeft = outerLeft - outerRadius * sinAngle;
            const float outerXRight = outerRight + outerRadius * sinAngle;

            Row row;
            const float dp = part ? std::min(0.0f, height - y - delta) : std::max(0.0f, delta - y);
            row.fillRight = {xRight, y, fillColor, width - xRight - delta, dp};
            row.fillLeft = {xLeft, y, fillColor, delta - xLeft, dp};
            const float edge = part ? delta : -delta;
            if (penWidth > 0) {
                const float reach = 0.49f * penWidth;
                row.innerRight = {xRight, y, borderColor, reach * sinAngle, -reach * cosAngle};
                row.innerLeft = {xLeft, y, borderColor, -reach * sinAngle, -reach * cosAngle};
                row.outerRight = {outerXRight, outerY, borderColor, -reach * sinAngle, reach * cosAngle};
                row.outerLeft = {outerXLeft, outerY, borderColor, reach * sinAngle, reach * cosAngle};
                row.edgeRight = {outerXRight, outerY, outerColor, delta, edge};
                row.edgeLeft = {outerXLeft, outerY, outerColor, -delta, edge};
            } else {
                row.edgeRight = {xRight, y, outerColor, delta, edge};
                row.edgeLeft = {xLeft, y, outerColor, -delta, edge};
            }
            rows.push_back(row);
        }
    }

    // A closed ring: the right side bottom to top, the
    // left side top to bottom, each pair inner then outer, closed by the
    // left side's last pair in front.
    const auto ring = [&rows](SmoothVertex Row::*inner, SmoothVertex Row::*innerLeft, SmoothVertex Row::*outer,
                              SmoothVertex Row::*outerLeft) {
        std::vector<SmoothVertex> strip;
        strip.push_back(rows.back().*innerLeft);
        strip.push_back(rows.back().*outerLeft);
        for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
            strip.push_back((*it).*inner);
            strip.push_back((*it).*outer);
        }
        for (const Row& row : rows) {
            strip.push_back(row.*innerLeft);
            strip.push_back(row.*outerLeft);
        }
        return strip;
    };

    if (fill.a > 0) {
        std::vector<SmoothVertex> strip;
        for (const Row& row : rows) {
            strip.push_back(row.fillRight);
            strip.push_back(row.fillLeft);
        }
        drawStrip(canvas, strip, rect.x, rect.y, scale, opacity);
    }
    if (penWidth > 0) {
        drawStrip(canvas, ring(&Row::fillRight, &Row::fillLeft, &Row::innerRight, &Row::innerLeft), rect.x, rect.y,
                  scale, opacity);
        drawStrip(canvas, ring(&Row::innerRight, &Row::innerLeft, &Row::outerRight, &Row::outerLeft), rect.x, rect.y,
                  scale, opacity);
        drawStrip(canvas, ring(&Row::outerRight, &Row::outerLeft, &Row::edgeRight, &Row::edgeLeft), rect.x, rect.y,
                  scale, opacity);
    } else {
        drawStrip(canvas, ring(&Row::fillRight, &Row::fillLeft, &Row::edgeRight, &Row::edgeLeft), rect.x, rect.y,
                  scale, opacity);
    }
}

} // namespace Lingmo::ui
