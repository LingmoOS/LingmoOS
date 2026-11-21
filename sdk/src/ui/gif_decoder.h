// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace Lingmo::ui {

// GIF frames composited onto the logical screen one after another: each
// frame's disposal applied when the next one starts, "restore to
// background" clearing to transparent. A first frame smaller than the
// screen sits on the background colour.
class GifDecoder
{
public:
    // Reads the header; false when the data is not a GIF. The data must
    // outlive the decoder.
    bool open(std::string_view data);

    int width() const { return m_width; }
    int height() const { return m_height; }
    // NETSCAPE2.0's count: 0 loops forever, -1 when the file sets none
    // (played once).
    int loopCount() const { return m_loopCount; }

    // Composites the next frame. `pixels` is the screen, premultiplied
    // 0xAARRGGBB words, width * height of them, which the decoder owns and
    // keeps; `delayMs` is how long the frame shows. False at the end or on
    // broken data with no frame left.
    bool nextFrame(const uint32_t** pixels, int* delayMs);
    // Back to the first frame, for another loop.
    void rewind();

private:
    struct Control
    {
        int disposal = 0;
        int delay = 0;
        int transparent = -1;
    };

    bool readImage(const Control& control);
    void dispose(int newTransparent);
    void fill(int left, int top, int width, int height, uint32_t color);

    std::string_view m_data;
    size_t m_position = 0;
    size_t m_firstBlock = 0;
    int m_width = 0;
    int m_height = 0;
    int m_loopCount = -1;
    std::vector<uint32_t> m_globalPalette;
    std::vector<uint32_t> m_screen;
    std::vector<uint32_t> m_saved;
    // The last frame's rect and disposal, applied before the next frame.
    int m_lastDisposal = 0;
    int m_background = -1;
    bool m_first = true;
    int m_lastLeft = 0;
    int m_lastTop = 0;
    int m_lastWidth = 0;
    int m_lastHeight = 0;
};

} // namespace Lingmo::ui
