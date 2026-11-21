// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/launcher.h"

#include "base/process.h"

#include <gtest/gtest.h>

#include <climits>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Lingmo;

static std::string target(const std::string& link)
{
    char buffer[PATH_MAX];
    const ssize_t length = ::readlink(link.c_str(), buffer, sizeof(buffer));
    return length > 0 ? std::string(buffer, size_t(length)) : std::string();
}

// Whether the program's descriptors include the socket this process holds at fd.
static bool childHolds(pid_t pid, int fd)
{
    const std::string socket = target("/proc/self/fd/" + std::to_string(fd));
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator("/proc/" + std::to_string(pid) + "/fd", error)) {
        if (target(entry.path().string()) == socket)
            return true;
    }
    return false;
}

static bool spawnedChildHolds(int fd)
{
    // posix_spawn returns once the program is executed, so its descriptors are final.
    const pid_t pid = base::Process::startDetached("sleep", {"30"});
    EXPECT_GT(pid, 0);
    if (pid <= 0)
        return false;
    const bool holds = childHolds(pid, fd);
    ::kill(pid, SIGKILL);
    return holds;
}

TEST(LauncherTest, HandedDescriptorStaysOutOfChildren)
{
    int pair[2];
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, pair), 0);

    // As the session hands it over: inheritable.
    EXPECT_TRUE(spawnedChildHolds(pair[0]));

    ASSERT_TRUE(fm::launcher::keepFromChildren(pair[0]));
    EXPECT_FALSE(spawnedChildHolds(pair[0]));
    EXPECT_FALSE(fm::launcher::keepFromChildren(-1));

    ::close(pair[0]);
    ::close(pair[1]);
}

TEST(LauncherTest, LocalExecutableDesktopIsTrusted)
{
    char directory[] = "/tmp/lingmo-launcher-desktop-XXXXXX";
    ASSERT_NE(::mkdtemp(directory), nullptr);
    const std::string path = std::string(directory) + "/test.desktop";
    {
        std::ofstream file(path);
        file << "[Desktop Entry]\nType=Application\nName=Test\nExec=true\n";
    }

    ASSERT_EQ(::chmod(path.c_str(), 0644), 0);
    EXPECT_FALSE(fm::launcher::isTrusted(path));

    ASSERT_EQ(::chmod(path.c_str(), 0755), 0);
    EXPECT_TRUE(fm::launcher::isTrusted(path));

    std::filesystem::remove_all(directory);
}
