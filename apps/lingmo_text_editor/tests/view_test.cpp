// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "document.h"
#include "editorview.h"

#include "base/elapsed_timer.h"
#include "base/file.h"
#include "test_support.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/window.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>

using namespace Lingmo::ui;

namespace
{
struct Fixture
{
    explicit Fixture(SizeF size = {900, 560}, double scale = 1)
    {
        window.setScale(scale);
        auto content = std::make_unique<EditorView>();
        view = content.get();
        view->setDocument(&document);
        view->setFont(FontSpec{"Noto Mono", 9});
        view->setSidePadding(14);
        window.setContentView(std::move(content));
        window.show();
        TestInput::configure(window, int(size.width), int(size.height), {true});
        TestInput::setKeyboardFocus(window, true);
        view->setFocus();
        window.grab();
    }

    Document document;
    Window window;
    EditorView *view = nullptr;
};
}

static std::string writeFile(const std::string &name, std::string_view data)
{
    const std::string path = TestSupport::directory() + "/" + name;
    Lingmo::base::file::writeAtomically(path, data);
    return path;
}

static double ms(const Lingmo::base::ElapsedTimer &timer)
{
    return std::chrono::duration<double, std::milli>(timer.elapsed()).count();
}

TEST(ViewTest, TypesAndMoves)
{
    Fixture f;
    TestInput::typeText(f.window, "hello world");
    EXPECT_EQ(f.document.text(), "hello world");
    TestInput::typeKey(f.window, key::Home);
    TestInput::typeKey(f.window, key::Right, ShiftModifier);
    EXPECT_EQ(f.document.selectedText(), "h");
    TestInput::typeKey(f.window, key::Return);
    EXPECT_EQ(f.document.text(), "\nello world");
    TestInput::typeKey(f.window, key::letter('z'), ControlModifier);
    EXPECT_EQ(f.document.text(), "hello world");
    f.window.grab().savePng(TestSupport::directory() + "/../text-editor-typed.png");
}

TEST(ViewTest, ClicksPlaceTheCursor)
{
    Fixture f;
    f.document.setText("first line\nsecond line");
    f.window.grab();
    const double lineHeight = f.view->lineHeight();
    // The middle of the second line, past its end.
    TestInput::click(f.window, {800, 6 + lineHeight * 1.5});
    EXPECT_EQ(f.document.cursor(), (TextPosition{1, 11}));
    TestInput::click(f.window, {f.view->gutterWidth() + 1, 6 + lineHeight * 0.5});
    EXPECT_EQ(f.document.cursor(), (TextPosition{0, 0}));
}

TEST(ViewTest, LargeFilePerformance)
{
    // About 24 MB of code-like lines.
    std::string data;
    data.reserve(25 * 1024 * 1024);
    for (int i = 0; i < 400000; ++i)
        data += "    if (value_" + std::to_string(i) + " > limit) { total += compute(value, \"text\"); } // note\n";
    const std::string path = writeFile("large.c", data);

    Fixture f;
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    f.document.load(path);
    ASSERT_TRUE(f.document.waitForLoaded(60000));
    const double loadMs = ms(timer);
    f.window.grab();
    const double firstFrameMs = ms(timer);
    ASSERT_EQ(f.document.lineCount(), 400001);

    f.document.setCursor({200000, 10});
    f.view->ensureCursorVisible();
    f.window.grab();
    timer.start();
    for (int i = 0; i < 50; ++i) {
        TestInput::typeText(f.window, "a");
        f.window.grab();
    }
    const double typingMs = ms(timer);

    timer.start();
    for (int i = 0; i < 200; ++i) {
        f.document.insertText("b");
        f.view->updateNow();
    }
    const double editMs = ms(timer);

    timer.start();
    for (int i = 0; i < 20; ++i) {
        TestInput::typeKey(f.window, key::Return);
        f.window.grab();
    }
    const double enterMs = ms(timer);

    timer.start();
    for (int i = 0; i < 50; ++i) {
        f.view->setContentPosition(f.view->contentX(), f.view->contentY() + f.view->geometry().height);
        f.window.grab();
    }
    const double scrollMs = ms(timer);

    timer.start();
    f.view->scrollToLine(399000);
    f.window.grab();
    const double jumpMs = ms(timer);
    f.window.grab().savePng(TestSupport::directory() + "/../text-editor-large.png");

    std::printf("large file: load %.0f ms, first frame %.0f ms, typing %.1f ms/char (%.2f ms without rendering), "
                "enter %.1f ms, scroll %.1f ms/screen, jump %.0f ms\n",
                loadMs, firstFrameMs, typingMs / 50, editMs / 200, enterMs / 20, scrollMs / 50, jumpMs);
    EXPECT_LT(typingMs / 50, 50);
    EXPECT_LT(scrollMs / 50, 100);
}

TEST(ViewTest, LongLinePerformance)
{
    // A minified file: one 4 MB line.
    std::string data;
    for (int i = 0; i < 200000; ++i)
        data += "a" + std::to_string(i) + "=b+c;";
    const std::string path = writeFile("min.js", data);
    Fixture f;
    f.document.load(path);
    ASSERT_TRUE(f.document.waitForLoaded(60000));
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    f.window.grab();
    const double firstMs = ms(timer);
    f.document.setCursor({0, 1000});
    f.view->ensureCursorVisible();
    f.window.grab();
    timer.start();
    for (int i = 0; i < 20; ++i) {
        TestInput::typeText(f.window, "x");
        f.window.grab();
    }
    const double typingMs = ms(timer);
    timer.start();
    for (int i = 0; i < 50; ++i) {
        f.document.insertText("y");
        f.view->updateNow();
    }
    const double editMs = ms(timer);
    std::printf("long line: first frame %.0f ms, typing %.1f ms/char (%.2f ms without rendering)\n", firstMs,
                typingMs / 20, editMs / 50);
    EXPECT_LT(typingMs / 20, 200);
}
