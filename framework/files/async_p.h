// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>

namespace Lingmo::Files::internal {

// Folders from this size on spread even a refresh over threads.
constexpr ptrdiff_t LargeFolder = 20000;

inline int idealThreadCount()
{
    return std::max(1u, std::thread::hardware_concurrency());
}

// Helpers for work split into blocks: half the logical CPUs work as fast as all of
// them for less CPU time, and leave the GUI and the compositor theirs.
inline ptrdiff_t blockHelpers(ptrdiff_t blocks)
{
    return std::max(ptrdiff_t(0), std::min(ptrdiff_t(idealThreadCount() / 2 - 1), blocks - 1));
}

// Hands work to a helper thread that is idle, or starts one while fewer than
// the logical CPUs run; false when neither is possible right now.
bool tryStartHelper(std::function<void()> work);

// Runs body on this thread and on up to helpers threads free right now; body
// shares the work out itself and stops once stop is set, after an exception. The
// first exception is rethrown when every run has returned, as the helpers use this
// stack.
template<typename Body>
void runOnFreeThreads(ptrdiff_t helpers, Body body)
{
    std::atomic_bool stop = false;
    std::mutex mutex;
    std::condition_variable finishedCondition;
    std::exception_ptr failure;
    ptrdiff_t finished = 0;

    const auto run = [&] {
        try {
            body(stop);
        } catch (...) {
            const std::lock_guard lock(mutex);
            if (!failure)
                failure = std::current_exception();
            stop = true;
        }
    };

    ptrdiff_t started = 0;

    // Only threads free now: queued helpers could wait behind work that waits for them.
    while (started < helpers && tryStartHelper([&] {
               run();
               const std::lock_guard lock(mutex);
               ++finished;
               finishedCondition.notify_all();
           }))
        ++started;

    run();

    std::unique_lock lock(mutex);
    finishedCondition.wait(lock, [&] {
        return finished == started;
    });

    if (failure)
        std::rethrow_exception(failure);
}

} // namespace Lingmo::Files::internal
