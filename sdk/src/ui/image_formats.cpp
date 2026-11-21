// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/image_formats.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "ui/gif_decoder.h"

namespace Lingmo::ui::image_formats {

// A 256 MiB allocation limit, in pixels of four bytes.
static constexpr int64_t kMaximumPixels = int64_t(256) * 1024 * 1024 / 4;

static bool sizeAllowed(int64_t width, int64_t height)
{
    return width > 0 && height > 0 && width * height <= kMaximumPixels;
}

static uint32_t premultiply(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    if (a == 255)
        return 0xFF000000u | r << 16 | g << 8 | b;
    const auto scale = [a](uint32_t c) { return (c * a + 127) / 255; };
    return a << 24 | scale(r) << 16 | scale(g) << 8 | scale(b);
}

bool isGif(std::string_view data)
{
    return data.starts_with("GIF87a") || data.starts_with("GIF89a");
}

RawImage decodeGif(std::string_view data)
{
    GifDecoder decoder;
    RawImage image;
    if (!decoder.open(data))
        return image;
    const uint32_t* pixels = nullptr;
    int delay = 0;
    if (!decoder.nextFrame(&pixels, &delay))
        return image;
    image.width = decoder.width();
    image.height = decoder.height();
    image.pixels.assign(pixels, pixels + size_t(image.width) * size_t(image.height));
    return image;
}

bool isPnm(std::string_view data)
{
    return data.size() >= 3 && data[0] == 'P' && data[1] >= '1' && data[1] <= '6' && std::isspace(uint8_t(data[2]));
}

namespace {

// The header's tokens, skipping white space and comments.
struct PnmReader
{
    std::string_view data;
    size_t position = 0;

    bool number(int* value)
    {
        while (position < data.size()) {
            const char c = data[position];
            if (c == '#') {
                while (position < data.size() && data[position] != '\n')
                    ++position;
            } else if (std::isspace(uint8_t(c))) {
                ++position;
            } else {
                break;
            }
        }
        if (position >= data.size() || !std::isdigit(uint8_t(data[position])))
            return false;
        int64_t result = 0;
        while (position < data.size() && std::isdigit(uint8_t(data[position]))) {
            result = result * 10 + (data[position] - '0');
            if (result > 1 << 30)
                return false;
            ++position;
        }
        *value = int(result);
        return true;
    }

    // A single ASCII bit for P1, where "01" may run together.
    bool bit(int* value)
    {
        while (position < data.size() && (std::isspace(uint8_t(data[position])) || data[position] == '#')) {
            if (data[position] == '#') {
                while (position < data.size() && data[position] != '\n')
                    ++position;
            } else {
                ++position;
            }
        }
        if (position >= data.size() || (data[position] != '0' && data[position] != '1'))
            return false;
        *value = data[position++] - '0';
        return true;
    }
};

} // namespace

RawImage decodePnm(std::string_view data)
{
    RawImage image;
    if (!isPnm(data))
        return image;
    const char kind = data[1];
    PnmReader reader{data, 2};
    int width = 0;
    int height = 0;
    int maximum = 1;
    if (!reader.number(&width) || !reader.number(&height))
        return image;
    if (kind != '1' && kind != '4' && (!reader.number(&maximum) || maximum <= 0 || maximum > 65535))
        return image;
    if (!sizeAllowed(width, height))
        return image;
    const bool binary = kind >= '4';
    // One white space byte ends the header of a binary file.
    if (binary)
        ++reader.position;
    image.width = width;
    image.height = height;
    image.pixels.assign(size_t(width) * size_t(height), 0xFF000000u);
    // 8-bit grey truncates to 255ths, 16-bit grey keeps the high byte, and
    // colour goes through 16 bits, then to 8 as (v - (v >> 8) + 0x80) >> 8.
    const auto through16 = [maximum](int value) {
        const uint32_t wide = uint32_t(std::min(value, maximum)) * 0xFFFFu / uint32_t(maximum);
        return (wide - (wide >> 8) + 0x80) >> 8;
    };
    const auto scaledGrey = [&](int value) {
        if (maximum == 255)
            return uint32_t(value & 0xFF);
        if (maximum < 255)
            return uint32_t(std::min(value, maximum)) * 255 / uint32_t(maximum);
        // 16-bit grey converts by dropping the low byte.
        return (uint32_t(std::min(value, maximum)) * 0xFFFFu / uint32_t(maximum)) >> 8;
    };
    const auto scaled = [&](int value) {
        return maximum == 255 ? uint32_t(value & 0xFF) : through16(value);
    };
    const int sampleBytes = maximum > 255 ? 2 : 1;
    const auto sample = [&](int* value) {
        if (!binary)
            return reader.number(value);
        if (reader.position + size_t(sampleBytes) > data.size())
            return false;
        *value = uint8_t(data[reader.position]);
        if (sampleBytes == 2)
            *value = *value << 8 | uint8_t(data[reader.position + 1]);
        reader.position += size_t(sampleBytes);
        return true;
    };
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint32_t& pixel = image.pixels[size_t(y) * size_t(width) + size_t(x)];
            if (kind == '1' || kind == '4') {
                int bit = 0;
                if (kind == '1') {
                    if (!reader.bit(&bit))
                        return image;
                } else {
                    const size_t byte = reader.position + size_t(x / 8);
                    if (byte >= data.size())
                        return image;
                    bit = (uint8_t(data[byte]) >> (7 - x % 8)) & 1;
                }
                pixel = bit ? 0xFF000000u : 0xFFFFFFFFu;
            } else if (kind == '2' || kind == '5') {
                int value = 0;
                if (!sample(&value))
                    return image;
                const uint32_t grey = scaledGrey(value);
                pixel = 0xFF000000u | grey << 16 | grey << 8 | grey;
            } else {
                int r = 0;
                int g = 0;
                int b = 0;
                if (!sample(&r) || !sample(&g) || !sample(&b))
                    return image;
                pixel = 0xFF000000u | scaled(r) << 16 | scaled(g) << 8 | scaled(b);
            }
        }
        if (kind == '4')
            reader.position += size_t((width + 7) / 8);
    }
    return image;
}

bool isXbm(std::string_view data)
{
    const std::string_view head = data.substr(0, 512);
    return head.find("#define") != std::string_view::npos && head.find("_width") != std::string_view::npos;
}

static int xbmDefine(std::string_view data, std::string_view suffix)
{
    size_t position = 0;
    while ((position = data.find("#define", position)) != std::string_view::npos) {
        position += 7;
        while (position < data.size() && std::isspace(uint8_t(data[position])))
            ++position;
        const size_t nameStart = position;
        while (position < data.size() && !std::isspace(uint8_t(data[position])))
            ++position;
        const std::string_view name = data.substr(nameStart, position - nameStart);
        if (name.ends_with(suffix))
            return std::atoi(std::string(data.substr(position, 16)).c_str());
    }
    return 0;
}

RawImage decodeXbm(std::string_view data)
{
    RawImage image;
    const int width = xbmDefine(data, "_width");
    const int height = xbmDefine(data, "_height");
    const size_t brace = data.find('{');
    if (!sizeAllowed(width, height) || brace == std::string_view::npos)
        return image;
    image.width = width;
    image.height = height;
    image.pixels.assign(size_t(width) * size_t(height), 0xFFFFFFFFu);
    const int bytesPerRow = (width + 7) / 8;
    size_t position = brace + 1;
    for (int index = 0; index < bytesPerRow * height; ++index) {
        const size_t hex = data.find("0x", position);
        const size_t hexUpper = data.find("0X", position);
        const size_t start = std::min(hex, hexUpper);
        if (start == std::string_view::npos)
            break;
        char* end = nullptr;
        const std::string token(data.substr(start + 2, 4));
        const unsigned long value = std::strtoul(token.c_str(), &end, 16);
        position = start + 2 + size_t(end - token.c_str());
        const int y = index / bytesPerRow;
        const int xByte = index % bytesPerRow;
        // Least significant bit first.
        for (int bit = 0; bit < 8; ++bit) {
            const int x = xByte * 8 + bit;
            if (x < width && (value >> bit) & 1)
                image.pixels[size_t(y) * size_t(width) + size_t(x)] = 0xFF000000u;
        }
    }
    return image;
}

bool isTga(std::string_view data, std::string_view path)
{
    if (data.size() < 18)
        return false;
    if (data.size() >= 26 && data.substr(data.size() - 18, 16) == "TRUEVISION-XFILE")
        return true;
    std::string lower(path.substr(path.size() >= 4 ? path.size() - 4 : 0));
    std::transform(lower.begin(), lower.end(), lower.begin(), [](char c) { return char(std::tolower(uint8_t(c))); });
    return lower == ".tga";
}

RawImage decodeTga(std::string_view data)
{
    RawImage image;
    if (data.size() < 18)
        return image;
    const auto u8 = [&](size_t i) { return uint32_t(uint8_t(data[i])); };
    const uint32_t idLength = u8(0);
    const uint32_t colorMapType = u8(1);
    const uint32_t type = u8(2);
    const uint32_t mapLength = u8(5) | u8(6) << 8;
    const uint32_t mapEntryBits = u8(7);
    const int width = int(u8(12) | u8(13) << 8);
    const int height = int(u8(14) | u8(15) << 8);
    const uint32_t depth = u8(16);
    const uint32_t descriptor = u8(17);
    const bool grey = type == 3 || type == 11;
    const bool rle = type == 10 || type == 11;
    if ((type != 2 && type != 3 && type != 10 && type != 11) || !sizeAllowed(width, height))
        return image;
    if (grey ? depth != 8 && depth != 16 : depth != 15 && depth != 16 && depth != 24 && depth != 32)
        return image;
    const uint32_t alphaBits = descriptor & 15;
    size_t position = 18 + idLength;
    if (colorMapType == 1)
        position += size_t(mapLength) * ((mapEntryBits + 7) / 8);
    const size_t bytes = (depth + 7) / 8;

    const auto readPixel = [&](size_t at) -> uint32_t {
        if (grey) {
            const uint32_t v = u8(at);
            const uint32_t a = depth == 16 ? u8(at + 1) : 255;
            return premultiply(v, v, v, a);
        }
        if (depth == 15 || depth == 16) {
            const uint32_t word = u8(at) | u8(at + 1) << 8;
            const auto expand = [](uint32_t five) { return (five << 3) | (five >> 2); };
            const uint32_t a = depth == 16 && alphaBits ? ((word & 0x8000) ? 255 : 0) : 255;
            return premultiply(expand((word >> 10) & 31), expand((word >> 5) & 31), expand(word & 31), a);
        }
        const uint32_t a = depth == 32 && alphaBits ? u8(at + 3) : 255;
        return premultiply(u8(at + 2), u8(at + 1), u8(at), a);
    };

    std::vector<uint32_t> decoded;
    decoded.reserve(size_t(width) * size_t(height));
    const size_t total = size_t(width) * size_t(height);
    while (decoded.size() < total) {
        if (!rle) {
            if (position + bytes > data.size())
                break;
            decoded.push_back(readPixel(position));
            position += bytes;
            continue;
        }
        if (position >= data.size())
            break;
        const uint32_t header = u8(position++);
        const size_t count = (header & 0x7F) + 1;
        if (header & 0x80) {
            if (position + bytes > data.size())
                break;
            const uint32_t pixel = readPixel(position);
            position += bytes;
            for (size_t i = 0; i < count && decoded.size() < total; ++i)
                decoded.push_back(pixel);
        } else {
            for (size_t i = 0; i < count && decoded.size() < total; ++i) {
                if (position + bytes > data.size())
                    break;
                decoded.push_back(readPixel(position));
                position += bytes;
            }
        }
    }
    if (decoded.empty())
        return image;
    decoded.resize(total, 0);
    image.width = width;
    image.height = height;
    image.pixels.resize(total);
    const bool topDown = descriptor & 0x20;
    const bool rightToLeft = descriptor & 0x10;
    for (int y = 0; y < height; ++y) {
        const int sourceRow = topDown ? y : height - 1 - y;
        for (int x = 0; x < width; ++x) {
            const int sourceColumn = rightToLeft ? width - 1 - x : x;
            image.pixels[size_t(y) * size_t(width) + size_t(x)]
                    = decoded[size_t(sourceRow) * size_t(width) + size_t(sourceColumn)];
        }
    }
    return image;
}

bool isIcns(std::string_view data)
{
    return data.size() >= 8 && data.starts_with("icns");
}

std::optional<std::string> largestIcnsPng(std::string_view data)
{
    if (!isIcns(data))
        return std::nullopt;
    const auto u32 = [&](size_t i) {
        return uint32_t(uint8_t(data[i])) << 24 | uint32_t(uint8_t(data[i + 1])) << 16
                | uint32_t(uint8_t(data[i + 2])) << 8 | uint32_t(uint8_t(data[i + 3]));
    };
    std::string_view best;
    size_t position = 8;
    while (position + 8 <= data.size()) {
        const uint32_t length = u32(position + 4);
        if (length < 8 || position + length > data.size())
            break;
        const std::string_view chunk = data.substr(position + 8, length - 8);
        if (chunk.starts_with("\x89PNG") && chunk.size() > best.size())
            best = chunk;
        position += length;
    }
    if (best.empty())
        return std::nullopt;
    return std::string(best);
}

} // namespace Lingmo::ui::image_formats
