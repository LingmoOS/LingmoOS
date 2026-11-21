// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "async_p.h"

#include <deque>

namespace Lingmo::Files::internal {

namespace {

struct Helpers
{
    void threadMain()
    {
        std::unique_lock lock(mutex);

        for (;;) {
            ++idle;
            wake.wait(lock, [this] {
                return !queue.empty();
            });
            --idle;

            std::function<void()> work = std::move(queue.front());
            queue.pop_front();
            lock.unlock();
            work();
            work = nullptr;
            lock.lock();
        }
    }

    std::mutex mutex;
    std::condition_variable wake;
    std::deque<std::function<void()>> queue;
    int threads = 0;
    int idle = 0;
};

} // namespace

bool tryStartHelper(std::function<void()> work)
{
    // Never destroyed: its threads wait on it until the process ends.
    static Helpers* helpers = new Helpers;

    std::unique_lock lock(helpers->mutex);

    if (helpers->idle > int(helpers->queue.size())) {
        helpers->queue.push_back(std::move(work));
        helpers->wake.notify_one();
        return true;
    }

    if (helpers->threads >= idealThreadCount())
        return false;

    ++helpers->threads;
    helpers->queue.push_back(std::move(work));
    std::thread([] {
        helpers->threadMain();
    }).detach();
    return true;
}

} // namespace Lingmo::Files::internal
