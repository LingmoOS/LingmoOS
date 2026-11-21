// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <functional>

#include "base/event_loop.h"
#include "test_views.h"
#include "ui/application.h"
#include "ui/drop_down_popup.h"
#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/popup_glass.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// A month's worth of content: 252 wide, its height from its width, counting
// its clicks and keys.
class Content : public View
{
public:
    SizeF sizeHint() const override { return {251.4, 0}; }
    double heightForWidth(double width) const override { return width / 2 + 0.3; }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return focusable; }

    bool focusable = false;
    bool takesEscape = false;
    int clicks = 0;
    int keys = 0;
    std::function<void()> onClicked;

protected:
    void pointerRelease(const PointerEvent&) override
    {
        ++clicks;
        if (onClicked)
            onClicked();
    }
    bool keyPress(const KeyEvent& event) override
    {
        ++keys;
        return takesEscape && event.key == key::Escape;
    }
};

struct Fixture
{
    Fixture()
    {
        auto column = std::make_unique<Column>();
        column->setMargins(Margins::all(10));
        anchor = column->addChild<test::Box>(SizeF{150, 35});
        window.setScreenSize({1280, 800});
        window.resize({400, 300});
        window.setContentView(std::move(column));
        window.show();
        window.grab();
        content = popup.setContent<Content>();
        popup.onClosed = [this] { ++closed; };
    }

    Window window;
    test::Box* anchor;
    DropDownPopup popup;
    Content* content;
    int closed = 0;
};

} // namespace

// Sized to the content, rounded up as MenuPopupWindow sizes its window.
TEST(DropDownPopupTest, TakesTheContentsSize)
{
    Fixture f;
    f.popup.popupFor(f.anchor);
    ASSERT_TRUE(f.popup.isOpen());
    Window* window = f.popup.window();
    ASSERT_NE(window, nullptr);
    window->grab();
    EXPECT_EQ(window->size(), (SizeF{252, 127}));
    EXPECT_EQ(f.content->geometry(), (RectF{0, 0, 252, 127}));
    EXPECT_EQ(f.content->window(), window);
}

// Escape and the compositor's dismissal close it; the content stays the
// same view between openings.
TEST(DropDownPopupTest, EscapeAndDismissalClose)
{
    Fixture f;
    f.popup.popupFor(f.anchor);
    TestInput::typeKey(*f.popup.window(), key::Escape);
    EXPECT_FALSE(f.popup.isOpen());
    EXPECT_EQ(f.closed, 1);
    EXPECT_EQ(f.popup.contentView(), f.content);
    EXPECT_EQ(f.content->window(), nullptr);

    f.popup.popupFor(f.anchor);
    ASSERT_TRUE(f.popup.isOpen());
    EXPECT_EQ(f.popup.contentView(), f.content);
    f.popup.window()->onCloseRequested();
    EXPECT_FALSE(f.popup.isOpen());
    EXPECT_EQ(f.closed, 2);
    // Closing a closed popup tells nobody.
    f.popup.close();
    EXPECT_EQ(f.closed, 2);
}

// A content that takes the focus sees the keys first, Escape included.
TEST(DropDownPopupTest, ContentSeesKeysFirst)
{
    Fixture f;
    f.content->focusable = true;
    f.content->takesEscape = true;
    f.popup.popupFor(f.anchor);
    TestInput::setKeyboardFocus(*f.popup.window(), true);
    TestInput::typeKey(*f.popup.window(), key::Escape);
    EXPECT_TRUE(f.popup.isOpen());
    EXPECT_EQ(f.content->keys, 1);
    f.content->takesEscape = false;
    TestInput::typeKey(*f.popup.window(), key::Escape);
    EXPECT_FALSE(f.popup.isOpen());
    EXPECT_EQ(f.content->keys, 2);
}

// The content can close the popup from its own click; it survives, and
// the window goes once the loop runs.
TEST(DropDownPopupTest, ContentClosesFromItsOwnClick)
{
    Fixture f;
    f.content->onClicked = [&f] { f.popup.close(); };
    f.popup.popupFor(f.anchor);
    Window* window = f.popup.window();
    window->grab();
    TestInput::click(*window, {100, 50});
    EXPECT_EQ(f.content->clicks, 1);
    EXPECT_FALSE(f.popup.isOpen());
    EXPECT_EQ(f.closed, 1);
    EXPECT_EQ(f.popup.contentView(), f.content);
    Application::instance()->loop().dispatch();
    EXPECT_EQ(f.content->window(), nullptr);
}

// Without the compositor's blur, the second background, opaque, in the
// window radius; no glass is asked for.
TEST(DropDownPopupTest, OpaqueWithoutGlass)
{
    Fixture f;
    f.popup.popupFor(f.anchor);
    const Image image = f.popup.window()->grab();
    const Color expected = Theme::current().secondBackground;
    const Color centre = image.pixel(image.width() / 2, image.height() / 2);
    EXPECT_EQ(centre.a, 255);
    EXPECT_NEAR(centre.r, expected.r, 1);
    // Rounded: the corner pixel is clear.
    EXPECT_EQ(image.pixel(0, 0).a, 0);
    EXPECT_FALSE(TestInput::backgroundGlass(*f.popup.window()));
}

// With the compositor's glass: the popup glass and no fill of its own.
TEST(DropDownPopupTest, PopupGlassWithoutFill)
{
    TestInput::setGlassAvailable(true);
    Fixture f;
    f.popup.popupFor(f.anchor);
    const Image image = f.popup.window()->grab();
    EXPECT_EQ(image.pixel(image.width() / 2, image.height() / 2).a, 0);
    EXPECT_EQ(TestInput::backgroundGlass(*f.popup.window()), popupGlass(Theme::current().darkMode));
    TestInput::setGlassAvailable(false);
}

TEST(DropDownPopupTest, ResizesToItsContent)
{
    Fixture f;
    f.popup.popupFor(f.anchor);
    auto other = std::make_unique<test::Box>(SizeF{100, 40});
    test::Box* box = other.get();
    f.popup.setContentView(std::move(other));
    EXPECT_EQ(f.popup.contentView(), box);
    EXPECT_EQ(f.popup.window()->size(), (SizeF{100, 40}));
}
