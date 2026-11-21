// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>

#include "base/event_loop.h"
#include "base/timer.h"

namespace testsupport {

// Runs the loop until the condition holds or the time is up.
inline bool waitFor(Lingmo::base::EventLoop &loop, const std::function<bool()> &condition,
                    std::chrono::milliseconds timeout = std::chrono::seconds(5))
{
    if (condition())
        return true;
    Lingmo::base::Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    Lingmo::base::Timer limit(&loop);
    limit.setSingleShot(true);
    limit.onTimeout = [&loop] {
        loop.quit();
    };
    poll.start(std::chrono::milliseconds(5));
    limit.start(timeout);
    loop.run();
    return condition();
}

// Runs the loop for a while, so what should not happen has its chance.
inline void wait(Lingmo::base::EventLoop &loop, std::chrono::milliseconds time)
{
    waitFor(loop, [] { return false; }, time);
}

} // namespace testsupport
