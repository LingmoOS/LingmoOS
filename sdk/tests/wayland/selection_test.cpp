// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include <chrono>
#include <optional>
#include <string>

#include "base/event_loop.h"
#include "base/timer.h"
#include "wayland/selection.h"

using namespace Lingmo;
using namespace std::chrono_literals;

// A paste target that closes its end before the data is all written must
// not end the source, which may be the compositor.
TEST(SelectionTest, WriterSurvivesAReaderThatLeaves)
{
    base::EventLoop loop;
    int fds[2];
    ASSERT_EQ(pipe2(fds, O_CLOEXEC), 0);
    // More than a pipe holds, so the writer waits on the loop.
    ui::wayland::writeToPipe(loop, fds[1], std::string(4 << 20, 'x'));
    char buffer[4096];
    ASSERT_GT(read(fds[0], buffer, sizeof buffer), 0);
    close(fds[0]);
    base::Timer timeout(&loop);
    timeout.setSingleShot(true);
    timeout.onTimeout = [&loop] { loop.quit(); };
    timeout.start(300ms);
    loop.run();
    SUCCEED();
}

TEST(SelectionTest, ReadsWhatTheSourceWrites)
{
    base::EventLoop loop;
    int fds[2];
    ASSERT_EQ(pipe2(fds, O_CLOEXEC), 0);
    std::optional<std::string> received;
    bool done = false;
    ui::wayland::readFromPipe(loop, fds[0], [&](std::optional<std::string> data) {
        received = std::move(data);
        done = true;
        loop.quit();
    });
    ui::wayland::writeToPipe(loop, fds[1], std::string(1 << 20, 'y'));
    base::Timer timeout(&loop);
    timeout.setSingleShot(true);
    timeout.onTimeout = [&loop] { loop.quit(); };
    timeout.start(5s);
    loop.run();
    ASSERT_TRUE(done);
    ASSERT_TRUE(received);
    EXPECT_EQ(received->size(), size_t(1 << 20));
}
