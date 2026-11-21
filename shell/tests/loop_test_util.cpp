// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "loop_test_util.h"

#include <poll.h>

#include <algorithm>
#include <thread>

#include "base/event_loop.h"
#include "core/main_loop.h"

namespace Lingmo::Test {

bool runUntil(const std::function<bool()>& done, std::chrono::milliseconds timeout)
{
    using namespace std::chrono;
    const auto deadline = steady_clock::now() + timeout;
    base::EventLoop *loop = core::mainLoop();
    // Whatever is ready runs at least once, however short the timeout.
    if (loop)
        loop->dispatch();
    while (!done()) {
        const auto now = steady_clock::now();
        if (now >= deadline)
            return done();
        // Rounded up, so the wait never ends before the deadline.
        const auto left = ceil<milliseconds>(deadline - now);
        if (!loop) {
            std::this_thread::sleep_for(std::min(left, milliseconds(1)));
            continue;
        }
        // The descriptor turns readable for any watch, timer or posted task;
        // done() may also turn on another thread's work, so it is checked
        // every few milliseconds whatever the loop does.
        pollfd ready {loop->fd(), POLLIN, 0};
        poll(&ready, 1, int(std::min(left, milliseconds(5)).count()));
        // Ends with the loop's prepare hooks, as before any wait, so what the
        // work queued is flushed before the next poll.
        loop->dispatch();
    }
    return true;
}

void runFor(std::chrono::milliseconds duration)
{
    runUntil([] { return false; }, duration);
}

void processPendingEvents()
{
    base::EventLoop *loop = core::mainLoop();
    if (!loop)
        return;
    loop->dispatch();
}

} // namespace Lingmo::Test
