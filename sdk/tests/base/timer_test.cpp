// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include "base/event_loop.h"
#include "base/timer.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

TEST(TimerTest, CurrentLoopFollowsCreation)
{
    EventLoop outer;
    EXPECT_EQ(EventLoop::current(), &outer);
    {
        EventLoop inner;
        EXPECT_EQ(EventLoop::current(), &inner);
    }
    EXPECT_EQ(EventLoop::current(), &outer);
}

TEST(TimerTest, SingleShotFiresOnce)
{
    EventLoop loop;
    Timer timer;
    int count = 0;
    timer.setSingleShot(true);
    timer.onTimeout = [&] { ++count; };
    timer.start(2ms);
    EXPECT_TRUE(timer.isActive());
    loop.startTimer(30ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_EQ(count, 1);
    EXPECT_FALSE(timer.isActive());
}

TEST(TimerTest, RestartPostponesAndStopCancels)
{
    EventLoop loop;
    Timer timer;
    timer.setSingleShot(true);
    int count = 0;
    timer.onTimeout = [&] { ++count; };
    timer.start(20ms);
    loop.startTimer(10ms, false, [&] { timer.start(); });
    loop.startTimer(25ms, false, [&] { EXPECT_EQ(count, 0); });
    loop.startTimer(45ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_EQ(count, 1);

    timer.start(5ms);
    timer.stop();
    loop.startTimer(20ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_EQ(count, 1);
}

TEST(TimerTest, RepeatingTimerMayDestroyItself)
{
    EventLoop loop;
    auto timer = std::make_unique<Timer>(&loop);
    int count = 0;
    timer->onTimeout = [&] {
        if (++count == 3)
            timer.reset();
    };
    timer->start(2ms);
    loop.startTimer(40ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_EQ(count, 3);
}

TEST(TimerTest, SingleShotFlagIsReadAtEachExpiry)
{
    EventLoop loop;
    int count = 0;
    {
        auto timer = std::make_unique<Timer>(&loop);
        timer->onTimeout = [&] { ++count; };
        timer->start(2ms);
        // Set after start: the timer still fires once and stops.
        timer->setSingleShot(true);
        loop.startTimer(30ms, false, [&] { loop.quit(); });
        loop.run();
        EXPECT_EQ(count, 1);
        EXPECT_FALSE(timer->isActive());
    }

    // The other way: a single-shot timer made repeating before it fires
    // keeps going.
    count = 0;
    Timer repeating(&loop);
    repeating.setSingleShot(true);
    repeating.onTimeout = [&] {
        if (++count == 3) {
            repeating.stop();
            loop.quit();
        }
    };
    repeating.start(2ms);
    repeating.setSingleShot(false);
    loop.startTimer(2s, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_EQ(count, 3);
}

TEST(TimerTest, DestroyedOwnerIsNeverCalled)
{
    EventLoop loop;
    bool fired = false;
    {
        Timer timer(&loop);
        timer.onTimeout = [&] { fired = true; };
        timer.start(1ms);
        timer.setSingleShot(true);
    }
    loop.startTimer(20ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_FALSE(fired);
}

TEST(TimerTest, RestartsMoveTheDeadlineBothWays)
{
    EventLoop loop;
    Timer timer;
    timer.setSingleShot(true);
    std::chrono::steady_clock::time_point fired;
    timer.onTimeout = [&] { fired = std::chrono::steady_clock::now(); };
    const auto start = std::chrono::steady_clock::now();
    timer.start(10ms);
    // Pushed back every 2 ms for 30 ms, as a burst of reads restarts a timer.
    int restarts = 0;
    const int pusher = loop.startTimer(2ms, true, [&] {
        if (std::chrono::steady_clock::now() - start < 30ms) {
            timer.start(10ms);
            ++restarts;
        }
    });
    loop.startTimer(80ms, false, [&] { loop.quit(); });
    loop.run();
    loop.stopTimer(pusher);
    EXPECT_GT(restarts, 5);
    ASSERT_NE(fired.time_since_epoch().count(), 0);
    // Not before the last restart's deadline, which is past 30 ms.
    EXPECT_GE(fired - start, 30ms);
    EXPECT_LT(fired - start, 60ms);

    // A shorter interval brings it forward.
    fired = {};
    const auto again = std::chrono::steady_clock::now();
    timer.start(50ms);
    loop.startTimer(5ms, false, [&] { timer.start(5ms); });
    loop.startTimer(40ms, false, [&] { loop.quit(); });
    loop.run();
    ASSERT_NE(fired.time_since_epoch().count(), 0);
    EXPECT_LT(fired - again, 30ms);
}
