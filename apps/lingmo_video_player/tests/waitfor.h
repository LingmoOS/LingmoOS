// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"

#include <chrono>
#include <poll.h>

// Runs the loop until done() holds or the time is up; done()'s last answer.
template<typename Done>
bool waitFor(Lingmo::base::EventLoop& loop, Done done, std::chrono::milliseconds timeout = std::chrono::seconds(5))
{
    using Clock = std::chrono::steady_clock;
    const Clock::time_point deadline = Clock::now() + timeout;
    while (!done()) {
        if (Clock::now() >= deadline)
            return done();
        pollfd descriptor{loop.fd(), POLLIN, 0};
        poll(&descriptor, 1, 10);
        loop.dispatch();
    }
    return true;
}

// Runs the loop for the time given.
inline void runFor(Lingmo::base::EventLoop& loop, std::chrono::milliseconds time)
{
    waitFor(loop, [] { return false; }, time);
}
