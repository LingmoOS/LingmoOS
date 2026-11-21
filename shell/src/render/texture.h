// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_TEXTURE_H_
#define LINGMO_RENDER_TEXTURE_H_

#include <cstdint>
#include "base/geometry.h"


namespace Lingmo::Render {

// Byte order of the pixel data a texture was uploaded from, in increasing
// address order. wl_shm names formats by their little-endian word layout, so
// WL_SHM_FORMAT_ARGB8888 is Bgra8888 here and WL_SHM_FORMAT_XRGB8888 is
// Bgrx8888. Alpha is premultiplied, as Wayland requires.
enum class PixelFormat {
    Rgba8888,
    Rgbx8888,
    Bgra8888,
    Bgrx8888,
};

inline bool pixelFormatHasAlpha(PixelFormat format)
{
    return format == PixelFormat::Rgba8888 || format == PixelFormat::Bgra8888;
}

// Whether sampling yields the components in BGR order and the shader has to
// swizzle. DMA-BUF imports never do: EGL knows the DRM fourcc.
inline bool pixelFormatIsBgr(PixelFormat format)
{
    return format == PixelFormat::Bgra8888 || format == PixelFormat::Bgrx8888;
}

class Texture
{
public:
    virtual ~Texture() = default;
    virtual base::Size size() const = 0;

    virtual PixelFormat format() const { return PixelFormat::Rgba8888; }

    // Set by whoever filled it when every texel is opaque, as viz clears a
    // quad's needs_blending: it is drawn without reading what lies below.
    bool contentOpaque = false;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_TEXTURE_H_
