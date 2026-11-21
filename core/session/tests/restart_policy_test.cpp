// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "restartpolicy.h"
#include "testutil.h"

using Lingmo::Base::EventLoop;
using Lingmo::RestartPolicy;
using SessionTest::runFor;
using SessionTest::runUntil;

TEST(RestartPolicy, givesUpAfterTheLimit)
{
    EventLoop loop;
    int restarts = 0;
    RestartPolicy policy(QStringLiteral("test"), [&] { ++restarts; }, {3, 60000, 10, 10});
    policy.started();
    for (int i = 0; i < 3; ++i) {
        policy.exited(1, true);
        EXPECT_TRUE(policy.isPending());
        ASSERT_TRUE(runUntil(loop, [&] { return restarts == i + 1; }));
        policy.started();
    }
    policy.exited(1, true);
    EXPECT_FALSE(policy.isPending());
    EXPECT_TRUE(policy.hasGivenUp());
}

TEST(RestartPolicy, backsOff)
{
    EventLoop loop;
    int restarts = 0;
    RestartPolicy policy(QStringLiteral("test"), [&] { ++restarts; }, {5, 60000, 50, 200});
    policy.started();
    policy.exited(1, true);
    ASSERT_TRUE(runUntil(loop, [&] { return restarts == 1; }));
    policy.started();
    policy.exited(1, true);
    runFor(loop, 150);
    EXPECT_EQ(restarts, 1);
    EXPECT_TRUE(runUntil(loop, [&] { return restarts == 2; }));
}

TEST(RestartPolicy, stableUptimeResetsTheCount)
{
    EventLoop loop;
    int restarts = 0;
    RestartPolicy policy(QStringLiteral("test"), [&] { ++restarts; }, {1, 100, 10, 10});
    policy.started();
    policy.exited(1, true);
    ASSERT_TRUE(runUntil(loop, [&] { return restarts == 1; }));
    policy.started();
    runFor(loop, 150);
    policy.exited(1, true);
    EXPECT_TRUE(runUntil(loop, [&] { return restarts == 2; }));
    EXPECT_FALSE(policy.hasGivenUp());
}

TEST(RestartPolicy, stopCancelsAPendingRestart)
{
    EventLoop loop;
    int restarts = 0;
    RestartPolicy policy(QStringLiteral("test"), [&] { ++restarts; }, {5, 60000, 50, 10});
    policy.started();
    policy.exited(0, false);
    policy.stop();
    runFor(loop, 100);
    EXPECT_EQ(restarts, 0);
    policy.exited(0, false);
    EXPECT_FALSE(policy.isPending());
}
