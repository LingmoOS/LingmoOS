// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A very small software rasterizer for the pieces of the frame the compositor
// renderer cannot draw itself: rounded corners, circles and icon strokes. The
// renderer knows solid rectangles and textured quads, so everything with a
// curve in it is rasterized here once, uploaded, and then drawn as a quad.
//
// Not a general 2D library. Coverage is 4x4 supersampled, which is enough for
// a 30px titlebar and keeps the whole thing in one file.

#ifndef LINGMO_DECORATION_RASTER_H_
#define LINGMO_DECORATION_RASTER_H_

#include <cstdint>
#include <vector>

#include "base/geometry.h"

#include "render/renderer.h"

namespace Lingmo::Decoration {

// Premultiplied RGBA8888, which is what the renderer's texture path blends.
class Image
{
public:
    explicit Image(const base::Size& size);

    base::Size size() const { return m_size; }
    int stride() const { return m_size.width() * 4; }
    const uint8_t* bits() const { return m_pixels.data(); }

    // Blends `color` over the pixel with the given coverage, 0..1.
    void blend(int x, int y, const render::Color& color, float coverage);

private:
    base::Size m_size;
    std::vector<uint8_t> m_pixels;
};

// Optional per-corner selection for small raster assets.
struct Corners {
    bool topLeft = true;
    bool topRight = true;
    bool bottomLeft = true;
    bool bottomRight = true;

    static Corners top() { return Corners {true, true, false, false}; }
    static Corners all() { return Corners(); }
};

void fillRoundedRect(Image& image, const base::RectF& rect, double radius, const render::Color& color,
                     const Corners& corners = Corners::all());

// Window silhouettes use continuous corners; decoration controls keep circular arcs.
void fillContinuousRoundedRect(Image& image, const base::RectF& rect, double radius,
                               const render::Color& color, const Corners& corners = Corners::all());

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_RASTER_H_
