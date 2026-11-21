// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <deque>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "fake_renderer.h"
#include "render/output_scheduler.h"
#include "scene/node.h"
#include "scene/surface_node.h"
#include "shm_test_util.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using Lingmo::render::OutputScheduler;
using namespace std::chrono_literals;
using Reason = OutputScheduler::Reason;
using State = OutputScheduler::State;

struct SchedulerTest : testing::Test {
    OutputScheduler::Time now {};
    OutputScheduler scheduler {10ms, [&] {
                                   return now;
                               }};
    uint64_t pending = 0;
    int renders = 0, commits = 0;

    void submit()
    {
        ASSERT_EQ(pending, 0u);
        pending = scheduler.begin();
        ASSERT_NE(pending, 0u);
        ++renders;
        ASSERT_TRUE(scheduler.submitted(pending));
        ++commits;
    }

    void flip()
    {
        ASSERT_TRUE(scheduler.complete(pending));
        pending = 0;
    }

    void start()
    {
        scheduler.resume();
        submit();
        flip();
    }
};

TEST_F(SchedulerTest, ControlledIdleHasNoWakeupsOrSubmissions)
{
    start();
    now += 10s;
    EXPECT_FALSE(scheduler.deadline());
    EXPECT_EQ(scheduler.begin(), 0u);
    EXPECT_EQ(renders, 1);
    EXPECT_EQ(commits, 1);
}

TEST_F(SchedulerTest, BurstCoalescesAndNeverPostponesDeadline)
{
    start();
    scheduler.request(Reason::Scene);
    const auto deadline = scheduler.deadline();
    for (int i = 0; i < 100; ++i) {
        now += 50us;
        scheduler.request(Reason::Scene);
        EXPECT_EQ(deadline, scheduler.deadline());
    }
    now = *deadline;
    submit();
    flip();
    EXPECT_EQ(commits, 2);
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(SchedulerTest, RenderingAndFlipRequestsSurviveSnapshot)
{
    scheduler.resume();
    auto id = scheduler.begin();
    scheduler.request(Reason::Scene);
    ASSERT_TRUE(scheduler.submitted(id));
    scheduler.request(Reason::Scene);
    EXPECT_FALSE(scheduler.deadline());
    ASSERT_TRUE(scheduler.complete(id));
    ASSERT_TRUE(scheduler.deadline());
    now = *scheduler.deadline();
    submit();
    flip();
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(SchedulerTest, ImmediateCompletionsCannotDriveUnlimitedFrames)
{
    start();
    for (int i = 0; i < 1000; ++i) {
        scheduler.request(Reason::Surface);
        EXPECT_EQ(scheduler.begin(), 0u);
    }
    EXPECT_EQ(commits, 1);
    now += 10ms;
    submit();
    flip();
    EXPECT_EQ(commits, 2);
}

TEST_F(SchedulerTest, PacedCompletionsDoNotHalveThroughput)
{
    scheduler.resume();
    for (int i = 0; i < 100; ++i) {
        submit();
        scheduler.request(Reason::Surface);
        now += 10ms;
        flip();
        EXPECT_EQ(*scheduler.deadline(), now);
    }
    EXPECT_EQ(commits, 100);
}

TEST_F(SchedulerTest, LateWakeupSkipsMissedSlotsWithoutBurst)
{
    start();
    scheduler.request(Reason::Scene);
    now += 105ms;
    submit();
    scheduler.request(Reason::Scene);
    flip();
    EXPECT_EQ(*scheduler.deadline(), OutputScheduler::Time {} + 110ms);
    EXPECT_EQ(scheduler.begin(), 0u);
}

TEST_F(SchedulerTest, FrameTimeIsTheNominalSlotNotTheWakeup)
{
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline() + 3ms;
    submit();
    // The wakeup ran 3 ms into the slot; the frame is still that slot's.
    EXPECT_EQ(scheduler.frameTime(), OutputScheduler::Time {} + 10ms);
    now += 6ms;
    flip();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline() + 1ms;
    submit();
    EXPECT_EQ(scheduler.frameTime(), OutputScheduler::Time {} + 20ms);
    now += 10s;
    flip();
    scheduler.request(Reason::Scene);
    submit();
    // After a gap the slot starts where the request woke the output.
    EXPECT_EQ(scheduler.frameTime(), now);
    flip();
    scheduler.request(Reason::Scene);
    const auto deadline = *scheduler.deadline();
    now = deadline + 35ms;
    // Missed slots are skipped: the frame belongs to the slot the wakeup fell in.
    submit();
    EXPECT_EQ(scheduler.frameTime(), deadline + 30ms);
}

TEST_F(SchedulerTest, TwoPendingFramesRenderTheNextSlotBeforeTheFlipLands)
{
    scheduler.setMaxPendingFrames(2);
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    const auto first = pending;
    pending = 0;
    // No flip yet, but the next slot's demand is served anyway.
    scheduler.request(Reason::Scene);
    ASSERT_TRUE(scheduler.deadline());
    EXPECT_EQ(*scheduler.deadline(), now + 10ms);
    EXPECT_EQ(scheduler.state(), State::Scheduled);
    now = *scheduler.deadline();
    submit();
    const auto second = pending;
    pending = 0;
    EXPECT_EQ(scheduler.pendingFrames(), 2u);
    // Two are on their way; a third waits for the oldest to land.
    scheduler.request(Reason::Scene);
    EXPECT_EQ(scheduler.state(), State::FlipPending);
    EXPECT_FALSE(scheduler.deadline());
    // Completions arrive in order only.
    EXPECT_FALSE(scheduler.complete(second));
    now += 4ms;
    ASSERT_TRUE(scheduler.complete(first));
    EXPECT_EQ(scheduler.pendingFrames(), 1u);
    ASSERT_TRUE(scheduler.deadline());
    EXPECT_EQ(*scheduler.deadline(), OutputScheduler::Time {} + 30ms);
    now = *scheduler.deadline();
    submit();
    pending = 0;
    ASSERT_TRUE(scheduler.complete(second));
    EXPECT_EQ(scheduler.stats().missed, 0u);
}

TEST_F(SchedulerTest, ARefusedPendingFrameLeavesTheQueueAndBacksOff)
{
    scheduler.setMaxPendingFrames(2);
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    const auto first = pending;
    pending = 0;
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    const auto second = pending;
    pending = 0;
    ASSERT_TRUE(scheduler.complete(first));
    scheduler.failed(second);
    EXPECT_EQ(scheduler.pendingFrames(), 0u);
    EXPECT_EQ(scheduler.stats().failures, 1u);
    EXPECT_FALSE(scheduler.complete(second));
    scheduler.request(Reason::Scene);
    EXPECT_GE(*scheduler.deadline(), now + 50ms);
}

TEST_F(SchedulerTest, LoweringTheLimitThrottlesUntilTheQueueDrains)
{
    scheduler.setMaxPendingFrames(2);
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    const auto first = pending;
    pending = 0;
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    const auto second = pending;
    pending = 0;
    scheduler.setMaxPendingFrames(1);
    scheduler.request(Reason::Scene);
    EXPECT_EQ(scheduler.state(), State::FlipPending);
    ASSERT_TRUE(scheduler.complete(first));
    EXPECT_EQ(scheduler.state(), State::FlipPending);
    ASSERT_TRUE(scheduler.complete(second));
    EXPECT_EQ(scheduler.state(), State::Scheduled);
}

TEST_F(SchedulerTest, CompletionsPastTheAimedSlotCountAsMissed)
{
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now += 14ms;
    flip();
    EXPECT_EQ(scheduler.stats().missed, 0u);
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now += 16ms;
    flip();
    EXPECT_EQ(scheduler.stats().missed, 1u);
}

TEST_F(SchedulerTest, StampedFlipsLayTheSlotsOnTheVblank)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    start();
    // The display flips on its own phase, 3 ms off the scheduler's.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(13ms);
    ASSERT_TRUE(scheduler.complete(pending, at(13ms)));
    pending = 0;
    // Demand early in the refresh waits until a third of a period before the
    // vblank, so later client commits still make that vblank.
    now = at(14ms);
    scheduler.request(Reason::Scene);
    const auto budget = scheduler.drawBudget();
    EXPECT_EQ(budget, OutputScheduler::Duration(10ms) / 3);
    EXPECT_EQ(*scheduler.deadline(), at(33ms) - budget);
    now = *scheduler.deadline();
    submit();
    EXPECT_EQ(scheduler.frameTime(), at(23ms));
    now = at(33ms);
    ASSERT_TRUE(scheduler.complete(pending, at(33ms)));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 0u);
    // Demand past the draw point is drawn at once, aimed at the next vblank.
    now = at(42ms);
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), now);
    submit();
    EXPECT_EQ(scheduler.frameTime(), at(33ms));
    now = at(43ms);
    ASSERT_TRUE(scheduler.complete(pending, at(43ms)));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 0u);
}

TEST_F(SchedulerTest, AWakeupJustShortOfTheDeadlineDrawsInItsSlot)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(10ms);
    ASSERT_TRUE(scheduler.complete(pending, at(10ms)));
    pending = 0;
    scheduler.request(Reason::Scene);
    const auto deadline = *scheduler.deadline();
    // A timer rounded down to whole milliseconds fires a little early.
    now = deadline - OutputScheduler::kWakeupSlack;
    EXPECT_EQ(scheduler.begin(), 0u);
    now = deadline - OutputScheduler::kWakeupSlack / 2;
    submit();
    EXPECT_EQ(scheduler.frameTime(), at(20ms));
}

TEST_F(SchedulerTest, AWakeupJustShortOfADrawPointOnTheVblankDrawsInTheNewSlot)
{
    start();
    // A full repaint as slow as a refresh is begun on the vblank itself.
    scheduler.request(Reason::Scene, true);
    now = *scheduler.deadline();
    submit();
    scheduler.drawn(pending, now + 10ms);
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    scheduler.request(Reason::Scene, true);
    const auto deadline = *scheduler.deadline();
    EXPECT_EQ(deadline, scheduler.frameTime() + 10ms);
    now = deadline - OutputScheduler::kWakeupSlack / 2;
    submit();
    EXPECT_EQ(scheduler.frameTime(), deadline);
}

TEST_F(SchedulerTest, FramesShowTheirDrawPoint)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(10ms);
    ASSERT_TRUE(scheduler.complete(pending, at(10ms)));
    pending = 0;
    const auto budget = scheduler.drawBudget();

    // A run of frames shows each at its draw point, a period apart, however
    // late or early its wakeup ran.
    for (const auto late : {0ms, 3ms, 0ms}) {
        scheduler.request(Reason::Scene);
        const auto deadline = *scheduler.deadline();
        now = deadline + late;
        submit();
        EXPECT_EQ(scheduler.sampleTime(), deadline);
        EXPECT_EQ(scheduler.sampleTime(), scheduler.frameTime() + 10ms - budget);
        now = scheduler.frameTime() + 10ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
    }
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline() - OutputScheduler::kWakeupSlack / 2;
    submit();
    EXPECT_EQ(scheduler.sampleTime(), scheduler.frameTime() + 10ms - budget);
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;

    // Demand early in an idle slot is drawn at once, and shows that moment.
    now += 1s + 1ms;
    scheduler.request(Reason::Scene);
    submit();
    EXPECT_EQ(scheduler.sampleTime(), now);
}

TEST_F(SchedulerTest, ALateFrameDoesNotQueueTheFramesAfterIt)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    scheduler.setMaxPendingFrames(2);
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(10ms);
    ASSERT_TRUE(scheduler.complete(pending, at(10ms)));
    pending = 0;
    const auto budget = scheduler.drawBudget();

    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(30ms);
    ASSERT_TRUE(scheduler.complete(pending, at(30ms)));
    pending = 0;
    ASSERT_EQ(scheduler.stats().missed, 0u);

    // Demand past the draw point is drawn at once and lands a refresh late.
    now = at(39ms);
    scheduler.request(Reason::Scene);
    submit();
    const uint64_t late = std::exchange(pending, 0);
    // The next frame would land behind it; it waits for it instead.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    EXPECT_EQ(scheduler.begin(), 0u);
    EXPECT_EQ(scheduler.state(), State::FlipPending);
    now = at(50ms);
    ASSERT_TRUE(scheduler.complete(late, at(50ms)));
    EXPECT_EQ(scheduler.stats().missed, 0u);
    EXPECT_EQ(scheduler.stats().lateDemand, 1u);
    // Begun late, it leaves the budget alone, and the next frame is drawn
    // for the slot it landed in.
    EXPECT_EQ(scheduler.drawBudget(), budget);
    EXPECT_EQ(*scheduler.deadline(), at(60ms) - budget);
    now = *scheduler.deadline();
    submit();
    now = at(60ms);
    ASSERT_TRUE(scheduler.complete(pending, at(60ms)));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 0u);
    EXPECT_EQ(scheduler.stats().lateDemand, 1u);
}

TEST_F(SchedulerTest, AFullRepaintFromIdlePastItsDrawPointBeginsAtTheNextOne)
{
    start();
    scheduler.request(Reason::Scene, true);
    now = *scheduler.deadline();
    submit();
    scheduler.drawn(pending, now + 6ms);
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    const auto slot = now + 1s;
    const auto budget = 6ms + OutputScheduler::kLatchMargin;

    // Begun at once it would miss the vblank and show a stale slot.
    now = slot + 5ms;
    scheduler.request(Reason::Scene, true);
    EXPECT_EQ(*scheduler.deadline(), slot + 20ms - budget);
    now = *scheduler.deadline();
    submit();
    EXPECT_EQ(scheduler.frameTime(), slot + 10ms);
    EXPECT_EQ(scheduler.sampleTime(), slot + 20ms - budget);
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;

    // So does one that turns full after cheaper demand was scheduled.
    now += 1s + 5ms;
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), now);
    scheduler.request(Reason::Scene, true);
    EXPECT_EQ(*scheduler.deadline(), now + 15ms - budget);
}

TEST_F(SchedulerTest, AFullRepaintAfterCheapFramesIsBegunAsEarlyAsTheLastOne)
{
    const auto frame = [this](OutputScheduler::Duration gpu, bool full) {
        scheduler.request(Reason::Scene, full);
        now = *scheduler.deadline();
        const auto began = now;
        submit();
        scheduler.drawn(pending, began + gpu);
        now = scheduler.frameTime() + 10ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
    };
    start();
    frame(1ms, false);
    frame(6ms, true);
    const auto full = 6ms + OutputScheduler::kLatchMargin;
    EXPECT_EQ(scheduler.fullRepaintDrawTime(), 6ms);
    // Cheap frames bring the budget back down...
    for (size_t i = 0; i < OutputScheduler::kDrawTimeWindow + 200; ++i)
        frame(1ms, false);
    EXPECT_EQ(scheduler.drawBudget(), OutputScheduler::Duration(10ms) / 3);
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), scheduler.frameTime() + 20ms - OutputScheduler::Duration(10ms) / 3);
    frame(1ms, false);
    // ...but a full repaint is begun as early as the last one needed, and
    // the budget stays there once it has landed.
    scheduler.request(Reason::Scene, true);
    EXPECT_EQ(*scheduler.deadline(), scheduler.frameTime() + 20ms - full);
    now = *scheduler.deadline();
    submit();
    scheduler.drawn(pending, now + 6ms);
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 0u);
    EXPECT_EQ(scheduler.drawBudget(), full);

    // Forgotten once it is kFullRepaintMemory old.
    now += OutputScheduler::kFullRepaintMemory + 1s;
    EXPECT_EQ(scheduler.fullRepaintDrawTime(), OutputScheduler::Duration::zero());
}

TEST_F(SchedulerTest, FramesSlowerThanAThirdOfARefreshAreBegunInTime)
{
    // Each frame lands on the vblank it was aimed at, `gpu` after its wakeup.
    const auto frame = [this](OutputScheduler::Duration gpu) {
        scheduler.request(Reason::Scene);
        now = *scheduler.deadline();
        const auto began = now;
        submit();
        scheduler.drawn(pending, began + gpu);
        now = scheduler.frameTime() + 10ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
    };
    start();
    frame(1ms);
    EXPECT_EQ(scheduler.drawBudget(), OutputScheduler::Duration(10ms) / 3);

    // 5 ms of GPU time is past a third of the 10 ms refresh; the next frame
    // is begun early enough for it and the flip to make the vblank.
    frame(5ms);
    const auto budget = 5ms + OutputScheduler::kLatchMargin;
    EXPECT_EQ(scheduler.drawBudget(), budget);
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), scheduler.frameTime() + 20ms - budget);

    // It holds while the slow frame is in the window, and is back as soon as
    // the frame has left it.
    for (size_t i = 0; i < OutputScheduler::kDrawTimeWindow - 1; ++i)
        frame(1ms);
    EXPECT_EQ(scheduler.drawBudget(), budget);
    frame(1ms);
    EXPECT_EQ(scheduler.drawBudget(), OutputScheduler::Duration(10ms) / 3);
    EXPECT_EQ(scheduler.stats().missed, 0u);
}

TEST_F(SchedulerTest, LateStampedFlipsBeginFramesEarlierThenWinTheTimeBack)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(10ms);
    ASSERT_TRUE(scheduler.complete(pending, at(10ms)));
    pending = 0;
    // Two frames that land a refresh late: the budget grows to the whole
    // period, drawing at the vblank itself.
    for (int i = 0; i < 2; ++i) {
        scheduler.request(Reason::Scene);
        now = *scheduler.deadline();
        submit();
        const auto aimed = scheduler.frameTime() + 10ms;
        now = aimed + 10ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
    }
    EXPECT_EQ(scheduler.stats().missed, 2u);
    EXPECT_EQ(scheduler.drawBudget(), 10ms);
    // Its slot is gone, so the demand is drawn at once.
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), now);
    submit();
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    // Then from just after the vblank.
    scheduler.request(Reason::Scene);
    EXPECT_EQ(*scheduler.deadline(), now + OutputScheduler::Duration(10ms) / 300);
    // On time again, it creeps back to a third of the period.
    for (int i = 0; i < 400; ++i) {
        now = *scheduler.deadline();
        submit();
        now = scheduler.frameTime() + 10ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
        scheduler.request(Reason::Scene);
    }
    EXPECT_EQ(scheduler.stats().missed, 2u);
    EXPECT_EQ(scheduler.drawBudget(), OutputScheduler::Duration(10ms) / 3);
    // A suspended display may come back on other timing.
    scheduler.suspend();
    scheduler.resume();
    EXPECT_EQ(*scheduler.deadline(), now);
}

TEST_F(SchedulerTest, AFrameLateForItsOwnDrawTimeDoesNotDoubleTheBudget)
{
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = scheduler.frameTime() + 10ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    // Begun a third of the refresh before its vblank, it takes 5 ms and lands
    // a refresh late: the budget becomes what it needed, not the whole period.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    const auto began = now;
    submit();
    scheduler.drawn(pending, began + 5ms);
    now = scheduler.frameTime() + 20ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 1u);
    EXPECT_EQ(scheduler.drawBudget(), 5ms + OutputScheduler::kLatchMargin);

    // One late although it finished in time still doubles it, up to the period.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    const auto next = now;
    submit();
    scheduler.drawn(pending, next + 1ms);
    now = scheduler.frameTime() + 20ms;
    ASSERT_TRUE(scheduler.complete(pending, now));
    pending = 0;
    EXPECT_EQ(scheduler.stats().missed, 2u);
    EXPECT_EQ(scheduler.drawBudget(), 10ms);
}

// With two frames in flight and the budget at a whole period, each frame is
// begun at the vblank while the previous one is still queued, and lands a
// refresh after its own slot. That is the queue, not a slow draw: counting it
// as a miss kept the budget at the period for good, and a client committing
// just after the vblank waited three refreshes for every frame.
TEST_F(SchedulerTest, FramesAfterALateFlipLandOnTheirOwnVblanks)
{
    const auto at = [](auto offset) {
        return OutputScheduler::Time {} + offset;
    };
    scheduler.setMaxPendingFrames(2);
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    submit();
    now = at(10ms);
    ASSERT_TRUE(scheduler.complete(pending, at(10ms)));
    pending = 0;
    for (int i = 0; i < 2; ++i) {
        scheduler.request(Reason::Scene);
        now = *scheduler.deadline();
        submit();
        now = scheduler.frameTime() + 20ms;
        ASSERT_TRUE(scheduler.complete(pending, now));
        pending = 0;
    }
    ASSERT_EQ(scheduler.stats().missed, 2u);
    ASSERT_EQ(scheduler.drawBudget(), 10ms);

    // A late flip holds the next frame back until it lands, so the ones
    // after it land on their own vblanks instead of queueing behind it.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    const uint64_t late = scheduler.begin();
    ASSERT_TRUE(scheduler.submitted(late));
    OutputScheduler::Time vblank = scheduler.frameTime() + 20ms;
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline() + 1ms;
    EXPECT_EQ(scheduler.begin(), 0u);
    now = vblank;
    ASSERT_TRUE(scheduler.complete(late, vblank));
    for (int i = 0; i < 50; ++i) {
        ASSERT_TRUE(scheduler.deadline());
        now = *scheduler.deadline();
        ASSERT_LT(now, vblank + 10ms);
        submit();
        vblank = scheduler.frameTime() + 10ms;
        now = vblank;
        ASSERT_TRUE(scheduler.complete(pending, vblank));
        pending = 0;
        scheduler.request(Reason::Scene);
    }
    // Only the late flip itself was a miss, so the budget is winning the time
    // back.
    EXPECT_EQ(scheduler.stats().missed, 3u);
    EXPECT_LT(scheduler.drawBudget(), 10ms);
}

TEST_F(SchedulerTest, FailureRetainsWorkAndBacksOff)
{
    scheduler.resume();
    auto id = scheduler.begin();
    scheduler.failed(id);
    EXPECT_GE(*scheduler.deadline(), now + 50ms);
    for (int i = 0; i < 100; ++i)
        scheduler.request(Reason::Scene);
    EXPECT_GE(*scheduler.deadline(), now + 50ms);
    now = *scheduler.deadline();
    id = scheduler.begin();
    scheduler.failed(id);
    EXPECT_GE(*scheduler.deadline(), now + 100ms);
    now = *scheduler.deadline();
    submit();
    flip();
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(SchedulerTest, ASkippedFrameAnswersItsDemandAndKeepsTheCadence)
{
    start();
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    auto frame = scheduler.begin();
    ASSERT_NE(frame, 0u);
    scheduler.skipped(frame);
    EXPECT_EQ(scheduler.state(), State::Idle);
    EXPECT_FALSE(scheduler.deadline());
    EXPECT_EQ(scheduler.pendingFrames(), 0u);
    EXPECT_EQ(scheduler.stats().skips, 1u);
    EXPECT_EQ(scheduler.stats().failures, 0u);
    // Demand arriving while the frame was begun waits for the next slot.
    scheduler.request(Reason::Scene);
    now = *scheduler.deadline();
    frame = scheduler.begin();
    scheduler.request(Reason::Scene);
    scheduler.skipped(frame);
    ASSERT_TRUE(scheduler.deadline());
    EXPECT_EQ(*scheduler.deadline(), now + 10ms);
    now = *scheduler.deadline();
    submit();
    flip();
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(SchedulerTest, SuspendCancelsWakeupAndOldCompletionCannotCompleteNewFrame)
{
    scheduler.resume();
    submit();
    const auto old = pending;
    scheduler.suspend();
    pending = 0;
    scheduler.request(Reason::Scene);
    EXPECT_FALSE(scheduler.deadline());
    EXPECT_FALSE(scheduler.complete(old));
    now += 1s;
    scheduler.resume();
    submit();
    EXPECT_FALSE(scheduler.complete(old));
    EXPECT_EQ(scheduler.state(), State::FlipPending);
    flip();
    EXPECT_FALSE(scheduler.complete(old));
}

struct ScheduledSceneTest : SchedulerTest {
    Lingmo::scene::Node root;
    Lingmo::compositor::Compositor compositor {&root};
    Lingmo::base::Subscription repaintNeeded;
    Lingmo::testsupport::FakeRenderer renderer;
    std::shared_ptr<Lingmo::compositor::ShmPool> pool = Lingmo::testsupport::makePool(4096);

    ScheduledSceneTest()
    {
        root.setChangeCallback([this] {
            scheduler.request(Reason::Scene);
        });
        repaintNeeded = compositor.repaintNeeded.connect([this](bool) {
            scheduler.request(Reason::Surface);
        });
    }

    auto surface()
    {
        auto s = compositor.createSurface();
        s->attach(
            pool->createBuffer({16, 16}, 0, 64, Lingmo::render::PixelFormat::Bgra8888, nullptr),
            {});
        s->commit();
        return s;
    }

    uint64_t draw()
    {
        if (scheduler.deadline())
            now = *scheduler.deadline();
        const auto frame = scheduler.begin();
        EXPECT_NE(frame, 0u);
        compositor.updateTextures(renderer);
        compositor.frameSubmitted(frame);
        root.render(renderer);
        scheduler.submitted(frame);
        return frame;
    }

    void present(uint64_t frame)
    {
        ASSERT_TRUE(scheduler.complete(frame));
        compositor.frameDrawn(42, frame);
    }
};

TEST_F(ScheduledSceneTest, CallbackOnlyVisibleCommitProgressesAndLaterCommitWaits)
{
    auto s = surface();
    scheduler.resume();
    present(draw());
    int first = 0, second = 0;
    s->requestFrameCallback([&](auto t) {
        if (t)
            ++first;
    });
    s->commit();
    const auto frame = draw();
    s->requestFrameCallback([&](auto t) {
        if (t)
            ++second;
    });
    s->commit();
    present(frame);
    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 0);
    present(draw());
    EXPECT_EQ(second, 1);
}

TEST_F(ScheduledSceneTest, HiddenCallbackWaitsUntilVisibleAndFailedSnapshotRetries)
{
    auto s = surface();
    scheduler.resume();
    present(draw());
    s->setHidden(true);
    present(draw());
    int done = 0;
    s->requestFrameCallback([&](auto t) {
        if (t)
            ++done;
    });
    s->commit();
    EXPECT_FALSE(scheduler.deadline());
    s->setHidden(false);
    now = *scheduler.deadline();
    auto frame = scheduler.begin();
    compositor.frameSubmitted(frame);
    compositor.frameDiscarded(frame);
    scheduler.failed(frame);
    EXPECT_EQ(done, 0);
    present(draw());
    EXPECT_EQ(done, 1);
}

TEST_F(ScheduledSceneTest, SynchronizedCachedCallbackCannotRequestOrCompleteUntilApplied)
{
    auto parent = surface();
    auto child = surface();
    auto sub = Lingmo::compositor::SubSurface::create(child.get(), parent.get());
    ASSERT_NE(sub, nullptr);
    parent->commit();
    scheduler.resume();
    present(draw());
    int done = 0;
    child->requestFrameCallback([&](auto t) {
        if (t)
            ++done;
    });
    child->commit();
    EXPECT_FALSE(scheduler.deadline());
    EXPECT_EQ(done, 0);
    parent->commit();
    present(draw());
    EXPECT_EQ(done, 1);
}

TEST_F(ScheduledSceneTest, CursorGeometryAndRemovalRequestRepaint)
{
    auto cursor = root.addChild(std::make_unique<Lingmo::scene::Node>("cursor"));
    scheduler.resume();
    present(draw());
    cursor->setPosition({10, 10});
    EXPECT_TRUE(scheduler.deadline());
    present(draw());
    root.removeChild(cursor);
    EXPECT_TRUE(scheduler.deadline());
    present(draw());
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(ScheduledSceneTest, CompletionCanDestroySelfAndAnotherSurface)
{
    auto a = surface(), b = surface();
    int calls = 0;
    a->requestFrameCallback([&](auto t) {
        if (t) {
            ++calls;
            a.reset();
            b.reset();
        }
    });
    b->requestFrameCallback([&](auto t) {
        if (t)
            ADD_FAILURE();
    });
    a->commit();
    b->commit();
    scheduler.resume();
    present(draw());
    EXPECT_EQ(calls, 1);
}

TEST_F(ScheduledSceneTest, SelfDestructionCancelsRemainingCallbacksAndStaleFrameCannotDrainNewOnes)
{
    auto s = surface();
    int cancelled = 0;
    s->requestFrameCallback([&](auto t) {
        if (t)
            s.reset();
    });
    s->requestFrameCallback([&](auto t) {
        EXPECT_FALSE(t);
        ++cancelled;
    });
    s->commit();
    scheduler.resume();
    present(draw());
    EXPECT_EQ(cancelled, 1);
    s = surface();
    int done = 0;
    s->requestFrameCallback([&](auto t) {
        if (t)
            ++done;
    });
    s->commit();
    const auto frame = draw();
    compositor.frameDrawn(42, frame - 1);
    EXPECT_EQ(done, 0);
    present(frame);
    EXPECT_EQ(done, 1);
}

TEST_F(ScheduledSceneTest, AnOutputOwesAFrameOnlyToSurfacesItShows)
{
    auto s = surface();
    scheduler.resume();
    present(draw());
    s->requestFrameCallback([](auto) {});
    s->commit();
    const Lingmo::base::Rect bounds = s->node()->absoluteGeometry();
    EXPECT_TRUE(compositor.awaitsFrame(bounds));
    EXPECT_FALSE(compositor.awaitsFrame(bounds.translated(bounds.width() + 1000, 0)));
    draw();
    EXPECT_FALSE(compositor.awaitsFrame(bounds));
    s->requestFrameCallback([](auto) {});
    s->commit();
    s->node()->setVisible(false);
    EXPECT_FALSE(compositor.awaitsFrame(bounds));
}

TEST_F(ScheduledSceneTest, OnlySurfacesThatMovedAreReportedForOutputUpdates)
{
    auto a = surface();
    auto b = surface();
    EXPECT_EQ(compositor.takeMovedSurfaces().size(), 2u);
    EXPECT_TRUE(compositor.takeMovedSurfaces().empty());
    a->node()->setPosition({50, 50});
    const auto moved = compositor.takeMovedSurfaces();
    ASSERT_EQ(moved.size(), 1u);
    EXPECT_EQ(*moved.front(), a.get());
    // New content at the same size stays where it was.
    b->attach(pool->createBuffer({16, 16}, 0, 64, Lingmo::render::PixelFormat::Bgra8888,
                                 nullptr),
              {});
    b->commit();
    EXPECT_TRUE(compositor.takeMovedSurfaces().empty());
    b->node()->setVisible(false);
    ASSERT_EQ(compositor.takeMovedSurfaces().size(), 1u);
}

TEST_F(SchedulerTest, SuspensionDuringRenderingDoesNotConsumeResumeWork)
{
    scheduler.resume();
    const auto old = scheduler.begin();
    scheduler.request(Reason::Scene);
    scheduler.suspend();
    EXPECT_FALSE(scheduler.submitted(old));
    scheduler.failed(old);
    EXPECT_EQ(scheduler.state(), State::Suspended);
    scheduler.resume();
    submit();
    flip();
    EXPECT_FALSE(scheduler.deadline());
}

TEST_F(ScheduledSceneTest, DecorationOnlyTitleChangeAndWindowDestructionRequestFrames)
{
    Lingmo::decoration::Theme theme;
    Lingmo::decoration::DecorationProvider provider(&theme);
    Lingmo::wm::WindowManager wm(&compositor, nullptr);
    wm.setFrameProvider(&provider);
    wm.setOutputBounds({0, 0, 800, 600});
    auto s = surface();
    auto* w = wm.createWindow(s.get(), {.requestedSize = {200, 100}, .title = "before", .decorated = true});
    scheduler.resume();
    present(draw());
    w->setTitle("after");
    EXPECT_TRUE(scheduler.deadline());
    present(draw());
    EXPECT_FALSE(scheduler.deadline());
    w->setCloseCallback([&] {
        wm.destroyWindow(w);
    });
    s->requestFrameCallback([&](auto t) {
        if (t)
            w->requestClose();
    });
    s->commit();
    present(draw());
    EXPECT_TRUE(wm.windows().empty());
    EXPECT_TRUE(scheduler.deadline());
}

TEST_F(ScheduledSceneTest, RaisingAWindowChangesOnlyThatWindow)
{
    Lingmo::wm::WindowManager wm(&compositor, nullptr);
    wm.setOutputBounds({0, 0, 800, 600});
    std::vector<std::unique_ptr<Lingmo::compositor::Surface>> surfaces;
    std::vector<Lingmo::wm::Window*> windows;
    for (int i = 0; i < 4; ++i) {
        surfaces.push_back(surface());
        windows.push_back(wm.createWindow(surfaces.back().get(), {.requestedSize = {200, 100}}));
    }
    const auto revisions = [&] {
        std::vector<uint64_t> result;
        for (const auto* window : windows)
            result.push_back(window->node()->revision());
        return result;
    };
    const auto before = revisions();
    wm.raise(windows.front());
    const auto after = revisions();
    EXPECT_NE(after[0], before[0]);
    for (size_t i = 1; i < windows.size(); ++i)
        EXPECT_EQ(after[i], before[i]);
    wm.raise(windows.front());
    EXPECT_EQ(revisions(), after);
}

TEST_F(ScheduledSceneTest, ShadowTransitionsRequestOneRepaintAndSettle)
{
    Lingmo::decoration::Theme theme;
    Lingmo::decoration::DecorationProvider provider(&theme);
    Lingmo::wm::WindowManager wm(&compositor, nullptr);
    wm.setFrameProvider(&provider);
    wm.setOutputBounds({0, 0, 800, 600});
    auto s = surface();
    auto* window = wm.createWindow(s.get(), {.requestedSize = {300, 200}, .decorated = true});
    scheduler.resume();
    present(draw());
    EXPECT_FALSE(scheduler.deadline());
    const auto settle = [&] {
        EXPECT_TRUE(scheduler.deadline());
        present(draw());
        EXPECT_FALSE(scheduler.deadline());
    };
    window->setBounds(window->bounds().translated(20, 10));
    settle();
    window->activate();
    // Mapping may already have activated it. Moving always makes demand.
    window->setBounds(window->bounds().translated(1, 0));
    settle();
    window->minimize();
    settle();
    window->restore();
    settle();
    window->maximize();
    settle();
    window->restore();
    settle();
    window->setFullscreen(true);
    settle();
    window->setFullscreen(false);
    settle();
    window->setDecorated(false);
    settle();
    window->setDecorated(true);
    settle();
    const auto misses = renderer.textureCacheMisses();
    root.render(renderer); // Sampling a clean shadow cannot create demand.
    EXPECT_FALSE(scheduler.deadline());
    EXPECT_EQ(renderer.textureCacheMisses(), misses);
    wm.destroyWindow(window);
    settle();
}
