// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <vector>

#include "base/event_loop.h"
#include "ui/animated_image.h"
#include "ui/application.h"
#include "ui/gif_decoder.h"
#include "ui/image_formats.h"
#include "ui/view.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

constexpr uint32_t kRed = 0xFFFF0000;
constexpr uint32_t kGreen = 0xFF00FF00;
constexpr uint32_t kBlue = 0xFF0000FF;
constexpr uint32_t kWhite = 0xFFFFFFFF;

void put16(std::string& out, int value)
{
    out += char(value & 0xFF);
    out += char((value >> 8) & 0xFF);
}

// LZW with a clear code after every two codes, so the code size stays at
// three bits: valid GIF, if not small.
std::string lzw(const std::vector<int>& indices)
{
    std::vector<int> codes = {4};
    int sinceClear = 0;
    for (int index : indices) {
        if (sinceClear == 2) {
            codes.push_back(4);
            sinceClear = 0;
        }
        codes.push_back(index);
        ++sinceClear;
    }
    codes.push_back(5);
    std::string bytes;
    uint32_t bits = 0;
    int count = 0;
    for (int code : codes) {
        bits |= uint32_t(code) << count;
        count += 3;
        while (count >= 8) {
            bytes += char(bits & 0xFF);
            bits >>= 8;
            count -= 8;
        }
    }
    if (count > 0)
        bytes += char(bits & 0xFF);
    std::string blocks;
    blocks += char(2);
    for (size_t i = 0; i < bytes.size(); i += 255) {
        const std::string chunk = bytes.substr(i, 255);
        blocks += char(chunk.size());
        blocks += chunk;
    }
    blocks += char(0);
    return blocks;
}

struct Frame
{
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    std::vector<int> indices;
    int delay = 0;
    int disposal = 0;
    int transparent = -1;
    bool interlaced = false;
};

// A screen with the palette red, green, blue, white.
std::string gif(int width, int height, const std::vector<Frame>& frames, int loops = -1)
{
    std::string out = "GIF89a";
    put16(out, width);
    put16(out, height);
    out += char(0x81);
    out += char(3);
    out += char(0);
    out += std::string("\xFF\x00\x00\x00\xFF\x00\x00\x00\xFF\xFF\xFF\xFF", 12);
    if (loops >= 0) {
        out += std::string("\x21\xFF\x0BNETSCAPE2.0\x03\x01", 16);
        put16(out, loops);
        out += char(0);
    }
    for (const Frame& frame : frames) {
        out += std::string("\x21\xF9\x04", 3);
        out += char((frame.disposal << 2) | (frame.transparent >= 0 ? 1 : 0));
        put16(out, frame.delay);
        out += char(frame.transparent >= 0 ? frame.transparent : 0);
        out += char(0);
        out += char(0x2C);
        put16(out, frame.left);
        put16(out, frame.top);
        put16(out, frame.width);
        put16(out, frame.height);
        out += char(frame.interlaced ? 0x40 : 0);
        out += lzw(frame.indices);
    }
    out += char(0x3B);
    return out;
}

} // namespace

TEST(ImageFormatsTest, DecodesAStillGif)
{
    const std::string data = gif(3, 2, {{0, 0, 3, 2, {0, 1, 2, 3, 2, 1}, 0, 0, 3}});
    ASSERT_TRUE(image_formats::isGif(data));
    const image_formats::RawImage image = image_formats::decodeGif(data);
    ASSERT_EQ(image.width, 3);
    ASSERT_EQ(image.height, 2);
    EXPECT_EQ(image.pixels, (std::vector<uint32_t>{kRed, kGreen, kBlue, 0, kBlue, kGreen}));
}

TEST(ImageFormatsTest, AnimatesWithDisposalAndLoops)
{
    const std::string data = gif(2, 2,
                                 {{0, 0, 2, 2, {0, 0, 0, 0}, 0, 2},
                                  {1, 1, 1, 1, {1}, 30, 3},
                                  {0, 0, 1, 1, {2}, 7, 0}},
                                 0);
    GifDecoder decoder;
    ASSERT_TRUE(decoder.open(data));
    EXPECT_EQ(decoder.loopCount(), 0);
    const uint32_t* pixels = nullptr;
    int delay = 0;
    ASSERT_TRUE(decoder.nextFrame(&pixels, &delay));
    EXPECT_EQ(delay, 100);
    EXPECT_EQ(std::vector<uint32_t>(pixels, pixels + 4), (std::vector<uint32_t>{kRed, kRed, kRed, kRed}));
    // The first frame restores to the background: cleared before the second.
    ASSERT_TRUE(decoder.nextFrame(&pixels, &delay));
    EXPECT_EQ(delay, 300);
    EXPECT_EQ(std::vector<uint32_t>(pixels, pixels + 4), (std::vector<uint32_t>{0, 0, 0, kGreen}));
    // The second restores what was under it.
    ASSERT_TRUE(decoder.nextFrame(&pixels, &delay));
    EXPECT_EQ(delay, 70);
    EXPECT_EQ(std::vector<uint32_t>(pixels, pixels + 4), (std::vector<uint32_t>{kBlue, 0, 0, 0}));
    EXPECT_FALSE(decoder.nextFrame(&pixels, &delay));
    decoder.rewind();
    ASSERT_TRUE(decoder.nextFrame(&pixels, &delay));
    EXPECT_EQ(pixels[0], kRed);
}

TEST(ImageFormatsTest, DeinterlacesRows)
{
    // Rows 0..4 stored in the order 0, 4, 2, 1, 3.
    const std::string data = gif(1, 5, {{0, 0, 1, 5, {0, 3, 2, 1, 1}, 0, 0, -1, true}});
    const image_formats::RawImage image = image_formats::decodeGif(data);
    EXPECT_EQ(image.pixels, (std::vector<uint32_t>{kRed, kGreen, kBlue, kGreen, kWhite}));
}

TEST(ImageFormatsTest, SurvivesBrokenGifs)
{
    std::string data = gif(4, 4, {{0, 0, 4, 4, std::vector<int>(16, 1)}});
    for (size_t cut = 0; cut < data.size(); ++cut)
        image_formats::decodeGif(std::string_view(data).substr(0, cut));
    // Garbage after the header decodes to whatever made it.
    std::string garbage = data.substr(0, 30) + std::string(200, '\xAB');
    image_formats::decodeGif(garbage);
    SUCCEED();
}

TEST(ImageFormatsTest, DecodesNetpbm)
{
    const image_formats::RawImage bits = image_formats::decodePnm("P1\n# c\n3 2\n101\n0 1 0\n");
    EXPECT_EQ(bits.pixels, (std::vector<uint32_t>{0xFF000000, kWhite, 0xFF000000, kWhite, 0xFF000000, kWhite}));
    const image_formats::RawImage grey = image_formats::decodePnm("P2\n2 1\n1000\n500 1000\n");
    // 16-bit grey keeps the high byte.
    EXPECT_EQ(grey.pixels, (std::vector<uint32_t>{0xFF7F7F7F, kWhite}));
    const image_formats::RawImage colour = image_formats::decodePnm(std::string("P6 1 1 255\n\x10\x20\x30", 14));
    EXPECT_EQ(colour.pixels, (std::vector<uint32_t>{0xFF102030}));
    EXPECT_TRUE(image_formats::decodePnm("P3\n2 2\n255\n1 2").pixels.size() == 4);
}

TEST(ImageFormatsTest, DecodesXbm)
{
    const std::string data = "#define t_width 10\n#define t_height 2\nstatic char t_bits[] = {\n"
                             "0x01, 0x02, 0xff, 0x03 };\n";
    ASSERT_TRUE(image_formats::isXbm(data));
    const image_formats::RawImage image = image_formats::decodeXbm(data);
    ASSERT_EQ(image.width, 10);
    // Least significant bit first: the first pixel, then the tenth.
    EXPECT_EQ(image.pixels[0], 0xFF000000u);
    EXPECT_EQ(image.pixels[1], kWhite);
    EXPECT_EQ(image.pixels[9], 0xFF000000u);
    EXPECT_EQ(image.pixels[10 + 9], 0xFF000000u);
}

TEST(ImageFormatsTest, DecodesTga)
{
    // 2x1 true colour, 32 bits with alpha, top-left origin.
    std::string data(18, '\0');
    data[2] = 2;
    data[12] = 2;
    data[14] = 1;
    data[16] = 32;
    data[17] = 0x28;
    data += std::string("\x30\x20\x10\xFF\x00\x00\xFF\x80", 8);
    ASSERT_TRUE(image_formats::isTga(data, "a.TGA"));
    const image_formats::RawImage image = image_formats::decodeTga(data);
    ASSERT_EQ(image.pixels.size(), 2u);
    EXPECT_EQ(image.pixels[0], 0xFF102030u);
    // Premultiplied red at half alpha.
    EXPECT_EQ(image.pixels[1], 0x80800000u);
}

TEST(AnimatedImageTest, PlaysGifFramesOnTheFrameClock)
{
    const std::string data = gif(2, 2, {{0, 0, 2, 2, {0, 0, 0, 0}, 5}, {0, 0, 2, 2, {1, 1, 1, 1}, 5}}, 0);
    const std::string path = "/tmp/lingmo-toolkit-animated-test-" + std::to_string(getpid()) + ".gif";
    {
        FILE* file = fopen(path.c_str(), "wb");
        ASSERT_NE(file, nullptr);
        fwrite(data.data(), 1, data.size(), file);
        fclose(file);
    }
    Window window;
    window.resize({20, 20});
    window.setContentView(std::make_unique<View>());
    window.show();
    AnimatedImage image(&window);
    bool loaded = false;
    int changes = 0;
    image.onLoaded = [&](bool ok) {
        loaded = ok;
        Application::instance()->loop().quit();
    };
    image.onFrameChanged = [&] { ++changes; };
    image.load(path);
    const int timeout = Application::instance()->loop().startTimer(std::chrono::seconds(5), false,
                                                                   [] { Application::instance()->loop().quit(); });
    Application::instance()->loop().run();
    Application::instance()->loop().stopTimer(timeout);
    ASSERT_TRUE(loaded);
    EXPECT_EQ(image.frameCount(), 2);
    EXPECT_EQ(image.size(), Lingmo::Base::Size(2, 2));
    EXPECT_EQ(image.loopCount(), -1);
    EXPECT_EQ(image.currentFrame(), 0);
    // 50 ms frames (delay 5): a frame clock of 16.7 ms moves on every third.
    uint64_t time = 1000000000;
    for (int i = 0; i < 4; ++i) {
        window.grab(time);
        time += 16666667;
    }
    EXPECT_EQ(image.currentFrame(), 1);
    image.setPlaying(false);
    for (int i = 0; i < 10; ++i) {
        window.grab(time);
        time += 16666667;
    }
    EXPECT_EQ(image.currentFrame(), 1);
    EXPECT_GE(changes, 2);
    std::remove(path.c_str());
}
