// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The compositor's main loop.

#include <atomic>
#include <thread>
#include <unistd.h>

#include <sys/epoll.h>

#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "base/timer.h"
#include "core/main_loop.h"
#include "loop_test_util.h"
#include "toolkit/toolkit_host.h"
#include "ui/application.h"

namespace Lingmo {
namespace {

using namespace std::chrono_literals;

TEST(MainLoopTest, ExistsFromStartUp)
{
    ASSERT_NE(core::mainLoop(), nullptr);
}

TEST(MainLoopTest, RunsTimers)
{
    bool firstFired = false;
    bool secondFired = false;
    base::Timer first(core::mainLoop());
    first.setSingleShot(true);
    first.onTimeout = [&] { firstFired = true; };
    first.start(10ms);
    base::Timer second(core::mainLoop());
    second.setSingleShot(true);
    second.onTimeout = [&] { secondFired = true; };
    second.start(10ms);

    EXPECT_TRUE(test::runUntil([&] { return firstFired && secondFired; }));
}

TEST(MainLoopTest, RunsTasksPostedFromAnotherThread)
{
    std::atomic<bool> ran = false;
    const base::LoopHandle handle = core::mainLoop()->handle();
    std::thread poster([&] { handle.post([&] { ran = true; }); });
    poster.join();

    EXPECT_TRUE(test::runUntil([&] { return ran.load(); }));
}

TEST(MainLoopTest, WatchesDescriptors)
{
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    char received = 0;
    base::EventLoop &loop = *core::mainLoop();
    const int id = loop.watch(fds[0], EPOLLIN, [&](uint32_t) {
        ASSERT_EQ(read(fds[0], &received, 1), 1);
    });
    ASSERT_NE(id, 0);
    ASSERT_EQ(write(fds[1], "x", 1), 1);

    EXPECT_TRUE(test::runUntil([&] { return received == 'x'; }));
    loop.unwatch(id);
    close(fds[0]);
    close(fds[1]);
}

TEST(MainLoopTest, RunsTheToolkitApplicationsLoop)
{
    EXPECT_EQ(&toolkit::application().loop(), core::mainLoop());
    bool ran = false;
    toolkit::application().loop().post([&] { ran = true; });

    EXPECT_TRUE(test::runUntil([&] { return ran; }));
}

TEST(MainLoopTest, RunForReturnsAfterItsTime)
{
    const auto start = std::chrono::steady_clock::now();
    test::runFor(30ms);
    EXPECT_GE(std::chrono::steady_clock::now() - start, 30ms);
}

} // namespace
} // namespace Lingmo
