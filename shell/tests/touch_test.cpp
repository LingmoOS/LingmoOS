// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Lingmo counterparts to chromium/src/components/exo/touch_unittest.cc,
// against a fake delegate, and the touch half of InputManager.
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <algorithm>
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "input/event_sink.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "input/touch.h"
#include "input/touch_delegate.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shm_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Input;

namespace {

Timestamp at(int ms)
{
    return Timestamp(std::chrono::milliseconds(ms));
}

class FakeTouchDelegate : public TouchDelegate
{
public:
    struct Down {
        compositor::Surface* surface;
        int32_t id;
        base::PointF location;
    };

    std::vector<std::string> calls;
    std::vector<Down> downs;
    std::vector<std::pair<int32_t, base::PointF>> motions;
    std::vector<int32_t> ups;
    // Surfaces this delegate will accept; empty means all of them.
    std::vector<compositor::Surface*> acceptable;

    void onTouchDestroying(Touch*) override { }

    bool canAcceptTouchEventsForSurface(compositor::Surface* surface) const override
    {
        return acceptable.empty()
                || std::find(acceptable.begin(), acceptable.end(), surface) != acceptable.end();
    }

    void onTouchDown(compositor::Surface* surface, Timestamp, int32_t id,
                     const base::PointF& location) override
    {
        calls.push_back("down");
        downs.push_back({surface, id, location});
    }

    void onTouchUp(Timestamp, int32_t id) override
    {
        calls.push_back("up");
        ups.push_back(id);
    }

    void onTouchMotion(Timestamp, int32_t id, const base::PointF& location) override
    {
        calls.push_back("motion");
        motions.push_back({id, location});
    }

    void onTouchFrame() override { calls.push_back("frame"); }

    void onTouchCancel() override { calls.push_back("cancel"); }
};

class TouchTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node* surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);
    Seat seat {&compositor};

    void SetUp() override { scene.setSize(base::Size(800, 600)); }

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

using Calls = std::vector<std::string>;

} // namespace

TEST_F(TouchTest, OnTouchDown)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(50, 40));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    // Nothing is under the point: no client hears of it.
    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchFrame();
    EXPECT_TRUE(delegate.calls.empty());

    seat.onTouchDown(at(2), 2, base::PointF(120, 110));
    seat.onTouchFrame();
    ASSERT_EQ(1u, delegate.downs.size());
    EXPECT_EQ(surface.get(), delegate.downs[0].surface);
    EXPECT_EQ(2, delegate.downs[0].id);
    EXPECT_EQ(base::PointF(20, 10), delegate.downs[0].location);
    EXPECT_EQ(Calls({"down", "frame"}), delegate.calls);
}

TEST_F(TouchTest, OnTouchUp)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchDown(at(1), 2, base::PointF(20, 20));
    seat.onTouchFrame();
    seat.onTouchUp(at(2), 1);
    seat.onTouchFrame();
    seat.onTouchUp(at(3), 2);
    seat.onTouchFrame();
    EXPECT_EQ(Calls({"down", "down", "frame", "up", "frame", "up", "frame"}), delegate.calls);
    EXPECT_EQ(std::vector<int32_t>({1, 2}), delegate.ups);
    EXPECT_FALSE(touch.tracks(1));
}

TEST_F(TouchTest, OnTouchMotionStaysWithTheSurfaceItWentDownOn)
{
    auto surface = compositor.createSurface();
    auto other = compositor.createSurface();
    map(surface.get(), base::Point(100, 100), base::Size(50, 50));
    map(other.get(), base::Point(300, 100), base::Size(50, 50));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(110, 110));
    seat.onTouchFrame();
    // Over another surface, and off every surface, the point still reports
    // relative to where it went down.
    seat.onTouchMotion(at(2), 1, base::PointF(310, 120));
    seat.onTouchMotion(at(3), 1, base::PointF(50, 60));
    seat.onTouchFrame();
    ASSERT_EQ(2u, delegate.motions.size());
    EXPECT_EQ(base::PointF(210, 20), delegate.motions[0].second);
    EXPECT_EQ(base::PointF(-50, -40), delegate.motions[1].second);
    EXPECT_EQ(Calls({"down", "frame", "motion", "motion", "frame"}), delegate.calls);

    // A point that never went down on this client's surfaces is not reported.
    seat.onTouchMotion(at(4), 7, base::PointF(120, 120));
    seat.onTouchFrame();
    EXPECT_EQ(2u, delegate.motions.size());
}

TEST_F(TouchTest, OnTouchCancel)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchDown(at(1), 2, base::PointF(20, 20));
    seat.onTouchFrame();
    seat.onTouchCancel();
    EXPECT_EQ(Calls({"down", "down", "frame", "cancel"}), delegate.calls);
    // Nothing more is sent for the cancelled points.
    seat.onTouchUp(at(2), 1);
    seat.onTouchMotion(at(2), 2, base::PointF(30, 30));
    seat.onTouchFrame();
    EXPECT_EQ(Calls({"down", "down", "frame", "cancel"}), delegate.calls);
    // A cancel with nothing down tells the client nothing.
    seat.onTouchCancel();
    EXPECT_EQ(4u, delegate.calls.size());
}

TEST_F(TouchTest, OnTouchCancelWhenSurfaceDestroying)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchFrame();
    surface.reset();
    EXPECT_EQ(Calls({"down", "frame", "cancel"}), delegate.calls);
    seat.onTouchUp(at(2), 1);
    seat.onTouchFrame();
    EXPECT_EQ(3u, delegate.calls.size());
}

TEST_F(TouchTest, OnTouchCancelNotTriggeredAfterTouchReleased)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchFrame();
    seat.onTouchUp(at(2), 1);
    seat.onTouchFrame();
    surface.reset();
    EXPECT_EQ(Calls({"down", "frame", "up", "frame"}), delegate.calls);
}

TEST_F(TouchTest, TouchMultiple2Surfaces)
{
    auto first = compositor.createSurface();
    auto second = compositor.createSurface();
    map(first.get(), base::Point(0, 0), base::Size(100, 100));
    map(second.get(), base::Point(200, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchDown(at(1), 2, base::PointF(210, 10));
    seat.onTouchFrame();
    ASSERT_EQ(2u, delegate.downs.size());
    EXPECT_EQ(first.get(), delegate.downs[0].surface);
    EXPECT_EQ(second.get(), delegate.downs[1].surface);
    // One of them going cancels both, as upstream does.
    second.reset();
    EXPECT_EQ(Calls({"down", "down", "frame", "cancel"}), delegate.calls);
    EXPECT_FALSE(touch.tracks(1));
}

TEST_F(TouchTest, ATouchIgnoresSurfacesItsClientDoesNotOwn)
{
    auto mine = compositor.createSurface();
    auto theirs = compositor.createSurface();
    map(mine.get(), base::Point(0, 0), base::Size(100, 100));
    map(theirs.get(), base::Point(200, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    delegate.acceptable = {mine.get()};
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(250, 50));
    seat.onTouchFrame();
    EXPECT_TRUE(delegate.calls.empty());
    seat.onTouchDown(at(2), 2, base::PointF(50, 50));
    seat.onTouchFrame();
    ASSERT_EQ(1u, delegate.downs.size());
    EXPECT_EQ(mine.get(), delegate.downs[0].surface);
}

TEST_F(TouchTest, APointGoesDownOnTheSubsurfaceItHits)
{
    auto parent = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    sub->setPosition(base::Point(50, 60));
    map(child.get(), base::Point(0, 0), base::Size(40, 40));
    map(parent.get(), base::Point(0, 0), base::Size(200, 200));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(60, 70));
    seat.onTouchFrame();
    ASSERT_EQ(1u, delegate.downs.size());
    EXPECT_EQ(child.get(), delegate.downs[0].surface);
    EXPECT_EQ(base::PointF(10, 10), delegate.downs[0].location);
}

TEST_F(TouchTest, TheSeatFindsTheNewestPointOnAWindow)
{
    auto parent = compositor.createSurface();
    auto child = compositor.createSurface();
    auto sub = compositor::SubSurface::create(child.get(), parent.get());
    sub->setPosition(base::Point(50, 60));
    map(child.get(), base::Point(0, 0), base::Size(40, 40));
    map(parent.get(), base::Point(0, 0), base::Size(200, 200));
    auto other = compositor.createSurface();
    map(other.get(), base::Point(300, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    EXPECT_FALSE(seat.touchPointOn(parent.get()));
    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    // A point on a subsurface is on the window too.
    seat.onTouchDown(at(2), 2, base::PointF(60, 70));
    seat.onTouchDown(at(3), 3, base::PointF(310, 10));
    seat.onTouchMotion(at(4), 2, base::PointF(65, 75));
    auto point = seat.touchPointOn(parent.get());
    ASSERT_TRUE(point);
    EXPECT_EQ(2, point->id);
    EXPECT_EQ(base::PointF(65, 75), point->location);
    EXPECT_EQ(3, seat.touchPointOn(other.get())->id);

    seat.onTouchUp(at(5), 2);
    EXPECT_EQ(1, seat.touchPointOn(parent.get())->id);
    EXPECT_FALSE(seat.touchPointOn(child.get()));
}

TEST_F(TouchTest, TakingAPointCancelsTheClientsThatHaveIt)
{
    auto first = compositor.createSurface();
    map(first.get(), base::Point(0, 0), base::Size(100, 100));
    auto second = compositor.createSurface();
    map(second.get(), base::Point(200, 0), base::Size(100, 100));
    FakeTouchDelegate firstClient;
    firstClient.acceptable = {first.get()};
    Touch firstTouch(&firstClient, &seat);
    FakeTouchDelegate secondClient;
    secondClient.acceptable = {second.get()};
    Touch secondTouch(&secondClient, &seat);

    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchDown(at(1), 2, base::PointF(20, 20));
    seat.onTouchDown(at(1), 3, base::PointF(210, 10));
    seat.onTouchFrame();
    seat.takeTouchPoint(1);
    EXPECT_EQ("cancel", firstClient.calls.back());
    EXPECT_EQ(Calls({"down", "frame"}), secondClient.calls);
    // The cancel ended the client's other point as well; nobody has either now.
    EXPECT_FALSE(seat.touchPointOn(first.get()));
    EXPECT_EQ(3, seat.touchPointOn(second.get())->id);

    seat.onTouchMotion(at(2), 1, base::PointF(30, 30));
    seat.onTouchUp(at(2), 2);
    seat.onTouchFrame();
    EXPECT_EQ("cancel", firstClient.calls.back());
}

TEST_F(TouchTest, AFrameOnlyFollowsSomethingSent)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);

    seat.onTouchFrame();
    EXPECT_TRUE(delegate.calls.empty());
    // Several points of one device frame make one client frame.
    seat.onTouchDown(at(1), 1, base::PointF(10, 10));
    seat.onTouchDown(at(1), 2, base::PointF(20, 20));
    seat.onTouchMotion(at(1), 1, base::PointF(11, 11));
    seat.onTouchFrame();
    seat.onTouchFrame();
    EXPECT_EQ(Calls({"down", "down", "motion", "frame"}), delegate.calls);
}

namespace {

struct RecordingSink : EventSink {
    bool take = false;
    // Takes a point a client already has, from its next motion on.
    bool takeOnMotion = false;
    std::vector<std::string> calls;
    std::vector<std::pair<uint32_t, bool>> buttons;

    bool onPointerMotion(Timestamp, const base::PointF&) override
    {
        calls.push_back("pointer-motion");
        return true;
    }

    bool onPointerButton(Timestamp, const base::PointF&, uint32_t code, bool pressed) override
    {
        calls.push_back("pointer-button");
        buttons.push_back({code, pressed});
        return true;
    }

    bool onTouchDown(Timestamp, int32_t, const base::PointF&) override
    {
        calls.push_back("touch-down");
        return take;
    }

    bool onTouchMotion(Timestamp, int32_t id, const base::PointF&) override
    {
        calls.push_back("touch-motion " + std::to_string(id));
        return takeOnMotion;
    }

    bool onTouchUp(Timestamp, int32_t id) override
    {
        calls.push_back("touch-up " + std::to_string(id));
        return false;
    }

    void onTouchCancel(Timestamp) override { calls.push_back("touch-cancel"); }
};

class LockedGate : public SessionGate
{
public:
    explicit LockedGate(EventSink* sink)
        : m_sink(sink)
    {
    }

    bool restrictsInput() const override { return locked; }

    EventSink* secureInput() const override { return m_sink; }

    scene::Node* inputRoot() const override { return nullptr; }

    bool allowsFocus(const compositor::Surface*) const override { return !locked; }

    bool allowsActivation() const override { return !locked; }

    bool locked = false;

private:
    EventSink* m_sink;
};

} // namespace

TEST_F(TouchTest, ThePanelSpansTheDesktopWithoutDisplayConfiguration)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(400, 300), base::Size(100, 100));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));

    manager.onTouchDown(at(1), 1, base::PointF(0.5, 0.5));
    manager.onTouchFrame(at(1));
    ASSERT_EQ(1u, delegate.downs.size());
    EXPECT_EQ(base::PointF(0, 0), delegate.downs[0].location);
    manager.onTouchMotion(at(2), 1, base::PointF(0.75, 0.75));
    manager.onTouchUp(at(3), 1);
    manager.onTouchFrame(at(3));
    ASSERT_EQ(1u, delegate.motions.size());
    EXPECT_EQ(base::PointF(200, 150), delegate.motions[0].second);
    EXPECT_EQ(Calls({"down", "frame", "motion", "up", "frame"}), delegate.calls);
}

TEST_F(TouchTest, ATouchHidesTheCursorUntilTheMouseMoves)
{
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    EXPECT_FALSE(seat.cursorHiddenByTouch());
    manager.onTouchDown(at(1), 1, base::PointF(0.5, 0.5));
    manager.onTouchUp(at(2), 1);
    manager.onTouchFrame(at(2));
    EXPECT_TRUE(seat.cursorHiddenByTouch());
    manager.onPointerMotion(at(3), base::PointF(1, 0));
    EXPECT_FALSE(seat.cursorHiddenByTouch());
}

TEST_F(TouchTest, TheEventSinkCanTakeAWholePoint)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(400, 300));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);
    RecordingSink sink;
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    manager.setEventSink(&sink);

    sink.take = true;
    manager.onTouchDown(at(1), 1, base::PointF(0.1, 0.1));
    // A second point the sink leaves alone goes to the client.
    sink.take = false;
    manager.onTouchDown(at(1), 2, base::PointF(0.2, 0.2));
    manager.onTouchFrame(at(1));
    manager.onTouchMotion(at(2), 1, base::PointF(0.3, 0.3));
    manager.onTouchUp(at(3), 1);
    manager.onTouchUp(at(3), 2);
    manager.onTouchFrame(at(3));
    // The sink is asked about the client's point too, and leaves it.
    EXPECT_EQ(Calls({"touch-down", "touch-down", "touch-motion 1", "touch-up 1", "touch-up 2"}),
              sink.calls);
    ASSERT_EQ(1u, delegate.downs.size());
    EXPECT_EQ(2, delegate.downs[0].id);
    EXPECT_EQ(std::vector<int32_t>({2}), delegate.ups);
}

TEST_F(TouchTest, TheEventSinkCanTakeAPointAClientHas)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(400, 300));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);
    RecordingSink sink;
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));
    manager.setEventSink(&sink);

    manager.onTouchDown(at(1), 1, base::PointF(0.1, 0.1));
    manager.onTouchMotion(at(2), 1, base::PointF(0.15, 0.1));
    manager.onTouchFrame(at(2));
    sink.takeOnMotion = true;
    manager.onTouchMotion(at(3), 1, base::PointF(0.2, 0.1));
    sink.takeOnMotion = false;
    manager.onTouchMotion(at(4), 1, base::PointF(0.25, 0.1));
    manager.onTouchUp(at(5), 1);
    manager.onTouchFrame(at(5));
    EXPECT_EQ(Calls({"touch-down", "touch-motion 1", "touch-motion 1", "touch-motion 1",
                     "touch-up 1"}),
              sink.calls);
    // Once taken, the rest of the point stays with the sink.
    EXPECT_EQ(Calls({"down", "motion", "frame"}), delegate.calls);
}

TEST_F(TouchTest, ALockedSessionTakesTheFirstPointAsItsPointer)
{
    auto surface = compositor.createSurface();
    map(surface.get(), base::Point(0, 0), base::Size(400, 300));
    FakeTouchDelegate delegate;
    Touch touch(&delegate, &seat);
    RecordingSink lockScreen;
    LockedGate gate(&lockScreen);
    seat.setSessionGate(&gate);
    InputManager manager(&seat);
    manager.setBounds(base::Size(800, 600));

    // A point down before the lock is cancelled for its client.
    manager.onTouchDown(at(1), 1, base::PointF(0.1, 0.1));
    manager.onTouchFrame(at(1));
    gate.locked = true;
    manager.onTouchDown(at(2), 2, base::PointF(0.5, 0.5));
    manager.onTouchDown(at(2), 3, base::PointF(0.6, 0.6));
    manager.onTouchMotion(at(3), 2, base::PointF(0.55, 0.5));
    manager.onTouchUp(at(4), 3);
    manager.onTouchUp(at(4), 2);
    manager.onTouchFrame(at(4));
    EXPECT_EQ(Calls({"down", "frame", "cancel"}), delegate.calls);
    // Only the first point of the locked session is a pointer there.
    EXPECT_EQ(Calls({"pointer-motion", "pointer-button", "pointer-motion", "pointer-button"}),
              lockScreen.calls);
    EXPECT_EQ((std::vector<std::pair<uint32_t, bool>>({{BTN_LEFT, true}, {BTN_LEFT, false}})),
              lockScreen.buttons);
    seat.setSessionGate(nullptr);
}
