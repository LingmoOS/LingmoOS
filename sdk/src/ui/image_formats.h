// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::ui::image_formats {

// Premultiplied 0xAARRGGBB words, row by row.
struct RawImage
{
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;

    bool isNull() const { return pixels.empty(); }
};

// Formats read in process that Skia leaves out.
bool isGif(std::string_view data);
RawImage decodeGif(std::string_view data);
// Netpbm P1 to P6.
bool isPnm(std::string_view data);
RawImage decodePnm(std::string_view data);
// X bitmaps: set bits black, the rest white.
bool isXbm(std::string_view data);
RawImage decodeXbm(std::string_view data);
// Truevision TGA, uncompressed or RLE, true colour or grey. TGA has no
// signature: `path` ends in ".tga", or the file ends with the TGA 2 footer.
bool isTga(std::string_view data, std::string_view path);
RawImage decodeTga(std::string_view data);
// ICNS icon files: the largest PNG inside, for Skia to decode.
bool isIcns(std::string_view data);
std::optional<std::string> largestIcnsPng(std::string_view data);

} // namespace Lingmo::ui::image_formats
