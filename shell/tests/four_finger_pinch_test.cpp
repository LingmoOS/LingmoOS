// SPDX-License-Identifier: GPL-3.0-or-later
#include <gtest/gtest.h>
#include "wm/gestures/wm_gesture_handler.h"
#include <vector>

using namespace Lingmo;

TEST(FourFingerPinch, ClaimsOnlyFourFingerPinchesAndMapsBothDirectionsContinuously)
{
    wm::WmGestureHandler handler(nullptr);
    std::vector<double> updates;
    bool beganIn = false;
    std::vector<bool> ended;
    handler.setPinchActions({[&](bool inward) { beganIn = inward; return true; },
                             [&](double progress) { updates.push_back(progress); },
                             [&](bool cancelled) { ended.push_back(cancelled); }});
    const input::Timestamp at(std::chrono::milliseconds(1));
    EXPECT_FALSE(handler.claimGesture(at, input::GestureType::Pinch, 3));
    EXPECT_FALSE(handler.claimGesture(at, input::GestureType::Swipe, 4));
    ASSERT_TRUE(handler.claimGesture(at, input::GestureType::Pinch, 4));
    handler.onGestureUpdate(at, {{}, {}, 0.98, 0}); // Ignore initial noise.
    EXPECT_TRUE(updates.empty());
    handler.onGestureUpdate(at, {{}, {}, 0.775, 0});
    EXPECT_TRUE(beganIn);
    EXPECT_NEAR(0.5, updates.back(), 1e-9);
    handler.onGestureUpdate(at, {{}, {}, 0.55, 0});
    EXPECT_DOUBLE_EQ(1, updates.back());
    handler.onGestureUpdate(at, {{}, {}, 0.82, 0}); // Reverse before releasing.
    EXPECT_NEAR(0.4, updates.back(), 1e-9);
    handler.onGestureEnd(at, true);
    ASSERT_EQ(ended.size(), 1u);
    EXPECT_TRUE(ended.back());

    ASSERT_TRUE(handler.claimGesture(at, input::GestureType::Pinch, 4));
    updates.clear(); ended.clear();
    handler.onGestureUpdate(at, {{}, {}, 1.275, 0});
    EXPECT_FALSE(beganIn);
    EXPECT_NEAR(0.5, updates.back(), 1e-9);
    handler.onGestureEnd(at, false);
    EXPECT_FALSE(ended.back());

    int begins = 0;
    handler.setPinchActions({[&](bool) { ++begins; return false; },
                             [&](double) { ADD_FAILURE() << "ignored pinch updated"; },
                             [&](bool) { ADD_FAILURE() << "ignored pinch ended as handled"; }});
    ASSERT_TRUE(handler.claimGesture(at, input::GestureType::Pinch, 4));
    handler.onGestureUpdate(at, {{}, {}, 0.5, 0});
    handler.onGestureEnd(at, false);
    EXPECT_EQ(1, begins);
}

TEST(FourFingerPinch, DragAndLateUpdatesDoNotInvokeActions)
{
    bool dragging = true;
    int calls = 0;
    wm::WmGestureHandler handler(nullptr, [&] { return dragging; });
    handler.setPinchActions({[&](bool) { ++calls; return true; },
                             [&](double) { ++calls; }, [&](bool) { ++calls; }});
    const input::Timestamp at(std::chrono::milliseconds(1));
    EXPECT_TRUE(handler.claimGesture(at, input::GestureType::Pinch, 4));
    dragging = false;
    handler.onGestureUpdate(at, {{}, {}, 0.5, 0});
    handler.onGestureEnd(at, false);
    EXPECT_EQ(calls, 0);
    EXPECT_TRUE(handler.claimGesture(at, input::GestureType::Pinch, 4));
    handler.onGestureUpdate(at, {{}, {}, 0.5, 0});
    EXPECT_EQ(calls, 2);
    handler.onGestureEnd(at, true);
    EXPECT_EQ(calls, 3);
}
