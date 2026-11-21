// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The window overview against the window manager: entering and leaving by the
// shortcut and by three fingers, the input it takes and leaves, clicking,
// closing and dragging windows, and the desk bar. No DRM, no GL.

#include <linux/input-event-codes.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <vector>

#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "display/display_manager.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/desk_switch_node.h"
#include "scene/overview_node.h"
#include "scene/scene.h"
#include "scene/window_animation.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/desks/desk.h"
#include "wm/desks/desks_controller.h"
#include "wm/dock_layout_manager.h"
#include "wm/gestures/wm_gesture_handler.h"
#include "wm/overview/overview_controller.h"
#include "wm/stacking.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;
using namespace Lingmo::Wm;
using State = OverviewController::State;

namespace {

base::RectF lerp(const base::RectF &a, const base::RectF &b, double t)
{
    return base::RectF(a.x() + (b.x() - a.x()) * t, a.y() + (b.y() - a.y()) * t,
                  a.width() + (b.width() - a.width()) * t,
                  a.height() + (b.height() - a.height()) * t);
}

const base::Rect kOutput(0, 0, 1000, 800);

class FrameTexture : public render::Texture
{
public:
    explicit FrameTexture(base::Size size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class FrameRenderer : public testsupport::FakeRenderer
{
public:
    bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &rect,
                         const std::function<void()> &paint) override
    {
        const size_t before = calls.size();
        paint();
        if (!texture || texture->size() != rect.size())
            texture = std::make_unique<FrameTexture>(rect.size());
        // What the texture holds, for reading the draw order of cached windows.
        for (size_t i = before; i < calls.size(); ++i) {
            if (calls[i].texture)
                contents[texture.get()] = calls[i].texture;
        }
        return true;
    }

    std::map<const render::Texture *, const render::Texture *> contents;

    void drawBackgroundBlur(const base::Rect &bounds, int, const base::Rect &,
                            const render::GlassParams &, const ui::Path &, uint64_t) override
    {
        blurs.push_back(bounds);
    }

    std::vector<base::Rect> blurs;
};

class OverviewTest : public testing::Test
{
protected:
    int64_t now = 1000;
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    FrameRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    WindowManager manager {&compositor, &seat};
    DesksController *desks = manager.desks();
    OverviewController *overview = manager.overview();
    scene::OverviewNode *node = static_cast<scene::OverviewNode *>(
            surfaceContainer->addChild(std::make_unique<scene::OverviewNode>([this] {
                return now;
            })));
    scene::OverviewDeskBarNode *bar = static_cast<scene::OverviewDeskBarNode *>(
            surfaceContainer->addChild(std::make_unique<scene::OverviewDeskBarNode>(node)));

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setDeskOverlays({node, bar});
        overview->setNodes(node, bar);
        overview->setNext(&manager);
        input.setBounds(kOutput.size());
        input.setEventSink(overview);
    }

    void TearDown() override { seat.shutdown(); }

    compositor::Surface *makeSurface(const base::Size &size)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        auto buffer = pool->createBuffer(size, 0, size.width() * 4, render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(renderer);
        return surface;
    }

    Window *makeWindow(const base::Rect &bounds)
    {
        Window *window = manager.createWindow(makeSurface(bounds.size()),
                                              {.requestedSize = bounds.size(),
                                               .applicationId = "app"});
        window->setBounds(bounds);
        manager.activation()->activate(window);
        return window;
    }

    Window *active() { return manager.activation()->activeWindow(); }

    Desk *desk(int index) const { return desks->deskAt(index); }

    void flush()
    {
        core::mainLoop()->dispatch();
    }

    void paint()
    {
        renderer.calls.clear();
        renderer.setOutputContext({"out", kOutput, kOutput.size(), 1, 0, 0, {}});
        renderer.frameTime = double(now);
        surfaceContainer->render(renderer);
        flush();
    }

    // Draws until the overview has come to rest.
    void settle()
    {
        paint();
        now += 1500;
        paint();
        paint();
    }

    static input::Timestamp at(int64_t ms)
    {
        return input::Timestamp(std::chrono::milliseconds(ms));
    }

    void moveTo(const base::PointF &location) { input.onPointerMotionLogical(at(now), location); }

    void click(const base::PointF &location)
    {
        moveTo(location);
        input.onPointerButton(at(now), BTN_LEFT, true);
        input.onPointerButton(at(now), BTN_LEFT, false);
    }

    void key(uint32_t code)
    {
        input.onKey(at(now), code, true);
        input.onKey(at(now), code, false);
    }

    int indexOf(const Window *window) const
    {
        const auto &items = overview->items();
        for (size_t index = 0; index < items.size(); ++index) {
            if (items[index].window == window)
                return int(index);
        }
        return -1;
    }

    // Whether the last paint drew the window's client content anywhere.
    bool drewContentOf(const Window *window) const
    {
        const render::Texture *texture = window->rootSurface()->node()->texture();
        return texture
                && std::any_of(renderer.calls.begin(), renderer.calls.end(),
                               [texture](const auto &call) {
                                   return call.texture == texture;
                               });
    }

    base::RectF placeOf(const Window *window) const
    {
        const int index = indexOf(window);
        return index >= 0 ? overview->items()[index].bounds : base::RectF();
    }

    void show()
    {
        ASSERT_TRUE(overview->enter());
        settle();
        ASSERT_EQ(State::Shown, overview->state());
    }
};

} // namespace

TEST_F(OverviewTest, TheShortcutBringsItInAndTakesItAway)
{
    Window *first = makeWindow(base::Rect(100, 100, 500, 400));
    Window *second = makeWindow(base::Rect(300, 200, 500, 400));
    overview->toggle();
    EXPECT_EQ(State::Entering, overview->state());
    EXPECT_TRUE(node->isActive());
    settle();
    EXPECT_EQ(State::Shown, overview->state());

    // Both, apart, below the desk bar and inside the work area, smaller than
    // they are.
    ASSERT_EQ(2u, overview->items().size());
    const base::RectF a = placeOf(first);
    const base::RectF b = placeOf(second);
    EXPECT_FALSE(a.intersects(b));
    ASSERT_EQ(1u, overview->bars().size());
    for (const base::RectF &place : {a, b}) {
        EXPECT_GE(place.top(), overview->bars()[0].bounds.bottom());
        EXPECT_TRUE(base::RectF(kOutput).contains(place));
        EXPECT_LT(place.width(), 500);
    }
    // Where they were relative to each other.
    EXPECT_LT(a.left(), b.left());

    overview->toggle();
    EXPECT_EQ(State::Exiting, overview->state());
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_FALSE(node->isActive());
    EXPECT_TRUE(overview->items().empty());
    // Nothing changed hands.
    EXPECT_EQ(second, active());
}

TEST_F(OverviewTest, TheShortcutDoesNothingWhileAWindowIsDragged)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(300, 110)));
    overview->toggle();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_FALSE(node->isActive());

    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Success);
    overview->toggle();
    EXPECT_EQ(State::Entering, overview->state());
}

TEST_F(OverviewTest, ThePointerAndTheKeysAreTheOverviewsWhileItIsUp)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    EXPECT_FALSE(overview->onPointerMotion(at(now), base::PointF(300, 300)));
    EXPECT_FALSE(overview->onKey(at(now), KEY_A, true));
    EXPECT_FALSE(overview->onKey(at(now), KEY_A, false));

    show();
    // The window gets neither motion nor keys.
    EXPECT_TRUE(overview->onPointerMotion(at(now), placeOf(window).center()));
    EXPECT_EQ(0, overview->hovered());
    EXPECT_TRUE(overview->onKey(at(now), KEY_A, true));
    EXPECT_TRUE(overview->onKey(at(now), KEY_A, false));

    overview->exit();
    // On its way out the windows have their input again.
    EXPECT_FALSE(overview->onPointerMotion(at(now), base::PointF(300, 300)));
    EXPECT_FALSE(overview->onKey(at(now), KEY_A, true));
    EXPECT_FALSE(overview->onKey(at(now), KEY_A, false));
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
}

TEST_F(OverviewTest, ThePanelsAndTheDockKeepTheirInput)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    Window *dock = makeWindow(base::Rect(300, 740, 400, 60));
    dock->setRole(WindowRole::Dock);
    dock->setFocusPolicy(FocusPolicy::NoFocus);
    manager.activation()->activate(window);
    show();
    ASSERT_EQ(1u, overview->items().size());

    EXPECT_EQ(OverviewController::Hit::Type::Panel, overview->hitTest(base::PointF(500, 770)).type);
    EXPECT_FALSE(overview->onPointerMotion(at(now), base::PointF(500, 770)));
    EXPECT_FALSE(overview->onPointerButton(at(now), base::PointF(500, 770), BTN_LEFT, true));
    EXPECT_FALSE(overview->onPointerButton(at(now), base::PointF(500, 770), BTN_LEFT, false));
    EXPECT_EQ(State::Shown, overview->state());
    // Off the dock, the overview has it again.
    EXPECT_TRUE(overview->onPointerMotion(at(now), base::PointF(500, 600)));
}

TEST_F(OverviewTest, ThePanelsGiveWayAsItComesIn)
{
    DockLayoutManager layout {&manager};
    Window *panel = makeWindow(base::Rect(0, 0, 1000, 30));
    panel->setRole(WindowRole::Panel);
    panel->setFocusPolicy(FocusPolicy::NoFocus);
    makeWindow(base::Rect(100, 100, 500, 400));
    layout.advance(double(now));
    layout.advance(double(now + DockLayoutManager::kSlideDurationMs));
    ASSERT_EQ(base::Point(), panel->presentationOffset());

    // Off the top edge with the overview's progress.
    ASSERT_TRUE(overview->enter());
    paint();
    layout.advance(double(now) + scene::OverviewNode::kTransitionDurationMs / 2);
    EXPECT_GT(0, panel->presentationOffset().y());
    EXPECT_LT(-30, panel->presentationOffset().y());
    EXPECT_FALSE(panel->isPresentationHidden());
    settle();
    layout.advance(double(now));
    EXPECT_EQ(base::Point(0, -30), panel->presentationOffset());
    EXPECT_TRUE(panel->isPresentationHidden());
    // The desk bar takes the edge it left, and the pointer there is the overview's.
    EXPECT_EQ(kOutput.top(), overview->bars().front().bounds.top());
    // Its previews keep half the margin they had under the panel.
    const base::Rect work = manager.outputAreas().front().workArea;
    EXPECT_EQ(std::round((work.top() + OverviewController::kBarPadding) / 2.0),
              overview->bars().front().desks.front().bounds.top());
    EXPECT_NE(OverviewController::Hit::Type::Panel, overview->hitTest(base::PointF(500, 10)).type);

    overview->toggle();
    settle();
    layout.advance(double(now));
    EXPECT_EQ(base::Point(), panel->presentationOffset());
    EXPECT_FALSE(panel->isPresentationHidden());
}

TEST_F(OverviewTest, AClickedWindowIsActivatedAndLandsOnTop)
{
    Window *first = makeWindow(base::Rect(100, 100, 500, 400));
    Window *second = makeWindow(base::Rect(300, 200, 500, 400));
    show();
    moveTo(placeOf(first).center());
    EXPECT_EQ(indexOf(first), overview->hovered());
    click(placeOf(first).center());
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(first, active());
    // Drawn last on its way back.
    EXPECT_EQ(first, overview->items().back().window);
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_EQ(first, manager.windows().back());
    EXPECT_NE(second, active());
}

TEST_F(OverviewTest, EscapeAndAClickOnNothingLeaveAsItWas)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    show();
    key(KEY_ESC);
    EXPECT_EQ(State::Exiting, overview->state());
    settle();
    EXPECT_EQ(State::Hidden, overview->state());

    show();
    click(base::PointF(10, 790));
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(window, active());
    settle();

    // The desk bar's empty space too.
    show();
    ASSERT_EQ(OverviewController::Hit::Type::Bar, overview->hitTest(base::PointF(10, 10)).type);
    click(base::PointF(10, 10));
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(window, active());
}

TEST_F(OverviewTest, TheKeyboardPicksAWindow)
{
    Window *left = makeWindow(base::Rect(0, 200, 400, 300));
    Window *right = makeWindow(base::Rect(600, 200, 400, 300));
    show();
    // The active window first, then the one to its left.
    key(KEY_RIGHT);
    EXPECT_EQ(indexOf(right), overview->selected());
    key(KEY_LEFT);
    EXPECT_EQ(indexOf(left), overview->selected());
    key(KEY_ENTER);
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(left, active());
}

TEST_F(OverviewTest, APressStaysWithItsWindowWhenTheOthersChange)
{
    Window *bottom = makeWindow(base::Rect(0, 100, 300, 300));
    Window *middle = makeWindow(base::Rect(350, 100, 300, 300));
    Window *top = makeWindow(base::Rect(700, 100, 300, 300));
    show();
    ASSERT_EQ(3u, overview->items().size());

    // The window under the press closes: letting go picks nothing.
    moveTo(placeOf(middle).center());
    input.onPointerButton(at(now), BTN_LEFT, true);
    manager.destroyWindow(middle);
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(top, active());

    // One below it closes: letting go still picks it.
    moveTo(placeOf(top).center());
    input.onPointerButton(at(now), BTN_LEFT, true);
    manager.destroyWindow(bottom);
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(top, active());
}

TEST_F(OverviewTest, AKeyHeldSinceBeforeGoesUpAtTheWindow)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    EXPECT_FALSE(overview->onKey(at(now), KEY_LEFTMETA, true));
    show();
    EXPECT_TRUE(overview->onKey(at(now), KEY_A, true));
    // The window saw it go down, so it sees it go up; the overview's own key
    // stays the overview's.
    EXPECT_FALSE(overview->onKey(at(now), KEY_LEFTMETA, false));
    EXPECT_TRUE(overview->onKey(at(now), KEY_A, false));
}

TEST_F(OverviewTest, ActivationFromElsewhereEndsIt)
{
    Window *first = makeWindow(base::Rect(100, 100, 500, 400));
    makeWindow(base::Rect(300, 200, 500, 400));
    show();
    // The dock asking for a window.
    manager.activation()->activate(first);
    EXPECT_EQ(State::Exiting, overview->state());
}

TEST_F(OverviewTest, MinimizedWindowsAreLeftOut)
{
    Window *shown = makeWindow(base::Rect(100, 100, 500, 400));
    Window *minimized = makeWindow(base::Rect(300, 200, 500, 400));
    // Minimized from the click on, its animation still running.
    minimized->minimize();
    ASSERT_TRUE(minimized->state()->isMinimized());
    const auto *animation = dynamic_cast<const scene::WindowAnimation *>(minimized->node());
    ASSERT_TRUE(animation && animation->type() == scene::WindowAnimation::Type::Minimize
                && animation->isRunning());
    show();
    ASSERT_EQ(1u, overview->items().size());
    EXPECT_EQ(shown, overview->items()[0].window);
    paint();
    EXPECT_FALSE(drewContentOf(minimized));
}

TEST_F(OverviewTest, ClickingADeskSwitchesThereAndTakesItAway)
{
    Window *first = makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    desks->activateDesk(desk(1), DesksSwitchSource::DeskSwitchShortcut);
    Window *second = makeWindow(base::Rect(200, 100, 500, 400));
    desks->activateDesk(desk(0), DesksSwitchSource::DeskSwitchShortcut);
    ASSERT_EQ(first, active());
    show();
    ASSERT_EQ(1u, overview->items().size());
    // With no frames to slide, it switches at once and leaves.
    click(overview->bars()[0].desks[1].bounds.center());
    EXPECT_EQ(desk(1), desks->activeDesk());
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_TRUE(node->covers());
    EXPECT_EQ(second, active());
}

TEST_F(OverviewTest, AClickedDeskSlidesInWhileTheDeskBarGoes)
{
    auto *switchNode = static_cast<scene::DeskSwitchNode *>(
            surfaceContainer->addChild(std::make_unique<scene::DeskSwitchNode>([this] {
                return now;
            })));
    manager.setDeskOverlays({node, switchNode, bar});
    desks->setSwitchNode(switchNode);
    int screenshots = 0;
    desks->setScreenshotter([this, &screenshots](const std::function<void(render::Renderer &)> &paint) {
        ++screenshots;
        paint(renderer);
        scene::DeskSwitchNode::Frames frames;
        frames["out"] = std::make_unique<FrameTexture>(kOutput.size());
        return frames;
    });
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    show();

    // The strip starts from the overview as it is shown, and the overview
    // heads out at once under it, drawing nothing there.
    click(overview->bars()[0].desks[1].bounds.center());
    EXPECT_EQ(1, screenshots);
    ASSERT_NE(nullptr, desks->animation());
    EXPECT_TRUE(switchNode->isActive());
    EXPECT_EQ(desk(1), desks->activeDesk());
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_TRUE(node->isActive());
    EXPECT_FALSE(node->covers());

    // The desk bar goes as the strip slides.
    paint();
    now += scene::OverviewNode::kTransitionDurationMs / 2;
    paint();
    EXPECT_TRUE(switchNode->isAnimating());
    const double halfway = node->progress(double(now));
    EXPECT_GT(halfway, 0);
    EXPECT_LT(halfway, 1);

    settle();
    settle();
    EXPECT_EQ(nullptr, desks->animation());
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_EQ(desk(1), desks->activeDesk());
    EXPECT_FALSE(node->isActive());
}

TEST_F(OverviewTest, ClickingTheDeskItIsOnTakesItAway)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    show();
    click(overview->bars()[0].desks[0].bounds.center());
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(window, active());
}

TEST_F(OverviewTest, AHoveredDeskIsNotRingedAndAPressedOneDimsWhileTheButtonIsOnIt)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    show();
    const auto other = [&] {
        return overview->bars()[0].desks[1];
    };
    const base::PointF on = other().bounds.center();
    moveTo(on);
    EXPECT_FALSE(other().highlighted);
    EXPECT_FALSE(other().pressed);
    EXPECT_TRUE(other().closeShown);

    input.onPointerButton(at(now), BTN_LEFT, true);
    EXPECT_TRUE(other().pressed);
    EXPECT_FALSE(other().highlighted);
    moveTo(on + base::PointF(0, 4));
    EXPECT_TRUE(other().pressed);
    // Further, it is dragged rather than pressed; let go where it was,
    // nothing is clicked or moved.
    moveTo(on + base::PointF(0, 40));
    EXPECT_FALSE(other().pressed);
    EXPECT_TRUE(other().dragged);
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_FALSE(other().pressed);
    EXPECT_FALSE(other().dragged);
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(State::Shown, overview->state());
}

TEST_F(OverviewTest, ALoneDeskPressedAndLeftDimsOnlyWhileTheButtonIsOnIt)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    show();
    const auto lone = [&] {
        return overview->bars()[0].desks[0];
    };
    const base::PointF on = lone().bounds.center();
    moveTo(on);
    input.onPointerButton(at(now), BTN_LEFT, true);
    EXPECT_TRUE(lone().pressed);
    // A lone desk has nowhere to go: off it the press would do nothing, and
    // it shows so.
    moveTo(on + base::PointF(0, 200));
    EXPECT_FALSE(lone().pressed);
    EXPECT_FALSE(lone().dragged);
    moveTo(on);
    EXPECT_TRUE(lone().pressed);
    moveTo(on + base::PointF(0, 200));
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_FALSE(lone().pressed);
    EXPECT_EQ(State::Shown, overview->state());
}

TEST_F(OverviewTest, ADeskDraggedAlongTheBarTakesItsNewPlaceAndTheNamesFollowTheOrder)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    Desk *first = desk(0);
    Desk *second = desk(1);
    Desk *third = desk(2);
    show();
    const auto slots = overview->bars()[0].desks;
    ASSERT_EQ(3u, slots.size());
    const auto desksShown = [&] {
        return overview->bars()[0].desks;
    };

    const base::PointF from = slots[0].bounds.center();
    moveTo(from);
    input.onPointerButton(at(now), BTN_LEFT, true);
    // Up and down does nothing; along the bar it follows the pointer.
    moveTo(from + base::PointF(30, 120));
    auto shown = desksShown();
    ASSERT_EQ(3u, shown.size());
    EXPECT_TRUE(shown[0].dragged);
    EXPECT_EQ(slots[0].bounds.translated(30, 0), shown[0].bounds);
    EXPECT_EQ("Desk 1", shown[0].name);

    // Past the second desk's middle, the second one makes way.
    const double step = slots[1].bounds.x() - slots[0].bounds.x();
    moveTo(from + base::PointF(step * 0.6, 0));
    shown = desksShown();
    EXPECT_EQ(uintptr_t(second), shown[0].id);
    EXPECT_EQ(slots[0].bounds, shown[0].bounds);
    EXPECT_EQ(uintptr_t(first), shown[1].id);
    EXPECT_TRUE(shown[1].dragged);

    // Held past the last place, it stops there.
    moveTo(from + base::PointF(step * 5, 0));
    shown = desksShown();
    EXPECT_EQ(uintptr_t(first), shown[2].id);
    EXPECT_EQ(slots[2].bounds, shown[2].bounds);
    input.onPointerButton(at(now), BTN_LEFT, false);

    EXPECT_EQ(second, desk(0));
    EXPECT_EQ(third, desk(1));
    EXPECT_EQ(first, desk(2));
    // Still up, on the same desk, which is now the last.
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(first, desks->activeDesk());
    shown = desksShown();
    ASSERT_EQ(3u, shown.size());
    for (size_t index = 0; index < shown.size(); ++index) {
        EXPECT_FALSE(shown[index].dragged);
        EXPECT_EQ(slots[index].bounds, shown[index].bounds);
        EXPECT_EQ("Desk " + std::to_string(index + 1), shown[index].name);
    }
    EXPECT_EQ(uintptr_t(first), shown[2].id);
    EXPECT_TRUE(shown[2].active);
}

TEST_F(OverviewTest, ADeskDroppedWhereItWasStaysAndIsNotClicked)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    Desk *second = desk(1);
    show();
    const base::PointF from = overview->bars()[0].desks[1].bounds.center();
    moveTo(from);
    input.onPointerButton(at(now), BTN_LEFT, true);
    moveTo(from + base::PointF(20, 0));
    moveTo(from);
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(second, desk(1));
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_FALSE(overview->bars()[0].desks[1].dragged);
}

TEST_F(OverviewTest, AWindowDraggedOntoADeskMovesThere)
{
    Window *moved = makeWindow(base::Rect(100, 100, 500, 400));
    Window *stays = makeWindow(base::Rect(300, 200, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    show();
    const base::PointF from = placeOf(moved).center();
    const base::PointF to = overview->bars()[0].desks[1].bounds.center();
    moveTo(from);
    input.onPointerButton(at(now), BTN_LEFT, true);
    for (int step = 1; step <= 10; ++step)
        moveTo(from + (to - from) * step / 10);
    EXPECT_TRUE(overview->bars()[0].desks[1].highlighted);
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(desk(1), desks->deskOf(moved));
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(State::Shown, overview->state());
    ASSERT_EQ(1u, overview->items().size());
    EXPECT_EQ(stays, overview->items()[0].window);
}

TEST_F(OverviewTest, AWindowLetGoElsewhereGoesBack)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    show();
    const base::RectF place = placeOf(window);
    moveTo(place.center());
    input.onPointerButton(at(now), BTN_LEFT, true);
    moveTo(place.center() + base::PointF(60, 40));
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(place, placeOf(window));
    EXPECT_EQ(desk(0), desks->deskOf(window));
}

TEST_F(OverviewTest, ThreeFingersUpFollowAndDownLeave)
{
    // Large enough that the overview shows it smaller.
    Window *window = makeWindow(base::Rect(0, 30, 1000, 740));
    WmGestureHandler gestures(desks, {}, overview);
    input.setGestureFilter(&gestures);
    const auto swipe = [this](double mm) {
        const base::PointF travel(0, mm / WmGestureHandler::kMmPerUnacceleratedUnit);
        input.onPointerGestureUpdate(at(now), input::GestureType::Swipe, travel * 5, travel, 1, 0);
    };

    // Down on the desk does nothing.
    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    swipe(10);
    EXPECT_EQ(State::Hidden, overview->state());
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);

    // A slowing hold, which the fingers show before they stop.
    const auto hold = [&](double direction) {
        for (int step = 0; step < 3; ++step) {
            now += 10;
            swipe(0.05 * direction);
        }
    };

    // Up, halfway and a bit, held, then let go: in.
    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    for (int step = 0; step < 6; ++step) {
        now += 10;
        swipe(-WmGestureHandler::kOverviewSwipeLengthMm * 0.1);
    }
    hold(-1);
    EXPECT_EQ(State::Entering, overview->state());
    const double progress = node->progress(double(now) + 5);
    EXPECT_NEAR(0.6, progress, 0.05);
    const base::RectF halfway = node->presentedRect(0, progress, double(now));
    EXPECT_LT(halfway.width(), window->bounds().width());
    EXPECT_GT(halfway.width(), placeOf(window).width());
    now += 200;
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    settle();
    EXPECT_EQ(State::Shown, overview->state());

    // Down a little, held, let go: it stays.
    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    for (int step = 0; step < 3; ++step) {
        now += 10;
        swipe(WmGestureHandler::kOverviewSwipeLengthMm * 0.1);
    }
    hold(1);
    now += 200;
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    settle();
    EXPECT_EQ(State::Shown, overview->state());

    // A flick down: out.
    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    for (int step = 0; step < 3; ++step) {
        now += 8;
        swipe(WmGestureHandler::kOverviewSwipeLengthMm * 0.1);
    }
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    EXPECT_EQ(State::Exiting, overview->state());
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
    input.setGestureFilter(nullptr);
}

TEST_F(OverviewTest, AFlickInOneEventBringsItIn)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    WmGestureHandler gestures(desks, {}, overview);
    input.setGestureFilter(&gestures);

    // A quick flick that libinput reports in a single event past the threshold.
    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    now += 8;
    const base::PointF travel(0, -6 / WmGestureHandler::kMmPerUnacceleratedUnit);
    input.onPointerGestureUpdate(at(now), input::GestureType::Swipe, travel * 5, travel, 1, 0);
    now += 9;
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    EXPECT_EQ(State::Entering, overview->state());
    settle();
    EXPECT_EQ(State::Shown, overview->state());
    input.setGestureFilter(nullptr);
}

TEST_F(OverviewTest, SlowFingersPastTheThresholdMoveItWithoutAJump)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    show();
    WmGestureHandler gestures(desks, {}, overview);
    input.setGestureFilter(&gestures);
    const double step = 0.5;
    const double length = WmGestureHandler::kOverviewSwipeLengthMm;

    input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
    double previous = 1;
    double travel = 0;
    while (travel < 4 * WmGestureHandler::kContinuousGestureMoveThresholdMm) {
        now += 10;
        travel += step;
        const base::PointF delta(0, step / WmGestureHandler::kMmPerUnacceleratedUnit);
        input.onPointerGestureUpdate(at(now), input::GestureType::Swipe, delta * 5, delta, 1, 0);
        const double progress = node->progress(double(now) + 5);
        // Moving from the first step past the threshold, never more than
        // twice as fast as the fingers.
        if (travel > WmGestureHandler::kContinuousGestureMoveThresholdMm)
            EXPECT_LT(progress, previous);
        EXPECT_LE(previous - progress, 2 * step / length + 1e-9);
        previous = progress;
    }
    // Under the fingers again.
    EXPECT_NEAR(1 - travel / length, previous, 1e-9);
    input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    settle();
    input.setGestureFilter(nullptr);
}

TEST_F(OverviewTest, EveryDeskPreviewShowsItsWindows)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    desks->activateDesk(desk(1), DesksSwitchSource::DeskSwitchShortcut);
    Window *there = makeWindow(base::Rect(200, 100, 500, 400));
    desks->activateDesk(desk(0), DesksSwitchSource::DeskSwitchShortcut);
    ASSERT_FALSE(there->isVisible());
    show();
    bar->invalidatePreviews();
    // One preview a frame: the active desk's, then the other.
    paint();
    paint();
    EXPECT_TRUE(drewContentOf(there));
}

TEST_F(OverviewTest, ADraggedWindowShrinksOnItsWayToTheBar)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    show();
    const base::RectF place = placeOf(window);
    const int index = indexOf(window);
    const auto dragged = [&] {
        return node->presentedRect(index, 1, double(now));
    };
    moveTo(place.center());
    input.onPointerButton(at(now), BTN_LEFT, true);
    // Level with or below where it sits, the size it has in the overview.
    moveTo(place.center() + base::PointF(40, 10));
    EXPECT_EQ(place.size(), dragged().size());
    EXPECT_TRUE(dragged().contains(place.center() + base::PointF(40, 10)));
    moveTo(place.center() + base::PointF(-40, 0));
    EXPECT_EQ(place.size(), dragged().size());

    // On the bar, the window at the expanded bar's share of the output's
    // height: 6 + 80 + 24 + 6 of 800.
    const auto bar = overview->bars()[0];
    const double expanded = 116;
    const base::PointF onBar = bar.desks[1].bounds.center();
    moveTo(onBar);
    EXPECT_NEAR(500 * expanded / 800, dragged().width(), 0.01);
    EXPECT_NEAR(400 * expanded / 800, dragged().height(), 0.01);
    EXPECT_TRUE(dragged().contains(onBar));

    // Halfway up from where it sits to the bar, halfway there.
    const base::PointF between(onBar.x(), (place.center().y() + bar.bounds.top() + expanded) / 2);
    moveTo(between);
    EXPECT_NEAR((place.width() + 500 * expanded / 800) / 2, dragged().width(), 0.01);
    input.onPointerButton(at(now), BTN_LEFT, false);
}

TEST_F(OverviewTest, GlassIsDrawnWhereTheWindowIsShown)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    compositor::Surface *surface = window->rootSurface();
    surface->setBackgroundBlur(base::Rect(0, 0, 100, 400), 10);
    surface->commit();
    compositor.updateTextures(renderer);
    show();
    renderer.blurs.clear();
    paint();
    const base::RectF place = placeOf(window);
    const double scale = place.width() / 500;
    ASSERT_EQ(2, renderer.blurs.size());
    EXPECT_EQ(base::RectF(place.topLeft(), base::SizeF(100, 400) * scale).toEnclosingRect(),
              renderer.blurs[0]);
    // The new-desk button is frosted over it.
    EXPECT_EQ(overview->bars()[0].newDesk.toEnclosingRect(), renderer.blurs[1]);
}

TEST_F(OverviewTest, ABlurredWindowIsNotDrawnAgainEachTimeItComesIn)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    compositor::Surface *surface = window->rootSurface();
    surface->setBackgroundBlur(base::Rect(0, 0, 100, 400), 10);
    surface->commit();
    compositor.updateTextures(renderer);
    paint();
    show();
    overview->exit();
    settle();
    ASSERT_EQ(State::Hidden, overview->state());

    // Its texture without the blur is kept beside the one with it.
    ASSERT_TRUE(overview->enter());
    paint();
    EXPECT_FALSE(drewContentOf(window));
}

TEST_F(OverviewTest, APreviewsCloseButtonCombinesItsDeskIntoTheNext)
{
    Window *here = makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_NE(nullptr, desks->newDesk(DesksCreationRemovalSource::Keyboard));
    desks->activateDesk(desk(1), DesksSwitchSource::DeskSwitchShortcut);
    Window *there = makeWindow(base::Rect(200, 100, 500, 400));
    desks->activateDesk(desk(0), DesksSwitchSource::DeskSwitchShortcut);
    show();
    ASSERT_EQ(1u, overview->items().size());

    // Only on the preview the pointer is on.
    const base::RectF preview = overview->bars()[0].desks[1].bounds;
    moveTo(preview.center());
    EXPECT_FALSE(overview->bars()[0].desks[0].closeShown);
    EXPECT_TRUE(overview->bars()[0].desks[1].closeShown);
    EXPECT_FALSE(overview->bars()[0].desks[1].closeHovered);
    const base::RectF button = scene::OverviewDeskBarNode::closeButtonRect(preview);
    EXPECT_TRUE(preview.contains(button));
    moveTo(button.center());
    EXPECT_TRUE(overview->bars()[0].desks[1].closeHovered);

    click(button.center());
    ASSERT_EQ(1u, desks->desks().size());
    EXPECT_EQ(desk(0), desks->deskOf(there));
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(2u, overview->items().size());
    EXPECT_GE(indexOf(here), 0);
    // One desk left, which cannot go.
    moveTo(overview->bars()[0].desks[0].bounds.center());
    EXPECT_FALSE(overview->bars()[0].desks[0].closeShown);
}

TEST_F(OverviewTest, ClosingTheActiveDeskSwitchesAtOnceWithoutTheRemovalStrip)
{
    // Outside the overview the active desk would go with the removal
    // animation, black where the removed desk's frame was.
    auto *switchNode = static_cast<scene::DeskSwitchNode *>(
            surfaceContainer->addChild(std::make_unique<scene::DeskSwitchNode>([this] {
                return now;
            })));
    manager.setDeskOverlays({node, switchNode, bar});
    desks->setSwitchNode(switchNode);
    desks->setScreenshotter([this](const std::function<void(render::Renderer &)> &paint) {
        paint(renderer);
        scene::DeskSwitchNode::Frames frames;
        frames["out"] = std::make_unique<FrameTexture>(kOutput.size());
        return frames;
    });
    Window *here = makeWindow(base::Rect(100, 100, 500, 400));
    show();

    // A new desk, clicked to, then closed.
    click(overview->bars()[0].newDesk.center());
    ASSERT_EQ(2u, desks->desks().size());
    desks->activateDesk(desk(1), DesksSwitchSource::MiniViewButton);
    settle();
    ASSERT_EQ(desk(1), desks->activeDesk());
    ASSERT_EQ(nullptr, desks->animation());
    ASSERT_EQ(State::Shown, overview->state());
    EXPECT_TRUE(overview->items().empty());

    const base::RectF preview = overview->bars()[0].desks[1].bounds;
    moveTo(preview.center());
    click(scene::OverviewDeskBarNode::closeButtonRect(preview).center());
    EXPECT_EQ(nullptr, desks->animation());
    EXPECT_FALSE(switchNode->isActive());
    ASSERT_EQ(1u, desks->desks().size());
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(State::Shown, overview->state());
    ASSERT_EQ(1u, overview->items().size());
    EXPECT_EQ(here, overview->items()[0].window);
    paint();
    EXPECT_TRUE(drewContentOf(here));
}

TEST_F(OverviewTest, EvenOneDeskGetsAPreview)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    show();
    ASSERT_EQ(1u, overview->bars().size());
    const auto one = overview->bars()[0];
    ASSERT_EQ(1u, one.desks.size());
    EXPECT_TRUE(one.desks[0].active);
    // A preview at the output's shape, the new-desk button half as wide.
    const base::RectF preview = one.desks[0].bounds;
    EXPECT_NEAR(double(kOutput.width()) / kOutput.height(), preview.width() / preview.height(),
                0.02);
    EXPECT_EQ(preview.height(), one.newDesk.height());
    EXPECT_NEAR(preview.width() / 2, one.newDesk.width(), 1);
    const base::RectF place = overview->items()[0].bounds;

    click(one.newDesk.center());
    EXPECT_EQ(2u, desks->desks().size());
    // Not switched to, and the windows stay where they are.
    EXPECT_EQ(desk(0), desks->activeDesk());
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(2u, overview->bars()[0].desks.size());
    EXPECT_EQ(one.bounds.height(), overview->bars()[0].bounds.height());
    now += 1500;
    EXPECT_EQ(place, overview->items()[0].bounds);
}

TEST_F(OverviewTest, AWindowDroppedOnNewGoesToANewDesk)
{
    Window *window = makeWindow(base::Rect(100, 100, 500, 400));
    makeWindow(base::Rect(300, 200, 500, 400));
    show();
    const base::PointF from = placeOf(window).center();
    const base::PointF to = overview->bars()[0].newDesk.center();
    moveTo(from);
    input.onPointerButton(at(now), BTN_LEFT, true);
    for (int step = 1; step <= 10; ++step)
        moveTo(from + (to - from) * step / 10);
    EXPECT_TRUE(overview->bars()[0].newDeskHighlighted);
    input.onPointerButton(at(now), BTN_LEFT, false);
    ASSERT_EQ(2u, desks->desks().size());
    EXPECT_EQ(desk(1), desks->deskOf(window));
    EXPECT_EQ(desk(0), desks->activeDesk());
}

TEST_F(OverviewTest, TheCursorIsTheDefaultOneThroughout)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_FALSE(seat.cursorOverride().has_value());
    ASSERT_TRUE(overview->enter());
    EXPECT_EQ(input::CursorShape::Default, seat.cursorOverride());
    settle();
    key(KEY_ESC);
    // Heading out, the windows have the pointer again, but not the cursor.
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(input::CursorShape::Default, seat.cursorOverride());
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_FALSE(seat.cursorOverride().has_value());
}

TEST_F(OverviewTest, ThreeFingersDownOverAWindowBringItToTheFront)
{
    Window *below = makeWindow(base::Rect(100, 100, 500, 400));
    Window *above = makeWindow(base::Rect(300, 200, 500, 400));
    WmGestureHandler gestures(desks, {}, overview);
    input.setGestureFilter(&gestures);
    const auto swipe = [this](double mm) {
        const base::PointF travel(0, mm / WmGestureHandler::kMmPerUnacceleratedUnit);
        input.onPointerGestureUpdate(at(now), input::GestureType::Swipe, travel * 5, travel, 1, 0);
    };
    const auto flickDown = [&] {
        input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
        for (int step = 0; step < 3; ++step) {
            now += 8;
            swipe(WmGestureHandler::kOverviewSwipeLengthMm * 0.1);
        }
        input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
        settle();
    };

    // Away from every window: the desk as it was.
    show();
    moveTo(base::PointF(990, 790));
    flickDown();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_EQ(above, active());

    // Over the one below: it comes to the front.
    show();
    moveTo(placeOf(below).center());
    ASSERT_EQ(indexOf(below), overview->hovered());
    flickDown();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_EQ(below, active());
    input.setGestureFilter(nullptr);
}

TEST_F(OverviewTest, AClickWhileItComesInTakesTheWindowUnderThePointer)
{
    Window *first = makeWindow(base::Rect(100, 100, 400, 300));
    Window *second = makeWindow(base::Rect(500, 300, 400, 300));
    ASSERT_EQ(second, active());
    ASSERT_TRUE(overview->enter());
    paint();
    now += 120;
    const double progress = node->progress(double(now));
    ASSERT_GT(progress, 0.1);
    ASSERT_LT(progress, 0.9);
    ASSERT_EQ(State::Entering, overview->state());
    // Where it is drawn now, not where it is going.
    const base::RectF drawn = node->presentedRect(indexOf(first), progress, double(now));
    click(drawn.center());
    EXPECT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(first, active());
}

TEST_F(OverviewTest, AClickedWindowIsDrawnOverTheRestAllTheWayOut)
{
    // Behind the active one on the desk, overlapping it.
    Window *behind = makeWindow(base::Rect(100, 100, 600, 400));
    Window *front = makeWindow(base::Rect(300, 200, 600, 400));
    ASSERT_EQ(front, active());
    show();
    click(placeOf(behind).center());
    ASSERT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(behind, active());
    // Topmost in the overview's own order from the first frame out.
    ASSERT_EQ(2u, overview->items().size());
    EXPECT_EQ(behind, overview->items().back().window);
    for (int step = 0; step < 6; ++step) {
        now += 40;
        paint();
        // The last draw of the window's content, live or from its cached texture.
        const auto drawnAt = [this](const Window *window) {
            const render::Texture *texture = window->rootSurface()->node()->texture();
            int last = -1;
            for (size_t i = 0; i < renderer.calls.size(); ++i) {
                const render::Texture *drawn = renderer.calls[i].texture;
                const auto cached = renderer.contents.find(drawn);
                if (drawn && (drawn == texture || (cached != renderer.contents.end() && cached->second == texture)))
                    last = int(i);
            }
            return last;
        };
        if (overview->state() != State::Exiting)
            break;
        ASSERT_GE(drawnAt(behind), 0);
        ASSERT_GE(drawnAt(front), 0);
        EXPECT_GT(drawnAt(behind), drawnAt(front)) << "step " << step;
    }
    settle();
    EXPECT_EQ(State::Hidden, overview->state());
    EXPECT_EQ(behind, active());
    EXPECT_EQ(behind, manager.windows().back());
}

TEST_F(OverviewTest, AClickKeepsEveryWindowsPlaceOnTheWayOut)
{
    // The same centre: the layout orders them by stacking, which the click
    // changes, so the places must be kept rather than laid out again.
    Window *behind = makeWindow(base::Rect(300, 200, 400, 300));
    Window *front = makeWindow(base::Rect(100, 100, 800, 500));
    ASSERT_EQ(front, active());
    show();
    const base::RectF behindPlace = placeOf(behind);
    const base::RectF frontPlace = placeOf(front);
    ASSERT_NE(behindPlace, frontPlace);
    click(behindPlace.center());
    ASSERT_EQ(State::Exiting, overview->state());
    EXPECT_EQ(behindPlace, placeOf(behind));
    EXPECT_EQ(frontPlace, placeOf(front));
    // And drawn from there, not from a new slot.
    now += 40;
    paint();
    const double progress = node->progress(double(now));
    const base::RectF drawn = node->presentedRect(indexOf(behind), progress, double(now));
    const base::RectF straight = lerp(base::RectF(behind->bounds()), behindPlace, progress);
    EXPECT_NEAR(straight.x(), drawn.x(), 1e-6);
    EXPECT_NEAR(straight.y(), drawn.y(), 1e-6);
}

TEST_F(OverviewTest, TheShortcutTurnsItAroundWhereverItIs)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    ASSERT_TRUE(overview->enter());
    paint();
    for (const State heading : {State::Exiting, State::Entering, State::Exiting}) {
        now += 50;
        const double before = node->progress(double(now));
        overview->toggle();
        EXPECT_EQ(heading, overview->state());
        EXPECT_NEAR(before, node->progress(double(now)), 1e-9);
        paint();
    }
    overview->toggle();
    settle();
    EXPECT_EQ(State::Shown, overview->state());
}

TEST_F(OverviewTest, ASidewaysSwipeLandsItWhereItWasHeading)
{
    makeWindow(base::Rect(100, 100, 500, 400));
    WmGestureHandler gestures(desks, {}, overview);
    input.setGestureFilter(&gestures);
    const auto sideways = [this] {
        const base::PointF travel(10 / WmGestureHandler::kMmPerUnacceleratedUnit, 0);
        input.onPointerGestureBegin(at(now), input::GestureType::Swipe, 3);
        input.onPointerGestureUpdate(at(now), input::GestureType::Swipe, travel * 5, travel, 1,
                                     0);
        input.onPointerGestureEnd(at(now), input::GestureType::Swipe, false);
    };

    ASSERT_TRUE(overview->enter());
    paint();
    now += 60;
    ASSERT_EQ(State::Entering, overview->state());
    sideways();
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(1, node->progress(double(now)));

    key(KEY_ESC);
    paint();
    now += 60;
    ASSERT_EQ(State::Exiting, overview->state());
    sideways();
    EXPECT_EQ(State::Hidden, overview->state());
    input.setGestureFilter(nullptr);
}

TEST_F(OverviewTest, ANewOutputSizeLaysItOutAgainAtOnce)
{
    Window *first = makeWindow(base::Rect(100, 100, 500, 400));
    Window *second = makeWindow(base::Rect(300, 200, 500, 400));
    show();
    const base::RectF place = placeOf(second);
    moveTo(place.center());
    input.onPointerButton(at(now), BTN_LEFT, true);
    moveTo(place.center() + base::PointF(60, 40));

    const base::Rect kWide(0, 0, 1600, 800);
    manager.setOutputBounds(kWide);
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(kWide, node->geometry());
    EXPECT_EQ(kWide, bar->geometry());
    ASSERT_EQ(1u, overview->bars().size());
    EXPECT_EQ(kWide, overview->bars()[0].output);
    ASSERT_EQ(2u, overview->items().size());
    // At its new place straight away, not on a tween from the old one.
    for (const Window *window : {first, second}) {
        EXPECT_TRUE(base::RectF(kWide).contains(placeOf(window)));
        EXPECT_EQ(placeOf(window), node->items()[indexOf(window)].to);
    }
    EXPECT_NE(place, placeOf(second));
    // The drag is dropped, so letting go does nothing.
    input.onPointerButton(at(now), BTN_LEFT, false);
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(desk(0), desks->deskOf(second));
}

namespace {

display::DisplayId displayId(const char *value)
{
    return display::DisplayId(value);
}

display::DisplaySnapshot physicalDisplay(const char *name, base::Size size)
{
    display::DisplaySnapshot result;
    result.sessionId = displayId(name);
    result.persistentKey = "edid-" + std::string(name);
    result.portKey = "port-" + std::string(name);
    result.connectorName = name;
    result.devicePath = "/dev/dri/card0";
    result.physicalSizeMm = {600, 340};
    result.modes = {{size, 60000, 148500, size.width() + 100, size.height() + 50, 0, "mode", true}};
    result.currentMode = result.modes.front();
    return result;
}

display::LogicalDisplay logicalDisplay(const char *name, base::Rect bounds, bool primary)
{
    return {displayId(name),
            {displayId(name)},
            "Lingmo-" + std::string(name),
            bounds,
            bounds,
            {},
            display::DisplayRotation::Normal,
            primary};
}

const base::Rect kSecondary(1000, 0, 800, 600);

} // namespace

class MultiOutputOverviewTest : public OverviewTest
{
protected:
    display::DisplayManager displays;
    uint64_t revision = 0;

    void SetUp() override
    {
        OverviewTest::SetUp();
        ASSERT_TRUE(displays.installHardware(
                {"instance",
                 "seat0",
                 1,
                 {physicalDisplay("a", kOutput.size()), physicalDisplay("b", kSecondary.size())}}));
        apply(true);
        manager.setDisplayManager(&displays);
    }

    void TearDown() override
    {
        manager.setDisplayManager(nullptr);
        OverviewTest::TearDown();
    }

    void apply(bool withSecondary)
    {
        std::vector<display::LogicalDisplay> logical {logicalDisplay("a", kOutput, true)};
        std::vector<display::DisplayPlacement> placements;
        if (withSecondary) {
            logical.push_back(logicalDisplay("b", kSecondary, false));
            placements.push_back({displayId("b"), displayId("a"), display::LayoutEdge::Right, 0});
        }
        ASSERT_TRUE(displays.installApplied(
                {"instance", 1, ++revision, {}, logical, placements}));
    }
};

TEST_F(MultiOutputOverviewTest, AnOutputGoingOrComingLaysItOutAgain)
{
    Window *here = makeWindow(base::Rect(100, 100, 500, 400));
    Window *there = makeWindow(base::Rect(1100, 100, 500, 400));
    show();
    ASSERT_EQ(2u, overview->bars().size());
    EXPECT_TRUE(base::RectF(kSecondary).contains(placeOf(there)));

    apply(false);
    EXPECT_EQ(State::Shown, overview->state());
    EXPECT_EQ(kOutput, node->geometry());
    ASSERT_EQ(1u, overview->bars().size());
    // The window brought back from the output that went is laid out with the rest.
    ASSERT_EQ(2u, overview->items().size());
    for (const Window *window : {here, there})
        EXPECT_TRUE(base::RectF(kOutput).contains(placeOf(window)));

    apply(true);
    const base::Rect both = kOutput.united(kSecondary);
    EXPECT_EQ(both, node->geometry());
    EXPECT_EQ(both, bar->geometry());
    ASSERT_EQ(2u, overview->bars().size());
    EXPECT_EQ(kSecondary, overview->bars()[1].output);
    EXPECT_EQ(2u, overview->items().size());
}
