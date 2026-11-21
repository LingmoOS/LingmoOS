// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/shape_mask.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "base/geometry.h"
#include "base/hash.h"
#include "base/strings.h"
#include "base/transform.h"
#include "ui/bitmap_painter.h"

namespace Lingmo::Render {

namespace {

base::AffineTransform deviceTransform(const ShapeRaster &raster)
{
    base::AffineTransform transform;
    transform.translate(raster.offset.x(), raster.offset.y());
    transform.scale(raster.scale, raster.scale);
    return transform;
}

ui::Bitmap transparentImage(const base::Size &size)
{
    return ui::Bitmap(size, ui::Bitmap::Format::Rgba8888Premultiplied);
}

constexpr ui::Color kWhite {255, 255, 255, 255};

} // namespace

ui::Color toUiColor(const Color &color)
{
    const auto channel = [](float value) {
        return uint8_t(base::roundToInt(std::clamp(value, 0.0f, 1.0f) * 255));
    };
    return {channel(color.r), channel(color.g), channel(color.b), channel(color.a)};
}

std::string shapeKey(const ui::Path &path)
{
    base::CryptographicHash hash(base::CryptographicHash::Algorithm::Sha1);
    for (const ui::Path::Element &element : path.elements()) {
        const double values[] = {double(element.type), element.x, element.y};
        hash.addData(values, sizeof(values));
    }
    hash.addData(path.fillRule() == ui::Path::FillRule::Winding ? "w" : "o");
    return base::strings::toHex(hash.result());
}

ShapeRaster ShapeRaster::covering(const base::RectF &logical, double scale, int margin)
{
    const base::Point first(int(std::floor(logical.left() * scale)) - margin,
                       int(std::floor(logical.top() * scale)) - margin);
    const base::Point last(int(std::ceil(logical.right() * scale)) + margin,
                      int(std::ceil(logical.bottom() * scale)) + margin);
    return {scale, -base::PointF(first), base::Size(last.x() - first.x(), last.y() - first.y())};
}

base::RectF ShapeRaster::target() const
{
    return base::RectF(-offset / scale, base::SizeF(size) / scale);
}

std::string ShapeRaster::key() const
{
    return std::format("{:.9g}:{}x{}:{:.9g},{:.9g}", scale, size.width(), size.height(), offset.x(),
                       offset.y());
}

ui::Bitmap rasterizeShape(const ui::Path &path, const ShapeRaster &raster)
{
    ui::Bitmap image = transparentImage(raster.size);
    ui::BitmapPainter painter(image);
    painter.setAntialiasing(true);
    painter.setTransform(deviceTransform(raster));
    painter.fillPath(path, kWhite);
    return image;
}

ui::Bitmap rasterizeShapeOutline(const ui::Path &path, const ShapeRaster &raster, double widthPx,
                                 const Color &outline, const Color &highlight)
{
    const base::AffineTransform transform = deviceTransform(raster);
    // Stroked in physical pixels, so the hairline stays one pixel at any scale.
    const ui::Path physical = path.transformed(transform);
    const ui::Path band = physical.stroked(2 * widthPx, ui::LineCap::Square, ui::LineJoin::Round);

    using Mode = ui::BitmapPainter::CompositionMode;
    const auto ring = [&](const Color &color, Mode cut) {
        ui::Bitmap image = transparentImage(raster.size);
        if (color.a <= 0)
            return image;
        ui::BitmapPainter painter(image);
        painter.setAntialiasing(true);
        painter.fillPath(band, toUiColor(color));
        painter.setCompositionMode(cut);
        painter.fillPath(physical, kWhite);
        return image;
    };

    ui::Bitmap image = ring(outline, Mode::DestinationOut);
    const ui::Bitmap inner = ring(highlight, Mode::DestinationIn);
    ui::BitmapPainter painter(image);
    painter.drawBitmap(base::PointF(), inner);
    return image;
}

} // namespace Lingmo::Render
