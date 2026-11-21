// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_OUTPUT_SCHEDULER_H_
#define LINGMO_RENDER_OUTPUT_SCHEDULER_H_

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>

namespace Lingmo::Render {

// Small demand/deadline model. No DRM, event loop or buffer ownership.
// Concepts: Viz DisplayScheduler pending swaps and DelayBasedTimeSource deadlines.
class OutputScheduler
{
public:
    using Clock = std::chrono::steady_clock;
    using Time = Clock::time_point;
    using Duration = Clock::duration;

    // Timers wake in whole milliseconds. One armed to fire at or before the
    // deadline draws when it wakes this close to it, rather than waking up to
    // a millisecond late, out of a budget of a third of a refresh.
    static constexpr Duration kWakeupSlack = std::chrono::milliseconds(1);

    // KMS takes a flip whose rendering finishes this long before the vblank's
    // timestamp; amdgpu missed some with up to 1.4 ms to spare.
    static constexpr Duration kLatchMargin = std::chrono::milliseconds(2);
    // How many measured frames the draw budget covers.
    static constexpr size_t kDrawTimeWindow = 60;
    // How long the slowest full repaint is remembered, so the first frame of
    // an animation covering the output is begun early enough after a stretch
    // of cheap frames has let the budget fall.
    static constexpr Duration kFullRepaintMemory = std::chrono::seconds(10);

    enum class State {
        Idle,
        Scheduled,
        Rendering,
        // As many frames await presentation as the display can hold, or the
        // newest is late; the next one is rendered once the oldest lands.
        FlipPending,
        Suspended
    };
    enum class Reason {
        Startup,
        Resume,
        Scene,
        Surface,
        Decoration,
        Count
    };

    struct Stats {
        uint64_t requests = 0, coalesced = 0, failures = 0, staleCompletions = 0, skips = 0;
        // Frames that landed at least a refresh after the one they were aimed at.
        uint64_t missed = 0;
        // Those begun past their draw point for demand that came late, counted
        // apart: their lost refresh is latency, not judder.
        uint64_t lateDemand = 0;
        std::array<uint64_t, static_cast<size_t>(Reason::Count)> reasons {};
    };

    OutputScheduler(Duration period, std::function<Time()> now = Clock::now);
    // `fullRepaint`: what is requested will redraw the whole output.
    void request(Reason reason, bool fullRepaint = false);
    void resume();
    void suspend();
    std::optional<Time> deadline() const;
    // Called only by the on-demand wakeup; early/spurious wakeups are harmless.
    uint64_t begin();
    bool submitted(uint64_t frame);
    // Frames land in submission order. `presented` is the display's own stamp
    // for the flip, on this clock, when it has one; without it the completion
    // is taken to be the presentation.
    bool complete(uint64_t frame, std::optional<Time> presented = std::nullopt);
    // The pending `frame`, begun by its wakeup, finished rendering on the GPU
    // at `done`. Called before its complete().
    void drawn(uint64_t frame, Time done);
    // The frame being rendered, or a submitted one the display refused.
    void failed(uint64_t frame);
    // The frame begun had nothing to draw, as when viz's Display finds no
    // damage and skips DrawAndSwap. It answers the demand it was begun for,
    // and the next one waits for the following slot.
    void skipped(uint64_t frame);

    // How many submitted frames may await presentation at once: viz's
    // max_pending_swaps, which ChromeOS's triple-buffered BufferQueue sets to
    // two. With two, the next frame is rendered in its own refresh slot while
    // the previous one is still on its way to the screen, so a slow frame costs
    // the one refresh it overran rather than that and the next one too. One is
    // double buffering: rendering waits for the flip.
    void setMaxPendingFrames(unsigned count);

    unsigned pendingFrames() const { return unsigned(m_pending.size()); }

    // The refresh slot the frame begun last was rendered in: the nominal slot
    // start, never the moment the wakeup happened to run.
    Time frameTime() const { return m_frameTime; }

    // The time the frame begun last shows, what viz hands out as
    // BeginFrameArgs::frame_time: the slot's draw point, since the draw begins
    // there as viz's begins at its BeginFrame. Animations and resampled fingers
    // step on it rather than on the wall clock, so a late wakeup or a slow
    // upload cannot move them off the display's cadence; on the slot start
    // they would reach the screen most of a refresh behind.
    Time sampleTime() const { return m_sampleTime; }

    State state() const { return m_state; }

    const Stats& stats() const { return m_stats; }

    Duration period() const { return m_period; }

    // Also drops the vblank the slots were laid on: the timing changed.
    void setPeriod(Duration period)
    {
        m_period = period;
        m_drawTimes.clear();
        m_drawnBudget = m_lateBudget = period / 3;
        m_vblank.reset();
    }

    // How long before a vblank the frame for it is begun, once flips carry
    // timestamps: viz's estimated display draw time, a third of the period, or
    // what recent frames took from wakeup to the GPU finishing when that is
    // longer, and grown by frames that land late.
    Duration drawBudget() const { return std::max(m_lateBudget, m_drawnBudget); }

    // The longest full repaint within kFullRepaintMemory, zero without one.
    Duration fullRepaintDrawTime() const;

    // The longest of the recent frames' wakeup-to-GPU-finished times.
    Duration slowestDrawTime() const
    {
        return m_drawTimes.empty() ? Duration::zero()
                                   : *std::max_element(m_drawTimes.begin(), m_drawTimes.end());
    }

private:
    struct Pending {
        uint64_t frame;
        // The refresh this frame was aimed at.
        Time aimed;
        // Submitted behind a frame still on its way, so it cannot land before
        // a refresh after that one has.
        bool queued = false;
        // When its wakeup began it.
        Time began {};
        // Begun by its draw point, so landing late means the budget is short.
        bool punctual = true;
        // Requested as a redraw of the whole output.
        bool fullRepaint = false;
        // How long before its vblank it was begun, and what it took to draw.
        Duration budget {};
        std::optional<Duration> took;
    };

    void schedule();
    void settle();
    // When the frame for the slot starting at `slot` is begun.
    Time drawPoint(Time slot) const;
    // The budget the next frame is begun with: the full repaint's when one is
    // asked for.
    Duration budget() const;
    // The latest slot start at or before `time` on the vblank grid.
    Time slotAt(Time time) const;
    bool behindPendingFrame(Time now) const;
    bool throttled() const { return m_pending.size() >= m_maxPending; }
    Duration m_period;
    std::function<Time()> m_now;
    State m_state = State::Suspended;
    Time m_next {}, m_deadline {}, m_frameTime {}, m_sampleTime {}, m_began {};
    // The display's last flip, which the slots are laid on; unset without
    // timestamps, and then slots only keep their own cadence.
    std::optional<Time> m_vblank;
    bool m_punctual = true;
    Duration m_frameBudget {};
    // Grown by frames that land late, and won back slowly.
    Duration m_lateBudget;
    // What the slowest recent frame needed; it drops as soon as that frame
    // leaves the window.
    Duration m_drawnBudget;
    std::deque<Duration> m_drawTimes;
    // Full repaints' wakeup-to-GPU-finished times, with when they finished.
    std::deque<std::pair<Time, Duration>> m_fullDrawTimes;
    // Whether the demand outstanding, and the frame begun for it, redraw the
    // whole output.
    bool m_fullDemand = false;
    bool m_fullFrame = false;
    uint64_t m_requested = 0, m_consumed = 0, m_snapshot = 0;
    uint64_t m_serial = 0, m_frame = 0;
    std::deque<Pending> m_pending;
    // When the last frame landed, for the one queued behind it.
    std::optional<Time> m_landed;
    unsigned m_maxPending = 1;
    unsigned m_failures = 0;
    Stats m_stats;
};
} // namespace Lingmo::Render
#endif
