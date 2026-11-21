// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/timer.h"

#include "TerminalSession.h"
#include "tools.h"

using namespace Konsole;

// Runs the loop until the condition holds, as QTRY_* does, for at most five seconds.
static bool waitFor(Lingmo::base::EventLoop& loop, const std::function<bool()>& condition)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline)
            return false;
        Lingmo::base::Timer tick;
        tick.setSingleShot(true);
        tick.onTimeout = [] {};
        tick.start(std::chrono::milliseconds(10));
        loop.dispatch();
    }
    return true;
}

TEST(SessionTest, TitleFollowsTheProgramInFront)
{
    set_kb_layout_dir(KB_LAYOUTS_DIR);
    Lingmo::base::EventLoop loop;
    Lingmo::base::file::TemporaryDirectory directory("/tmp", "session-test");
    ASSERT_TRUE(directory.isValid());
    const std::string directoryName = std::filesystem::path(directory.path()).filename().string();

    TerminalSession session;
    session.setShellProgram("/bin/sh");
    session.setInitialWorkingDirectory(directory.path());

    // What a tab shows each time it hears the title changed.
    std::vector<std::string> titles;
    auto subscription = session.titleChanged.connect([&] { titles.push_back(session.title()); });

    session.startShellProgram();
    EXPECT_TRUE(waitFor(loop, [&] { return session.title() == directoryName; }));

    // A program that sets no title shows its name, and the prompt its directory.
    session.sendText("sleep 30\r");
    EXPECT_TRUE(waitFor(loop, [&] { return std::count(titles.begin(), titles.end(), "sleep") > 0; }));
    session.sendText("\x03");
    EXPECT_TRUE(waitFor(loop, [&] { return !titles.empty() && titles.back() == directoryName; }));

    // A title the program sets wins over its name while it runs.
    session.sendText("sh -c 'printf \"\\033]2;hello\\007\"; sleep 30'\r");
    EXPECT_TRUE(waitFor(loop, [&] { return !titles.empty() && titles.back() == "hello"; }));
    session.sendText("\x03");
    EXPECT_TRUE(waitFor(loop, [&] { return !titles.empty() && titles.back() == directoryName; }));

    // A title the shell sets at its prompt does not replace the directory.
    const size_t heard = titles.size();
    session.sendText("printf '\\033]0;user@host: ~\\007'\r");
    EXPECT_TRUE(waitFor(loop, [&] { return titles.size() > heard; }));
    ASSERT_FALSE(titles.empty());
    EXPECT_EQ(titles.back(), directoryName);
}

TEST(SessionTest, ShellExitFinishesTheSession)
{
    set_kb_layout_dir(KB_LAYOUTS_DIR);
    Lingmo::base::EventLoop loop;

    TerminalSession session;
    session.setShellProgram("/bin/sh");
    bool finished = false;
    auto subscription = session.finished.connect([&] { finished = true; });

    session.startShellProgram();
    ASSERT_TRUE(session.isRunning());
    session.sendText("exit\r");
    EXPECT_TRUE(waitFor(loop, [&] { return finished; }));
    EXPECT_FALSE(session.isRunning());
}

TEST(SessionTest, ChildSeesTheTerminalEnvironment)
{
    set_kb_layout_dir(KB_LAYOUTS_DIR);
    Lingmo::base::EventLoop loop;
    Lingmo::base::file::TemporaryDirectory directory("/tmp", "session-env");
    ASSERT_TRUE(directory.isValid());
    const std::string output = directory.path() + "/env";

    TerminalSession session;
    session.setShellProgram("/bin/sh");
    session.setShellProgramArguments({"-c", "printf '%s|%s|%s|%s' \"$TERM\" \"$COLORTERM\" \"$COLORFGBG\" \"$(tty)\" > " + output});
    session.startShellProgram();

    std::optional<std::string> contents;
    EXPECT_TRUE(waitFor(loop, [&] {
        contents = Lingmo::base::file::read(output);
        return contents && contents->find("/dev/pts/") != std::string::npos;
    }));
    ASSERT_TRUE(contents);
    EXPECT_TRUE(contents->starts_with("xterm-256color|truecolor|15;0|/dev/pts/")) << *contents;
}

TEST(SessionTest, CurrentDirBehindAProgramOfAnotherUser)
{
    set_kb_layout_dir(KB_LAYOUTS_DIR);
    // Set-group-ID, so its working directory is not ours to read.
    const std::string agent = Lingmo::base::paths::findExecutable("ssh-agent");
    if (agent.empty())
        GTEST_SKIP() << "no ssh-agent";
    Lingmo::base::EventLoop loop;
    Lingmo::base::file::TemporaryDirectory directory("/tmp", "session-cwd");
    ASSERT_TRUE(directory.isValid());

    TerminalSession session;
    session.setShellProgram("/bin/sh");
    session.setInitialWorkingDirectory(directory.path());
    session.startShellProgram();
    ASSERT_TRUE(waitFor(loop, [&] { return session.currentDir() == directory.path(); }));

    // Looked at only once the agent runs: its socket is there.
    const std::string socket = directory.path() + "/agent";
    session.sendText(agent + " -D -a " + socket + "\r");
    ASSERT_TRUE(waitFor(loop, [&] { return std::filesystem::exists(socket); }));
    ASSERT_EQ(session.foregroundProcessName(), "ssh-agent");
    EXPECT_EQ(session.currentDir(), directory.path());
    session.sendText("\x03");
}
