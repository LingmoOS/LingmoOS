// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "ui/glyph_run.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// A label alone in a headless window of the size, at the scale.
struct Hosted
{
    explicit Hosted(SizeF size, double scale = 1)
    {
        auto root = std::make_unique<Column>();
        label = root->addChild<Label>();
        label->setLayoutHints({.fillWidth = true});
        window.setContentView(std::move(root));
        window.resize(size);
        window.setScale(scale);
        window.show();
    }

    void layout() { window.grab(); }

    Window window;
    Label* label = nullptr;
};

} // namespace

TEST(LabelTest, ElidesTheMiddleOfALongLine)
{
    Hosted hosted({120, 40});
    hosted.label->setText("/media/user/A Very Long Volume Name/Projects");
    hosted.label->setElide(Label::Elide::Middle);
    hosted.layout();
    EXPECT_TRUE(hosted.label->isElided());
    const Rect ink = TestInput::inkRect(*hosted.label, 1);
    EXPECT_LE(ink.x + ink.width, 120);

    hosted.label->setText("/short");
    hosted.layout();
    EXPECT_FALSE(hosted.label->isElided());
}

TEST(LabelTest, MiddleAndRightElisionDiffer)
{
    Hosted middle({120, 40});
    Hosted right({120, 40});
    for (Hosted* hosted : {&middle, &right})
        hosted->label->setText("/media/user/A Very Long Volume Name/Projects");
    middle.label->setElide(Label::Elide::Middle);
    const Image a = middle.window.grab();
    const Image b = right.window.grab();
    EXPECT_TRUE(right.label->isElided());
    ASSERT_EQ(a.width(), b.width());
    ASSERT_EQ(a.height(), b.height());
    EXPECT_FALSE(std::equal(a.bits(), a.bits() + size_t(a.width()) * a.height(), b.bits()));
}

TEST(LabelTest, WrapsToAMaximumOfLines)
{
    Hosted hosted({80, 200});
    hosted.label->setText("one two three four five six seven eight nine ten eleven twelve");
    hosted.label->setWrap(true);
    const double unlimited = hosted.label->heightForWidth(80);
    hosted.label->setMaximumLineCount(2);
    const double limited = hosted.label->heightForWidth(80);
    hosted.label->setText("one");
    const double single = hosted.label->heightForWidth(80);
    EXPECT_GT(unlimited, limited);
    EXPECT_NEAR(limited, 2 * single, 1);
    hosted.label->setText("one two three four five six seven eight nine ten eleven twelve");
    hosted.layout();
    EXPECT_TRUE(hosted.label->isElided());
}

TEST(LabelTest, SpansCarryTheirOwnWeight)
{
    Hosted hosted({300, 40});
    hosted.label->setText("October 2026");
    const double regular = hosted.label->sizeHint().width;
    hosted.label->setSpans({{"October", 700, {}}, {" 2026", 0, {}}});
    EXPECT_EQ(hosted.label->text(), "October 2026");
    EXPECT_GT(hosted.label->sizeHint().width, regular);
}

TEST(LabelTest, PlacesItsBaselineAsAlignedLabelDoes)
{
    for (const double scale : {1.0, 1.25, 1.5, 2.0}) {
        Hosted hosted({200, 60}, scale);
        hosted.label->setText("Baseline");
        hosted.layout();
        // The baseline offset: the ascent, unsnapped.
        EXPECT_DOUBLE_EQ(hosted.label->baselineOffset(), hosted.label->ascent());

        hosted.label->setBaseline(30);
        hosted.layout();
        // The top margin is snapped; the baseline follows it.
        const double margin = std::round((30 - hosted.label->ascent()) * scale) / scale;
        EXPECT_NEAR(hosted.label->baselineOffset(), margin + hosted.label->ascent(), 1e-9) << scale;
        EXPECT_GT(hosted.label->sizeHint().height, 30);
        const double cap = hosted.label->capHeight();
        EXPECT_EQ(cap, std::round(cap));
        EXPECT_NEAR(hosted.label->capCenter(), 30 - cap / 2, 1 / scale);
    }
}

TEST(LabelTest, PaintsEveryWrappedLineInside)
{
    for (const double scale : {1.0, 1.5}) {
        for (const auto alignment : {Label::TextAlignment::Left, Label::TextAlignment::Center,
                                     Label::TextAlignment::Right}) {
            Window window;
            window.setBackgroundColor(Color::rgb(0xFFFFFF));
            auto root = std::make_unique<Column>();
            Label* label = root->addChild<Label>(
                "Shortcuts that use the Command key work in every application, and the key can be changed");
            label->setWrap(true);
            label->setTextAlignment(alignment);
            label->setLayoutHints({.preferredWidth = 160});
            window.setContentView(std::move(root));
            window.resize({200, 200});
            window.setScale(scale);
            window.show();
            const Image image = window.grab();
            const RectF box = label->geometry();
            EXPECT_GT(box.height, 50);
            // Ink in every line's band, and none outside the label.
            const double lineHeight = label->heightForWidth(160) / std::round(label->heightForWidth(160) / 17);
            for (double top = 0; top + lineHeight <= box.height + 0.5; top += lineHeight) {
                bool inked = false;
                for (int y = int(top * scale); y < int((top + lineHeight) * scale) && !inked; ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        if (image.pixel(x, y).r < 200) {
                            inked = true;
                            break;
                        }
                    }
                }
                EXPECT_TRUE(inked) << "line at " << top << " scale " << scale;
            }
            for (int y = 0; y < image.height(); ++y) {
                for (int x = int(std::ceil((box.x + box.width) * scale)) + 1; x < image.width(); ++x)
                    EXPECT_GE(image.pixel(x, y).r, 250) << x << "," << y << " scale " << scale;
            }
        }
    }
}

TEST(LabelTest, FitsItsOwnSizeHintInEveryMode)
{
    using Alignment = Label::TextAlignment;
    using Elide = Label::Elide;
    for (const double scale : {1.0, 1.25, 1.5, 2.0}) {
        for (const Alignment alignment : {Alignment::Left, Alignment::Center, Alignment::Right}) {
            for (const Elide elide : {Elide::None, Elide::Right, Elide::Middle}) {
                for (const int lines : {0, 2}) {
                    for (const char* text : {"Left", "Right", "Less warm", "2160 × 1440", "Built-in", "Today"}) {
                        Window window;
                        auto root = std::make_unique<Row>();
                        // A fractional x, which device rounding narrows.
                        root->setMargins({10.3, 0, 0, 0});
                        Label* label = root->addChild<Label>(text);
                        label->setTextAlignment(alignment);
                        label->setElide(elide);
                        label->setWrap(lines > 0);
                        label->setMaximumLineCount(lines);
                        window.setContentView(std::move(root));
                        window.resize({400, 100});
                        window.setScale(scale);
                        window.show();
                        window.grab();
                        const SizeF hint = label->sizeHint();
                        const std::string what = std::string(text) + " at " + std::to_string(scale);
                        EXPECT_LT(hint.width, 200) << what;
                        EXPECT_GT(hint.width, 10) << what;
                        EXPECT_EQ(label->geometry().width, hint.width) << what;
                        EXPECT_FALSE(label->isElided()) << what;
                        // Every glyph inside the label's device box; subpixel
                        // text's LCD filter reaches one pixel further.
                        const Rect ink = TestInput::inkRect(*label, scale);
                        EXPECT_GE(ink.x, -1) << what;
                        EXPECT_LE(ink.x + ink.width, int(std::ceil(hint.width * scale)) + 1) << what;
                    }
                }
            }
        }
    }
}

TEST(LabelTest, FontFamilyIsUsed)
{
    Label narrow("iiiiiiii");
    Label wide("MMMMMMMM");
    narrow.setFontFamily("Noto Mono");
    wide.setFontFamily("Noto Mono");
    // A fixed-pitch family: both as wide.
    EXPECT_NEAR(narrow.sizeHint().width, wide.sizeHint().width, 0.5);
}

// Rounded advances make a wrapped line wider than the paragraph's run at
// 1.0; the line was cut at the run's width ("will be los").
TEST(LabelTest, WrappedLinesDrawWhole)
{
    for (const double scale : {1.0, 1.5}) {
        Hosted hosted({332, 80}, scale);
        hosted.label->setText("Your changes will be lost if you don't save them. "
                              "Your changes will be lost if you don't save them.");
        hosted.label->setFontSize(14);
        hosted.label->setWrap(true);
        hosted.layout();
        ASSERT_GT(hosted.label->lineCount(), 1u);
        const auto line = hosted.label->lineGeometry(0);
        ASSERT_TRUE(line);
        const Rect ink = TestInput::inkRect(*hosted.label, scale);
        // The ink is in device pixels; the widest line ends within a bearing of its width.
        EXPECT_GE(ink.x + ink.width, int(line->width * scale) - 3) << scale;
    }
}

// Reference elided text (and the last wrapped line) measured on the laptop:
// Noto Sans at 12 px, scale 1, hinted.
TEST(LabelTest, ElidesByWholeGraphemes)
{
    struct Case
    {
        const char* text;
        double width;
        const char* middle;
        const char* right;
        const char* twoLines;
    };
    const Case cases[] = {
        {"A file whose name is long enough to need two lines.txt", 60, "A fil…s.txt", "A file wh…", "A file whose n…"},
        {"A file whose name is long enough to need two lines.txt", 97, "A file w…ines.txt", "A file whose na…",
         "A file whose name is long e…"},
        {"A file whose name is long enough to need two lines.txt", 150, "A file whose …two lines.txt",
         "A file whose name is lon…", "A file whose name is long enough to need two lines…"},
        {"Screenshot from 2026-10-16 12-34-56.png", 120, "Screensh…4-56.png", "Screenshot from 20…",
         "Screenshot from 2026-10-16 12-34-5…"},
        {"very_long_file_name_without_any_spaces_at_all.tar.gz", 97, "very_lon…l.tar.gz", "very_long_file_n…",
         "very_long_file_name_without_an…"},
        {"Quarterly report final (2).pdf", 150, "Quarterly rep…final (2).pdf", "Quarterly report final (2).…",
         "Quarterly report final (2).pdf"},
    };
    for (const Case& c : cases) {
        if (FontMetrics(FontSpec{"Noto Sans", 9}).family() != "Noto Sans")
            GTEST_SKIP() << "Noto Sans is not installed";
        for (const Label::Elide elide : {Label::Elide::Middle, Label::Elide::Right}) {
            Hosted hosted({c.width, 40});
            hosted.label->setFontFamily("Noto Sans");
            hosted.label->setFontSize(12);
            hosted.label->setElide(elide);
            hosted.label->setText(c.text);
            hosted.layout();
            EXPECT_EQ(hosted.label->shownText(), elide == Label::Elide::Middle ? c.middle : c.right) << c.width;
        }
        Hosted wrapped({c.width, 60});
        wrapped.label->setFontFamily("Noto Sans");
        wrapped.label->setFontSize(12);
        wrapped.label->setWrap(true);
        wrapped.label->setMaximumLineCount(2);
        wrapped.label->setText(c.text);
        wrapped.layout();
        EXPECT_EQ(wrapped.label->shownText(), c.twoLines) << c.width;
    }
}

// At scale 1 glyphs take fontconfig's hinting and subpixel order as the
// reference renders them: the cross-section of the "l" stem in Noto Sans
// 10 pt, white on black, from a capture of the reference's glyph cache.
TEST(LabelTest, StemMatchesTheReferenceAtScaleOne)
{
    if (FontMetrics(FontSpec{"Noto Sans", 10}).family() != "Noto Sans")
        GTEST_SKIP() << "Noto Sans is not installed";
    Window window;
    window.setBackgroundColor(Color::rgb(0x000000));
    auto root = std::make_unique<Column>();
    Label* label = root->addChild<Label>("l H l");
    label->setFontFamily("Noto Sans");
    label->setFontSize(10.0 * 4 / 3);
    label->setColor(Color::rgb(0xFFFFFF));
    label->setLayoutHints({.fillWidth = true});
    window.setContentView(std::move(root));
    window.resize({100, 30});
    window.show();
    const Image image = window.grab();
    int left = -1;
    int top = -1;
    int bottom = -1;
    bool fringed = false;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const Color c = image.pixel(x, y);
            fringed = fringed || std::abs(int(c.r) - int(c.b)) > 40;
            if (std::max({c.r, c.g, c.b}) > 60) {
                left = left < 0 ? x : std::min(left, x);
                top = top < 0 ? y : top;
                bottom = y;
            }
        }
    }
    ASSERT_GE(left, 1);
    if (!fringed)
        GTEST_SKIP() << "fontconfig asks for no subpixel order";
    const int y = (top + bottom) / 2;
    const int expected[3][3] = {{0, 5, 59}, {142, 220, 225}, {149, 65, 6}};
    for (int i = 0; i < 3; ++i) {
        const Color c = image.pixel(left - 1 + i, y);
        EXPECT_NEAR(c.r, expected[i][0], 8) << i;
        EXPECT_NEAR(c.g, expected[i][1], 8) << i;
        EXPECT_NEAR(c.b, expected[i][2], 8) << i;
    }
}
