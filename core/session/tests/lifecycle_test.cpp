// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdlib>
#include <string>
#include <unistd.h>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>

#include "compositorprocess.h"
#include "testutil.h"

using Lingmo::Base::EventLoop;
using SessionTest::runUntil;

static QString selfPath()
{
    char path[4096];
    const ssize_t length = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    return length > 0 ? QString::fromLocal8Bit(path, int(length)) : QString();
}

struct Counts
{
    int ready  = 0;
    int failed = 0;
    int exited = 0;
};

static void track(CompositorProcess &process, Counts &counts)
{
    process.onReady  = [&counts] { ++counts.ready; };
    process.onFailed = [&counts](const QString &) { ++counts.failed; };
    process.onExited = [&counts](int) { ++counts.exited; };
}

TEST(SessionLifecycle, readyAndStop)
{
    EventLoop loop;
    setenv("SESSION_TEST_CHILD", "ready", 1);
    CompositorProcess process;
    Counts counts;
    track(process, counts);
    process.start(selfPath(), QStringLiteral("test-socket"),
                  QStringLiteral("test-control"));
    ASSERT_TRUE(runUntil(loop, [&] { return counts.ready == 1; }));
    EXPECT_TRUE(process.isReady());
    process.stop();
    EXPECT_FALSE(process.isReady());
    EXPECT_EQ(counts.failed, 0);
    EXPECT_EQ(counts.exited, 0);
}

TEST(SessionLifecycle, forwardsXwaylandReadinessPath)
{
    EventLoop loop;
    setenv("SESSION_TEST_CHILD", "xwayland", 1);
    CompositorProcess process;
    Counts counts;
    track(process, counts);
    process.start(selfPath(), QStringLiteral("test-socket"),
                  QStringLiteral("test-control"), 1000,
                  QStringLiteral("/test-ready.json"));
    ASSERT_TRUE(runUntil(loop, [&] { return counts.ready == 1; }));
    process.stop();
}

TEST(SessionLifecycle, startupFailure)
{
    EventLoop loop;
    setenv("SESSION_TEST_CHILD", "exit", 1);
    CompositorProcess process;
    Counts counts;
    track(process, counts);
    process.start(selfPath(), QStringLiteral("test-socket"),
                  QStringLiteral("test-control"));
    ASSERT_TRUE(runUntil(loop, [&] { return counts.failed == 1; }));
    EXPECT_FALSE(process.isReady());
}

TEST(SessionLifecycle, timeout)
{
    EventLoop loop;
    setenv("SESSION_TEST_CHILD", "hang", 1);
    CompositorProcess process;
    Counts counts;
    track(process, counts);
    process.start(selfPath(), QStringLiteral("test-socket"),
                  QStringLiteral("test-control"), 30);
    ASSERT_TRUE(runUntil(loop, [&] { return counts.failed == 1; }));
    EXPECT_FALSE(process.isReady());
}

TEST(SessionLifecycle, missingExecutable)
{
    EventLoop loop;
    CompositorProcess process;
    Counts counts;
    track(process, counts);
    process.start(QStringLiteral("/does-not-exist/lingmo-aurora"),
                  QStringLiteral("test-socket"),
                  QStringLiteral("test-control"));
    EXPECT_EQ(counts.failed, 1);
}

// Runs as the compositor: checks its command line and confirms startup.
static int runChild(const std::string &mode, int argc, char **argv)
{
    if (mode == "exit")
        return 1;
    const std::vector<std::string> arguments(argv, argv + argc);
    if (arguments.size() != (mode == "xwayland" ? 9u : 7u)
        || arguments.at(2) != "test-socket"
        || arguments.at(6) != "test-control")
        return 2;
    if (mode == "xwayland"
        && (arguments.at(7) != "--xwayland-ready-file"
            || arguments.at(8) != "/test-ready.json"))
        return 4;
    if (mode == "ready" || mode == "xwayland") {
        const int fd = std::atoi(arguments.at(4).c_str());
        if (write(fd, "R", 1) != 1)
            return 3;
        close(fd);
    }
    for (;;)
        pause();
}

int main(int argc, char **argv)
{
    const char *mode = std::getenv("SESSION_TEST_CHILD");
    if (mode && *mode)
        return runChild(mode, argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
