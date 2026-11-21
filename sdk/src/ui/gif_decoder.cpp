// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/gif_decoder.h"

#include <algorithm>
#include <cstring>
#include <string>

namespace Lingmo::ui {

// A 256 MiB allocation limit, in pixels of four bytes.
static constexpr int64_t kMaximumPixels = int64_t(256) * 1024 * 1024 / 4;

static uint16_t readU16(std::string_view data, size_t position)
{
    return uint16_t(uint8_t(data[position]) | uint16_t(uint8_t(data[position + 1])) << 8);
}

static std::vector<uint32_t> readPalette(std::string_view data, size_t& position, int entries)
{
    std::vector<uint32_t> palette;
    if (position + size_t(entries) * 3 > data.size())
        return palette;
    palette.reserve(size_t(entries));
    for (int i = 0; i < entries; ++i) {
        const auto r = uint8_t(data[position]);
        const auto g = uint8_t(data[position + 1]);
        const auto b = uint8_t(data[position + 2]);
        palette.push_back(0xFF000000u | uint32_t(r) << 16 | uint32_t(g) << 8 | b);
        position += 3;
    }
    return palette;
}

// Skips data sub-blocks up to and including the terminator.
static bool skipSubBlocks(std::string_view data, size_t& position)
{
    while (position < data.size()) {
        const auto length = uint8_t(data[position++]);
        if (length == 0)
            return true;
        position += length;
    }
    return false;
}

bool GifDecoder::open(std::string_view data)
{
    if (data.size() < 13 || (data.substr(0, 6) != "GIF87a" && data.substr(0, 6) != "GIF89a"))
        return false;
    m_data = data;
    m_width = readU16(data, 6);
    m_height = readU16(data, 8);
    const auto packed = uint8_t(data[10]);
    m_background = (packed & 0x80) ? int(uint8_t(data[11])) : -1;
    m_position = 13;
    if (packed & 0x80)
        m_globalPalette = readPalette(data, m_position, 2 << (packed & 7));
    if (m_width <= 0 || m_height <= 0 || int64_t(m_width) * m_height > kMaximumPixels)
        return false;
    m_firstBlock = m_position;
    // The loop count may sit anywhere before the first image; look ahead.
    size_t position = m_position;
    while (position + 1 < data.size() && uint8_t(data[position]) == 0x21) {
        const auto label = uint8_t(data[position + 1]);
        position += 2;
        if (label == 0xFF && position + 12 <= data.size() && uint8_t(data[position]) == 11
            && data.substr(position + 1, 11) == "NETSCAPE2.0") {
            size_t sub = position + 12;
            if (sub + 4 <= data.size() && uint8_t(data[sub]) == 3 && uint8_t(data[sub + 1]) == 1)
                m_loopCount = readU16(data, sub + 2);
        }
        if (!skipSubBlocks(data, position))
            break;
    }
    rewind();
    return true;
}

void GifDecoder::rewind()
{
    m_position = m_firstBlock;
    m_screen.assign(size_t(m_width) * size_t(m_height), 0);
    m_saved.clear();
    m_lastDisposal = 0;
    m_first = true;
    m_lastWidth = 0;
    m_lastHeight = 0;
}

// Past the palette or the transparent index is transparent.
static uint32_t paletteColor(const std::vector<uint32_t>& palette, int index, int transparent)
{
    if (index < 0 || size_t(index) >= palette.size() || index == transparent)
        return 0;
    return palette[size_t(index)];
}

void GifDecoder::fill(int left, int top, int width, int height, uint32_t color)
{
    for (int y = std::max(0, top); y < std::min(m_height, top + height); ++y) {
        for (int x = std::max(0, left); x < std::min(m_width, left + width); ++x)
            m_screen[size_t(y) * size_t(m_width) + size_t(x)] = color;
    }
}

void GifDecoder::dispose(int)
{
    // Restoring to the background clears to transparent, as browsers do.
    if (m_lastDisposal == 2)
        fill(m_lastLeft, m_lastTop, m_lastWidth, m_lastHeight, 0);
    else if (m_lastDisposal == 3 && m_saved.size() == m_screen.size())
        m_screen = m_saved;
    m_lastDisposal = 0;
}

bool GifDecoder::nextFrame(const uint32_t** pixels, int* delayMs)
{
    Control control;
    while (m_position < m_data.size()) {
        const auto introducer = uint8_t(m_data[m_position++]);
        if (introducer == 0x3B)
            return false;
        if (introducer == 0x21) {
            if (m_position >= m_data.size())
                return false;
            const auto label = uint8_t(m_data[m_position++]);
            if (label == 0xF9 && m_position + 5 < m_data.size() && uint8_t(m_data[m_position]) >= 4) {
                const auto packed = uint8_t(m_data[m_position + 1]);
                control.disposal = (packed >> 2) & 7;
                control.delay = readU16(m_data, m_position + 2);
                control.transparent = (packed & 1) ? int(uint8_t(m_data[m_position + 4])) : -1;
            }
            if (!skipSubBlocks(m_data, m_position))
                return false;
            continue;
        }
        if (introducer != 0x2C)
            return false;
        if (!readImage(control))
            return false;
        *pixels = m_screen.data();
        // "As fast as possible" frames are stretched to 100 ms, as browsers do.
        const int delay = control.delay * 10;
        *delayMs = delay <= 10 ? 100 : delay;
        return true;
    }
    return false;
}

bool GifDecoder::readImage(const Control& control)
{
    if (m_position + 9 > m_data.size())
        return false;
    const int left = readU16(m_data, m_position);
    const int top = readU16(m_data, m_position + 2);
    const int width = readU16(m_data, m_position + 4);
    const int height = readU16(m_data, m_position + 6);
    const auto packed = uint8_t(m_data[m_position + 8]);
    m_position += 9;
    std::vector<uint32_t> localPalette;
    if (packed & 0x80)
        localPalette = readPalette(m_data, m_position, 2 << (packed & 7));
    const std::vector<uint32_t>& palette = (packed & 0x80) ? localPalette : m_globalPalette;
    const bool interlaced = packed & 0x40;
    if (m_position >= m_data.size())
        return false;
    const int minimumCodeSize = uint8_t(m_data[m_position++]);
    if (minimumCodeSize < 1 || minimumCodeSize > 11)
        return false;

    // Gathers the LZW stream out of its sub-blocks.
    std::string stream;
    while (m_position < m_data.size()) {
        const auto length = uint8_t(m_data[m_position++]);
        if (length == 0)
            break;
        const size_t available = std::min(size_t(length), m_data.size() - m_position);
        stream.append(m_data.substr(m_position, available));
        m_position += available;
    }

    // LZW decode into palette indices; a truncated stream leaves the rest
    // of the frame untouched, as decoders do.
    const size_t count = size_t(width) * size_t(height);
    if (int64_t(width) * height > kMaximumPixels)
        return false;
    std::vector<uint8_t> indices;
    indices.reserve(count);
    const int clearCode = 1 << minimumCodeSize;
    const int endCode = clearCode + 1;
    std::vector<uint16_t> prefix(4096);
    std::vector<uint8_t> suffix(4096);
    std::vector<uint8_t> first(4096);
    std::vector<uint8_t> output;
    output.reserve(4096);
    for (int i = 0; i < clearCode; ++i) {
        suffix[size_t(i)] = uint8_t(i);
        first[size_t(i)] = uint8_t(i);
    }
    int codeSize = minimumCodeSize + 1;
    int nextCode = endCode + 1;
    int previous = -1;
    uint32_t bits = 0;
    int bitCount = 0;
    size_t byte = 0;
    while (indices.size() < count) {
        while (bitCount < codeSize && byte < stream.size()) {
            bits |= uint32_t(uint8_t(stream[byte++])) << bitCount;
            bitCount += 8;
        }
        if (bitCount < codeSize)
            break;
        const int code = int(bits & ((1u << codeSize) - 1));
        bits >>= codeSize;
        bitCount -= codeSize;
        if (code == clearCode) {
            codeSize = minimumCodeSize + 1;
            nextCode = endCode + 1;
            previous = -1;
            continue;
        }
        if (code == endCode)
            break;
        int emit = code;
        uint8_t firstByte;
        if (previous < 0) {
            if (code >= clearCode)
                break;
            indices.push_back(uint8_t(code));
            previous = code;
            continue;
        }
        if (code < nextCode) {
            firstByte = first[size_t(code)];
        } else if (code == nextCode) {
            firstByte = first[size_t(previous)];
            emit = previous;
        } else {
            break;
        }
        // The string of `emit`, reversed out of the table.
        output.clear();
        for (int walk = emit; ; walk = prefix[size_t(walk)]) {
            output.push_back(suffix[size_t(walk)]);
            if (walk < clearCode)
                break;
        }
        for (auto it = output.rbegin(); it != output.rend() && indices.size() < count; ++it)
            indices.push_back(*it);
        if (code == nextCode && indices.size() < count)
            indices.push_back(firstByte);
        if (nextCode < 4096) {
            prefix[size_t(nextCode)] = uint16_t(previous);
            suffix[size_t(nextCode)] = firstByte;
            first[size_t(nextCode)] = first[size_t(previous)];
            ++nextCode;
            if (nextCode == (1 << codeSize) && codeSize < 12)
                ++codeSize;
        }
        previous = code;
    }

    dispose(control.transparent);
    // A first frame smaller than the screen sits on the background.
    if (m_first && (left || top || width < m_width || height < m_height)) {
        if (control.transparent < 0 && m_background >= 0)
            fill(0, 0, m_width, m_height, paletteColor(palette, m_background, -1));
    }
    m_first = false;
    if (control.disposal == 3)
        m_saved = m_screen;
    // Interlaced rows come in four passes.
    std::vector<int> rows;
    rows.reserve(size_t(height));
    if (interlaced) {
        for (int y = 0; y < height; y += 8)
            rows.push_back(y);
        for (int y = 4; y < height; y += 8)
            rows.push_back(y);
        for (int y = 2; y < height; y += 4)
            rows.push_back(y);
        for (int y = 1; y < height; y += 2)
            rows.push_back(y);
    } else {
        for (int y = 0; y < height; ++y)
            rows.push_back(y);
    }
    for (size_t row = 0; row < rows.size(); ++row) {
        const int y = top + rows[row];
        if (y < 0 || y >= m_height)
            continue;
        for (int x = 0; x < width; ++x) {
            const size_t source = row * size_t(width) + size_t(x);
            if (source >= indices.size())
                break;
            const int index = indices[source];
            const int screenX = left + x;
            if (index == control.transparent || screenX >= m_width || size_t(index) >= palette.size())
                continue;
            m_screen[size_t(y) * size_t(m_width) + size_t(screenX)] = palette[size_t(index)];
        }
    }
    m_lastDisposal = control.disposal;
    m_lastLeft = left;
    m_lastTop = top;
    m_lastWidth = width;
    m_lastHeight = height;
    return true;
}

} // namespace Lingmo::ui
