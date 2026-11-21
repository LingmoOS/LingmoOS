// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ui/geometry.h"

namespace Lingmo::ui {

// Unpremultiplied 8-bit RGBA pixels, row by row.
class Image
{
public:
    Image() = default;
    Image(int width, int height);

    bool isNull() const { return m_width == 0 || m_height == 0; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    Color pixel(int x, int y) const { return Color::fromArgb(m_pixels[size_t(y) * m_width + x]); }
    void setPixel(int x, int y, Color color) { m_pixels[size_t(y) * m_width + x] = color.argb(); }

    // 0xAARRGGBB.
    uint32_t* bits() { return m_pixels.data(); }
    const uint32_t* bits() const { return m_pixels.data(); }

    bool savePng(const std::string& path) const;

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<uint32_t> m_pixels;
};

} // namespace Lingmo::ui
