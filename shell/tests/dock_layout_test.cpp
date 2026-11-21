// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Dock auto-hide policy. Behaviour modelled on ash/shelf/shelf_layout_manager_unittest.cc,
// rewritten against the Lingmo abstractions. No DRM, no GL, no Wayland client.

#include <gtest/gtest.h>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"
#include "wm/dock_layout_manager.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using namespace Lingmo::Wm;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1000, 800);
const base::Rect kDockBounds(200, 720, 600, 75);

class DockLayoutTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    WindowManager manager {&compositor, &seat};
    DockLayoutManager layout {&manager};

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);
    Window *dock = nullptr;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        dock = makeWindow(kDockBounds);
        dock->setRole(WindowRole::Dock);
        dock->setFocusPolicy(FocusPolicy::NoFocus);
        // The pointer rests in the middle of the screen unless a test moves it.
        moveTo(base::PointF(500, 400));
        // The dock slides in as it maps; every test starts with it in place.
        finishSlide();
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
        Window *window =
                manager.createWindow(makeSurface(bounds.size()), {.requestedSize = bounds.size()});
        window->setBounds(bounds);
        return window;
    }

    void moveTo(const base::PointF &location) { manager.onPointerMotion(at(0), location); }

    // Runs the event loop until the auto-hide delay has passed.
    void waitForReveal()
    {
        test::runFor(std::chrono::milliseconds(DockLayoutManager::kAutoHideDelayMs + 200));
    }

    // Plays the slide out to its end.
    void finishSlide()
    {
        layout.advance(1000);
        layout.advance(1000 + DockLayoutManager::kSlideDurationMs);
    }

    bool isShown() const { return layout.autoHideState(dock) == DockAutoHideState::Shown; }
};

TEST_F(DockLayoutTest, NeverHidesStaysPutOverMaximizedWindows)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    window->maximize();

    EXPECT_EQ(DockVisibilityState::Visible, layout.visibilityState(dock));
    EXPECT_TRUE(isShown());
    EXPECT_EQ(base::Point(), dock->presentationOffset());
    EXPECT_FALSE(layout.isSliding(dock));
}

TEST_F(DockLayoutTest, FullscreenWindowHidesTheDockWhateverTheSetting)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    window->activate();
    window->setFullscreen(true);

    EXPECT_EQ(DockVisibilityState::AutoHide, layout.visibilityState(dock));
    EXPECT_FALSE(isShown());
    EXPECT_TRUE(layout.isSliding(dock));

    finishSlide();
    EXPECT_FALSE(layout.isSliding(dock));
    EXPECT_TRUE(dock->isPresentationHidden());
    // Fully past the bottom edge.
    EXPECT_EQ(kOutput.bottom(), dock->node()->absoluteGeometry().top());

    window->setFullscreen(false);
    EXPECT_TRUE(isShown());
    EXPECT_FALSE(dock->isPresentationHidden());
    finishSlide();
    EXPECT_EQ(base::Point(), dock->presentationOffset());
}

TEST_F(DockLayoutTest, PanelOnlyGivesWayToFullscreen)
{
    Window *panel = makeWindow(base::Rect(0, 0, 1000, 30));
    panel->setRole(WindowRole::Panel);
    layout.setBehavior(DockAutoHideBehavior::Always);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    window->activate();
    // The setting is the dock's: the panel stays.
    EXPECT_EQ(DockVisibilityState::Visible, layout.visibilityState(panel));
    EXPECT_FALSE(isShown());

    window->maximize();
    EXPECT_EQ(DockVisibilityState::Visible, layout.visibilityState(panel));

    window->setFullscreen(true);
    EXPECT_EQ(DockVisibilityState::AutoHide, layout.visibilityState(panel));
    EXPECT_EQ(DockAutoHideState::Hidden, layout.autoHideState(panel));
    finishSlide();
    EXPECT_TRUE(panel->isPresentationHidden());
    EXPECT_EQ(base::Point(0, -30), panel->presentationOffset());

    // The top edge brings it back over the fullscreen window.
    moveTo(base::PointF(500, 2));
    waitForReveal();
    EXPECT_EQ(DockAutoHideState::Shown, layout.autoHideState(panel));

    window->setFullscreen(false);
    EXPECT_EQ(DockVisibilityState::Visible, layout.visibilityState(panel));
}

TEST_F(DockLayoutTest, DocksAndPanelsSlideInFromTheirEdgeAsTheyMap)
{
    Window *panel = makeWindow(base::Rect(0, 0, 1000, 30));
    panel->setRole(WindowRole::Panel);
    layout.update();

    // Parked past the top edge, then on its way in without the reveal delay.
    EXPECT_EQ(DockAutoHideState::Shown, layout.autoHideState(panel));
    EXPECT_TRUE(layout.isSliding(panel));
    EXPECT_FALSE(panel->isPresentationHidden());
    EXPECT_EQ(base::Point(0, -30), panel->presentationOffset());
    layout.advance(1000);
    layout.advance(1000 + DockLayoutManager::kSlideDurationMs / 2);
    EXPECT_LT(-30, panel->presentationOffset().y());
    EXPECT_GT(0, panel->presentationOffset().y());
    layout.advance(1000 + DockLayoutManager::kSlideDurationMs);
    EXPECT_FALSE(layout.isSliding(panel));
    EXPECT_EQ(base::Point(), panel->presentationOffset());
}

TEST_F(DockLayoutTest, DocksAndPanelsWaitForTheSessionToStart)
{
    layout.setSessionStarting(true);
    Window *panel = makeWindow(base::Rect(0, 0, 1000, 30));
    panel->setRole(WindowRole::Panel);
    layout.update();
    EXPECT_EQ(DockAutoHideState::Hidden, layout.autoHideState(panel));
    EXPECT_FALSE(layout.isSliding(panel));
    EXPECT_TRUE(panel->isPresentationHidden());
    EXPECT_EQ(base::Point(0, -30), panel->presentationOffset());

    // Released: in at once, without the reveal delay.
    layout.setSessionStarting(false);
    EXPECT_EQ(DockAutoHideState::Shown, layout.autoHideState(panel));
    EXPECT_TRUE(layout.isSliding(panel));
    EXPECT_FALSE(panel->isPresentationHidden());
    finishSlide();
    EXPECT_EQ(base::Point(), panel->presentationOffset());
}

TEST_F(DockLayoutTest, DockMappedOverAFullscreenWindowStaysParked)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    window->activate();
    window->setFullscreen(true);
    Window *second = makeWindow(base::Rect(0, 720, 1000, 75));
    second->setRole(WindowRole::Dock);
    layout.update();

    EXPECT_EQ(DockAutoHideState::Hidden, layout.autoHideState(second));
    EXPECT_FALSE(layout.isSliding(second));
    EXPECT_TRUE(second->isPresentationHidden());
    EXPECT_EQ(base::Point(0, kOutput.bottom() - 720), second->presentationOffset());
}

TEST_F(DockLayoutTest, EndingTheSessionPutsDocksAndPanelsAwayUntilItCarriesOn)
{
    Window *panel = makeWindow(base::Rect(0, 0, 1000, 30));
    panel->setRole(WindowRole::Panel);
    // Even with nothing to make room for, and the pointer on the dock.
    moveTo(base::PointF(500, centerPixel(kDockBounds).y()));

    layout.setSessionEnding(true);
    EXPECT_FALSE(isShown());
    EXPECT_EQ(DockAutoHideState::Hidden, layout.autoHideState(panel));
    finishSlide();
    EXPECT_TRUE(dock->isPresentationHidden());
    EXPECT_EQ(base::Point(0, -30), panel->presentationOffset());

    // Cancelled: both come straight back, without the reveal delay.
    layout.setSessionEnding(false);
    EXPECT_TRUE(isShown());
    EXPECT_EQ(DockAutoHideState::Shown, layout.autoHideState(panel));
    finishSlide();
    EXPECT_EQ(base::Point(), dock->presentationOffset());
    EXPECT_EQ(base::Point(), panel->presentationOffset());
}

TEST_F(DockLayoutTest, EndingTheSessionHidesTheDesktopUntilItCarriesOn)
{
    Window *desktop = makeWindow(kOutput);
    desktop->setRole(WindowRole::Desktop);
    layout.update();
    EXPECT_FALSE(desktop->isPresentationHidden());

    layout.setSessionEnding(true);
    EXPECT_TRUE(desktop->isPresentationHidden());
    EXPECT_FALSE(desktop->node()->isEffectivelyVisible());

    // One that maps meanwhile, as a restarted desktop does, stays hidden too.
    Window *second = makeWindow(kOutput);
    second->setRole(WindowRole::Desktop);
    layout.update();
    EXPECT_TRUE(second->isPresentationHidden());

    layout.setSessionEnding(false);
    EXPECT_FALSE(desktop->isPresentationHidden());
    EXPECT_FALSE(second->isPresentationHidden());
    EXPECT_EQ(base::Point(), desktop->presentationOffset());
}

TEST_F(DockLayoutTest, DialogOverFullscreenWindowKeepsTheDockAway)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    window->setFullscreen(true);
    Window *dialog = manager.createWindow(makeSurface(base::Size(200, 100)),
                                          {.requestedSize = base::Size(200, 100),
                                           .transientParent = window});
    dialog->activate();

    EXPECT_EQ(WorkspaceState::Fullscreen, layout.workspaceStateFor(kOutput));
    EXPECT_FALSE(isShown());
}

TEST_F(DockLayoutTest, InactiveFullscreenWindowUnderAnotherDoesNotHide)
{
    Window *fullscreen = makeWindow(base::Rect(100, 100, 300, 200));
    fullscreen->setFullscreen(true);
    Window *front = makeWindow(base::Rect(100, 100, 300, 200));
    front->activate();

    EXPECT_EQ(WorkspaceState::Default, layout.workspaceStateFor(kOutput));
    EXPECT_TRUE(isShown());
}

TEST_F(DockLayoutTest, WhenObstructedFollowsMaximizedWindows)
{
    layout.setBehavior(DockAutoHideBehavior::WhenObstructed);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    EXPECT_TRUE(isShown());

    window->maximize();
    EXPECT_EQ(DockVisibilityState::AutoHide, layout.visibilityState(dock));
    EXPECT_FALSE(isShown());

    window->restore();
    EXPECT_TRUE(isShown());

    window->maximize();
    EXPECT_FALSE(isShown());
    window->minimize();
    EXPECT_TRUE(isShown());
}

TEST_F(DockLayoutTest, YieldingZoneLeavesTheWorkAreaToMaximizedWindows)
{
    layout.setBehavior(DockAutoHideBehavior::WhenObstructed);
    manager.setExclusiveZone(dock, ScreenEdge::Bottom, 80, {}, true);
    const base::Rect reserved(0, 0, 1000, 720);
    EXPECT_EQ(reserved, manager.workAreaForOutput(kOutput));

    const base::Rect underTheDock(100, 600, 300, 190);
    Window *other = makeWindow(underTheDock);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->maximize();
    EXPECT_EQ(kOutput, manager.workAreaForOutput(kOutput));
    EXPECT_EQ(kOutput, window->bounds());
    EXPECT_EQ(reserved, manager.restingWorkAreaForOutput(kOutput));
    EXPECT_EQ(reserved, manager.outputAreas().front().workArea);

    window->restore();
    EXPECT_EQ(reserved, manager.workAreaForOutput(kOutput));
    EXPECT_EQ(base::Rect(100, 100, 300, 200), window->bounds());
    // A window left under the dock is not pushed out or shrunk.
    EXPECT_EQ(underTheDock, other->bounds());

    window->maximize();
    window->minimize();
    EXPECT_EQ(reserved, manager.workAreaForOutput(kOutput));
}

TEST_F(DockLayoutTest, FixedZoneStaysUnderMaximizedWindows)
{
    manager.setExclusiveZone(dock, ScreenEdge::Bottom, 80);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->maximize();
    EXPECT_EQ(base::Rect(0, 0, 1000, 720), manager.workAreaForOutput(kOutput));
    EXPECT_EQ(base::Rect(0, 0, 1000, 720), window->bounds());
}

TEST_F(DockLayoutTest, AlwaysHideStaysShownWithNothingToMakeRoomFor)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    EXPECT_EQ(DockVisibilityState::AutoHide, layout.visibilityState(dock));
    EXPECT_TRUE(isShown());

    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    EXPECT_FALSE(isShown());

    manager.destroyWindow(window);
    // Coming back is delayed like any reveal.
    EXPECT_FALSE(isShown());
    waitForReveal();
    EXPECT_TRUE(isShown());
}

TEST_F(DockLayoutTest, PointerAtTheEdgeRevealsAfterTheDelay)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();
    ASSERT_FALSE(isShown());
    ASSERT_TRUE(dock->isPresentationHidden());

    moveTo(base::PointF(500, 799));
    // Not yet: brushing the edge does not pull the dock out.
    EXPECT_FALSE(isShown());

    waitForReveal();
    EXPECT_TRUE(isShown());
    EXPECT_FALSE(dock->isPresentationHidden());
    EXPECT_TRUE(layout.isSliding(dock));
}

TEST_F(DockLayoutTest, LeavingTheEdgeBeforeTheDelayCancelsTheReveal)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();

    moveTo(base::PointF(500, 799));
    moveTo(base::PointF(500, 400));
    waitForReveal();
    EXPECT_FALSE(isShown());
}

TEST_F(DockLayoutTest, PointerMovingOntoTheDockDuringTheDelayStillReveals)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();

    moveTo(base::PointF(500, 799));
    moveTo(base::PointF(500, 750));
    waitForReveal();
    EXPECT_TRUE(isShown());
}

TEST_F(DockLayoutTest, LeavingTheDockHidesAfterTheDelayAndTheGapCounts)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();
    moveTo(base::PointF(500, 799));
    waitForReveal();
    ASSERT_TRUE(isShown());
    finishSlide();

    // Between the dock and the edge, and off the dock's span but on the edge.
    moveTo(base::PointF(500, 797));
    waitForReveal();
    EXPECT_TRUE(isShown());
    moveTo(base::PointF(50, 798));
    waitForReveal();
    EXPECT_TRUE(isShown());

    moveTo(base::PointF(500, 700));
    EXPECT_TRUE(isShown());
    waitForReveal();
    EXPECT_FALSE(isShown());
    EXPECT_TRUE(layout.isSliding(dock));
}

TEST_F(DockLayoutTest, ComingBackBeforeTheDelayKeepsTheDock)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();
    moveTo(base::PointF(500, 799));
    waitForReveal();
    finishSlide();
    moveTo(base::PointF(500, 750));
    ASSERT_TRUE(isShown());

    moveTo(base::PointF(500, 650));
    moveTo(base::PointF(500, 750));
    waitForReveal();
    EXPECT_TRUE(isShown());
    EXPECT_FALSE(layout.isSliding(dock));
}

TEST_F(DockLayoutTest, TooltipGoingWithThePointerWaitsForTheDelay)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();
    moveTo(base::PointF(500, 799));
    waitForReveal();
    finishSlide();
    moveTo(base::PointF(500, 750));
    Window *tooltip = manager.createWindow(makeSurface(base::Size(100, 40)),
                                           {.type = WindowType::Popup, .transientParent = dock});
    ASSERT_TRUE(isShown());

    moveTo(base::PointF(500, 650));
    manager.destroyWindow(tooltip);
    EXPECT_TRUE(isShown());
    waitForReveal();
    EXPECT_FALSE(isShown());
}

TEST_F(DockLayoutTest, AnythingButThePointerHidesAtOnce)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    EXPECT_TRUE(isShown());
    makeWindow(base::Rect(100, 100, 300, 200));
    EXPECT_FALSE(isShown());
}

TEST_F(DockLayoutTest, HiddenDockTakesNoInputWhileItSlidesAway)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    EXPECT_FALSE(dock->node()->isEventTargetingBlocked());
    makeWindow(base::Rect(100, 100, 300, 200));
    ASSERT_TRUE(layout.isSliding(dock));
    EXPECT_TRUE(dock->node()->isEventTargetingBlocked());
    finishSlide();

    moveTo(base::PointF(500, 799));
    waitForReveal();
    ASSERT_TRUE(layout.isSliding(dock));
    EXPECT_FALSE(dock->node()->isEventTargetingBlocked());
}

TEST_F(DockLayoutTest, OpenMenuAndLauncherKeepTheDockShown)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    ASSERT_FALSE(isShown());

    Window *menu = manager.createWindow(makeSurface(base::Size(100, 40)),
                                        {.type = WindowType::Popup, .transientParent = dock});
    waitForReveal();
    EXPECT_TRUE(isShown());
    manager.destroyWindow(menu);
    EXPECT_FALSE(isShown());

    Window *launcher = makeWindow(kOutput);
    launcher->setRole(WindowRole::Launcher);
    layout.update();
    waitForReveal();
    EXPECT_TRUE(isShown());
    manager.destroyWindow(launcher);
    EXPECT_FALSE(isShown());
}

TEST_F(DockLayoutTest, SlideEasesToTheTargetAndParksOffScreen)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    ASSERT_TRUE(layout.isSliding(dock));

    layout.advance(1000);
    EXPECT_EQ(base::Point(), dock->presentationOffset());
    layout.advance(1000 + DockLayoutManager::kSlideDurationMs / 2);
    const int half = dock->presentationOffset().y();
    EXPECT_GT(half, 0);
    EXPECT_LT(half, kOutput.bottom() - kDockBounds.top());
    EXPECT_FALSE(dock->isPresentationHidden());

    layout.advance(1000 + DockLayoutManager::kSlideDurationMs);
    EXPECT_EQ(base::Point(0, kOutput.bottom() - kDockBounds.top()), dock->presentationOffset());
    EXPECT_TRUE(dock->isPresentationHidden());
    EXPECT_FALSE(layout.isSliding(dock));
}

TEST_F(DockLayoutTest, ReversingMidSlideStartsFromWhereTheDockIs)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    layout.advance(1000);
    layout.advance(1000 + DockLayoutManager::kSlideDurationMs / 2);
    const base::Point midway = dock->presentationOffset();

    manager.destroyWindow(window);
    waitForReveal();
    ASSERT_TRUE(isShown());
    layout.advance(2000);
    EXPECT_EQ(midway, dock->presentationOffset());
    layout.advance(2000 + DockLayoutManager::kSlideDurationMs);
    EXPECT_EQ(base::Point(), dock->presentationOffset());
}

TEST_F(DockLayoutTest, HiddenDockFollowsItsOwnBoundsOffScreen)
{
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();

    dock->setBounds(base::Rect(200, 700, 600, 95));
    finishSlide();
    EXPECT_EQ(base::Point(0, kOutput.bottom() - 700), dock->presentationOffset());
    EXPECT_TRUE(dock->isPresentationHidden());
}

TEST_F(DockLayoutTest, SideDockLeavesThroughItsOwnEdge)
{
    dock->setBounds(base::Rect(5, 100, 75, 600));
    layout.setBehavior(DockAutoHideBehavior::Always);
    makeWindow(base::Rect(100, 100, 300, 200));
    finishSlide();
    EXPECT_EQ(base::Point(-80, 0), dock->presentationOffset());

    moveTo(base::PointF(2, 400));
    waitForReveal();
    EXPECT_TRUE(isShown());
}

} // namespace
