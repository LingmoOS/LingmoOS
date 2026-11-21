// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/timer.h"

#include "base/event_loop.h"

namespace Lingmo::base {

Timer::Timer(EventLoop* loop)
    : m_loop(loop ? *loop : *EventLoop::current())
{
}

Timer::~Timer()
{
    stop();
}

void Timer::start()
{
    // A running timer moves its deadline, mostly without a system call.
    if (m_id && m_loop.restartTimer(m_id, m_interval))
        return;
    stop();
    // Always repeating underneath: the single-shot flag is read at each
    // expiry, so it may change while the timer runs.
    m_id = m_loop.startTimer(m_interval, true, [this] {
        if (m_singleShot)
            stop();
        if (onTimeout)
            onTimeout();
    });
}

void Timer::start(std::chrono::nanoseconds interval)
{
    m_interval = interval;
    start();
}

void Timer::stop()
{
    if (!m_id)
        return;
    m_loop.stopTimer(m_id);
    m_id = 0;
}

} // namespace Lingmo::base
