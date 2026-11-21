// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Lingmo counterparts to chromium/src/components/exo/pointer_unittest.cc,
// keyboard_unittest.cc and seat_unittest.cc, against fake delegates.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <string>
#include <vector>

#include "base/event_loop.h"
#include "base/geometry.h"
#include "base/region.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/event_sink.h"
#include "input/input_manager.h"
#include "input/keyboard.h"
#include "input/pointer.h"
#include "input/pointer_delegate.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Input;

namespace {

Timestamp at(int ms)
{
    return Timestamp(std::chrono::milliseconds(ms));
}

// Records the delegate calls in order, so tests can assert on event sequencing
// and frame batching rather than only on final state.
class FakePointerDelegate : public PointerDelegate
{
public:
    struct Enter {
        compositor::Surface* surface;
        base::PointF location;
        ButtonFlags buttons;
    };

    std::vector<std::string> calls;
    std::vector<Enter> enters;
    std::vector<compositor::Surface*> leaves;
    std::vector<base::PointF> motions;
    std::vector<std::pair<ButtonFlags, bool>> buttons;
    std::vector<std::pair<base::PointF, bool>> scrolls;
    int scrollStops = 0;
    int frames = 0;
    bool destroying = false;

    // Surfaces this delegate will accept; empty means all of them.
    std::vector<compositor::Surface*> acceptable;

    void onPointerDestroying(Pointer*) override { destroying = true; }

    bool canAcceptPointerEventsForSurface(compositor::Surface* surface) const override
    {
        if (acceptable.empty())
            return true;
        return std::find(acceptable.begin(), acceptable.end(), surface) != acceptable.end();
    }

    void onPointerEnter(compositor::Surface* surface, const base::PointF& location,
                        ButtonFlags pressedButtonFlags) override
    {
        calls.push_back("enter");
        enters.push_back({surface, location, pressedButtonFlags});
    }

    void onPointerLeave(compositor::Surface* surface) override
    {
        calls.push_back("leave");
        leaves.push_back(surface);
    }

    void onPointerMotion(Timestamp, const base::PointF& location) override
    {
        calls.push_back("motion");
        motions.push_back(location);
    }

    void onPointerButton(Timestamp, ButtonFlags changed, bool pressed) override
    {
        calls.push_back("button");
        buttons.emplace_back(changed, pressed);
    }

    void onPointerScroll(Timestamp, const base::PointF& offset, bool discrete) override
    {
        calls.push_back("scroll");
        scrolls.emplace_back(offset, discrete);
    }

    void onFingerScrollStop(Timestamp) override
    {
        calls.push_back("scroll_stop");
        ++scrollStops;
    }

    void onPointerFrame() override
    {
        calls.push_back("frame");
        ++frames;
    }
};

class FakeKeyboardDelegate : public KeyboardDelegate
{
public:
    struct Key {
        uint32_t code;
        bool pressed;
    };

    std::vector<std::string> calls;
    std::vector<compositor::Surface*> enters;
    std::vector<compositor::Surface*> leaves;
    PressedKeys enterKeys;
    std::vector<Key> keys;
    std::vector<KeyboardModifiers> modifiers;
    std::string keymap;
    int repeatRateHz = -1;
    int repeatDelayMs = -1;
    uint32_t nextSerial = 1;

    std::vector<compositor::Surface*> acceptable;
    std::function<void(uint32_t, bool)> onKey;

    bool canAcceptKeyboardEventsForSurface(compositor::Surface* surface) const override
    {
        if (acceptable.empty())
            return true;
        return std::find(acceptable.begin(), acceptable.end(), surface) != acceptable.end();
    }

    void onKeyboardEnter(compositor::Surface* surface, const PressedKeys& pressedKeys) override
    {
        calls.push_back("enter");
        enters.push_back(surface);
        enterKeys = pressedKeys;
    }

    void onKeyboardLeave(compositor::Surface* surface) override
    {
        calls.push_back("leave");
        leaves.push_back(surface);
    }

    uint32_t onKeyboardKey(Timestamp, uint32_t evdevCode, bool pressed) override
    {
        calls.push_back("key");
        keys.push_back({evdevCode, pressed});
        if (onKey)
            onKey(evdevCode, pressed);
        return nextSerial++;
    }

    void onKeyboardModifiers(const KeyboardModifiers& value) override
    {
        calls.push_back("modifiers");
        modifiers.push_back(value);
    }

    void onKeyRepeatSettingsChanged(bool enabled, std::chrono::milliseconds delay,
                                    std::chrono::milliseconds interval) override
    {
        calls.push_back("repeat_info");
        repeatRateHz = enabled && interval.count() > 0
            ? static_cast<int>(1000 / interval.count())
            : 0;
        repeatDelayMs = static_cast<int>(delay.count());
    }

    void onKeyboardLayoutUpdated(const std::string& value) override
    {
        calls.push_back("keymap");
        keymap = value;
    }
};

class InputTest : public testing::Test
{
protected:
    scene::Scene scene;
    // Mirrors the compositor's own layout: surfaces in one container, cursor
    // surfaces lifted out into another.
    scene::Node* surfaceContainer =
        scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node* cursorContainer =
        scene.root()->addChild(std::make_unique<scene::Node>("cursor"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    Seat seat {&compositor};

    void SetUp() override
    {
        scene.setSize(base::Size(800, 600));
        seat.setCursorContainer(cursorContainer);
    }

    // Gives `surface` content of `size` and places it at `position`.
    void map(compositor::Surface* surface, const base::Point& position, const base::Size& size)
    {
        auto buffer = pool->createBuffer(size, 0, size.width() * 4,
                                         render::PixelFormat::Bgrx8888, nullptr);
        surface->attach(buffer, {});
        surface->commit();
        surface->setPosition(position);
        m_buffers.push_back(std::move(buffer));
    }

private:
    std::vector<std::shared_ptr<compositor::ShmBuffer>> m_buffers;
};

} // namespace

TEST_F(InputTest, PointerEntersAndLeavesAsItCrossesASurface)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(50, 40));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(10, 10));
    EXPECT_TRUE(delegate.enters.empty());

    seat.onPointerMotion(at(2), base::PointF(120, 110));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(surface.get(), delegate.enters[0].surface);
    // Enter and leave are each followed by their own frame.
    EXPECT_EQ(std::vector<std::string>({"enter", "frame"}), delegate.calls);

    seat.onPointerMotion(at(3), base::PointF(400, 400));
    ASSERT_EQ(1u, delegate.leaves.size());
    EXPECT_EQ(surface.get(), delegate.leaves[0]);
    EXPECT_EQ(std::vector<std::string>({"enter", "frame", "leave", "frame"}), delegate.calls);
}

TEST_F(InputTest, OnePointerMotionIsHitTestedOnceForAllItsHandlers)
{
    auto below = compositor.createSurface();
    map(below.get(), base::Point(100, 100), base::Size(50, 40));
    auto above = compositor.createSurface();
    map(above.get(), base::Point(300, 300), base::Size(50, 40));
    EXPECT_EQ(compositor.surfaceForNode(above->node()), above.get());
    EXPECT_EQ(compositor.surfaceForNode(surfaceContainer), nullptr);

    base::PointF local;
    seat.beginPointerMotion();
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(120, 110), &local), below.get());
    EXPECT_EQ(local, base::PointF(20, 10));
    // Whatever changes while the motion is dispatched, its handlers agree.
    above->setPosition(base::Point(100, 100));
    local = {};
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(120, 110), &local), below.get());
    EXPECT_EQ(local, base::PointF(20, 10));
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(121, 110), &local), above.get());
    seat.endPointerMotion();
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(120, 110), &local), above.get());
    // A surface destroyed in the meantime is not handed out.
    seat.beginPointerMotion();
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(120, 110), &local), above.get());
    above.reset();
    EXPECT_EQ(seat.pointerTargetAt(surfaceContainer, base::PointF(120, 110), &local), below.get());
    seat.endPointerMotion();
}

TEST_F(InputTest, EnterAndMotionCarrySurfaceLocalCoordinates)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(50, 40));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(120, 110));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(base::PointF(20, 10), delegate.enters[0].location);
    // The enter already carried the location, so no motion follows it.
    EXPECT_TRUE(delegate.motions.empty());

    seat.onPointerMotion(at(2), base::PointF(130.5, 115.25));
    ASSERT_EQ(1u, delegate.motions.size());
    EXPECT_EQ(base::PointF(30.5, 15.25), delegate.motions[0]);
}

TEST_F(InputTest, ViewportPreservesSurfaceLocalPointerCoordinates)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(400, 200));
    surface->setBufferScale(2);
    surface->setCrop(base::RectF(25, 10, 100, 50));
    surface->setViewport(base::Size(200, 100));
    surface->commit();

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(120, 110));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(base::PointF(20, 10), delegate.enters[0].location);
    // The enter already carried the location, so no motion follows it.
    EXPECT_TRUE(delegate.motions.empty());

    seat.onPointerMotion(at(2), base::PointF(130.5, 115.25));
    ASSERT_EQ(1u, delegate.motions.size());
    EXPECT_EQ(base::PointF(30.5, 15.25), delegate.motions[0]);
}

TEST_F(InputTest, AMotionToTheSameLocationIsSuppressed)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(200, 200));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(10, 10));
    seat.onPointerMotion(at(2), base::PointF(10, 10));
    EXPECT_TRUE(delegate.motions.empty());

    seat.onPointerMotion(at(3), base::PointF(11, 10));
    EXPECT_EQ(1u, delegate.motions.size());
}

TEST_F(InputTest, HitTestingDescendsIntoSubsurfaces)
{
    auto parent = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    sub->setPosition(base::Point(20, 20));

    map(child.get(), base::Point(0, 0), base::Size(30, 30));
    map(parent.get(), base::Point(100, 100), base::Size(80, 80));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    // (105,105) is over the parent only.
    seat.onPointerMotion(at(1), base::PointF(105, 105));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(parent.get(), delegate.enters[0].surface);
    EXPECT_EQ(base::PointF(5, 5), delegate.enters[0].location);

    // (125,125) is inside the subsurface, which is stacked above the parent.
    seat.onPointerMotion(at(2), base::PointF(125, 125));
    ASSERT_EQ(2u, delegate.enters.size());
    EXPECT_EQ(child.get(), delegate.enters[1].surface);
    // Local to the subsurface, not to the parent.
    EXPECT_EQ(base::PointF(5, 5), delegate.enters[1].location);
    EXPECT_EQ(1u, delegate.leaves.size());
}

TEST_F(InputTest, AnInputRegionExcludesPartOfASurface)
{
    auto surface = compositor.createSurface();
    surface->setInputRegion(base::Region(base::Rect(0, 0, 20, 20)));
    map(surface.get(), base::Point(100, 100), base::Size(80, 80));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(150, 150));
    EXPECT_TRUE(delegate.enters.empty());

    seat.onPointerMotion(at(2), base::PointF(110, 110));
    EXPECT_EQ(1u, delegate.enters.size());
}

TEST_F(InputTest, ButtonsAreReportedAndSeedThePressedSetOnEnter)
{
    auto first = compositor.createSurface();
    auto second = compositor.createSurface();
    map(first.get(), base::Point(0, 0), base::Size(100, 100));
    map(second.get(), base::Point(200, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(50, 50));
    seat.onPointerButton(at(2), base::PointF(50, 50), BTN_LEFT, true);

    ASSERT_EQ(1u, delegate.buttons.size());
    EXPECT_EQ(LeftButton, delegate.buttons[0].first);
    EXPECT_TRUE(delegate.buttons[0].second);

    // The press holds the pointer: crossing the second surface changes nothing
    // until the button comes back up.
    seat.onPointerMotion(at(3), base::PointF(250, 50));
    EXPECT_EQ(1u, delegate.enters.size());
    EXPECT_TRUE(delegate.leaves.empty());

    seat.onPointerButton(at(4), base::PointF(250, 50), BTN_LEFT, false);
    ASSERT_EQ(2u, delegate.buttons.size());
    EXPECT_FALSE(delegate.buttons[1].second);
    EXPECT_EQ(NoButton, seat.pressedButtons());

    // With the grab gone the next motion lands where the pointer actually is.
    seat.onPointerMotion(at(5), base::PointF(251, 50));
    ASSERT_EQ(2u, delegate.enters.size());
    EXPECT_EQ(second.get(), delegate.enters[1].surface);
}

TEST_F(InputTest, AnEnterReportsButtonsHeldOutsideTheClient)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    // A press the window manager consumed: the seat tracks the button, but no
    // client took the implicit grab.
    seat.updatePointerButtonState(BTN_LEFT, true);
    seat.onPointerMotion(at(1), base::PointF(50, 50));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(LeftButton, delegate.enters[0].buttons);
}

TEST_F(InputTest, AGrabbedPointerKeepsReportingLocationsOutsideTheSurface)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(50, 40));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(120, 110));
    seat.onPointerButton(at(2), base::PointF(120, 110), BTN_LEFT, true);
    EXPECT_EQ(surface.get(), seat.pointerGrabSurface());

    // Well outside the surface, and negative in both axes.
    seat.onPointerMotion(at(3), base::PointF(10, 20));
    EXPECT_EQ(surface.get(), pointer.focusedSurface());
    ASSERT_EQ(1u, delegate.motions.size());
    EXPECT_EQ(base::PointF(-90, -80), delegate.motions[0]);
    EXPECT_TRUE(delegate.leaves.empty());

    // A second button joins the drag and does not end it.
    seat.onPointerButton(at(4), base::PointF(10, 20), BTN_RIGHT, true);
    seat.onPointerButton(at(5), base::PointF(10, 20), BTN_RIGHT, false);
    EXPECT_EQ(surface.get(), seat.pointerGrabSurface());

    seat.onPointerButton(at(6), base::PointF(10, 20), BTN_LEFT, false);
    EXPECT_EQ(nullptr, seat.pointerGrabSurface());

    // The grab ended over nothing, so the surface is left on the next motion.
    seat.onPointerMotion(at(7), base::PointF(11, 20));
    EXPECT_EQ(nullptr, pointer.focusedSurface());
    ASSERT_EQ(1u, delegate.leaves.size());
    EXPECT_EQ(surface.get(), delegate.leaves[0]);
}

TEST_F(InputTest, AGrabHidesTheEventsFromEveryOtherClient)
{
    auto mine = compositor.createSurface();
    auto theirs = compositor.createSurface();
    map(mine.get(), base::Point(0, 0), base::Size(100, 100));
    map(theirs.get(), base::Point(200, 0), base::Size(100, 100));

    FakePointerDelegate first, second;
    first.acceptable = {mine.get()};
    second.acceptable = {theirs.get()};
    Pointer firstPointer(&first, &seat), secondPointer(&second, &seat);

    seat.onPointerMotion(at(1), base::PointF(50, 50));
    seat.onPointerButton(at(2), base::PointF(50, 50), BTN_LEFT, true);
    const auto secondEvents = second.calls.size();

    seat.onPointerMotion(at(3), base::PointF(250, 50));
    EXPECT_EQ(mine.get(), firstPointer.focusedSurface());
    EXPECT_EQ(nullptr, secondPointer.focusedSurface());
    EXPECT_EQ(secondEvents, second.calls.size());
}

TEST_F(InputTest, DestroyingTheGrabbedSurfaceEndsTheGrab)
{
    auto surface = compositor.createSurface();
    auto other = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    map(other.get(), base::Point(200, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(50, 50));
    seat.onPointerButton(at(2), base::PointF(50, 50), BTN_LEFT, true);
    surface.reset();
    EXPECT_EQ(nullptr, seat.pointerGrabSurface());

    seat.onPointerMotion(at(3), base::PointF(250, 50));
    ASSERT_EQ(2u, delegate.enters.size());
    EXPECT_EQ(other.get(), delegate.enters[1].surface);
}

TEST_F(InputTest, ScrollAndScrollStopReachTheDelegate)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));

    seat.onPointerAxis(at(2), base::PointF(50, 50), base::PointF(0, 10), true);
    ASSERT_EQ(1u, delegate.scrolls.size());
    EXPECT_EQ(base::PointF(0, 10), delegate.scrolls[0].first);
    EXPECT_TRUE(delegate.scrolls[0].second);

    seat.onPointerAxis(at(3), base::PointF(50, 50), base::PointF(3.5, -2.0), false);
    ASSERT_EQ(2u, delegate.scrolls.size());
    EXPECT_FALSE(delegate.scrolls[1].second);

    seat.onPointerAxisStop(at(4), base::PointF(50, 50));
    EXPECT_EQ(1, delegate.scrollStops);
    // Every scroll is its own frame.
    EXPECT_EQ(4, delegate.frames);
}

TEST_F(InputTest, APointerIgnoresSurfacesItsClientDoesNotOwn)
{
    auto mine = compositor.createSurface();
    auto theirs = compositor.createSurface();
    map(mine.get(), base::Point(0, 0), base::Size(100, 100));
    map(theirs.get(), base::Point(200, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    delegate.acceptable = {mine.get()};
    Pointer pointer(&delegate, &seat);

    seat.onPointerMotion(at(1), base::PointF(250, 50));
    EXPECT_TRUE(delegate.enters.empty());

    seat.onPointerMotion(at(2), base::PointF(50, 50));
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(mine.get(), delegate.enters[0].surface);
}

TEST_F(InputTest, DestroyingTheFocusedSurfaceLeavesAndClearsFocus)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));
    ASSERT_EQ(surface.get(), pointer.focusedSurface());

    compositor::Surface* raw = surface.get();
    surface.reset();

    EXPECT_EQ(nullptr, pointer.focusedSurface());
    ASSERT_EQ(1u, delegate.leaves.size());
    EXPECT_EQ(raw, delegate.leaves[0]);

    // Nothing dangles afterwards.
    seat.onPointerMotion(at(2), base::PointF(60, 60));
    EXPECT_EQ(1u, delegate.enters.size());
}

TEST_F(InputTest, SetCursorLiftsTheSurfaceOutOfTheStackAndTracksTheHotspot)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(200, 200));
    map(cursor.get(), base::Point(0, 0), base::Size(24, 24));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    // set_cursor before focus does nothing, per the protocol.
    pointer.setCursor(cursor.get(), base::Point(4, 4));
    EXPECT_EQ(nullptr, pointer.cursorSurface());

    seat.onPointerMotion(at(1), base::PointF(50, 60));
    pointer.setCursor(cursor.get(), base::Point(4, 4));

    EXPECT_EQ(cursor.get(), pointer.cursorSurface());
    EXPECT_EQ(base::Point(4, 4), pointer.hotspot());
    EXPECT_TRUE(cursor->hasRole());
    // Out of the surface container, into the cursor one.
    EXPECT_EQ(cursorContainer, cursor->node()->parent());
    EXPECT_EQ(base::Point(46, 56), cursor->position());

    seat.onPointerMotion(at(2), base::PointF(100, 100));
    EXPECT_EQ(base::Point(96, 96), cursor->position());

    // A cursor surface is never a hit-test target.
    EXPECT_EQ(surface.get(), pointer.focusedSurface());
}

TEST_F(InputTest, ViewportCursorPreservesHotspotAndSampling)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(200, 200));
    map(cursor.get(), base::Point(0, 0), base::Size(24, 24));

    cursor->setCrop(base::RectF(4, 4, 16, 16));
    cursor->setViewport(base::Size(32, 32));
    cursor->commit();
    EXPECT_EQ(cursor->contentSize(), base::Size(32, 32));
    EXPECT_EQ(cursor->surfaceToBuffer().map(base::PointF()), base::PointF(4, 4));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);

    // set_cursor before focus does nothing, per the protocol.
    pointer.setCursor(cursor.get(), base::Point(4, 4));
    EXPECT_EQ(nullptr, pointer.cursorSurface());

    seat.onPointerMotion(at(1), base::PointF(50, 60));
    pointer.setCursor(cursor.get(), base::Point(4, 4));

    EXPECT_EQ(cursor.get(), pointer.cursorSurface());
    EXPECT_EQ(base::Point(4, 4), pointer.hotspot());
    EXPECT_TRUE(cursor->hasRole());
    // Out of the surface container, into the cursor one.
    EXPECT_EQ(cursorContainer, cursor->node()->parent());
    EXPECT_EQ(base::Point(46, 56), cursor->position());

    seat.onPointerMotion(at(2), base::PointF(100, 100));
    EXPECT_EQ(base::Point(96, 96), cursor->position());

    // A cursor surface is never a hit-test target.
    EXPECT_EQ(surface.get(), pointer.focusedSurface());
}

TEST_F(InputTest, LosingFocusDropsTheCursorSurface)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    map(cursor.get(), base::Point(0, 0), base::Size(24, 24));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));
    pointer.setCursor(cursor.get(), base::Point(0, 0));
    ASSERT_EQ(cursor.get(), pointer.cursorSurface());

    // The client must call set_cursor again in response to the next enter.
    seat.onPointerMotion(at(2), base::PointF(500, 500));
    EXPECT_EQ(nullptr, pointer.cursorSurface());
    EXPECT_FALSE(cursor->hasRole());
}

TEST_F(InputTest, ASurfaceWithARoleCannotBecomeTheCursor)
{
    auto surface = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), surface.get());
    map(child.get(), base::Point(0, 0), base::Size(20, 20));
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));

    pointer.setCursor(child.get(), base::Point(0, 0));
    EXPECT_EQ(nullptr, pointer.cursorSurface());
}

TEST_F(InputTest, ACursorSurfaceCannotBecomeASubsurface)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    map(cursor.get(), base::Point(0, 0), base::Size(24, 24));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));
    pointer.setCursor(cursor.get(), base::Point(0, 0));
    ASSERT_EQ(cursor.get(), pointer.cursorSurface());

    EXPECT_EQ(nullptr, compositor::SubSurface::create(cursor.get(), surface.get()));
}

TEST_F(InputTest, DestroyingTheCursorSurfaceClearsIt)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    map(cursor.get(), base::Point(0, 0), base::Size(24, 24));

    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    seat.onPointerMotion(at(1), base::PointF(50, 50));
    pointer.setCursor(cursor.get(), base::Point(0, 0));

    cursor.reset();
    EXPECT_EQ(nullptr, pointer.cursorSurface());
    EXPECT_EQ(surface.get(), pointer.focusedSurface());
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard

TEST_F(InputTest, AKeyboardSendsItsKeymapAndRepeatInfoOnCreation)
{
    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);

    EXPECT_FALSE(raw->keymap.empty());
    EXPECT_NE(std::string::npos, raw->keymap.find("xkb_keymap"));
    EXPECT_EQ(25, raw->repeatRateHz);
    EXPECT_EQ(500, raw->repeatDelayMs);
    EXPECT_EQ(std::vector<std::string>({"keymap", "repeat_info"}), raw->calls);
}

TEST_F(InputTest, ALayoutChangeResendsTheKeymapWithTheHeldModifiers)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    seat.setFocusedSurface(surface.get());
    seat.onKey(at(1), KEY_LEFTSHIFT, true);
    const std::string original = raw->keymap;
    raw->calls.clear();
    raw->modifiers.clear();

    ASSERT_TRUE(seat.setKeyboardLayout("de"));
    EXPECT_EQ(std::vector<std::string>({"keymap", "modifiers"}), raw->calls);
    EXPECT_NE(original, raw->keymap);
    ASSERT_EQ(1u, raw->modifiers.size());
    EXPECT_NE(0u, raw->modifiers[0].depressed);

    raw->calls.clear();
    EXPECT_FALSE(seat.setKeyboardLayout("definitely-not-a-layout"));
    EXPECT_TRUE(raw->calls.empty());
}

TEST_F(InputTest, RepeatSettingsReachEveryKeyboardOnlyWhenTheyChange)
{
    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    raw->calls.clear();

    KeyRepeatSettings settings;
    settings.delay = std::chrono::milliseconds(300);
    settings.interval = std::chrono::milliseconds(20);
    seat.setKeyRepeatSettings(settings);
    seat.setKeyRepeatSettings(settings);
    EXPECT_EQ(std::vector<std::string>({"repeat_info"}), raw->calls);
    EXPECT_EQ(50, raw->repeatRateHz);
    EXPECT_EQ(300, raw->repeatDelayMs);

    settings.enabled = false;
    seat.setKeyRepeatSettings(settings);
    EXPECT_EQ(0, raw->repeatRateHz);

    // A keyboard bound later starts from the seat's settings.
    auto lateDelegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* late = lateDelegate.get();
    Keyboard lateKeyboard(std::move(lateDelegate), &seat);
    EXPECT_EQ(0, late->repeatRateHz);
    EXPECT_EQ(300, late->repeatDelayMs);
}

TEST_F(InputTest, KeyboardFocusFollowsTheSeatWithModifiersBeforeEnter)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    raw->calls.clear();

    seat.setFocusedSurface(surface.get());
    ASSERT_EQ(1u, raw->enters.size());
    EXPECT_EQ(surface.get(), raw->enters[0]);
    EXPECT_EQ(std::vector<std::string>({"modifiers", "enter"}), raw->calls);

    seat.setFocusedSurface(nullptr);
    ASSERT_EQ(1u, raw->leaves.size());
    EXPECT_EQ(surface.get(), raw->leaves[0]);
}

TEST_F(InputTest, EnterCarriesTheKeysAlreadyPressed)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);

    seat.onKey(at(1), KEY_A, true);
    seat.onKey(at(2), KEY_LEFTSHIFT, true);
    // Nothing is focused yet, so nothing reached the client.
    EXPECT_TRUE(raw->keys.empty());

    seat.setFocusedSurface(surface.get());
    ASSERT_EQ(1u, raw->enters.size());
    ASSERT_EQ(2u, raw->enterKeys.size());
    EXPECT_EQ(1u, raw->enterKeys.count(KEY_A));
    EXPECT_EQ(1u, raw->enterKeys.count(KEY_LEFTSHIFT));

    // A key pressed before focus is still released to the focused surface.
    seat.onKey(at(3), KEY_A, false);
    ASSERT_EQ(1u, raw->keys.size());
    EXPECT_EQ(KEY_A, raw->keys[0].code);
    EXPECT_FALSE(raw->keys[0].pressed);
}

TEST_F(InputTest, KeyEventsReachTheFocusedSurfaceBeforeTheModifiersTheyChange)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    seat.setFocusedSurface(surface.get());
    raw->calls.clear();
    raw->modifiers.clear();

    // As wlroots, KWin and Mutter send them: Qt reads Shift's own press
    // against the state before it, and would otherwise report it without Shift.
    seat.onKey(at(1), KEY_LEFTSHIFT, true);
    EXPECT_EQ(std::vector<std::string>({"key", "modifiers"}), raw->calls);
    ASSERT_EQ(1u, raw->modifiers.size());
    EXPECT_NE(0u, raw->modifiers[0].depressed);

    seat.onKey(at(2), KEY_A, true);
    seat.onKey(at(3), KEY_A, false);
    ASSERT_EQ(3u, raw->keys.size());
    EXPECT_EQ(KEY_A, raw->keys[1].code);
    EXPECT_TRUE(raw->keys[1].pressed);
    EXPECT_FALSE(raw->keys[2].pressed);

    raw->calls.clear();
    seat.onKey(at(4), KEY_LEFTSHIFT, false);
    EXPECT_EQ(std::vector<std::string>({"key", "modifiers"}), raw->calls);
    ASSERT_FALSE(raw->modifiers.empty());
    EXPECT_EQ(0u, raw->modifiers.back().depressed);
}

TEST_F(InputTest, ARepeatedPressForAHeldKeyIsNotForwarded)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    seat.setFocusedSurface(surface.get());

    seat.onKey(at(1), KEY_A, true);
    seat.onKey(at(2), KEY_A, true);
    EXPECT_EQ(1u, raw->keys.size());
    EXPECT_EQ(1u, seat.pressedKeys().size());
}

TEST_F(InputTest, DestroyingTheFocusedSurfaceLeavesTheKeyboard)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    seat.setFocusedSurface(surface.get());
    ASSERT_EQ(surface.get(), keyboard.focusedSurface());

    compositor::Surface* rawSurface = surface.get();
    surface.reset();

    EXPECT_EQ(nullptr, keyboard.focusedSurface());
    EXPECT_EQ(nullptr, seat.focusedSurface());
    ASSERT_EQ(1u, raw->leaves.size());
    EXPECT_EQ(rawSurface, raw->leaves[0]);

    // Keys after the surface is gone go nowhere.
    seat.onKey(at(2), KEY_A, true);
    EXPECT_TRUE(raw->keys.empty());
}

TEST_F(InputTest, AKeyboardIgnoresFocusOnSurfacesItsClientDoesNotOwn)
{
    auto mine = compositor.createSurface();
    auto theirs = compositor.createSurface();
    map(mine.get(), base::Point(0, 0), base::Size(100, 100));
    map(theirs.get(), base::Point(200, 0), base::Size(100, 100));

    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    raw->acceptable = {mine.get()};
    Keyboard keyboard(std::move(delegate), &seat);

    seat.setFocusedSurface(theirs.get());
    EXPECT_TRUE(raw->enters.empty());

    seat.setFocusedSurface(mine.get());
    EXPECT_EQ(1u, raw->enters.size());
}

TEST_F(InputTest, ABorrowedFocusLeavesTheOtherClientFocusedWithoutNewKeys)
{
    auto app = compositor.createSurface();
    auto capture = compositor.createSurface();
    map(app.get(), base::Point(0, 0), base::Size(100, 100));
    map(capture.get(), base::Point(0, 0), base::Size(800, 600));

    auto appDelegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* appKeys = appDelegate.get();
    appKeys->acceptable = {app.get()};
    Keyboard appKeyboard(std::move(appDelegate), &seat);
    auto captureDelegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* captureKeys = captureDelegate.get();
    captureKeys->acceptable = {capture.get()};
    Keyboard captureKeyboard(std::move(captureDelegate), &seat);

    seat.setFocusedSurface(app.get());
    // The shortcut that opened the capture interface is still held.
    seat.onKey(at(1), KEY_LEFTSHIFT, true);
    seat.setFocusedSurface(capture.get(), true);
    EXPECT_TRUE(appKeys->leaves.empty());
    EXPECT_EQ(app.get(), appKeyboard.focusedSurface());
    ASSERT_EQ(1u, captureKeys->enters.size());

    appKeys->keys.clear();
    seat.onKey(at(2), KEY_ESC, true);
    seat.onKey(at(3), KEY_LEFTSHIFT, false);
    seat.onKey(at(4), KEY_ESC, false);
    // Only the release of what it held, so no key stays stuck.
    ASSERT_EQ(1u, appKeys->keys.size());
    EXPECT_EQ(KEY_LEFTSHIFT, appKeys->keys[0].code);
    EXPECT_FALSE(appKeys->keys[0].pressed);
    EXPECT_EQ(0u, appKeys->modifiers.back().depressed);
    ASSERT_EQ(3u, captureKeys->keys.size());
    EXPECT_EQ(KEY_ESC, captureKeys->keys[0].code);
    EXPECT_TRUE(captureKeys->keys[0].pressed);

    seat.setFocusedSurface(app.get());
    EXPECT_TRUE(appKeys->leaves.empty());
    EXPECT_EQ(1u, appKeys->enters.size());
    ASSERT_EQ(1u, captureKeys->leaves.size());
    seat.onKey(at(5), KEY_A, true);
    ASSERT_EQ(2u, appKeys->keys.size());
    EXPECT_EQ(KEY_A, appKeys->keys[1].code);
}

TEST_F(InputTest, TheKeyThatEndsABorrowStaysWithTheBorrower)
{
    auto app = compositor.createSurface();
    auto capture = compositor.createSurface();
    map(app.get(), base::Point(0, 0), base::Size(100, 100));
    map(capture.get(), base::Point(0, 0), base::Size(800, 600));

    // The borrower's keyboard comes first, so the seat reaches the other
    // client after the borrow has already ended.
    auto captureDelegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* captureKeys = captureDelegate.get();
    captureKeys->acceptable = {capture.get()};
    Keyboard captureKeyboard(std::move(captureDelegate), &seat);
    auto appDelegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* appKeys = appDelegate.get();
    appKeys->acceptable = {app.get()};
    Keyboard appKeyboard(std::move(appDelegate), &seat);

    seat.setFocusedSurface(app.get());
    seat.setFocusedSurface(capture.get(), true);
    captureKeys->onKey = [&](uint32_t code, bool pressed) {
        if (code == KEY_ESC && pressed)
            seat.setFocusedSurface(app.get());
    };

    seat.onKey(at(1), KEY_ESC, true);
    EXPECT_EQ(app.get(), appKeyboard.focusedSurface());
    EXPECT_TRUE(appKeys->keys.empty());
    seat.onKey(at(2), KEY_ESC, false);
    EXPECT_TRUE(appKeys->keys.empty());

    seat.onKey(at(3), KEY_ESC, true);
    ASSERT_EQ(1u, appKeys->keys.size());
    EXPECT_EQ(KEY_ESC, appKeys->keys[0].code);
    EXPECT_TRUE(appKeys->keys[0].pressed);
}

////////////////////////////////////////////////////////////////////////////////
// InputManager

// Click-to-activate belongs to the window manager; see wm_test.cpp.
// InputManager only offers events to its EventSink.

TEST_F(InputTest, ThePointerIsClampedToTheOutput)
{
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    manager.warpPointer(base::PointF(400, 300));

    manager.onPointerMotion(at(1), base::PointF(-1000, -1000));
    EXPECT_EQ(base::PointF(0, 0), manager.pointerPosition());

    manager.onPointerMotion(at(2), base::PointF(5000, 5000));
    EXPECT_EQ(base::PointF(799, 599), manager.pointerPosition());
}

// aura clears hover when a touch hides the cursor (a mouse exit with
// EF_CURSOR_HIDE); a click or a wheel brings both back, as a move does.
TEST_F(InputTest, ATouchTakesTheHoverAwayUntilTheMouseIsUsedAgain)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(400, 300));
    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    manager.onPointerMotionAbsolute(at(0), {100, 100});
    ASSERT_EQ(1u, delegate.enters.size());

    delegate.calls.clear();
    manager.onTouchDown(at(1), 1, base::PointF(0.5, 0.5));
    manager.onTouchUp(at(2), 1);
    manager.onTouchFrame(at(2));
    EXPECT_TRUE(seat.cursorHiddenByTouch());
    EXPECT_EQ(std::vector<std::string>({"leave", "frame"}), delegate.calls);

    delegate.calls.clear();
    manager.onPointerAxis(at(3), base::PointF(0, 10), true);
    EXPECT_FALSE(seat.cursorHiddenByTouch());
    EXPECT_EQ(std::vector<std::string>({"enter", "frame", "scroll", "frame"}), delegate.calls);
    EXPECT_EQ(base::PointF(100, 100), delegate.enters.back().location);

    manager.onTouchDown(at(4), 1, base::PointF(0.5, 0.5));
    manager.onTouchUp(at(5), 1);
    delegate.calls.clear();
    manager.onPointerButton(at(6), BTN_LEFT, true);
    EXPECT_FALSE(seat.cursorHiddenByTouch());
    EXPECT_EQ(std::vector<std::string>({"enter", "frame", "button", "frame"}), delegate.calls);

    // The surface the mouse is pressing on keeps it.
    delegate.calls.clear();
    manager.onTouchDown(at(7), 1, base::PointF(0.5, 0.5));
    manager.onTouchUp(at(8), 1);
    EXPECT_FALSE(seat.cursorHiddenByTouch());
    EXPECT_TRUE(delegate.calls.empty());
    manager.onPointerButton(at(9), BTN_LEFT, false);
}

// aura's PostSynthesizeMouseMove: a surface mapped under a still pointer
// gets its enter without the pointer moving, but not while a button is held.
TEST_F(InputTest, ASurfaceMappedUnderAStillPointerIsEntered)
{
    base::EventLoop *loop = base::EventLoop::current();
    ASSERT_NE(loop, nullptr);
    FakePointerDelegate delegate;
    Pointer pointer(&delegate, &seat);
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    manager.onPointerMotionAbsolute(at(0), {100, 100});
    ASSERT_TRUE(delegate.enters.empty());

    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(400, 300));
    manager.synthesizeMotion();
    manager.synthesizeMotion();
    EXPECT_TRUE(delegate.enters.empty());
    loop->dispatch();
    ASSERT_EQ(1u, delegate.enters.size());
    EXPECT_EQ(surface.get(), delegate.enters[0].surface);
    EXPECT_EQ(base::PointF(100, 100), delegate.enters[0].location);
    EXPECT_TRUE(delegate.motions.empty());

    manager.onPointerButton(at(1), BTN_LEFT, true);
    auto above = compositor.createSurface();
    map(above.get(), base::Point(0, 0), base::Size(400, 300));
    manager.synthesizeMotion();
    loop->dispatch();
    EXPECT_EQ(1u, delegate.enters.size());
    manager.onPointerButton(at(2), BTN_LEFT, false);
}

TEST_F(InputTest, PointerStartsAtThePrimaryDisplaysCentre)
{
    // Upstream DrmCursor::OnWindowAdded: a cursor not placed yet goes to the
    // centre, not the origin a new compositor would otherwise show it at.
    InputManager manager(&seat);
    manager.setTopology({base::Rect(-200, 0, 200, 100), base::Rect(0, 0, 300, 200)}, base::Rect(0, 0, 300, 200));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(150, 100));
    // Placed once: a later layout keeps the pointer where it is.
    manager.warpPointer(base::PointF(-100, 50));
    manager.setTopology({base::Rect(-200, 0, 200, 100), base::Rect(0, 0, 400, 300)}, base::Rect(0, 0, 400, 300));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(-100, 50));
}

TEST_F(InputTest, PointerKeepsItsSpotOnADisplayThatChangesScale)
{
    // The greeter centres the pointer before its display gets 150%; it has to
    // stay centred, not keep a logical point that is now bottom right. The
    // coordinator republishes through an empty layout, as every reprobe does.
    InputManager manager(&seat);
    manager.setTopology({base::Rect(0, 0, 2160, 1440)});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(1080, 720));
    manager.setTopology({});
    manager.setTopology({base::Rect(0, 0, 1440, 960)});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(720, 480));
    manager.warpPointer(base::PointF(1200, 60));
    manager.setTopology({base::Rect(0, 0, 1080, 720)});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(900, 45));
    // A reprobe that brings the same layout back leaves the pointer alone.
    manager.setTopology({});
    manager.setTopology({base::Rect(0, 0, 1080, 720)});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(900, 45));
}

TEST_F(InputTest, PointerProjectsOntoLogicalTopologyAndCrossesSharedEdges)
{
    InputManager manager(&seat);
    manager.setTopology({base::Rect(-200, 0, 200, 100), base::Rect(0, 40, 300, 200)});
    manager.warpPointer(base::PointF(-10, 50));

    manager.onPointerMotion(at(1), base::PointF(20, 0));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(10, 50));

    manager.warpPointer(base::PointF(-10, 10));
    manager.onPointerMotion(at(2), base::PointF(20, 0));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(-1, 10));

    manager.warpPointer(base::PointF(150, 500));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(150, 239));
}

TEST_F(InputTest, LogicalTopologyDoesNotApplyOneOutputsDeviceScaleToRelativeMotion)
{
    InputManager manager(&seat);
    manager.setDeviceScale(2);
    manager.setTopology({base::Rect(0, 0, 100, 100), base::Rect(100, 0, 100, 100)});
    manager.warpPointer(base::PointF(50, 50));
    manager.onPointerMotion(at(1), base::PointF(25, 0));
    EXPECT_EQ(manager.pointerPosition(), base::PointF(75, 50));
}

TEST_F(InputTest, AbsolutePointerClicksUseCurrentResolutionRatherThanStartupMode)
{
    InputManager manager(&seat);
    manager.setDeviceBounds({1280, 800});
    manager.setDeviceScale(1);
    for (const base::Rect desktop : {base::Rect(0, 0, 1280, 800), base::Rect(0, 0, 1920, 1200),
                                base::Rect(0, 0, 960, 600), base::Rect(0, 0, 1280, 800)}) {
        manager.setTopology({desktop});
        const base::Point target(desktop.width() * 3 / 4, desktop.height() * 3 / 4);
        auto surface = compositor.createSurface();
        map(surface.get(), target - base::Point(10, 10), {20, 20});
        FakePointerDelegate delegate;
        delegate.acceptable = {surface.get()};
        Pointer pointer(&delegate, &seat);
        manager.onPointerMotionNormalized(at(1), {0.75, 0.75});
        EXPECT_EQ(manager.pointerPosition(), base::PointF(target));
        ASSERT_FALSE(delegate.enters.empty());
        EXPECT_EQ(delegate.enters.back().location, base::PointF(10, 10));
        manager.onPointerButton(at(2), BTN_LEFT, true);
        manager.onPointerButton(at(3), BTN_LEFT, false);
        ASSERT_EQ(delegate.buttons.size(), 2u);
        EXPECT_TRUE(delegate.buttons.front().second);
        EXPECT_FALSE(delegate.buttons.back().second);
    }
}

TEST_F(InputTest, NormalizedAbsolutePointerUsesLogicalScaleAndNegativeDesktopOrigin)
{
    InputManager manager(&seat);
    manager.setDeviceBounds({1920, 1200});
    manager.setDeviceScale(2);
    manager.setTopology({base::Rect(-960, 0, 960, 600), base::Rect(0, 0, 960, 600)});
    manager.onPointerMotionNormalized(at(1), {0.25, 0.5});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(-480, 300));
    manager.onPointerMotionNormalized(at(2), {0.75, 0.5});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(480, 300));
    manager.onPointerMotionNormalized(at(3), {1, 1});
    EXPECT_EQ(manager.pointerPosition(), base::PointF(959, 599));
}

TEST_F(InputTest, CursorPresentationSurvivesConsumedMotionAndHonorsHideWhileIdle)
{
    auto surface = compositor.createSurface();
    auto cursor = compositor.createSurface();
    auto replacement = compositor.createSurface();
    map(surface.get(), {0, 0}, {200, 200});
    map(cursor.get(), {}, {24, 24});
    map(replacement.get(), {}, {16, 16});
    FakePointerDelegate delegate;
    delegate.acceptable = {surface.get()};
    Pointer pointer(&delegate, &seat);
    InputManager manager(&seat);
    auto* fallback =
        cursorContainer->addChild(std::make_unique<scene::ColorNode>(render::Color {1, 1, 1, 1}));
    fallback->setGeometry({0, 0, 14, 14});
    manager.setFallbackCursor(fallback, {});
    manager.onPointerMotionAbsolute(at(1), {50, 50});
    seat.setFocusedSurface(surface.get());
    const auto generation = seat.selection()->generation();
    const auto recipient = seat.selection()->recipient();
    int repaint = 0;
    scene.root()->setChangeCallback([&] {
        ++repaint;
    });
    auto connection = compositor.repaintNeeded.connect([&](bool) {
        ++repaint;
    });
    pointer.setCursor(cursor.get(), {4, 5});
    compositor.updateTextures(renderer);
    auto checkCursor = [&](compositor::Surface* expected) {
        renderer.calls.clear();
        cursorContainer->render(renderer);
        ASSERT_EQ(renderer.calls.size(), 1);
        EXPECT_EQ(renderer.calls.front().texture, expected->node()->texture());
        EXPECT_FALSE(renderer.calls.front().rect.isEmpty());
        EXPECT_TRUE(expected->node()->isEffectivelyVisible());
        EXPECT_EQ(expected->node()->parent(), cursorContainer);
        ASSERT_NE(expected->node()->texture(), nullptr);
        EXPECT_EQ(expected->node()->absolutePosition() + pointer.hotspot(),
                  manager.pointerPosition().toPoint());
        EXPECT_FALSE(fallback->isEffectivelyVisible());
    };
    checkCursor(cursor.get());
    EXPECT_GT(repaint, 0);

    struct ConsumingSink : EventSink {
        bool onPointerMotion(Timestamp, const base::PointF&) override { return true; }
    } sink;

    manager.setEventSink(&sink);
    const auto events = delegate.calls.size();
    repaint = 0;
    manager.onPointerMotionAbsolute(at(2), {300, 300});
    checkCursor(cursor.get());
    EXPECT_GT(repaint, 0);
    EXPECT_EQ(pointer.focusedSurface(), surface.get());
    EXPECT_EQ(delegate.calls.size(), events);
    repaint = 0;
    manager.onPointerMotionAbsolute(at(3), {300, 300});
    pointer.setCursor(cursor.get(), {4, 5});
    compositor.updateTextures(renderer);
    checkCursor(cursor.get());
    EXPECT_EQ(repaint, 0);
    pointer.setCursor(replacement.get(), {2, 3});
    compositor.updateTextures(renderer);
    checkCursor(replacement.get());
    replacement->attach(nullptr, {});
    replacement->commit();
    compositor.updateTextures(renderer);
    renderer.calls.clear();
    cursorContainer->render(renderer);
    EXPECT_TRUE(renderer.calls.empty());
    EXPECT_FALSE(fallback->isEffectivelyVisible());
    map(replacement.get(), {}, {16, 16});
    pointer.setCursor(replacement.get(), {3, 4});
    compositor.updateTextures(renderer);
    checkCursor(replacement.get());
    EXPECT_EQ(cursor->node()->parent(), nullptr);
    EXPECT_GT(repaint, 0);
    repaint = 0;
    pointer.setCursor(nullptr, {});
    renderer.calls.clear();
    cursorContainer->render(renderer);
    EXPECT_TRUE(renderer.calls.empty());
    EXPECT_FALSE(fallback->isEffectivelyVisible());
    EXPECT_GT(repaint, 0);
    repaint = 0;
    pointer.setCursor(nullptr, {});
    manager.onPointerMotionAbsolute(at(4), {310, 310});
    EXPECT_FALSE(fallback->isEffectivelyVisible());
    // A hidden fallback's location update must not wake output.
    EXPECT_EQ(repaint, 0);
    pointer.setCursor(replacement.get(), {2, 3});
    replacement.reset();
    EXPECT_FALSE(seat.hasClientCursorRequest());
    EXPECT_TRUE(fallback->isEffectivelyVisible());
    pointer.setCursor(cursor.get(), {4, 5});
    surface.reset();
    EXPECT_FALSE(pointer.focusedSurface());
    EXPECT_EQ(cursor->node()->parent(), nullptr);
    EXPECT_TRUE(fallback->isEffectivelyVisible());
    EXPECT_EQ(seat.selection()->generation(), generation);
    EXPECT_EQ(seat.selection()->recipient(), recipient);
    connection.reset();
    scene.root()->setChangeCallback({});
}

TEST_F(InputTest, ConsumedReleaseCompletesOnlyTheOriginalClientPress)
{
    auto mine = compositor.createSurface();
    auto other = compositor.createSurface();
    map(mine.get(), {0, 0}, {100, 100});
    map(other.get(), {200, 0}, {100, 100});
    FakePointerDelegate first, second;
    first.acceptable = {mine.get()};
    second.acceptable = {other.get()};
    Pointer firstPointer(&first, &seat), secondPointer(&second, &seat);
    InputManager manager(&seat);
    manager.onPointerMotionAbsolute(at(1), {50, 50});
    seat.setFocusedSurface(mine.get());
    manager.onPointerButton(at(2), BTN_LEFT, true);
    ASSERT_EQ(first.buttons.size(), 1u);
    const auto firstEvents = first.calls.size();
    const auto secondEvents = second.calls.size();
    const auto selectionGeneration = seat.selection()->generation();

    struct Sink : EventSink {
        Seat* seat = nullptr;

        bool onPointerButton(Timestamp, const base::PointF&, uint32_t button, bool pressed) override
        {
            if (!pressed)
                EXPECT_EQ(seat->pressedButtons() & buttonFlagFromEvdev(button), NoButton);
            return true;
        }
    } sink;

    sink.seat = &seat;
    manager.setEventSink(&sink);
    manager.warpPointer({250, 50});
    manager.onPointerButton(at(3), BTN_RIGHT, false);
    EXPECT_EQ(first.calls.size(), firstEvents);
    EXPECT_EQ(seat.pressedButtons(), LeftButton);
    manager.onPointerButton(at(4), BTN_LEFT, false);
    ASSERT_EQ(first.buttons.size(), 2u);
    EXPECT_EQ(first.buttons.back(), std::make_pair(ButtonFlags(LeftButton), false));
    EXPECT_EQ(second.calls.size(), secondEvents);
    EXPECT_EQ(firstPointer.focusedSurface(), mine.get());
    EXPECT_EQ(seat.focusedSurface(), mine.get());
    EXPECT_EQ(seat.selection()->generation(), selectionGeneration);
    EXPECT_EQ(seat.pressedButtons(), NoButton);
    manager.onPointerButton(at(5), BTN_LEFT, false);
    EXPECT_EQ(first.buttons.size(), 2u);

    // A shell-owned press was never delivered and must not manufacture a release.
    manager.onPointerButton(at(6), BTN_LEFT, true);
    manager.onPointerButton(at(7), BTN_LEFT, false);
    EXPECT_EQ(first.buttons.size(), 2u);
    EXPECT_EQ(second.calls.size(), secondEvents);

    // Destruction during a drag removes the pending client recipient.
    manager.setEventSink(nullptr);
    manager.onPointerMotionAbsolute(at(8), {50, 50});
    manager.onPointerButton(at(9), BTN_LEFT, true);
    manager.setEventSink(&sink);
    mine.reset();
    manager.onPointerButton(at(10), BTN_LEFT, false);
    EXPECT_EQ(seat.pressedButtons(), NoButton);
    EXPECT_EQ(second.calls.size(), secondEvents);
}

TEST_F(InputTest, PairedReleaseMayDestroyItsPointer)
{
    auto surface = compositor.createSurface();
    map(surface.get(), {}, {100, 100});
    std::unique_ptr<Pointer> pointer;

    struct DestroyingDelegate : FakePointerDelegate {
        std::unique_ptr<Pointer>* pointer;

        void onPointerButton(Timestamp, ButtonFlags, bool pressed) override
        {
            if (!pressed)
                pointer->reset();
        }
    } delegate;

    delegate.pointer = &pointer;
    pointer = std::make_unique<Pointer>(&delegate, &seat);
    seat.onPointerMotion(at(1), {50, 50});
    seat.onPointerButton(at(2), {50, 50}, BTN_LEFT, true);
    const auto frames = delegate.frames;
    seat.releasePointerButton(at(3), BTN_LEFT);
    EXPECT_FALSE(pointer);
    EXPECT_TRUE(delegate.destroying);
    EXPECT_EQ(delegate.frames, frames);
    EXPECT_EQ(seat.pressedButtons(), NoButton);
}

// Exo's OnKeyboardKey_ChangeFocusInPreTargetHandler: a handler that moves
// focus while it takes a key must leave the new surface with a consistent
// view of what is held.
TEST_F(InputTest, FocusMovedWhileAKeyIsConsumedKeepsTheNewSurfaceConsistent)
{
    auto first = compositor.createSurface();
    auto second = compositor.createSurface();
    map(first.get(), base::Point(0, 0), base::Size(100, 100));
    map(second.get(), base::Point(200, 0), base::Size(100, 100));
    auto delegate = std::make_unique<FakeKeyboardDelegate>();
    FakeKeyboardDelegate* raw = delegate.get();
    Keyboard keyboard(std::move(delegate), &seat);
    seat.setFocusedSurface(first.get());
    raw->keys.clear();

    struct Sink : EventSink {
        Seat* seat = nullptr;
        compositor::Surface* target = nullptr;

        bool onKey(Timestamp, uint32_t code, bool pressed) override
        {
            if (code != KEY_A)
                return false;
            if (pressed)
                seat->setFocusedSurface(target);
            return true;
        }
    } sink;

    sink.seat = &seat;
    sink.target = second.get();
    InputManager manager(&seat);
    manager.setEventSink(&sink);
    manager.onKey(at(1), KEY_A, true);
    EXPECT_EQ(second.get(), keyboard.focusedSurface());
    ASSERT_EQ(1u, raw->leaves.size());
    ASSERT_EQ(2u, raw->enters.size());
    manager.onKey(at(2), KEY_A, false);
    manager.setEventSink(nullptr);

    // Neither the press nor a release the surface could not pair reaches it.
    size_t presses = 0, releases = 0;
    for (const auto& key : raw->keys) {
        if (key.code == KEY_A)
            ++(key.pressed ? presses : releases);
    }
    EXPECT_EQ(0u, presses);
    EXPECT_EQ(raw->enterKeys.count(KEY_A), releases);

    // Keys after the change go to the new surface as usual.
    raw->keys.clear();
    manager.onKey(at(3), KEY_B, true);
    manager.onKey(at(4), KEY_B, false);
    ASSERT_EQ(2u, raw->keys.size());
    EXPECT_EQ(KEY_B, raw->keys[0].code);
    EXPECT_TRUE(raw->keys[0].pressed);
}
