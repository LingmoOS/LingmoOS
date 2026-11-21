// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>

#include "base/geometry.h"
#include "render/renderer.h"
#include "ui/bitmap.h"
#include "ui/path.h"

namespace Lingmo::Render {

// Eight bits a channel, rounded.
ui::Color toUiColor(const Color &color);

// Identity of a path for texture cache keys.
std::string shapeKey(const ui::Path &path);

// A logical path placed on physical pixels: pixel = point * scale + offset.
struct ShapeRaster {
    double scale = 1;
    base::PointF offset;
    base::Size size;

    // Covers `logical` with `margin` physical pixels around it, keeping the
    // image origin on a physical pixel of the output.
    static ShapeRaster covering(const base::RectF &logical, double scale, int margin = 0);

    // Where the image goes, in logical output coordinates.
    base::RectF target() const;

    std::string key() const;

    bool operator==(const ShapeRaster &other) const
    {
        return scale == other.scale && base::fuzzyCompare(offset, other.offset) && size == other.size;
    }
};

// Premultiplied RGBA8888 white, so the alpha is the coverage.
ui::Bitmap rasterizeShape(const ui::Path &path, const ShapeRaster &raster);

// A hairline of `widthPx` physical pixels just outside the path in `outline`
// and just inside it in `highlight`.
ui::Bitmap rasterizeShapeOutline(const ui::Path &path, const ShapeRaster &raster, double widthPx,
                                 const Color &outline, const Color &highlight);

} // namespace Lingmo::Render
