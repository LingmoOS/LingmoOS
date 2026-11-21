// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "ui/keys.h"
#include "ui/label.h"
#include "ui/line_breaks.h"
#include "ui/test_input.h"
#include "ui/text_area.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

std::string toUtf8(const std::u16string& text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        uint32_t c = text[i];
        if (c >= 0xD800 && c < 0xDC00 && i + 1 < text.size())
            c = 0x10000 + ((c - 0xD800) << 10) + (text[++i] - 0xDC00);
        if (c < 0x80) {
            out += char(c);
        } else if (c < 0x800) {
            out += char(0xC0 | (c >> 6));
            out += char(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            out += char(0xE0 | (c >> 12));
            out += char(0x80 | ((c >> 6) & 0x3F));
            out += char(0x80 | (c & 0x3F));
        } else {
            out += char(0xF0 | (c >> 18));
            out += char(0x80 | ((c >> 12) & 0x3F));
            out += char(0x80 | ((c >> 6) & 0x3F));
            out += char(0x80 | (c & 0x3F));
        }
    }
    return out;
}

// The UTF-16 position of a UTF-8 offset.
size_t utf16Offset(const std::string& text, size_t utf8)
{
    size_t units = 0;
    for (size_t i = 0; i < utf8 && i < text.size();) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        const size_t length = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        units += length == 4 ? 2 : 1;
        i += length;
    }
    return units;
}

} // namespace

// Reference line break opportunities, with the breakable white space lines
// also end after, measured on the test laptop: UTF-16 positions a break may
// come before.
TEST(LineBreaksTest, BreakOpportunitiesMatchReference)
{
    struct Case
    {
        std::u16string text;
        std::vector<size_t> breaks;
    };
    const std::vector<Case> cases = {
        {u"Import the configuration file your VPN provider gives you, such as an OpenVPN .ovpn or a WireGuard .conf file.",
         {7, 11, 25, 30, 35, 39, 48, 54, 59, 64, 67, 70, 78, 84, 87, 89, 99, 105, 110}},
        {u"Visit https://github.com/LingmoOS/settings/issues?q=is%3Aopen+label%3Abug for open bugs.",
         {6, 14, 25, 36, 45, 52, 76, 80, 85, 90}},
        {u"Files in /usr/share/icons/hicolor/48x48/apps/ and ~/.local/share/icons stay.",
         {6, 9, 10, 14, 20, 26, 34, 40, 46, 50, 59, 65, 71, 76}},
        {u"Prices: $1,234.56, (USD) 12%, -5.5, +3; ranges 10-20 and 3/4.",
         {8, 19, 25, 30, 36, 40, 47, 53, 57, 61}},
        {u"He said “hello, world!” and ‘goodbye’; then (quietly) left… really?",
         {3, 8, 16, 24, 28, 39, 44, 54, 60, 67}},
        {u"A well-known, state-of-the-art e-mail client—fast—for everyone.",
         {2, 7, 14, 20, 23, 27, 31, 33, 38, 44, 45, 49, 50, 54, 63}},
        {u"中文的换行规则：标点符号「不能」出现在行首，例如，句号。逗号、顿号！",
         {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 16, 17, 18, 19, 20, 22, 23, 25, 26, 28, 29, 31, 32, 34}},
        {u"日本語のテキストはどこでも改行できますが、「括弧」や句読点は例外です。",
         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 23, 25, 26, 27, 28, 29, 30, 31, 32, 33, 35}},
        {u"한국어 문장은 띄어쓰기 단위로 줄을 바꿉니다. 예를 들어 이 문장처럼.",
         {4, 8, 13, 17, 20, 26, 29, 32, 34, 39}},
        {u"Emoji 👍🏽 and flags 🇩🇪🇫🇷 with ZWJ 👩‍💻 sequences.",
         {6, 11, 15, 21, 25, 30, 35, 39, 45, 55}},
        {u"Version 6.8.2 (build 2024-11-27T10:00) on x86_64; 1.5× scale, 150 %.",
         {8, 14, 21, 39, 42, 50, 55, 62, 66, 68}},
        {u"Mixed 中文English混排 text, with 数字123和English words.",
         {6, 7, 8, 15, 16, 18, 24, 29, 30, 31, 34, 35, 43, 49}},
        {u"Tabs\tand  double  spaces, trailing.  ",
         {5, 10, 18, 26, 37}},
        {u"key=value;another_key=some/long/path.conf,third-item",
         {27, 32, 48, 52}},
    };
    for (const Case& test : cases) {
        const std::vector<uint8_t> breaks = lineBreaks(test.text);
        std::vector<size_t> positions;
        for (size_t i = 1; i < breaks.size(); ++i) {
            if (breaks[i])
                positions.push_back(i);
        }
        EXPECT_EQ(positions, test.breaks) << std::string(test.text.begin(), test.text.end());
    }
}

// A line ends after any space, even before the "." of a file extension;
// ICU's LB13 would keep "WireGuard .conf" together and move "WireGuard" down.
TEST(LineBreaksTest, WrappedTextBreaksAfterAnySpace)
{
    const std::string head = "Import the configuration file your VPN provider gives you, such as an "
                             "OpenVPN .ovpn or a WireGuard";
    const Label firstLine(head);
    const Label upToExtension(head + " .conf");
    Window window;
    auto root = std::make_unique<View>();
    TextArea* area = root->addChild<TextArea>(head + " .conf file.");
    // Wide enough for "WireGuard", too narrow for ".conf", inside the padding.
    const double width = (firstLine.sizeHint().width + upToExtension.sizeHint().width) / 2 + 12;
    area->setGeometry({0, 0, width, 200});
    window.setContentView(std::move(root));
    window.resize({800, 300});
    window.show();
    window.grab();
    area->setFocus();
    TestInput::setKeyboardFocus(window, true);
    area->setCursorPosition(0);
    TestInput::typeKey(window, key::End);
    EXPECT_GE(area->cursorPosition(), head.size());
    EXPECT_LE(area->cursorPosition(), head.size() + 1);
}

// Reference lines of the same sentences in Noto Sans at 14
// px, WrapAtWordBoundaryOrAnywhere, at several widths and at scales 1 and
// 1.5: the UTF-16 position each line starts at. Left out, as the widths
// differ: tab stops, colour emoji advances, and at scale 1 the hinted
// advances of curly quotes, a pixel apart.
TEST(LineBreaksTest, WrapsAtReferenceWidths)
{
    using Wraps = std::vector<std::pair<double, std::vector<size_t>>>;
    struct Case
    {
        std::u16string text;
        Wraps atScale1;
        Wraps atScale15;
    };
    const std::vector<Case> cases = {
        {u"Import the configuration file your VPN provider gives you, such as an OpenVPN .ovpn or a WireGuard .conf file.",
         {{60, {0, 7, 11, 19, 30, 35, 39, 48, 54, 59, 67, 70, 76, 84, 89, 96, 105}}, {100, {0, 11, 25, 39, 54, 67, 78, 89, 99}}, {160, {0, 11, 35, 59, 78, 99}}, {240, {0, 35, 70, 99}}, {590, {0, 87}}},
         {{60, {0, 7, 11, 19, 30, 35, 39, 48, 54, 59, 67, 70, 76, 84, 89, 96, 105}}, {100, {0, 11, 25, 39, 54, 67, 78, 89, 99}}, {160, {0, 11, 35, 59, 78, 99}}, {240, {0, 35, 70, 99}}, {590, {0, 89}}}},
        {u"Visit https://github.com/LingmoOS/settings/issues?q=is%3Aopen+label%3Abug for open bugs.",
         {{60, {0, 6, 14, 22, 25, 33, 36, 45, 52, 59, 66, 73, 80, 85}}, {100, {0, 14, 25, 36, 45, 52, 64, 76}}, {160, {0, 14, 36, 52, 71}}, {240, {0, 36, 52, 80}}, {590, {0, 80}}},
         {{60, {0, 6, 14, 22, 25, 33, 36, 45, 52, 59, 66, 73, 80, 85}}, {100, {0, 14, 25, 36, 45, 52, 64, 76}}, {160, {0, 14, 36, 52, 72}}, {240, {0, 36, 52, 80}}, {590, {0, 85}}}},
        {u"Files in /usr/share/icons/hicolor/48x48/apps/ and ~/.local/share/icons stay.",
         {{60, {0, 10, 14, 20, 26, 34, 40, 46, 50, 59, 65, 71}}, {100, {0, 14, 26, 40, 50, 65}}, {160, {0, 20, 40, 59}}, {240, {0, 34, 65}}, {590, {0}}},
         {{60, {0, 10, 14, 20, 26, 34, 40, 46, 50, 59, 65, 71}}, {100, {0, 14, 26, 40, 50, 65}}, {160, {0, 20, 40, 59}}, {240, {0, 34, 65}}, {590, {0}}}},
        {u"Prices: $1,234.56, (USD) 12%, -5.5, +3; ranges 10-20 and 3/4.",
         {{60, {0, 8, 16, 25, 30, 40, 47, 53}}, {100, {0, 8, 19, 30, 40, 53}}, {160, {0, 25, 47}}, {240, {0, 36}}, {590, {0}}},
         {{60, {0, 8, 16, 25, 30, 40, 47, 53}}, {100, {0, 8, 19, 30, 40, 53}}, {160, {0, 25, 47}}, {240, {0, 36}}, {590, {0}}}},
        {u"He said “hello, world!” and ‘goodbye’; then (quietly) left… really?",
         {{60, {0, 8, 16, 24, 28, 36, 44, 54, 60}}, {160, {0, 24, 44}}, {240, {0, 28, 60}}, {590, {0}}},
         {{60, {0, 8, 16, 24, 28, 36, 44, 54, 60}}, {100, {0, 16, 28, 44, 60}}, {160, {0, 24, 44}}, {240, {0, 28}}, {590, {0}}}},
        {u"A well-known, state-of-the-art e-mail client—fast—for everyone.",
         {{60, {0, 7, 14, 23, 31, 38, 45, 54, 61}}, {100, {0, 14, 27, 38, 50}}, {160, {0, 23, 45}}, {240, {0, 33}}, {590, {0}}},
         {{60, {0, 7, 14, 23, 31, 38, 45, 54, 61}}, {100, {0, 14, 31, 45, 54}}, {160, {0, 23, 45}}, {240, {0, 38}}, {590, {0}}}},
        {u"中文的换行规则：标点符号「不能」出现在行首，例如，句号。逗号、顿号！",
         {{60, {0, 4, 8, 12, 16, 20, 23, 26, 29, 32}}, {100, {0, 6, 12, 19, 26, 32}}, {160, {0, 11, 22, 32}}, {240, {0, 17}}, {590, {0}}},
         {{60, {0, 4, 8, 12, 16, 20, 23, 26, 29, 32}}, {100, {0, 6, 12, 19, 26, 32}}, {160, {0, 11, 22, 32}}, {240, {0, 17}}, {590, {0}}}},
        {u"日本語のテキストはどこでも改行できますが、「括弧」や句読点は例外です。",
         {{60, {0, 4, 8, 12, 16, 19, 23, 27, 31}}, {100, {0, 7, 14, 21, 28}}, {160, {0, 11, 21, 32}}, {240, {0, 17, 33}}, {590, {0}}},
         {{60, {0, 4, 8, 12, 16, 19, 23, 27, 31}}, {100, {0, 7, 14, 21, 28}}, {160, {0, 11, 21, 32}}, {240, {0, 17, 33}}, {590, {0}}}},
        {u"한국어 문장은 띄어쓰기 단위로 줄을 바꿉니다. 예를 들어 이 문장처럼.",
         {{60, {0, 4, 8, 13, 17, 20, 26, 32, 34}}, {100, {0, 8, 17, 26, 34}}, {160, {0, 13, 29}}, {240, {0, 20}}, {590, {0}}},
         {{60, {0, 4, 8, 13, 17, 20, 26, 32, 34}}, {100, {0, 8, 17, 26, 34}}, {160, {0, 13, 29}}, {240, {0, 20}}, {590, {0}}}},
        {u"Version 6.8.2 (build 2024-11-27T10:00) on x86_64; 1.5× scale, 150 %.",
         {{60, {0, 8, 14, 21, 29, 37, 42, 50, 55, 62}}, {100, {0, 14, 21, 34, 50, 66}}, {160, {0, 21, 42, 66}}, {240, {0, 21, 55}}, {590, {0}}},
         {{60, {0, 8, 14, 21, 29, 37, 42, 50, 55, 62}}, {100, {0, 14, 21, 34, 50, 66}}, {160, {0, 21, 42, 66}}, {240, {0, 21, 55}}, {590, {0}}}},
        {u"Mixed 中文English混排 text, with 数字123和English words.",
         {{60, {0, 7, 8, 15, 18, 24, 30, 35, 43}}, {100, {0, 8, 18, 31, 43}}, {160, {0, 18, 35}}, {240, {0, 30}}, {590, {0}}},
         {{60, {0, 7, 8, 15, 18, 24, 31, 35, 43}}, {100, {0, 8, 18, 31, 43}}, {160, {0, 18, 35}}, {240, {0, 30}}, {590, {0}}}},
        {u"key=value;another_key=some/long/path.conf,third-item",
         {{60, {0, 8, 16, 24, 27, 32, 40, 48}}, {100, {0, 14, 26, 32, 46}}, {160, {0, 22, 32}}, {240, {0, 32}}, {590, {0}}},
         {{60, {0, 8, 16, 24, 27, 32, 40, 48}}, {100, {0, 14, 26, 32, 47}}, {160, {0, 22, 32}}, {240, {0, 32}}, {590, {0}}}},
    };
    const Theme saved = Theme::current();
    Theme theme = saved;
    theme.fontFamily = "Noto Sans";
    Theme::setCurrent(theme);
    for (const Case& test : cases) {
        const std::string text = toUtf8(test.text);
        for (const double scale : {1.0, 1.5}) {
            for (const auto& [width, expected] : scale == 1 ? test.atScale1 : test.atScale15) {
                Window window;
                window.setScale(scale);
                auto root = std::make_unique<View>();
                TextArea* area = root->addChild<TextArea>(text);
                area->setFontSize(14);
                area->setWrapMode(TextArea::WrapMode::Wrap);
                area->setPadding({});
                area->setGeometry({0, 0, width, 400});
                window.setContentView(std::move(root));
                window.resize({700, 450});
                window.show();
                window.grab();
                area->setFocus();
                TestInput::setKeyboardFocus(window, true);
                area->setCursorPosition(0);
                std::vector<size_t> starts = {0};
                for (size_t line = 0; line < 40; ++line) {
                    TestInput::typeKey(window, key::Down);
                    TestInput::typeKey(window, key::Home);
                    const size_t start = utf16Offset(text, area->cursorPosition());
                    if (start == starts.back())
                        break;
                    starts.push_back(start);
                }
                EXPECT_EQ(starts, expected) << text << " at " << width << ", scale " << scale;
            }
        }
    }
    Theme::setCurrent(saved);
}
