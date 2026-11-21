// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "base/event_loop.h"
#include "base/thread_pool.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

TEST(ThreadPoolTest, ResultArrivesOnTheLoopThread)
{
    EventLoop loop;
    const std::thread::id loopThread = std::this_thread::get_id();
    std::thread::id workThread;
    std::string result;
    Job job = runInThread(
        [&workThread] {
            workThread = std::this_thread::get_id();
            return std::string("done");
        },
        [&](std::string value) {
            EXPECT_EQ(std::this_thread::get_id(), loopThread);
            result = std::move(value);
            loop.quit();
        });
    EXPECT_TRUE(job.isPending());
    loop.run();
    EXPECT_EQ(result, "done");
    EXPECT_NE(workThread, loopThread);
    EXPECT_FALSE(job.isPending());
}

TEST(ThreadPoolTest, MoveOnlyResultsAndVoidWork)
{
    EventLoop loop;
    int value = 0;
    bool ran = false;
    Job first = runInThread([] { return std::make_unique<int>(42); },
                            [&](std::unique_ptr<int> result) { value = *result; });
    Job second = runInThread([] {}, [&] {
        ran = true;
        loop.quit();
    });
    loop.run();
    // One pool thread may finish the second before the first.
    if (!value) {
        loop.startTimer(50ms, false, [&] { loop.quit(); });
        loop.run();
    }
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(ran);
}

TEST(ThreadPoolTest, DroppedJobDropsItsResult)
{
    EventLoop loop;
    bool delivered = false;
    {
        Job job = runInThread([] {
            std::this_thread::sleep_for(10ms);
            return 1;
        }, [&](int) { delivered = true; });
    }
    Job replaced = runInThread([] { return 1; }, [&](int) { delivered = true; });
    replaced = runInThread([] { return 2; }, [&](int value) { EXPECT_EQ(value, 2); });
    loop.startTimer(50ms, false, [&] { loop.quit(); });
    loop.run();
    EXPECT_FALSE(delivered);
}

TEST(ThreadPoolTest, ResultForAGoneLoopIsDropped)
{
    ThreadPool pool(1);
    bool delivered = false;
    Job job;
    {
        EventLoop loop;
        job = pool.run([] {
            std::this_thread::sleep_for(20ms);
            return 1;
        }, [&](int) { delivered = true; });
    }
    job.detach();
    std::this_thread::sleep_for(40ms);
    EXPECT_FALSE(delivered);
}

TEST(ThreadPoolTest, SingleThreadPoolKeepsOrder)
{
    EventLoop loop;
    ThreadPool pool(1);
    std::vector<int> order;
    std::vector<Job> jobs;
    for (int i = 0; i < 5; ++i)
        jobs.push_back(pool.run([i] { return i; }, [&](int value) {
            order.push_back(value);
            if (order.size() == 5)
                loop.quit();
        }));
    loop.run();
    EXPECT_EQ(order, (std::vector<int>{0, 1, 2, 3, 4}));
}
