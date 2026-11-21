// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "ui/glyph_run.h"
#include "ui/painter.h"
#include "ui/view.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

class Canvas : public View
{
public:
    explicit Canvas(std::function<void(Painter&)> draw)
        : m_draw(std::move(draw))
    {
    }

protected:
    void paint(Painter& painter) override { m_draw(painter); }

private:
    std::function<void(Painter&)> m_draw;
};

Image render(std::function<void(Painter&)> draw, double scale = 1, SizeF size = {100, 60})
{
    Window window;
    window.setScale(scale);
    window.resize(size);
    window.setBackgroundColor(Color::rgb(0xFFFFFF));
    window.setContentView(std::make_unique<Canvas>(std::move(draw)));
    window.show();
    return window.grab();
}

// The ink's bounds: pixels that are not white.
Rect inkBounds(const Image& image)
{
    int left = image.width();
    int top = image.height();
    int right = -1;
    int bottom = -1;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const Color pixel = image.pixel(x, y);
            if (pixel.r < 250 || pixel.g < 250 || pixel.b < 250) {
                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x);
                bottom = std::max(bottom, y);
            }
        }
    }
    return right < 0 ? Rect{} : Rect{left, top, right - left + 1, bottom - top + 1};
}

// Reference font metrics measured on the laptop (offscreen, fontconfig
// hintfull) for the 95 printable ASCII characters.
struct ExpectedMetrics
{
    const char* family;
    double points;
    int ascent;
    int descent;
    int height;
    int leading;
    int underlinePos;
    int strikeOutPos;
    int overlinePos;
    int lineWidth;
    int advance;
    double underlinePosF;
    bool fixedPitch;
};

const ExpectedMetrics kReferenceMetrics[] = {
    {"DejaVu Sans Mono", 9, 11, 3, 14, 0, 0, 3, 12, 1, 665, 0.234375, true},
    {"DejaVu Sans Mono", 10, 12, 3, 15, 0, 0, 4, 13, 1, 760, 0.265625, true},
    {"DejaVu Sans Mono", 11, 14, 4, 18, 0, 0, 4, 15, 1, 855, 0.296875, true},
    {"DejaVu Sans Mono", 12.5, 16, 4, 20, 0, 0, 5, 17, 1, 950, 0.328125, true},
    {"Noto Sans Mono", 9, 13, 4, 17, 0, 1, 4, 14, 1, 665, 1.203125, false},
    {"Noto Sans Mono", 10, 14, 4, 18, 0, 1, 4, 15, 1, 760, 1.296875, false},
    {"Noto Sans Mono", 11, 16, 4, 20, 0, 2, 5, 17, 1, 855, 1.5, false},
    {"Noto Sans Mono", 12.5, 18, 5, 23, 0, 2, 6, 19, 1, 950, 1.703125, false},
    {"Noto Mono", 9, 11, 3, 14, 0, 1, 3, 12, 1, 665, 0.90625, true},
    {"Noto Mono", 11, 14, 4, 18, 0, 1, 4, 15, 1, 855, 1.125, true},
};

const char kPrintable[] =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

} // namespace

TEST(GlyphRunTest, MetricsMatchReference)
{
    for (const ExpectedMetrics& expected : kReferenceMetrics) {
        for (bool bold : {false, true}) {
            const FontMetrics metrics({expected.family, expected.points, bold, false});
            if (metrics.family() != expected.family) {
                GTEST_SKIP() << expected.family << " is not installed";
            }
            SCOPED_TRACE(std::string(expected.family) + " " + std::to_string(expected.points) + (bold ? " bold" : ""));
            EXPECT_EQ(metrics.ascent(), expected.ascent);
            EXPECT_EQ(metrics.descent(), expected.descent);
            EXPECT_EQ(metrics.height(), expected.height);
            EXPECT_EQ(metrics.leading(), expected.leading);
            EXPECT_EQ(metrics.underlinePos(), expected.underlinePos);
            EXPECT_DOUBLE_EQ(metrics.underlinePosF(), expected.underlinePosF);
            EXPECT_EQ(metrics.strikeOutPos(), expected.strikeOutPos);
            EXPECT_EQ(metrics.overlinePos(), expected.overlinePos);
            EXPECT_EQ(metrics.lineWidth(), expected.lineWidth);
            EXPECT_EQ(metrics.horizontalAdvance(kPrintable), expected.advance);
            EXPECT_EQ(metrics.fixedPitch(), expected.fixedPitch);
        }
    }
}

TEST(GlyphRunTest, DrawsClustersAtWholeDevicePixels)
{
    const FontSpec font{"DejaVu Sans Mono", 11, false, false};
    if (FontMetrics(font).family() != font.family)
        GTEST_SKIP() << "DejaVu Sans Mono is not installed";
    for (double scale : {1.0, 1.5, 2.0}) {
        GlyphRenderer renderer(font, scale);
        const std::u32string text = U"I";
        const Image one = render([&](Painter& painter) {
            const GlyphCluster cluster{text, 10};
            renderer.draw(painter, Color::rgb(0x000000), 30, {&cluster, 1});
        }, scale);
        const Image moved = render([&](Painter& painter) {
            const GlyphCluster cluster{text, 17};
            renderer.draw(painter, Color::rgb(0x000000), 30, {&cluster, 1});
        }, scale);
        const Rect a = inkBounds(one);
        const Rect b = inkBounds(moved);
        ASSERT_GT(a.width, 0);
        // Moving the origin by whole device pixels moves the same ink.
        EXPECT_EQ(b.x - a.x, 7);
        EXPECT_EQ(a.width, b.width);
        // The baseline is in device pixels: the stem ends on it.
        EXPECT_EQ(a.y + a.height, 30);
        EXPECT_EQ(renderer.cachedClusters(), 1u);
    }
}

TEST(GlyphRunTest, FallsBackForWhatTheFontLacks)
{
    const FontSpec font{"DejaVu Sans Mono", 11, false, false};
    GlyphRenderer renderer(font, 1);
    const std::u32string han = U"\u4E2D";
    const Image image = render([&](Painter& painter) {
        const GlyphCluster cluster{han, 10};
        renderer.draw(painter, Color::rgb(0x000000), 40, {&cluster, 1});
    });
    const Rect ink = inkBounds(image);
    // A full-width ideograph, about twice a cell wide.
    EXPECT_GT(ink.width, 10);
    EXPECT_GT(renderer.advance(han), 12);
}

// Block elements through the CFF fallback (Noto Sans CJK under Noto Mono)
// where the reference draws them: CFF stems darkened, which also moves
// their hinted edges. Reference ink rows on the laptop at scale 1, baseline
// 20.
TEST(GlyphRunTest, FallbackBlockElementsSitOnDarkenedStems)
{
    const FontSpec font{"Noto Mono", 10, false, false};
    if (FontMetrics(font).family() != "Noto Mono")
        GTEST_SKIP() << "Noto Mono is not installed";
    struct Expected
    {
        char32_t character;
        int top;
        int bottom;
    };
    for (const Expected& expected : {Expected{U'\u2581', 20, 21}, Expected{U'\u2585', 13, 21},
                                     Expected{U'\u2591', 7, 22}}) {
        GlyphRenderer renderer(font, 1);
        const std::u32string text(1, expected.character);
        const Image image = render([&](Painter& painter) {
            const GlyphCluster cluster{text, 10};
            renderer.draw(painter, Color::rgb(0x000000), 20, {&cluster, 1});
        });
        const Rect ink = inkBounds(image);
        if (ink.height == 0)
            GTEST_SKIP() << "no fallback for block elements";
        EXPECT_EQ(ink.y, expected.top) << uint32_t(expected.character);
        EXPECT_LE(ink.y + ink.height - 1, expected.bottom) << uint32_t(expected.character);
    }
}

// Scaled text draws unhinted at the device size: the reference ink for
// "find xenon mux 0123" in Noto Mono 10, a cell every 8 px from x 4,
// baseline 20, on white (bounds in device pixels, then the summed ink).
TEST(GlyphRunTest, ScaledTextIsUnhintedAtTheDeviceSize)
{
    const FontSpec font{"Noto Mono", 10, false, false};
    if (FontMetrics(font).family() != "Noto Mono")
        GTEST_SKIP() << "Noto Mono is not installed";
    struct Expected
    {
        double scale;
        Rect bounds;
        long ink;
    };
    for (const Expected& expected : {Expected{1, {4, 10, 151, 10}, 77258}, Expected{1.5, {7, 15, 226, 16}, 177153},
                                     Expected{2, {9, 20, 301, 21}, 314420}}) {
        GlyphRenderer renderer(font, expected.scale);
        const std::u32string text = U"find xenon mux 0123";
        const Image image = render(
            [&](Painter& painter) {
                std::vector<GlyphCluster> clusters;
                for (size_t i = 0; i < text.size(); ++i)
                    clusters.push_back({std::u32string_view(text).substr(i, 1), (4 + i * 8) * expected.scale});
                renderer.draw(painter, Color::rgb(0x000000), 20 * expected.scale, clusters);
            },
            expected.scale, {300, 30});
        const Rect ink = inkBounds(image);
        long sum = 0;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x)
                sum += 255 - image.pixel(x, y).r;
        }
        EXPECT_EQ(ink.y, expected.bounds.y) << expected.scale;
        EXPECT_EQ(ink.height, expected.bounds.height) << expected.scale;
        EXPECT_NEAR(double(sum), double(expected.ink), expected.ink * 0.005) << expected.scale;
    }
}

TEST(GlyphRunTest, ColourGlyphsKeepTheirColours)
{
    GlyphRenderer renderer({"DejaVu Sans Mono", 14, false, false}, 1);
    const std::u32string emoji = U"\U0001F600";
    const Image image = render([&](Painter& painter) {
        const GlyphCluster cluster{emoji, 10};
        renderer.draw(painter, Color::rgb(0x000000), 40, {&cluster, 1});
    });
    const Rect ink = inkBounds(image);
    if (ink.width == 0)
        GTEST_SKIP() << "no emoji font";
    bool coloured = false;
    for (int y = ink.y; y < ink.y + ink.height; ++y) {
        for (int x = ink.x; x < ink.x + ink.width; ++x) {
            const Color pixel = image.pixel(x, y);
            if (pixel.r > pixel.b + 60)
                coloured = true;
        }
    }
    EXPECT_TRUE(coloured);
}

TEST(GlyphRunTest, ShapesCombiningSequencesAsOneCluster)
{
    GlyphRenderer renderer({"DejaVu Sans Mono", 11, false, false}, 1);
    const std::u32string base = U"e";
    const std::u32string combined = U"é";
    const Image plain = render([&](Painter& painter) {
        const GlyphCluster cluster{base, 10};
        renderer.draw(painter, Color::rgb(0x000000), 40, {&cluster, 1});
    });
    const Image accented = render([&](Painter& painter) {
        const GlyphCluster cluster{combined, 10};
        renderer.draw(painter, Color::rgb(0x000000), 40, {&cluster, 1});
    });
    // The accent sits above the letter, within its cell.
    EXPECT_LT(inkBounds(accented).y, inkBounds(plain).y);
    EXPECT_LE(inkBounds(accented).width, inkBounds(plain).width + 2);
}

TEST(GlyphRunTest, StretchesAboutAnOrigin)
{
    GlyphRenderer renderer({"DejaVu Sans Mono", 11, false, false}, 1);
    const std::u32string text = U"H";
    const Image normal = render([&](Painter& painter) {
        const GlyphCluster cluster{text, 10};
        renderer.draw(painter, Color::rgb(0x000000), 30, {&cluster, 1});
    });
    const Image wide = render([&](Painter& painter) {
        const GlyphCluster cluster{text, 10};
        renderer.draw(painter, Color::rgb(0x000000), 30, {&cluster, 1}, {2, 2}, {10, 12});
    });
    const Rect a = inkBounds(normal);
    const Rect b = inkBounds(wide);
    EXPECT_NEAR(b.width, 2 * a.width, 2);
    EXPECT_NEAR(b.height, 2 * a.height, 2);
    EXPECT_NEAR(b.x - 10, 2 * (a.x - 10), 2);
}

TEST(GlyphRunTest, FillsDeviceRectsWithoutAntialiasing)
{
    const Image image = render([](Painter& painter) {
        painter.translate({3.4, 2});
        fillDeviceRect(painter, {1, 1, 5, 3}, Color::rgb(0xFF0000));
    }, 1.5);
    // The offset rounds to device pixels (5, 3), then the rect is exact.
    const Rect ink = inkBounds(image);
    EXPECT_EQ(ink.x, 6);
    EXPECT_EQ(ink.y, 4);
    EXPECT_EQ(ink.width, 5);
    EXPECT_EQ(ink.height, 3);
    EXPECT_EQ(image.pixel(6, 4), Color::rgb(0xFF0000));
}

TEST(GlyphRunTest, ListsFamiliesSortedWithPitch)
{
    const std::vector<FontFamily> families = fontFamilies();
    ASSERT_FALSE(families.empty());
    for (size_t i = 1; i < families.size(); ++i) {
        std::string a = families[i - 1].name;
        std::string b = families[i].name;
        for (char& c : a)
            c = char(std::tolower(static_cast<unsigned char>(c)));
        for (char& c : b)
            c = char(std::tolower(static_cast<unsigned char>(c)));
        EXPECT_LE(a, b);
    }
    for (const FontFamily& family : families) {
        if (family.name == "DejaVu Sans Mono")
            EXPECT_TRUE(family.fixedPitch);
        if (family.name == "DejaVu Sans")
            EXPECT_FALSE(family.fixedPitch);
    }
}

TEST(GlyphRunTest, CachedFullScreenFrame)
{
    GlyphRenderer renderer({"DejaVu Sans Mono", 11, false, false}, 1.5);
    const int columns = 175;
    const int rows = 52;
    std::vector<std::u32string> cells;
    for (int i = 0; i < columns; ++i)
        cells.push_back(std::u32string(1, char32_t(U'!' + i % 94)));
    const auto drawFrame = [&](Painter& painter) {
        std::vector<GlyphCluster> clusters(columns);
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                fillDeviceRect(painter, {column * 11, row * 27, 11, 27}, Color::rgb(0x101010 * (column % 8)));
                clusters[size_t(column)] = {cells[size_t(column)], double(column * 11)};
            }
            renderer.draw(painter, Color::rgb(0xE0E0E0), row * 27 + 21, clusters);
        }
    };
    render(drawFrame, 1.5, {1300, 950});
    const auto start = std::chrono::steady_clock::now();
    render(drawFrame, 1.5, {1300, 950});
    const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start);
    // Software rendering, so only reported; the GPU path is measured live.
    printf("175x52 cached frame, raster: %.1f ms\n", elapsed.count());
    EXPECT_EQ(renderer.cachedClusters(), 94u);
}

// Output that makes a new cluster every time, such as random combining
// sequences, keeps the cache bounded, and what is drawn stays the same.
TEST(GlyphRunTest, ClusterCacheStaysBounded)
{
    GlyphRenderer renderer({"DejaVu Sans Mono", 11, false, false}, 1);
    const std::u32string accented = U"e\u0301";
    const double advance = renderer.advance(accented);
    for (char32_t base = 0x4E00; base < 0x4E00 + 20000; ++base)
        renderer.advance(std::u32string{base, 0x0301});
    EXPECT_LE(renderer.cachedClusters(), 8192u);
    EXPECT_EQ(renderer.advance(accented), advance);
}

// Faces opened before fallback faces are added keep their font bytes: each
// new fallback face grows the face list, and the first face's FreeType face
// reads its font from memory that must not move (the terminal's `cat` of box
// drawing and other scripts crashed in FT_Load_Glyph).
TEST(GlyphRunTest, OpenFacesSurviveManyFallbackFaces)
{
    const FontSpec font{"DejaVu Sans Mono", 11, false, false};
    GlyphRenderer renderer(font, 1.5);
    const auto draw = [](GlyphRenderer& with, const std::u32string& text) {
        return render([&](Painter& painter) {
            const GlyphCluster cluster{text, 10};
            with.draw(painter, Color::rgb(0x000000), 40, {&cluster, 1});
        }, 1.5);
    };
    // The first face opened and used.
    EXPECT_GT(inkBounds(draw(renderer, U"A")).width, 0);
    // Characters from many scripts, each likely to bring a fallback face,
    // with a glyph of the first face rasterised for the first time after each.
    const std::u32string fallbacks[] = {U"\u4E2D", U"\u3042", U"\uD55C", U"ب", U"א", U"ก", U"अ", U"ক", U"த", U"ಕ", U"ქ", U"Ա",
                                        U"ᐁ", U"ᚠ", U"ሀ", U"ꦄ", U"ᠠ", U"ⴰ", U"𐌰", U"∮", U"⡿", U"♞", U"✈", U"𝄞"};
    const std::u32string own = U"BCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t next = 0;
    for (const std::u32string& fallback : fallbacks) {
        draw(renderer, fallback);
        for (int i = 0; i < 2 && next < own.size(); ++i, ++next) {
            const std::u32string glyph(1, own[next]);
            GlyphRenderer fresh(font, 1.5);
            const Image expected = draw(fresh, glyph);
            const Image actual = draw(renderer, glyph);
            ASSERT_EQ(inkBounds(actual), inkBounds(expected)) << char(own[next]);
        }
    }
}
