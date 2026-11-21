// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include <unicode/ubrk.h>

#include "base/strings.h"
#include "decoration/text.h"

using namespace Lingmo::Decoration;

namespace {
class TitleText : public testing::Test
{
protected:
    TextRenderer renderer;

    Lingmo::base::Size textSize(const std::string& text, int size)
    {
        return renderer.textSize(text, size);
    }

    Image renderText(const std::string& text, int size, const Lingmo::render::Color& color)
    {
        return renderer.renderText(text, size, color);
    }

    std::string elideText(const std::string& text, int size, int width)
    {
        return renderer.elideText(text, size, width);
    }

    std::vector<uint8_t> pixels(const std::string& text)
    {
        const auto image = renderText(text, 14, {1, 1, 1, 1});
        return {image.bits(), image.bits() + image.stride() * image.size().height()};
    }
};
} // namespace

TEST_F(TitleText, EmptyAndInvalidSizesAreSafeWithoutGuiApplication)
{
    EXPECT_TRUE(textSize({}, 14).isEmpty());
    EXPECT_TRUE(textSize("Title", 0).isEmpty());
    EXPECT_TRUE(textSize("Title", -1).isEmpty());
    EXPECT_TRUE(textSize("Title", 129).isEmpty());
    EXPECT_TRUE(elideText("Title", 14, 0).empty());
}

TEST_F(TitleText, RendererLifetimesAreIndependent)
{
    const std::string title = "Editor — \u4e2d\u6587 — العربية";
    const auto expected = pixels(title);
    ASSERT_FALSE(expected.empty());
    for (int iteration = 0; iteration < 3; ++iteration) {
        {
            TextRenderer temporary;
            EXPECT_EQ(temporary.textSize(title, 14), textSize(title, 14));
            EXPECT_FALSE(temporary.renderText(title, 18, {1, 1, 1, 1}).size().isEmpty());
            EXPECT_EQ(temporary.textSize(title, 14), textSize(title, 14));
        }
        EXPECT_EQ(pixels(title), expected);
    }
}

TEST_F(TitleText, UsesProportionalSystemGlyphsAndAntialiasing)
{
    EXPECT_GT(textSize("WWW", 14).width(),
              textSize("iii", 14).width());
    const auto image = renderText("Title", 14, {1, 0.5f, 0.25f, 0.5f});
    EXPECT_EQ(image.size(), textSize("Title", 14));
    bool ink = false, antialiasing = false;
    for (int i = 0; i < image.stride() * image.size().height(); i += 4) {
        const uint8_t* p = image.bits() + i;
        EXPECT_LE(p[0], p[3]);
        EXPECT_LE(p[1], p[3]);
        EXPECT_LE(p[2], p[3]);
        EXPECT_LE(p[3], 128);
        ink |= p[3] > 0;
        antialiasing |= p[3] > 0 && p[3] < 120;
    }
    EXPECT_TRUE(ink);
    EXPECT_TRUE(antialiasing);
}

TEST_F(TitleText, UnicodeTitlesAreNotRepeatedPlaceholderBoxes)
{
    // The test environment supplies Noto CJK and DejaVu Arabic/Hebrew fonts.
    EXPECT_NE(pixels("\u4e2d\u6587"), pixels("\u6807\u9898"));
    EXPECT_EQ(pixels("é"), pixels("e\u0301"));
    EXPECT_NE(pixels("سلام"), pixels("س ل ا م"));
    EXPECT_NE(pixels("אבג"), pixels("גבא"));
    EXPECT_EQ(pixels("אבג"), pixels("\u202Dגבא\u202C"));
    const auto mixed = pixels("Editor — \u4e2d\u6587 — العربية — 😀");
    EXPECT_TRUE(std::any_of(mixed.begin(), mixed.end(), [](uint8_t b) {
        return b != 0;
    }));
}

TEST_F(TitleText, BidiFormattingDoesNotAddFontMetrics)
{
    for (const std::string text : {"\u202D\u202C", "\u2066\u2069", "\u200E\u200F\u061C"}) {
        EXPECT_TRUE(textSize(text, 14).isEmpty());
        EXPECT_TRUE(renderText(text, 14, {1, 1, 1, 1}).size().isEmpty());
    }
    const std::string hebrew = "אבג";
    const std::string isolated = "\u2067אבג\u2069";
    EXPECT_EQ(textSize(hebrew, 14), textSize(isolated, 14));
    EXPECT_EQ(pixels(hebrew), pixels(isolated));
    EXPECT_EQ(pixels(hebrew), pixels("\u202Dגבא\u202C"));
}

TEST_F(TitleText, ElisionFitsAtGraphemeBoundaries)
{
    const std::string title = "A e\u0301 \u4e2d\u6587 👩‍💻 العربية document title";
    // Positions are UTF-16 units, as ICU's grapheme breaks take them.
    const std::u16string title16 = Lingmo::base::strings::utf8ToUtf16(title);
    UErrorCode error = U_ZERO_ERROR;
    std::unique_ptr<UBreakIterator, decltype(&ubrk_close)> graphemes(
        ubrk_open(UBRK_CHARACTER, nullptr, reinterpret_cast<const UChar*>(title16.data()),
                  int32_t(title16.size()), &error),
        ubrk_close);
    ASSERT_TRUE(U_SUCCESS(error));
    for (int width = 1; width < textSize(title, 14).width(); ++width) {
        const std::string elided = elideText(title, 14, width);
        EXPECT_LE(textSize(elided, 14).width(), width);
        if (elided.empty())
            continue;
        const std::u16string elided16 = Lingmo::base::strings::utf8ToUtf16(elided);
        const size_t ellipsis = elided16.find(u'\u2026');
        ASSERT_NE(ellipsis, std::u16string::npos);
        EXPECT_EQ(elided16.substr(0, ellipsis), title16.substr(0, ellipsis));
        EXPECT_TRUE(ubrk_isBoundary(graphemes.get(), int32_t(ellipsis)));
    }
    EXPECT_EQ(elideText(title, 14, 1000), title);
    EXPECT_EQ(elideText(title, 14, textSize("\u2026", 14).width()), "\u2026");
}

TEST_F(TitleText, NewlinesStayOnOneLineAndInputWorkIsBounded)
{
    EXPECT_EQ(pixels("a\nb\rc\td\u2028e\u2029f"),
              pixels("a b c d e f"));
    EXPECT_EQ(textSize(std::string(100000, 'a'), 14), textSize(std::string(4096, 'a'), 14));
}

TEST_F(TitleText, EmojiPresentationUsesColorAndScalesBitmapStrikes)
{
    const auto image = renderText("😀👩‍💻", 14, {1, 1, 1, 1});
    ASSERT_FALSE(image.size().isEmpty());
    EXPECT_LT(image.size().height(), 30);
    bool colored = false;
    for (int i = 0; i < image.stride() * image.size().height(); i += 4) {
        const auto* p = image.bits() + i;
        colored |= p[0] != p[1] || p[1] != p[2];
        EXPECT_LE(p[0], p[3]);
        EXPECT_LE(p[1], p[3]);
        EXPECT_LE(p[2], p[3]);
    }
    EXPECT_TRUE(colored);
}

TEST_F(TitleText, DeviceRasterKeepsLogicalShapingAndElision)
{
    for (const std::string title : {"Fractional title", "العربية \u4e2d\u6587", "😀👩‍💻"}) {
        const std::string elided = elideText(title, 14, 70);
        const Lingmo::base::Size logical = textSize(elided, 14);
        for (double density : {1.0, 2.0, 3.0, 4.0, 1.25, 1.5, 1.75, 1.0}) {
            const auto image = renderer.renderText(elided, 14, {1, 1, 1, 1}, density);
            EXPECT_EQ(image.size(), Lingmo::base::Size(int(std::ceil(logical.width() * density)),
                                                         int(std::ceil(logical.height() * density))))
                    << density;
            const uint8_t *bits = image.bits();
            const size_t bytes = size_t(image.stride()) * image.size().height();
            bool inked = false;
            for (size_t i = 3; i < bytes && !inked; i += 4)
                inked = bits[i] > 0;
            EXPECT_TRUE(inked) << density;
            EXPECT_EQ(textSize(elided, 14), logical);
        }
    }
}
