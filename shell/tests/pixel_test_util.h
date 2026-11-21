// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_PIXEL_TEST_UTIL_H_
#define LINGMO_TESTS_PIXEL_TEST_UTIL_H_

#include <GLES3/gl3.h>

#include <cstdint>

#include "base/geometry.h"
#include "ui/bitmap.h"

namespace Lingmo::Test {

// Bitmap::pixel() values: 0xAARRGGBB.
constexpr uint32_t kRed = 0xffff0000;
constexpr uint32_t kGreen = 0xff00ff00;
constexpr uint32_t kBlue = 0xff0000ff;
constexpr uint32_t kBlack = 0xff000000;
constexpr uint32_t kWhite = 0xffffffff;

inline int red(uint32_t pixel)
{
    return int(pixel >> 16 & 0xff);
}

inline int green(uint32_t pixel)
{
    return int(pixel >> 8 & 0xff);
}

inline int blue(uint32_t pixel)
{
    return int(pixel & 0xff);
}

inline int alpha(uint32_t pixel)
{
    return int(pixel >> 24);
}

inline uint32_t pixelAt(const ui::Bitmap &image, const base::Point &point)
{
    return image.pixel(point.x(), point.y());
}

// The bound framebuffer's bytes as GL keeps them, rows bottom-up.
inline ui::Bitmap readFramebuffer(const base::Size &size)
{
    ui::Bitmap image(size, ui::Bitmap::Format::Rgba8888Premultiplied);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    return image;
}

} // namespace Lingmo::Test

#endif // LINGMO_TESTS_PIXEL_TEST_UTIL_H_
