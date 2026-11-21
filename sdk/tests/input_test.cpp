// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <vector>

#include "test_views.h"
#include "ui/button.h"
#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo::UI;

namespace {

// Records what reaches it.
class Probe : public test::Box
{
public:
    explicit Probe(bool focusable = true)
        : Box({50, 20})
        , m_focusable(focusable)
    {
    }

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return m_focusable; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::vector<std::string> events;
    bool hovered = false;
    bool handlesKeys = true;
    bool handlesScroll = false;

protected:
    void pointerPress(const PointerEvent& event) override
    {
        events.push_back("press " + std::to_string(event.clickCount));
    }
    void pointerRelease(const PointerEvent&) override { events.push_back("release"); }
    void pointerEnter() override { hovered = true; }
    void pointerLeave() override { hovered = false; }
    void focusIn(FocusReason) override { events.push_back("focus in"); }
    void focusOut() override { events.push_back("focus out"); }
    bool keyPress(const KeyEvent& event) override
    {
        events.push_back("key " + event.text);
        return handlesKeys;
    }
    bool scroll(const ScrollEvent& event) override
    {
        events.push_back("scroll " + std::to_string(int(event.deltaY)));
        return handlesScroll;
    }

private:
    bool m_focusable;
};

struct Fixture
{
    Fixture()
    {
        auto content = std::make_unique<Column>();
        content->setSpacing(10);
        first = content->addChild<Probe>();
        second = content->addChild<Probe>(false);
        third = content->addChild<Probe>();
        window.resize({100, 100});
        window.setContentView(std::move(content));
        window.show();
        window.grab();
        TestInput::setKeyboardFocus(window, true);
    }

    Window window;
    Probe* first;
    Probe* second;
    Probe* third;
};

} // namespace

TEST(InputTest, PressFocusesAndCountsClicks)
{
    Fixture f;
    TestInput::click(f.window, {10, 10});
    TestInput::click(f.window, {11, 10});
    // Focus first, so the press already sees it.
    EXPECT_EQ(f.first->events,
              (std::vector<std::string>{"focus in", "press 1", "release", "press 2", "release"}));
    EXPECT_TRUE(f.first->hasFocus());
    EXPECT_FALSE(f.first->hasVisibleFocus());
}

TEST(InputTest, PressOutsideDoesNotCountAsDoubleClick)
{
    Fixture f;
    TestInput::click(f.window, {10, 5});
    TestInput::click(f.window, {10, 15});
    EXPECT_EQ(f.first->events.back(), "release");
    EXPECT_EQ(f.first->events[f.first->events.size() - 2], "press 1");
}

TEST(InputTest, TabSkipsViewsThatTakeNoFocus)
{
    Fixture f;
    f.first->handlesKeys = false;
    f.third->handlesKeys = false;
    TestInput::typeKey(f.window, key::Tab);
    EXPECT_TRUE(f.first->hasFocus());
    EXPECT_TRUE(f.first->hasVisibleFocus());
    TestInput::typeKey(f.window, key::Tab);
    EXPECT_TRUE(f.third->hasFocus());
    TestInput::typeKey(f.window, key::Tab, ShiftModifier);
    EXPECT_TRUE(f.first->hasFocus());
    // Disabled views drop out of the chain.
    f.third->setEnabled(false);
    TestInput::typeKey(f.window, key::Tab);
    EXPECT_TRUE(f.first->hasFocus());
    // So do click-focus-only ones, which a press still focuses.
    f.third->setEnabled(true);
    f.third->setClickFocusOnly(true);
    TestInput::typeKey(f.window, key::Tab);
    EXPECT_TRUE(f.first->hasFocus());
    TestInput::click(f.window, {10, f.third->geometry().y + 2});
    EXPECT_TRUE(f.third->hasFocus());
}

TEST(InputTest, UnhandledKeysReachShortcuts)
{
    Fixture f;
    int fired = 0;
    f.window.addShortcut(ControlModifier, key::letter('s'), [&fired] { ++fired; });
    f.first->setFocus();
    TestInput::typeKey(f.window, key::letter('s'), ControlModifier);
    EXPECT_EQ(fired, 0);
    f.first->handlesKeys = false;
    TestInput::typeKey(f.window, key::letter('s'), ControlModifier);
    EXPECT_EQ(fired, 1);
    TestInput::typeKey(f.window, key::letter('s'), ControlModifier | ShiftModifier);
    EXPECT_EQ(fired, 1);
}

TEST(InputTest, ScrollBubblesUntilHandled)
{
    Fixture f;
    ScrollEvent event;
    event.deltaY = 12;
    TestInput::scroll(f.window, {10, 35}, event);
    EXPECT_EQ(f.second->events, (std::vector<std::string>{"scroll 12"}));
}

TEST(InputTest, RemovedViewsLoseFocusAndPointer)
{
    Fixture f;
    TestInput::press(f.window, {10, 70});
    EXPECT_TRUE(f.third->hasFocus());
    auto* column = f.window.contentView();
    std::unique_ptr<View> removed = column->takeChild(f.third);
    EXPECT_EQ(f.window.focusView(), nullptr);
    // The release finds no view to deliver to.
    TestInput::release(f.window, {10, 70});
}

TEST(InputTest, ButtonClicksFromPointerAndKeyboard)
{
    Window window;
    auto content = std::make_unique<Column>();
    Button* button = content->addChild<Button>("Apply");
    int clicks = 0;
    button->onClicked = [&clicks] { ++clicks; };
    window.resize({200, 100});
    window.setContentView(std::move(content));
    window.show();
    window.grab();

    TestInput::click(window, {10, 10});
    EXPECT_EQ(clicks, 1);
    // Released outside, a press is no click.
    TestInput::press(window, {10, 10});
    TestInput::release(window, {150, 80});
    EXPECT_EQ(clicks, 1);

    TestInput::typeKey(window, key::Space);
    TestInput::typeKey(window, key::Return);
    EXPECT_EQ(clicks, 3);

    button->setEnabled(false);
    TestInput::click(window, {10, 10});
    EXPECT_EQ(clicks, 3);
}

TEST(InputTest, LayoutDoesNotHoverAPointerThatLeft)
{
    Fixture fixture;
    TestInput::movePointer(fixture.window, {10, 10});
    EXPECT_TRUE(fixture.first->hovered);
    TestInput::leavePointer(fixture.window);
    fixture.window.requestLayout();
    fixture.window.grab();
    EXPECT_FALSE(fixture.first->hovered);
}

namespace {

class HoverProbe : public View
{
public:
    HoverProbe() { setTracksHover(true); }

    int changes = 0;
    int moves = 0;
    PointF last;

protected:
    void hoverChanged() override { ++changes; }
    void hoverMove(const PointerEvent& event) override
    {
        ++moves;
        last = event.position;
    }
};

} // namespace

// A view stays hovered while the pointer is over a child of it, and hears every
// move.
TEST(InputTest, HoverTracksTheViewAndItsChildren)
{
    Window window;
    auto root = std::make_unique<View>();
    HoverProbe* outer = root->addChild<HoverProbe>();
    outer->setGeometry({10, 10, 100, 60});
    HoverProbe* inner = outer->addChild<HoverProbe>();
    inner->setGeometry({20, 20, 30, 20});
    window.setContentView(std::move(root));
    window.resize({200, 100});
    window.show();
    window.grab();

    TestInput::movePointer(window, {15, 15});
    EXPECT_TRUE(outer->isHovered());
    EXPECT_FALSE(inner->isHovered());
    TestInput::movePointer(window, {35, 35});
    EXPECT_TRUE(outer->isHovered());
    EXPECT_TRUE(inner->isHovered());
    EXPECT_EQ(inner->last, (PointF{5, 5}));
    TestInput::movePointer(window, {150, 50});
    EXPECT_FALSE(outer->isHovered());
    EXPECT_FALSE(inner->isHovered());
    EXPECT_EQ(outer->changes, 2);
    EXPECT_EQ(inner->changes, 2);
    // Entering counts as a move: outer at (15, 15), then (35, 35).
    EXPECT_EQ(outer->moves, 2);

    // A layout moving the view under a still pointer tells it where it is.
    TestInput::movePointer(window, {35, 35});
    const int moves = inner->moves;
    inner->setGeometry({22, 20, 30, 20});
    window.grab();
    EXPECT_EQ(inner->moves, moves + 1);
    EXPECT_EQ(inner->last, (PointF{3, 5}));

    TestInput::leavePointer(window);
    EXPECT_FALSE(outer->isHovered());
    EXPECT_FALSE(inner->isHovered());
}

// Disabling a view clears its hover, and the pointer over it again counts once
// it is enabled.
TEST(InputTest, DisablingAViewEndsItsHover)
{
    Fixture fixture;
    const PointF over = fixture.first->mapToWindow({5, 5});
    TestInput::movePointer(fixture.window, over);
    EXPECT_TRUE(fixture.first->hovered);
    fixture.first->setEnabled(false);
    EXPECT_FALSE(fixture.first->hovered);

    // Re-enabled with the pointer elsewhere: no hover.
    TestInput::movePointer(fixture.window, {90, 99});
    fixture.first->setEnabled(true);
    fixture.window.grab();
    EXPECT_FALSE(fixture.first->hovered);

    // Re-enabled under a still pointer: hovered again after a frame.
    TestInput::movePointer(fixture.window, over);
    fixture.first->setEnabled(false);
    fixture.first->setEnabled(true);
    fixture.window.grab();
    EXPECT_TRUE(fixture.first->hovered);
}
