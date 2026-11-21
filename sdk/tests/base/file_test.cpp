// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/file_watcher.h"
#include "base/paths.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

namespace {

class FileTest : public testing::Test
{
protected:
    void SetUp() override
    {
        char directory[] = "/tmp/file-test-XXXXXX";
        ASSERT_NE(mkdtemp(directory), nullptr);
        m_directory = directory;
    }
    void TearDown() override { std::filesystem::remove_all(m_directory); }

    void runFor(std::chrono::milliseconds limit)
    {
        const int timer = m_loop.startTimer(limit, false, [this] { m_loop.quit(); });
        m_loop.run();
        m_loop.stopTimer(timer);
    }

    EventLoop m_loop;
    std::string m_directory;
};

} // namespace

TEST_F(FileTest, WritesAtomicallyAndKeepsTheMode)
{
    const std::string path = m_directory + "/sub/dir/file.txt";
    ASSERT_TRUE(file::writeAtomically(path, "first"));
    EXPECT_EQ(file::read(path), "first");
    chmod(path.c_str(), 0600);
    ASSERT_TRUE(file::writeAtomically(path, "second"));
    EXPECT_EQ(file::read(path), "second");
    struct stat info{};
    ASSERT_EQ(stat(path.c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & 0777, 0600u);
    EXPECT_EQ(std::distance(std::filesystem::directory_iterator(m_directory + "/sub/dir"),
                            std::filesystem::directory_iterator()),
              1);
    EXPECT_FALSE(file::read(m_directory + "/missing"));
}

// As a configuration file kept in another directory under a link.
TEST_F(FileTest, WritesAtomicallyThroughSymbolicLinks)
{
    std::filesystem::create_directory(m_directory + "/real");
    const std::string target = m_directory + "/real/file.txt";
    const std::string link = m_directory + "/link.txt";
    ASSERT_TRUE(file::writeAtomically(target, "first"));
    std::filesystem::create_symlink("real/file.txt", link);
    std::filesystem::create_symlink("link.txt", m_directory + "/chain.txt");

    ASSERT_TRUE(file::writeAtomically(m_directory + "/chain.txt", "second"));
    EXPECT_TRUE(std::filesystem::is_symlink(m_directory + "/chain.txt"));
    EXPECT_TRUE(std::filesystem::is_symlink(link));
    EXPECT_EQ(file::read(target), "second");

    // A dangling link gets its target created.
    std::filesystem::create_symlink("real/new.txt", m_directory + "/dangling.txt");
    ASSERT_TRUE(file::writeAtomically(m_directory + "/dangling.txt", "third"));
    EXPECT_TRUE(std::filesystem::is_symlink(m_directory + "/dangling.txt"));
    EXPECT_EQ(file::read(m_directory + "/real/new.txt"), "third");
}

TEST_F(FileTest, WatchesAFileThroughReplacementAndRemoval)
{
    const std::string path = m_directory + "/watched.conf";
    int changes = 0;
    FileWatch watch = FileWatch::file(path, [&] {
        ++changes;
        m_loop.quit();
    });
    ASSERT_TRUE(watch.isActive());

    // Created, replaced by a rename, removed.
    ASSERT_TRUE(file::writeAtomically(path, "a"));
    runFor(1s);
    EXPECT_GE(changes, 1);
    changes = 0;
    ASSERT_TRUE(file::writeAtomically(path, "b"));
    runFor(1s);
    EXPECT_GE(changes, 1);
    changes = 0;
    std::filesystem::remove(path);
    runFor(1s);
    EXPECT_EQ(changes, 1);

    // Another file in the directory is not this one.
    changes = 0;
    ASSERT_TRUE(file::writeAtomically(m_directory + "/other", "x"));
    runFor(100ms);
    EXPECT_EQ(changes, 0);

    watch.reset();
    ASSERT_TRUE(file::writeAtomically(path, "c"));
    runFor(100ms);
    EXPECT_EQ(changes, 0);
}

TEST_F(FileTest, WatchesDirectoryEntries)
{
    std::vector<std::string> names;
    FileWatch watch = FileWatch::directory(m_directory, [&](const std::string& name) {
        names.push_back(name);
        m_loop.quit();
    });
    FileWatch second = FileWatch::directory(m_directory, [](const std::string&) {});
    second.reset();
    ASSERT_TRUE(file::writeAtomically(m_directory + "/entry", "x"));
    runFor(1s);
    ASSERT_FALSE(names.empty());
    EXPECT_EQ(names.back(), "entry");
}

TEST_F(FileTest, ReportsAQueueOverflow)
{
    int maximum = 16384;
    if (const auto limit = file::read("/proc/sys/fs/inotify/max_queued_events"))
        maximum = std::atoi(limit->c_str());
    if (maximum > 200000)
        GTEST_SKIP() << "the inotify queue is too long to fill";
    int overflows = 0;
    int changes = 0;
    int fileChanges = 0;
    FileWatch watch = FileWatch::directory(
        m_directory, [&](const std::string&) { ++changes; }, [&] { ++overflows; });
    FileWatch plain = FileWatch::directory(m_directory, [](const std::string&) {});
    FileWatch file = FileWatch::file(m_directory + "/watched", [&] { ++fileChanges; });
    // Each file is a create and a close-write event: overfill while nothing reads.
    for (int i = 0; i < maximum / 2 + 100; ++i) {
        const std::string path = m_directory + "/f" + std::to_string(i);
        const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
        ASSERT_GE(fd, 0);
        ::close(fd);
    }
    runFor(300ms);
    EXPECT_EQ(overflows, 1);
    EXPECT_GT(changes, 0);
    // A file watch takes the overflow as a change.
    EXPECT_EQ(fileChanges, 1);
}

// Watches made and dropped again and again, as by windows that each watch a
// folder while they are open, leave nothing behind. Measured on the heap; a
// sanitizer's allocator reports nothing to compare.
TEST_F(FileTest, WatchingAgainAndAgainDoesNotGrow)
{
    const auto cycle = [this] {
        FileWatch watch = FileWatch::directory(m_directory, [](const std::string&) {}, &m_loop);
        ASSERT_TRUE(watch.isActive());
    };
    for (int i = 0; i < 20; ++i)
        cycle();
    const size_t before = mallinfo2().uordblks;
    for (int i = 0; i < 1000; ++i)
        cycle();
    const size_t after = mallinfo2().uordblks;
    EXPECT_LT(after, before + 16 * 1024);
}

TEST(PathsTest, StandardDirectories)
{
    EXPECT_FALSE(paths::home().empty());
    EXPECT_EQ(paths::configHome(), paths::home() + "/.config");
    EXPECT_EQ(paths::lingmoConfig("calendar"), paths::home() + "/.config/lingmo/calendar.conf");
    EXPECT_EQ(paths::findExecutable("sh").empty(), false);
    EXPECT_TRUE(paths::findExecutable("no-such-program-anywhere").empty());
}

// A reader that went: EPIPE, and the process carries on.
TEST(FileWriteTest, WritesToAClosedPipeWithoutSigpipe)
{
    int fds[2];
    ASSERT_EQ(pipe2(fds, O_CLOEXEC), 0);
    close(fds[0]);
    const char data[] = "lost";
    EXPECT_EQ(file::writeWithoutSigpipe(fds[1], data, sizeof data), -1);
    EXPECT_EQ(errno, EPIPE);
    close(fds[1]);
}
