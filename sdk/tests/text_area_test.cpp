// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "ui/clipboard.h"
#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/text_area.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

struct Hosted
{
    Window window;
    TextArea* area = nullptr;

    explicit Hosted(double scale = 1.5, double width = 200)
    {
        auto root = std::make_unique<Column>();
        area = root->addChild<TextArea>();
        area->setLayoutHints({.preferredWidth = width});
        window.setContentView(std::move(root));
        window.resize({300, 300});
        window.setScale(scale);
        window.show();
        window.grab();
        TestInput::setKeyboardFocus(window, true);
        area->setFocus();
    }
};

} // namespace

TEST(TextAreaTest, TypesLinesAndGrows)
{
    Hosted hosted;
    const double oneLine = hosted.area->geometry().height;
    TestInput::typeText(hosted.window, "First line");
    TestInput::typeKey(hosted.window, key::Return);
    TestInput::typeText(hosted.window, "Second");
    EXPECT_EQ(hosted.area->text(), "First line\nSecond");
    hosted.window.grab();
    EXPECT_GT(hosted.area->geometry().height, oneLine);
    // The padding of 6 around two lines.
    EXPECT_DOUBLE_EQ(hosted.area->geometry().height, 2 * (oneLine - 12) + 12);
}

TEST(TextAreaTest, UpAndDownKeepTheColumn)
{
    Hosted hosted;
    hosted.area->setText("abcdef\nabcdef\nab");
    hosted.window.grab();
    hosted.area->setCursorPosition(4);
    TestInput::typeKey(hosted.window, key::Down);
    EXPECT_EQ(hosted.area->cursorPosition(), 11u);
    TestInput::typeKey(hosted.window, key::Down);
    // The last line is shorter; the caret goes to its end and comes back.
    EXPECT_EQ(hosted.area->cursorPosition(), 16u);
    TestInput::typeKey(hosted.window, key::Up);
    EXPECT_EQ(hosted.area->cursorPosition(), 11u);
    TestInput::typeKey(hosted.window, key::Up, ShiftModifier);
    EXPECT_EQ(hosted.area->selectedText(), "ef\nabcd");
}

TEST(TextAreaTest, WrapsAnywhereWhenAsked)
{
    Hosted hosted(1.5, 120);
    const std::string command = "/usr/lib/x86_64-linux-gnu/libexec/some-long-helper --option=value";
    hosted.area->setText(command);
    hosted.window.grab();
    const double words = hosted.area->geometry().height;
    hosted.area->setWrapMode(TextArea::WrapMode::WrapAnywhere);
    hosted.window.grab();
    const double anywhere = hosted.area->geometry().height;
    hosted.area->setWrapMode(TextArea::WrapMode::NoWrap);
    hosted.window.grab();
    const double single = hosted.area->geometry().height;
    EXPECT_LT(single, words);
    EXPECT_LE(anywhere, words);
    EXPECT_EQ(hosted.area->text(), command);
}

TEST(TextAreaTest, ReadOnlyTextSelectsButDoesNotChange)
{
    Hosted hosted;
    hosted.area->setText("Read only");
    hosted.area->setReadOnly(true);
    hosted.window.grab();
    TestInput::typeText(hosted.window, "x");
    TestInput::typeKey(hosted.window, key::BackSpace);
    EXPECT_EQ(hosted.area->text(), "Read only");
    TestInput::typeKey(hosted.window, key::letter('a'), ControlModifier);
    EXPECT_EQ(hosted.area->selectedText(), "Read only");
    // A double click selects a word.
    const RectF box = hosted.area->geometry();
    TestInput::click(hosted.window, {box.x + 10, box.y + 10});
    TestInput::click(hosted.window, {box.x + 10, box.y + 10});
    EXPECT_EQ(hosted.area->selectedText(), "Read");
}

TEST(TextAreaTest, InputMethodsComposeInPlace)
{
    Hosted hosted;
    TestInput::typeText(hosted.window, "ab");
    TestInput::inputMethod(hosted.window, {.preedit = "xy", .preeditCursorBegin = 2});
    EXPECT_EQ(hosted.area->preedit(), "xy");
    EXPECT_EQ(hosted.area->text(), "ab");
    hosted.window.grab();
    TestInput::inputMethod(hosted.window, {.commit = "XY"});
    EXPECT_EQ(hosted.area->text(), "abXY");
    EXPECT_TRUE(hosted.area->preedit().empty());
}

// Keys that delete nothing and composing alone change no text, and say so.
TEST(TextAreaTest, ReportsOnlyEditsThatChangeTheText)
{
    Hosted hosted;
    int changes = 0;
    hosted.area->onTextChanged = [&changes] { ++changes; };
    TestInput::typeKey(hosted.window, key::BackSpace);
    TestInput::typeKey(hosted.window, key::Delete);
    TestInput::inputMethod(hosted.window, {.preedit = "xy", .preeditCursorBegin = 2});
    EXPECT_EQ(changes, 0);
    TestInput::inputMethod(hosted.window, {.commit = "XY"});
    EXPECT_EQ(hosted.area->text(), "XY");
    EXPECT_EQ(changes, 1);
}

TEST(TextAreaTest, MiddleClickPastesThePrimarySelectionWhereItLands)
{
    Hosted hosted(1.0);
    hosted.area->setText("one two");
    hosted.window.grab();
    // Select "one" by double click: it becomes the primary selection.
    TestInput::click(hosted.window, {8, 10});
    TestInput::click(hosted.window, {8, 10});
    std::string selected;
    Clipboard::requestText([&](std::string text) { selected = std::move(text); }, ClipboardMode::Selection);
    EXPECT_EQ(selected, "one");
    // A middle click at the start inserts it there.
    TestInput::click(hosted.window, {1, 10}, MouseButton::Middle);
    EXPECT_EQ(hosted.area->text(), "oneone two");
}

TEST(TextAreaTest, CentresTextForAnInlineEditor)
{
    Window window;
    auto view = std::make_unique<TextArea>("name.txt");
    TextArea* area = view.get();
    area->setFrameVisible(false);
    area->setPadding(Margins::all(4));
    area->setWrapMode(TextArea::WrapMode::Wrap);
    area->setHorizontalAlignment(TextArea::HorizontalAlignment::Center);
    area->setVerticallyCentered(true);
    area->setFont({}, 13, 600);
    area->setTextColor(Color::rgb(0x000000));
    area->setSelectionColors(Color::rgb(0x2050E0), Color::rgb(0xFFFFFF));
    area->setPersistentSelection(true);
    window.resize({200, 60});
    window.setBackgroundColor(Color::rgb(0xFFFFFF));
    window.setContentView(std::move(view));
    window.show();
    area->selectAll();
    const Image image = window.grab();
    // The selection's blue, centred both ways, though the area has no focus.
    int left = image.width();
    int right = -1;
    int top = image.height();
    int bottom = -1;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const Color c = image.pixel(x, y);
            if (c.b > 180 && c.r < 80) {
                left = std::min(left, x);
                right = std::max(right, x);
                top = std::min(top, y);
                bottom = std::max(bottom, y);
            }
        }
    }
    ASSERT_GE(right, 0);
    EXPECT_NEAR((left + right) / 2.0, 100, 2);
    EXPECT_NEAR((top + bottom) / 2.0, 30, 2);
    // Grows with the text: the height for a width includes the padding.
    EXPECT_GT(area->heightForWidth(40), area->heightForWidth(200));
}
