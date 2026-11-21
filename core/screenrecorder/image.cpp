// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image.h"

#include <algorithm>
#include <cstring>

Image Image::view(const uint8_t *bits, int width, int height, int stride)
{
    Image image;
    image.bits = bits;
    image.width = width;
    image.height = height;
    image.stride = stride;
    return image;
}

Image Image::allocate(int width, int height)
{
    Image image;
    image.width = std::max(width, 0);
    image.height = std::max(height, 0);
    image.stride = image.width * 4;
    image.storage = std::make_shared<std::vector<uint8_t>>(size_t(image.stride) * image.height);
    image.bits = image.storage->data();
    return image;
}

namespace {

struct Point2
{
    int64_t x;
    int64_t y;
};

} // namespace

// The rotation by 90 degrees times the low two bits, turning +x towards +y,
// then the mirror of bit 4 across the vertical axis.
static Point2 mapped(Point2 point, uint32_t transform)
{
    for (uint32_t i = 0; i < (transform & 3); ++i)
        point = {-point.y, point.x};
    if (transform & 4)
        point.x = -point.x;
    return point;
}

Image upright(const Image &image, uint32_t transform)
{
    if (transform == 0 || image.isNull())
        return image;

    // Coordinates are doubled so pixel centres stay whole.
    const Point2 corners[] = {
        mapped({0, 0}, transform),
        mapped({2 * int64_t(image.width), 0}, transform),
        mapped({0, 2 * int64_t(image.height)}, transform),
        mapped({2 * int64_t(image.width), 2 * int64_t(image.height)}, transform),
    };
    int64_t left = corners[0].x;
    int64_t top = corners[0].y;
    for (const Point2 &corner : corners) {
        left = std::min(left, corner.x);
        top = std::min(top, corner.y);
    }

    const bool turned = transform & 1;
    Image result = Image::allocate(turned ? image.height : image.width, turned ? image.width : image.height);
    uint8_t *target = result.mutableBits();
    for (int y = 0; y < image.height; ++y) {
        const auto *row = reinterpret_cast<const uint32_t *>(image.bits + size_t(y) * image.stride);
        for (int x = 0; x < image.width; ++x) {
            const Point2 centre = mapped({2 * int64_t(x) + 1, 2 * int64_t(y) + 1}, transform);
            const int64_t tx = (centre.x - left - 1) / 2;
            const int64_t ty = (centre.y - top - 1) / 2;
            std::memcpy(target + ty * result.stride + tx * 4, &row[x], 4);
        }
    }
    return result;
}
