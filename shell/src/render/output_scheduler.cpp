// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/output_scheduler.h"

#include <algorithm>
#include <stdexcept>

namespace Lingmo::Render {

OutputScheduler::OutputScheduler(Duration period, std::function<Time()> now)
    : m_period(period)
    , m_now(std::move(now))
    , m_lateBudget(period / 3)
    , m_drawnBudget(period / 3)
{
    if (period <= Duration::zero())
        throw std::invalid_argument("nonpositive output period");
}

OutputScheduler::Time OutputScheduler::drawPoint(Time slot) const
{
    return m_vblank ? slot + (m_period - budget()) : slot;
}

OutputScheduler::Duration OutputScheduler::budget() const
{
    // A full repaint after cheap frames is begun as early as the last one
    // needed, not as late as they allowed: its first frame would miss, and
    // the doubling after that comes a refresh too late.
    if (!m_fullDemand)
        return drawBudget();
    const Duration slowest = fullRepaintDrawTime();
    if (slowest == Duration::zero())
        return drawBudget();
    return std::max(drawBudget(), std::clamp(slowest + kLatchMargin, m_period / 3, m_period));
}

OutputScheduler::Duration OutputScheduler::fullRepaintDrawTime() const
{
    const Time now = m_now();
    Duration slowest = Duration::zero();
    for (const auto &[done, took] : m_fullDrawTimes) {
        if (done + kFullRepaintMemory >= now)
            slowest = std::max(slowest, took);
    }
    return slowest;
}

OutputScheduler::Time OutputScheduler::slotAt(Time time) const
{
    auto slots = (time - *m_vblank) / m_period;
    if (*m_vblank + slots * m_period > time)
        --slots;
    return *m_vblank + slots * m_period;
}

void OutputScheduler::request(Reason reason, bool fullRepaint)
{
    ++m_stats.requests;
    ++m_stats.reasons.at(static_cast<size_t>(reason));
    if (m_requested != m_consumed)
        ++m_stats.coalesced;
    ++m_requested;
    // Full demand joining cheaper demand moves its draw point earlier.
    if (fullRepaint && !m_fullDemand && m_state == State::Scheduled)
        m_state = State::Idle;
    m_fullDemand = m_fullDemand || fullRepaint;
    schedule();
}

void OutputScheduler::schedule()
{
    if (m_state != State::Idle || m_requested == m_consumed)
        return;
    // As viz's DisplayScheduler: demand that arrives before the draw point
    // waits for it, so what clients commit early in the refresh still makes
    // it; later demand is drawn at once.
    const Time now = m_now();
    m_deadline = std::max(now, drawPoint(m_next));
    // A full repaint starting from idle past the draw point would miss the
    // vblank anyway: begun at the next one, it shows its own slot, not a stale one.
    if (m_vblank && m_fullDemand && m_next < slotAt(now)
        && now > drawPoint(slotAt(now)) + kWakeupSlack) {
        m_deadline = drawPoint(slotAt(now) + m_period);
    }
    m_state = State::Scheduled;
}

// Re-evaluates the throttle whenever the pending queue or its limit changed.
void OutputScheduler::settle()
{
    if (m_state != State::Idle && m_state != State::FlipPending)
        return;
    m_state = throttled() ? State::FlipPending : State::Idle;
    schedule();
}

void OutputScheduler::setMaxPendingFrames(unsigned count)
{
    m_maxPending = std::max(1u, count);
    settle();
}

void OutputScheduler::resume()
{
    if (m_state != State::Suspended)
        return;
    m_next = m_now();
    m_failures = 0;
    m_state = State::Idle;
    request(m_serial ? Reason::Resume : Reason::Startup);
}

void OutputScheduler::suspend()
{
    m_state = State::Suspended;
    m_frame = 0;
    m_pending.clear();
    m_landed.reset();
    // The display may come back on another timing.
    m_vblank.reset();
    m_drawTimes.clear();
    m_fullDrawTimes.clear();
    m_fullDemand = m_fullFrame = false;
    m_drawnBudget = m_lateBudget = m_period / 3;
}

std::optional<OutputScheduler::Time> OutputScheduler::deadline() const
{
    if (m_state == State::Scheduled)
        return m_deadline;
    return std::nullopt;
}

uint64_t OutputScheduler::begin()
{
    const auto now = m_now();
    if (m_state != State::Scheduled || now + kWakeupSlack <= m_deadline)
        return 0;
    if (behindPendingFrame(now)) {
        m_state = State::FlipPending;
        return 0;
    }
    m_snapshot = m_requested;
    m_began = now;
    m_frame = ++m_serial;
    m_state = State::Rendering;
    if (m_vblank) {
        // A wakeup up to kWakeupSlack short of a draw point on the vblank
        // itself still draws for the slot it was scheduled in, not the last one.
        m_frameTime = slotAt(std::max(now, m_deadline));
    } else {
        // Preserve the nominal phase for ordinary jitter; skip all missed slots.
        m_frameTime = m_deadline + ((now - m_deadline) / m_period) * m_period;
    }
    // Demand drawn at once, early in a slot, shows the moment it was drawn.
    m_sampleTime = std::min(drawPoint(m_frameTime), std::max(now, m_deadline));
    m_punctual = !m_vblank || now <= drawPoint(m_frameTime) + kWakeupSlack;
    m_frameBudget = budget();
    m_fullFrame = m_fullDemand;
    m_fullDemand = false;
    m_next = m_frameTime + m_period;
    return m_frame;
}

bool OutputScheduler::submitted(uint64_t frame)
{
    if (!frame || frame != m_frame || m_state != State::Rendering)
        return false;
    m_consumed = m_snapshot; // Re-entrant requests after begin remain outstanding.
    m_pending.push_back({frame, m_frameTime + m_period, !m_pending.empty(), m_began, m_punctual,
                         m_fullFrame, m_frameBudget});
    m_frame = 0;
    m_failures = 0;
    // Outstanding demand is served in the next slot, whether or not this frame
    // has landed by then, up to the display's limit.
    m_state = State::Idle;
    settle();
    return true;
}

bool OutputScheduler::complete(uint64_t frame, std::optional<Time> presented)
{
    if (!frame || m_pending.empty() || frame != m_pending.front().frame
        || m_state == State::Suspended) {
        ++m_stats.staleCompletions;
        return false;
    }
    const auto now = m_now();
    const Time landed = presented && *presented <= now ? *presented : now;
    // A queued frame waits for the flip ahead of it. Counted as a miss, that
    // wait keeps the budget at the full period, where every frame is begun at
    // the vblank, queues, and misses again.
    Time expected = m_pending.front().aimed;
    if (m_pending.front().queued && m_landed)
        expected = std::max(expected, *m_landed + m_period);
    const bool missed = landed >= expected + m_period / 2;
    m_landed = landed;
    if (missed && m_pending.front().punctual)
        ++m_stats.missed;
    else if (missed)
        ++m_stats.lateDemand;
    if (presented && *presented <= now) {
        // Frames drawn to a stamped flip are begun a budget before the vblank;
        // a late one moves them earlier, down to the vblank itself, and on-time
        // ones win the time back slowly so a steady load does not stutter. One
        // begun past its draw point, as late demand is, says nothing about it,
        // nor does one that took longer than it was given: drawn() has already
        // moved the draw point early enough for it, and doubling on top kept
        // frames begun near the vblank for seconds after a single slow one.
        const Pending &front = m_pending.front();
        const bool overran = front.took && *front.took + kLatchMargin > front.budget;
        if (missed && front.punctual && !overran)
            m_lateBudget = std::min(m_period, drawBudget() * 2);
        else if (m_lateBudget > m_period / 3)
            m_lateBudget = std::max(m_period / 3, m_lateBudget - m_period / 300);
        // Lay the slots on the display's own vblank, as viz feeds presentation
        // feedback back into its DelayBasedTimeSource timebase.
        m_vblank = *presented;
        m_next = slotAt(m_next + m_period / 2);
        // A frame that waited for a late one is drawn for the slot it landed in.
        m_next = std::max(m_next, slotAt(now));
        if (m_state == State::Scheduled)
            m_deadline = std::max(now, drawPoint(m_next));
    }
    m_pending.pop_front();
    settle(); // Only outstanding demand; never creates demand itself.
    return true;
}

bool OutputScheduler::behindPendingFrame(Time now) const
{
    if (!m_vblank || m_pending.empty())
        return false;
    // A frame begun now aims at the vblank ending its slot. One still pending
    // past its own vblank, or aimed at that one, takes it first; this frame
    // would land a refresh late, and every frame begun before it lands too.
    const Time aimed = m_pending.back().aimed;
    return aimed <= now || aimed > slotAt(now) + m_period / 2;
}

void OutputScheduler::drawn(uint64_t frame, Time done)
{
    const auto it = std::find_if(m_pending.begin(), m_pending.end(), [frame](const auto &p) {
        return p.frame == frame;
    });
    if (it == m_pending.end() || done < it->began)
        return;
    it->took = done - it->began;
    // Past viz's fixed third: a GPU clocked down for a light load takes longer
    // than that over a full-screen frame, and every frame it overran was a
    // refresh lost. The draw point moves early enough for the slowest recent one.
    m_drawTimes.push_back(done - it->began);
    if (m_drawTimes.size() > kDrawTimeWindow)
        m_drawTimes.pop_front();
    if (it->fullRepaint) {
        m_fullDrawTimes.emplace_back(done, done - it->began);
        while (m_fullDrawTimes.front().first + kFullRepaintMemory < done)
            m_fullDrawTimes.pop_front();
    }
    const Duration slowest = *std::max_element(m_drawTimes.begin(), m_drawTimes.end());
    m_drawnBudget = std::clamp(slowest + kLatchMargin, m_period / 3, m_period);
}

void OutputScheduler::skipped(uint64_t frame)
{
    if (!frame || frame != m_frame || m_state != State::Rendering)
        return;
    m_consumed = m_snapshot;
    m_frame = 0;
    ++m_stats.skips;
    m_state = State::Idle;
    settle();
}

void OutputScheduler::failed(uint64_t frame)
{
    if (!frame)
        return;
    if (frame == m_frame && m_state == State::Rendering) {
        m_frame = 0;
        m_state = State::Idle;
    } else {
        const auto it = std::find_if(m_pending.begin(), m_pending.end(), [frame](const auto &p) {
            return p.frame == frame;
        });
        if (it == m_pending.end())
            return;
        m_pending.erase(it);
    }
    ++m_stats.failures;
    const auto backoff = std::chrono::milliseconds(50 * (1u << std::min(m_failures++, 5u)));
    m_next = std::max(m_next, m_now() + backoff);
    settle();
}
} // namespace Lingmo::Render
