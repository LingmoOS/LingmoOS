// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <memory>
#include <poll.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "base/event_loop.h"
#include "base/timer.h"
#include "ui/animation.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/view.h"

using namespace Lingmo::UI;
using namespace std::chrono_literals;

extern Lingmo::Base::EventLoop* g_hostLoop;

static constexpr uint64_t kStart = 1000000000;

static uint64_t at(std::chrono::milliseconds offset)
{
    return kStart + uint64_t(std::chrono::nanoseconds(offset).count());
}

namespace {

// A rect of one colour at a logical place.
class Patch : public View
{
public:
    explicit Patch(Color color)
        : m_color(color)
    {
    }

    void setColor(Color color)
    {
        m_color = color;
        requestPaint();
    }

protected:
    void paint(Painter& painter) override { painter.fillRect(localRect(), m_color); }

private:
    Color m_color;
};

// Lays its children out at the geometry they were given.
class Free : public View
{
};

} // namespace

// RGBA bytes at a device pixel.
static Color pixelAt(const EmbeddedWindow::Frame& frame, int x, int y)
{
    const uint8_t* pixel = frame.pixels + size_t(y) * frame.stride + size_t(x) * 4;
    return {pixel[0], pixel[1], pixel[2], pixel[3]};
}

static bool contains(const std::vector<Rect>& rects, int x, int y)
{
    for (const Rect& rect : rects) {
        if (x >= rect.x && y >= rect.y && x < rect.x + rect.width && y < rect.y + rect.height)
            return true;
    }
    return false;
}

TEST(EmbeddedTest, NeedsNoSessionBus)
{
    EXPECT_EQ(Application::instance()->platform(), Application::Platform::Embedded);
    EXPECT_EQ(Application::instance()->sessionBus(), nullptr);
    EXPECT_FALSE(Application::instance()->hasConnection());
}

TEST(EmbeddedTest, RendersPremultipliedRgbaAtTheScale)
{
    for (const double scale : {1.0, 1.25, 1.5, 1.75, 2.0}) {
        EmbeddedWindow window;
        auto content = std::make_unique<Free>();
        Patch* patch = content->addChild<Patch>(Color{255, 0, 0, 128});
        patch->setGeometry({10, 10, 20, 20});
        window.setContentView(std::move(content));
        window.setScale(scale);
        window.setSize({101, 51});

        const EmbeddedWindow::Frame& frame = window.render(at(0ms));
        ASSERT_NE(frame.pixels, nullptr);
        EXPECT_EQ(frame.width, int(std::lround(101 * scale)));
        EXPECT_EQ(frame.height, int(std::lround(51 * scale)));
        EXPECT_EQ(frame.stride, size_t(frame.width) * 4);
        // Premultiplied red at half alpha, over nothing.
        const Color inside = pixelAt(frame, int(20 * scale), int(20 * scale));
        EXPECT_EQ(inside.r, 128);
        EXPECT_EQ(inside.g, 0);
        EXPECT_EQ(inside.b, 0);
        EXPECT_EQ(inside.a, 128);
        EXPECT_EQ(pixelAt(frame, 1, 1).a, 0);
        // The first frame is new everywhere.
        ASSERT_EQ(frame.damage.size(), 1u);
        EXPECT_EQ(frame.damage[0].width, frame.width);
        EXPECT_EQ(frame.damage[0].height, frame.height);
    }
}

TEST(EmbeddedTest, DamageCoversOnlyWhatChanged)
{
    EmbeddedWindow window;
    auto content = std::make_unique<Free>();
    Patch* left = content->addChild<Patch>(Color::rgb(0x0000ff));
    left->setGeometry({0, 0, 10, 10});
    Patch* right = content->addChild<Patch>(Color::rgb(0x00ff00));
    right->setGeometry({50, 0, 10, 10});
    window.setContentView(std::move(content));
    window.setScale(1.5);
    window.setSize({100, 20});
    window.render(at(0ms));
    EXPECT_FALSE(window.needsRender());

    // Nothing changed: nothing to upload.
    EXPECT_TRUE(window.render(at(16ms)).damage.empty());

    int requests = 0;
    window.onRenderRequested = [&requests] { ++requests; };
    right->setColor(Color::rgb(0xff0000));
    right->requestPaint();
    EXPECT_EQ(requests, 1);
    EXPECT_TRUE(window.needsRender());

    const EmbeddedWindow::Frame& frame = window.render(at(33ms));
    EXPECT_TRUE(contains(frame.damage, 80, 5));
    EXPECT_FALSE(contains(frame.damage, 5, 5));
    EXPECT_EQ(pixelAt(frame, 80, 5).r, 255);
    // The rest of the buffer still holds the last frame.
    EXPECT_EQ(pixelAt(frame, 5, 5).b, 255);
    EXPECT_FALSE(window.needsRender());
}

TEST(EmbeddedTest, AsksOnceUntilRendered)
{
    EmbeddedWindow window;
    auto content = std::make_unique<Free>();
    Patch* patch = content->addChild<Patch>(Color::rgb(0));
    patch->setGeometry({0, 0, 10, 10});
    window.setContentView(std::move(content));
    window.setSize({20, 20});
    window.render(at(0ms));

    int requests = 0;
    window.onRenderRequested = [&requests] { ++requests; };
    patch->requestPaint();
    patch->requestPaint();
    window.requestLayout();
    EXPECT_EQ(requests, 1);
    window.render(at(16ms));
    patch->requestPaint();
    EXPECT_EQ(requests, 2);
}

TEST(EmbeddedTest, ScaleAndAppearanceRepaint)
{
    EmbeddedWindow window;
    window.setContentView(std::make_unique<Free>());
    window.setSize({40, 30});
    window.render(at(0ms));

    int requests = 0;
    window.onRenderRequested = [&requests] { ++requests; };
    window.setScale(1.75);
    EXPECT_EQ(requests, 1);
    const EmbeddedWindow::Frame& frame = window.render(at(16ms));
    EXPECT_EQ(frame.width, 70);
    EXPECT_EQ(frame.height, 53);
    ASSERT_EQ(frame.damage.size(), 1u);
    EXPECT_EQ(frame.damage[0].width, 70);

    const Theme previous = Theme::current();
    Theme dark = Theme::dark();
    dark.highlight = accent::green;
    Theme::setCurrent(dark);
    EXPECT_EQ(requests, 2);
    EXPECT_EQ(window.render(at(33ms)).damage.size(), 1u);
    Theme::setCurrent(previous);
}

TEST(EmbeddedTest, AnimationsFollowTheHostsFrames)
{
    EmbeddedWindow window;
    window.setContentView(std::make_unique<Free>());
    window.setSize({10, 10});
    window.render(at(0ms));

    int requests = 0;
    window.onRenderRequested = [&requests] { ++requests; };
    Animation animation(&window);
    animation.setDuration(200ms);
    animation.setEasing(Animation::Easing::Linear);
    double value = -1;
    animation.onUpdate = [&value](double v) { value = v; };
    animation.start();
    EXPECT_EQ(requests, 1);
    EXPECT_TRUE(window.needsRender());

    // The first frame anchors the start; each render asks for the next.
    window.render(at(100ms));
    EXPECT_DOUBLE_EQ(value, 0);
    EXPECT_EQ(requests, 2);
    window.render(at(150ms));
    EXPECT_DOUBLE_EQ(value, 0.25);
    EXPECT_EQ(requests, 3);
    window.render(at(300ms));
    EXPECT_DOUBLE_EQ(value, 1);
    EXPECT_FALSE(animation.isRunning());
    // Finished: no more frames wanted.
    EXPECT_EQ(requests, 3);
    EXPECT_FALSE(window.needsRender());
}

TEST(EmbeddedTest, BeginningAFrameRunsItsAnimationsBeforeTheHostSizesIt)
{
    EmbeddedWindow window;
    window.setContentView(std::make_unique<Free>());
    window.setSize({10, 10});
    window.render(at(0ms));

    Animation animation(&window);
    animation.setDuration(200ms);
    animation.setEasing(Animation::Easing::Linear);
    int updates = 0;
    animation.onUpdate = [&](double v) {
        ++updates;
        window.resize({10, 10 + 20 * v});
    };
    animation.start();
    window.render(at(100ms));
    ASSERT_EQ(updates, 1);

    // The animation grows the window before the host reads its size, and
    // the render of that frame only paints it.
    window.beginFrame(at(200ms));
    EXPECT_EQ(updates, 2);
    EXPECT_EQ(window.pixelHeight(), 20);
    const EmbeddedWindow::Frame& frame = window.render(at(200ms));
    EXPECT_EQ(updates, 2);
    EXPECT_EQ(frame.height, 20);

    // A render at another time than the begun frame runs its own.
    window.beginFrame(at(250ms));
    EXPECT_EQ(updates, 3);
    window.render(at(300ms));
    EXPECT_EQ(updates, 4);
    EXPECT_EQ(window.render(at(300ms)).height, 30);
}

TEST(EmbeddedTest, PointerAndCursorShape)
{
    EmbeddedWindow window;
    auto content = std::make_unique<Free>();
    Button* button = content->addChild<Button>("Go");
    button->setGeometry({10, 10, 80, 30});
    TextField* field = content->addChild<TextField>();
    field->setGeometry({10, 50, 200, 35});
    window.setContentView(std::move(content));
    window.setScale(1.5);
    window.setSize({220, 100});
    window.render(at(0ms));

    std::vector<CursorShape> shapes;
    window.onCursorShapeChanged = [&shapes](CursorShape shape) { shapes.push_back(shape); };
    int clicks = 0;
    button->onClicked = [&clicks] { ++clicks; };

    window.pointerEnter({50, 25});
    window.pointerButton(MouseButton::Left, true);
    window.pointerButton(MouseButton::Left, false);
    EXPECT_EQ(clicks, 1);
    EXPECT_EQ(window.cursorShape(), CursorShape::Default);

    window.pointerMove({50, 60});
    EXPECT_EQ(window.cursorShape(), CursorShape::Text);
    window.pointerMove({215, 95});
    EXPECT_EQ(window.cursorShape(), CursorShape::Default);
    window.pointerMove({50, 60});
    window.pointerLeave();
    EXPECT_EQ(window.cursorShape(), CursorShape::Default);
    EXPECT_EQ(shapes, (std::vector<CursorShape>{CursorShape::Text, CursorShape::Default, CursorShape::Text,
                                                CursorShape::Default}));
}

TEST(EmbeddedTest, KeysAsTheHostsXkbGivesThem)
{
    EmbeddedWindow window;
    auto content = std::make_unique<Free>();
    TextField* field = content->addChild<TextField>();
    field->setGeometry({10, 10, 200, 35});
    window.setContentView(std::move(content));
    window.setSize({220, 60});
    window.setActive(true);
    window.setKeyboardFocus(true);
    EXPECT_TRUE(window.isActive());
    field->setFocus();

    const auto type = [&window](uint32_t keysym, std::string text, uint32_t modifiers = NoModifier) {
        KeyEvent event;
        event.key = keysym;
        event.text = std::move(text);
        event.modifiers = modifiers;
        window.key(event, true);
        window.key(event, false);
    };
    type(XKB_KEY_A, "A", ShiftModifier);
    type(XKB_KEY_b, "b");
    EXPECT_EQ(field->text(), "Ab");
    // Ctrl+A as xkb gives it: the control character, which is no text.
    type(XKB_KEY_a, "\x01", ControlModifier);
    EXPECT_TRUE(field->hasSelection());
    type(XKB_KEY_BackSpace, "\b");
    EXPECT_EQ(field->text(), "");

    // A repeat from the host.
    KeyEvent repeat;
    repeat.key = XKB_KEY_x;
    repeat.text = "x";
    window.key(repeat, true);
    repeat.repeat = true;
    window.key(repeat, true);
    window.key(repeat, false);
    EXPECT_EQ(field->text(), "xx");
}

static bool waitReadable(int fd, int timeoutMs)
{
    pollfd entry{fd, POLLIN, 0};
    return poll(&entry, 1, timeoutMs) == 1 && (entry.revents & POLLIN);
}

TEST(EmbeddedTest, TimersRunThroughTheHostsLoop)
{
    Lingmo::Base::EventLoop& loop = Application::instance()->loop();
    EmbeddedWindow window;
    auto content = std::make_unique<Free>();
    TextField* field = content->addChild<TextField>();
    field->setGeometry({10, 10, 200, 35});
    window.setContentView(std::move(content));
    window.setSize({220, 60});
    window.setKeyboardFocus(true);
    field->setFocus();
    window.render(at(0ms));

    // The caret blinks on a timer, which asks the host for a frame.
    int requests = 0;
    window.onRenderRequested = [&requests] { ++requests; };
    for (int i = 0; i < 20 && !requests; ++i) {
        if (waitReadable(loop.fd(), 100))
            loop.dispatch();
    }
    EXPECT_EQ(requests, 1);

    Lingmo::Base::Timer timer;
    bool fired = false;
    timer.onTimeout = [&fired] { fired = true; };
    timer.setSingleShot(true);
    timer.start(1ms);
    for (int i = 0; i < 20 && !fired; ++i) {
        if (waitReadable(loop.fd(), 100))
            loop.dispatch();
    }
    EXPECT_TRUE(fired);
}

TEST(EmbeddedTest, SeveralWindowsShareTheApplication)
{
    EmbeddedWindow first;
    EmbeddedWindow second;
    for (EmbeddedWindow* window : {&first, &second}) {
        auto content = std::make_unique<Free>();
        content->addChild<Patch>(Color::rgb(0xffffff))->setGeometry({0, 0, 4, 4});
        window->setContentView(std::move(content));
    }
    first.setScale(1.25);
    first.setSize({8, 8});
    second.setScale(2);
    second.setSize({8, 8});
    EXPECT_EQ(first.render(at(0ms)).width, 10);
    EXPECT_EQ(second.render(at(0ms)).width, 16);
    EXPECT_EQ(pixelAt(second.frame(), 7, 7).a, 255);
    EXPECT_EQ(pixelAt(second.frame(), 8, 8).a, 0);
}

TEST(EmbeddedApplicationTest, UsesTheHostLoop)
{
    ASSERT_NE(g_hostLoop, nullptr);
    EXPECT_EQ(&Application::instance()->loop(), g_hostLoop);
    EXPECT_EQ(Lingmo::Base::EventLoop::current(), g_hostLoop);
}
